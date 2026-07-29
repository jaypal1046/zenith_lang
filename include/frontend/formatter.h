#ifndef FORMATTER_H
#define FORMATTER_H

#include "../ast/ast.h"
#include <string>
#include <sstream>

class Formatter {
private:
    int indent_level = 0;
    std::stringstream output;

    void indent();
    void formatStatement(ASTNode* node);
    void formatExpression(ExprNode* expr);
    void formatPattern(PatternNode* pattern);
    void formatType(TypeNode* type);

public:
    Formatter() = default;
    std::string format(ProgramNode* program);
};

#endif // FORMATTER_H
