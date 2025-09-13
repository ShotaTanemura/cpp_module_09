#include "RPN.hpp"
#include <limits>
#include <iostream>

RPN::RPN() {}

RPN::~RPN() {}

RPN::RPN(const RPN& other) {
    (void)other;
}

RPN& RPN::operator=(const RPN& other) {
    (void)other;
    return *this;
}

bool RPN::evaluate(const std::string& expr, long& out) {
    // Step 1: Validate format
    if (!tokenizeAndValidate(expr)) {
        return false;
    }
    
    // Step 2: Evaluate using stack, processing tokens on-the-fly
    std::stack<long> st;
    
    if (expr.empty()) {
        return false;
    }
    
    // Parse tokens by splitting on single spaces
    size_t start = 0;
    for (size_t i = 0; i <= expr.length(); ++i) {
        if (i == expr.length() || expr[i] == ' ') {
            if (start < i) {
                std::string token = expr.substr(start, i - start);
                
                if (token.length() == 1 && token[0] >= '0' && token[0] <= '9') {
                    // Single digit number
                    long value = token[0] - '0';
                    st.push(value);
                }
                else if (token.length() == 1 && 
                        (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/')) {
                    // Operator
                    if (!applyOp(st, token[0])) {
                        return false;
                    }
                }
                else {
                    // Invalid token
                    return false;
                }
                
                start = i + 1;
            }
        }
    }
    
    // Step 3: Check final stack state
    if (st.size() != 1) {
        return false;
    }
    
    out = st.top();
    return true;
}

bool RPN::tokenizeAndValidate(const std::string& expr) {
    if (expr.empty()) {
        return false;
    }
    
    // Check for leading/trailing spaces
    if (expr[0] == ' ' || expr[expr.length() - 1] == ' ') {
        return false;
    }
    
    bool hasTokens = false;
    
    // Parse tokens by splitting on single spaces
    size_t start = 0;
    for (size_t i = 0; i <= expr.length(); ++i) {
        if (i == expr.length() || expr[i] == ' ') {
            if (start < i) {
                std::string token = expr.substr(start, i - start);
                
                // Validate token - must be exactly 1 character
                if (token.length() != 1) {
                    return false;
                }
                
                char c = token[0];
                if (!((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '*' || c == '/')) {
                    return false;
                }
                
                hasTokens = true;
                start = i + 1;
            }
            else {
                // Empty token means consecutive spaces
                return false;
            }
        }
    }
    
    return hasTokens;
}

bool RPN::applyOp(std::stack<long>& st, char op) {
    if (st.size() < 2) {
        return false;
    }
    
    long b = st.top(); st.pop();
    long a = st.top(); st.pop();
    long result;
    
    switch (op) {
        case '+':
            if (!safeAdd(a, b, result)) return false;
            break;
        case '-':
            if (!safeSub(a, b, result)) return false;
            break;
        case '*':
            if (!safeMul(a, b, result)) return false;
            break;
        case '/':
            if (!safeDiv(a, b, result)) return false;
            break;
        default:
            return false;
    }
    
    st.push(result);
    return true;
}

bool RPN::safeAdd(long a, long b, long& result) {
    if (b > 0 && a > std::numeric_limits<long>::max() - b) {
        return false; // Overflow
    }
    if (b < 0 && a < std::numeric_limits<long>::min() - b) {
        return false; // Underflow
    }
    result = a + b;
    return true;
}

bool RPN::safeSub(long a, long b, long& result) {
    // Convert subtraction to addition: a - b = a + (-b)
    // Check for -b overflow first
    if (b == std::numeric_limits<long>::min()) {
        return false; // Cannot negate LONG_MIN
    }
    return safeAdd(a, -b, result);
}

bool RPN::safeMul(long a, long b, long& result) {
    // Handle zero cases
    if (a == 0 || b == 0) {
        result = 0;
        return true;
    }
    
    // Check for overflow by division
    if (a > 0) {
        if (b > 0) {
            if (a > std::numeric_limits<long>::max() / b) return false;
        } else {
            if (b < std::numeric_limits<long>::min() / a) return false;
        }
    } else {
        if (b > 0) {
            if (a < std::numeric_limits<long>::min() / b) return false;
        } else {
            if (a != 0 && b < std::numeric_limits<long>::max() / (-a)) return false;
        }
    }
    
    result = a * b;
    return true;
}

bool RPN::safeDiv(long a, long b, long& result) {
    if (b == 0) {
        return false; // Division by zero
    }
    
    // Check for the special overflow case: LONG_MIN / -1
    if (a == std::numeric_limits<long>::min() && b == -1) {
        return false; // Would overflow to LONG_MAX + 1
    }
    
    result = a / b;
    return true;
}
