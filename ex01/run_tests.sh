#!/bin/bash

# RPN Calculator Test Runner
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

# Check if RPN executable exists
if [ ! -f "./RPN" ]; then
    echo -e "${RED}Error: RPN executable not found. Please run 'make' first.${NC}"
    exit 1
fi

echo -e "${BLUE}=== RPN Calculator Test Suite ===${NC}"
echo

# Function to run a single test
run_test() {
    local input="$1"
    local expected="$2"
    local description="$3"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Run the RPN calculator and capture both stdout and stderr
    local output
    local exit_code
    
    if [[ "$expected" == "ERROR" ]]; then
        # For error cases, we expect non-zero exit code and "Error" output
        output=$(./RPN "$input" 2>&1)
        exit_code=$?
        
        if [[ $exit_code -ne 0 && "$output" == "Error" ]]; then
            echo -e "${GREEN}✓ PASS${NC}: $description"
            echo -e "  Input: '$input' → Expected: ERROR, Got: ERROR"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}✗ FAIL${NC}: $description"
            echo -e "  Input: '$input'"
            echo -e "  Expected: ERROR"
            echo -e "  Got: '$output' (exit code: $exit_code)"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    else
        # For success cases, we expect zero exit code and specific output
        expected_value="${expected#OK:}"  # Remove "OK:" prefix
        output=$(./RPN "$input" 2>&1)
        exit_code=$?
        
        if [[ $exit_code -eq 0 && "$output" == "$expected_value" ]]; then
            echo -e "${GREEN}✓ PASS${NC}: $description"
            echo -e "  Input: '$input' → Expected: $expected_value, Got: $expected_value"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}✗ FAIL${NC}: $description"
            echo -e "  Input: '$input'"
            echo -e "  Expected: $expected_value (exit code: 0)"
            echo -e "  Got: '$output' (exit code: $exit_code)"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    fi
    echo
}

# Read and process test cases
while IFS='|' read -r input expected description; do
    # Skip comments and empty lines
    if [[ "$input" =~ ^[[:space:]]*# ]] || [[ -z "$input" ]]; then
        continue
    fi
    
    # Trim whitespace from expected and description, but preserve input exactly as written
    expected=$(echo "$expected" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
    description=$(echo "$description" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
    
    # For input, only trim from the beginning and end if it's not a spacing test case
    if [[ "$description" =~ "Leading space" || "$description" =~ "Trailing space" || "$description" =~ "Tab character" ]]; then
        # Don't trim input for spacing test cases - keep it exactly as is
        input="$input"
    else
        # Trim whitespace for normal cases
        input=$(echo "$input" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
    fi
    
    # Skip if any field is empty after processing
    if [[ -z "$expected" || -z "$description" ]]; then
        continue
    fi
    
    run_test "$input" "$expected" "$description"
    
done < test_cases.txt

# Print summary
echo -e "${BLUE}=== Test Summary ===${NC}"
echo -e "Total tests: $TOTAL_TESTS"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"

if [[ $FAILED_TESTS -gt 0 ]]; then
    echo -e "${RED}Failed: $FAILED_TESTS${NC}"
    echo -e "${YELLOW}Success rate: $(( (PASSED_TESTS * 100) / TOTAL_TESTS ))%${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed! 🎉${NC}"
    echo -e "${GREEN}Success rate: 100%${NC}"
    exit 0
fi
