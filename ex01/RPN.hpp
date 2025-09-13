#ifndef RPN_HPP
#define RPN_HPP

#include <string>
#include <stack>

class RPN {
private:
    bool tokenizeAndValidate(const std::string& expr);
    bool applyOp(std::stack<long>& st, char op);
    bool safeAdd(long a, long b, long& result);
    bool safeSub(long a, long b, long& result);
    bool safeMul(long a, long b, long& result);
    bool safeDiv(long a, long b, long& result);

public:
    RPN();
    ~RPN();
    RPN(const RPN& other);
    RPN& operator=(const RPN& other);

    bool evaluate(const std::string& expr, long& out);
};

#endif
