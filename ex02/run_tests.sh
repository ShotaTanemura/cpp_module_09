#!/bin/bash

# PmergeMe Test Runner
# Runs all test cases from test_cases.txt and reports results

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Check if PmergeMe executable exists
if [ ! -f "./PmergeMe" ]; then
    echo -e "${RED}Error: PmergeMe executable not found. Please run 'make' first.${NC}"
    exit 1
fi

# Check if test_cases.txt exists
if [ ! -f "./test_cases.txt" ]; then
    echo -e "${RED}Error: test_cases.txt not found.${NC}"
    exit 1
fi

echo -e "${BLUE}=== PmergeMe Test Suite ===${NC}"
echo -e "${BLUE}Running comprehensive tests...${NC}\n"

# Function to check if a sequence is sorted in ascending order
is_sorted() {
    local sequence="$1"
    local prev=""
    local first=true
    
    for num in $sequence; do
        if [ "$first" = true ]; then
            first=false
            prev="$num"
            continue
        fi
        
        if [ "$num" -lt "$prev" ]; then
            return 1  # Not sorted
        fi
        prev="$num"
    done
    return 0  # Sorted
}

# Function to extract the "After:" line numbers
extract_after_sequence() {
    local output="$1"
    echo "$output" | grep "^After: " | sed 's/^After: //'
}

# Function to extract the "Before:" line numbers
extract_before_sequence() {
    local output="$1"
    echo "$output" | grep "^Before: " | sed 's/^Before: //'
}

# Function to check if two sequences contain the same elements (same count of each)
same_elements() {
    local seq1="$1"
    local seq2="$2"
    
    # Sort both sequences and compare
    local sorted1=$(echo "$seq1" | tr ' ' '\n' | sort -n | tr '\n' ' ' | sed 's/ $//')
    local sorted2=$(echo "$seq2" | tr ' ' '\n' | sort -n | tr '\n' ' ' | sed 's/ $//')
    
    [ "$sorted1" = "$sorted2" ]
}

# Function to run a single test
run_test() {
    local expected_exit=$1
    local description="$2"
    local args="$3"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Run the test and capture output and exit code
    if [ -z "$args" ]; then
        # No arguments case
        output=$(./PmergeMe 2>&1)
        actual_exit=$?
    elif [ "$args" = "TAB_TEST_CASE" ]; then
        # Special case for tab character test - test string with embedded tab
        output=$(./PmergeMe $'123\t456' 2>&1)
        actual_exit=$?
    else
        # With arguments
        output=$(./PmergeMe $args 2>&1)
        actual_exit=$?
    fi
    
    # Check if exit code matches expected
    if [ $actual_exit -eq $expected_exit ]; then
        echo -e "${GREEN}✓ PASS${NC} | $description"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        
        # For successful tests, verify output format and sorting
        if [ $expected_exit -eq 0 ]; then
            # Check if output has required lines
            line_count=$(echo "$output" | wc -l)
            if [ $line_count -lt 4 ]; then
                echo -e "${YELLOW}  ⚠ Warning: Expected 4 output lines, got $line_count${NC}"
            fi
            
            # Check for required prefixes
            if ! echo "$output" | grep -q "^Before: "; then
                echo -e "${YELLOW}  ⚠ Warning: Missing 'Before: ' line${NC}"
            fi
            if ! echo "$output" | grep -q "^After: "; then
                echo -e "${YELLOW}  ⚠ Warning: Missing 'After: ' line${NC}"
            fi
            if ! echo "$output" | grep -q "Time to process.*std::vector"; then
                echo -e "${YELLOW}  ⚠ Warning: Missing vector timing line${NC}"
            fi
            if ! echo "$output" | grep -q "Time to process.*std::deque"; then
                echo -e "${YELLOW}  ⚠ Warning: Missing deque timing line${NC}"
            fi
            
            # CRITICAL: Check if the result is actually sorted and contains same elements
            after_sequence=$(extract_after_sequence "$output")
            before_sequence=$(extract_before_sequence "$output")
            
            if [ -n "$after_sequence" ] && [ -n "$before_sequence" ]; then
                # Check if sorted
                if ! is_sorted "$after_sequence"; then
                    echo -e "${RED}  ✗ CRITICAL: Result is NOT sorted! Sequence: $after_sequence${NC}"
                    FAILED_TESTS=$((FAILED_TESTS + 1))
                    PASSED_TESTS=$((PASSED_TESTS - 1))
                    return
                fi
                
                # Check if same elements
                if ! same_elements "$before_sequence" "$after_sequence"; then
                    echo -e "${RED}  ✗ CRITICAL: Elements changed during sorting!${NC}"
                    echo -e "${RED}    Before: $before_sequence${NC}"
                    echo -e "${RED}    After:  $after_sequence${NC}"
                    FAILED_TESTS=$((FAILED_TESTS + 1))
                    PASSED_TESTS=$((PASSED_TESTS - 1))
                    return
                fi
                
                echo -e "${GREEN}  ✓ Sort order verified & elements preserved${NC}"
            fi
        else
            # For error cases, check if output is just "Error"
            if [ "$output" != "Error" ]; then
                echo -e "${YELLOW}  ⚠ Warning: Expected 'Error', got '$output'${NC}"
            fi
        fi
    else
        echo -e "${RED}✗ FAIL${NC} | $description"
        echo -e "  Expected exit code: $expected_exit, Got: $actual_exit"
        echo -e "  Output: $output"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
}

