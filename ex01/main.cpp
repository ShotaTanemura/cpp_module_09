#include "RPN.hpp"
#include <iostream>

/*
** Test cases that must pass:
**
** From the subject (expected stdout exactly):
** ./RPN "8 9 * 9 - 9 - 9 - 4 - 1 +"        -> 42
** ./RPN "7 7 * 7 -"                         -> 42
** ./RPN "1 2 * 2 / 2 * 2 4 - +"             -> 0
** ./RPN "(1 + 1)"                           -> Error (to stderr)
**
** Strict-format rejections:
** ./RPN "3 4  +"        -> Error (double space)
** ./RPN " 3 4 +"        -> Error (leading space)
** ./RPN "3 4 + "        -> Error (trailing space)
** ./RPN "12 3 +"        -> Error (multi-digit)
** ./RPN "3 -4 +"        -> Error (negative token)
** ./RPN "3 a +"         -> Error (invalid token)
**
** Runtime errors:
** ./RPN "3 0 /"         -> Error (division by zero)
** ./RPN "3 +"           -> Error (insufficient operands)
** ./RPN "3 4 5 +"       -> Error (stack not size 1)
**
** Overflow probes (behavior: Error):
** Use near-LONG_MAX/LONG_MIN sequences built from digits, 
** e.g., repeat additions/multiplications to exceed bounds.
*/

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Error" << std::endl;
        return 1;
    }
    
    RPN calculator;
    long result;
    
    if (calculator.evaluate(argv[1], result)) {
        std::cout << result << std::endl;
        return 0;
    } else {
        std::cerr << "Error" << std::endl;
        return 1;
    }
}
