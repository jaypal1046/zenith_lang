#ifndef CODEGEN_H
#define CODEGEN_H

#include "../ast/ast.h"
#include <string>
#include <sstream>
#include <unordered_set>

class CodeGenerator {
private:
    std::stringstream output;
    int indent_level = 0;
    std::unordered_set<std::string> class_names;
    std::unordered_set<std::string> interface_names;
    std::unordered_set<std::string> function_names;
    bool is_inside_class_method = false;

    void indent();
    std::string mapType(TypeNode* type);
    
    // Visitor methods for Code Gen
    void generateClass(ClassDeclNode* node);
    void generateInterface(InterfaceDeclNode* node);
    void generateFunction(FunctionNode* node);
    void generateAgenticFunction(AgenticFunctionNode* node);
    void generateStatement(ASTNode* stmt);
    std::string generateExpression(ExprNode* expr);

public:
    std::string generate(ProgramNode* program);
};

#endif // CODEGEN_H
