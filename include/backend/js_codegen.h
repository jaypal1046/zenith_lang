#ifndef JS_CODEGEN_H
#define JS_CODEGEN_H

#include "../ast/ast.h"
#include <string>
#include <sstream>
#include <unordered_set>

class JSCodeGenerator {
private:
    std::stringstream output;
    int indent_level = 0;
    std::unordered_set<std::string> class_names;
    std::unordered_set<std::string> interface_names;
    std::unordered_set<std::string> function_names;
    std::unordered_set<std::string> agentic_functions;
    std::unordered_set<std::string> async_functions;
    std::unordered_set<std::string> current_class_fields;
    bool is_inside_class_method = false;

    void indent();
    bool containsAsyncCall(ASTNode* node, const std::unordered_set<std::string>& async_fns);
    
    // Visitor methods for JS Code Gen
    void generateClass(ClassDeclNode* node);
    void generateInterface(InterfaceDeclNode* node);
    void generateFunction(FunctionNode* node);
    void generateAgenticFunction(AgenticFunctionNode* node);
    void generateStatement(ASTNode* stmt);
    std::string generateExpression(ExprNode* expr);

public:
    std::string generate(ProgramNode* program);
};

#endif // JS_CODEGEN_H