# Function to generate and test random sequences
test_random_sequences() {
    echo -e "\n${BLUE}=== Random Sequence Tests ===${NC}"
    
    # Test with different sizes
    for size in 10 50 100 500 1000; do
        if command -v jot >/dev/null 2>&1; then
            # macOS
            args=$(jot -r $size 1 10000 | tr '\n' ' ')
        elif command -v shuf >/dev/null 2>&1; then
            # Linux
            args=$(seq 1 10000 | shuf -n $size | tr '\n' ' ')
        else
            echo -e "${YELLOW}Skipping random tests: neither 'jot' nor 'shuf' available${NC}"
            return
        fi
        
        run_test 0 "Random sequence of $size elements" "$args"
    done
}

# Function to test specific sorting scenarios
test_sorting_correctness() {
    echo -e "\n${BLUE}=== Sorting Correctness Tests ===${NC}"
    
    # Test worst-case scenarios for sorting algorithms
    local test_cases=(
        "Reverse sorted 10|10 9 8 7 6 5 4 3 2 1"
        "All duplicates|5 5 5 5 5 5 5"
        "Nearly sorted|1 2 3 4 6 5 7 8 9 10"
        "Single out of place|2 1 3 4 5 6 7 8 9 10"
        "Two swapped|1 10 3 4 5 6 7 8 9 2"
        "Mountain pattern|1 2 3 4 5 4 3 2 1"
        "Valley pattern|5 4 3 2 1 2 3 4 5"
        "Alternating|1 10 2 9 3 8 4 7 5 6"
    )
    
    for test_case in "${test_cases[@]}"; do
        local description=$(echo "$test_case" | cut -d'|' -f1)
        local args=$(echo "$test_case" | cut -d'|' -f2)
        
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        
        # Run the test
        output=$(./PmergeMe $args 2>&1)
        actual_exit=$?
        
        if [ $actual_exit -eq 0 ]; then
            # Extract and verify sorting
            after_sequence=$(extract_after_sequence "$output")
            before_sequence=$(extract_before_sequence "$output")
            
            if [ -n "$after_sequence" ] && [ -n "$before_sequence" ]; then
                # Check if sorted
                if ! is_sorted "$after_sequence"; then
                    echo -e "${RED}✗ FAIL${NC} | $description"
                    echo -e "${RED}  ✗ CRITICAL: Result is NOT sorted! Sequence: $after_sequence${NC}"
                    FAILED_TESTS=$((FAILED_TESTS + 1))
                    return
                fi
                
                # Check if same elements
                if ! same_elements "$before_sequence" "$after_sequence"; then
                    echo -e "${RED}✗ FAIL${NC} | $description"
                    echo -e "${RED}  ✗ CRITICAL: Elements changed during sorting!${NC}"
                    echo -e "${RED}    Before: $before_sequence${NC}"
                    echo -e "${RED}    After:  $after_sequence${NC}"
                    FAILED_TESTS=$((FAILED_TESTS + 1))
                    return
                fi
                
                echo -e "${GREEN}✓ PASS${NC} | $description"
                echo -e "${GREEN}  ✓ Sort order verified & elements preserved: $after_sequence${NC}"
                PASSED_TESTS=$((PASSED_TESTS + 1))
            else
                echo -e "${RED}✗ FAIL${NC} | $description - Could not extract result"
                FAILED_TESTS=$((FAILED_TESTS + 1))
            fi
        else
            echo -e "${RED}✗ FAIL${NC} | $description - Program failed with exit code $actual_exit"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    done
}

# Function to test performance with large datasets
test_performance() {
    echo -e "\n${BLUE}=== Performance Tests ===${NC}"
    
    # Test 3000 elements (requirement)
    if command -v jot >/dev/null 2>&1; then
        # macOS
        large_args=$(jot -r 3000 1 100000 | tr '\n' ' ')
    elif command -v shuf >/dev/null 2>&1; then
        # Linux
        large_args=$(seq 1 100000 | shuf -n 3000 | tr '\n' ' ')
    else
        echo -e "${YELLOW}Skipping performance tests: neither 'jot' nor 'shuf' available${NC}"
        return
    fi
    
    echo -n "Testing 3000 elements... "
    start_time=$(date +%s%N)
    ./PmergeMe $large_args > /dev/null 2>&1
    exit_code=$?
    end_time=$(date +%s%N)
    
    if [ $exit_code -eq 0 ]; then
        duration=$(( (end_time - start_time) / 1000000 )) # Convert to milliseconds
        echo -e "${GREEN}✓ PASS${NC} (${duration}ms)"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ FAIL${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
}

# Read and execute test cases from file
echo -e "${BLUE}=== Basic Test Cases ===${NC}"
while IFS='|' read -r expected_exit description args; do
    # Skip comments and empty lines
    if [[ $expected_exit =~ ^#.*$ ]] || [ -z "$expected_exit" ]; then
        continue
    fi
    
    run_test "$expected_exit" "$description" "$args"
done < test_cases.txt

# Run additional tests
test_sorting_correctness
test_random_sequences
test_performance

# Print summary
echo -e "\n${BLUE}=== Test Summary ===${NC}"
echo -e "Total tests: $TOTAL_TESTS"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"
echo -e "${RED}Failed: $FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "\n${GREEN}🎉 All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}❌ Some tests failed.${NC}"
    exit 1
fi
