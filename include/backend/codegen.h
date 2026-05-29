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
    bool is_generating_async_function = false;
    bool current_function_is_exported_with_string_return = false;
    std::string current_async_promise_name = "promise";

    void indent();
    std::string mapType(TypeNode* type);
    std::string mapTypeForCFFI(TypeNode* type, bool is_return);
    
    // Visitor methods for Code Gen
    void generateClass(ClassDeclNode* node);
    void generateInterface(InterfaceDeclNode* node);
    void generateFunction(FunctionNode* node);
    void generateAgenticFunction(AgenticFunctionNode* node);
    void generateOrchestration(AgentOrchestrationNode* node);
    void generateStatement(ASTNode* stmt);
    std::string generateExpression(ExprNode* expr);

public:
    std::string generate(ProgramNode* program);
};

#endif // CODEGEN_H
