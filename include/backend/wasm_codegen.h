#ifndef WASM_CODEGEN_H
#define WASM_CODEGEN_H

#include "../ast/ast.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

class WASMCodeGenerator {
private:
    std::stringstream output;
    int indent_level = 0;
    
    // Maps variable names to WASM local indices
    std::unordered_map<std::string, std::string> current_locals;
    
    // Linear Memory Map for String Literals
    std::unordered_map<std::string, int> string_literals;
    int next_string_offset = 1024;
    
    // UI temporary locals mapping
    int ui_node_counter = 0;
    
    // Class compiling state
    std::string current_class_name;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> class_field_offsets;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> class_field_types;
    std::unordered_map<std::string, int> class_sizes;
    std::unordered_map<std::string, std::string> local_types;
    
    // Interface resolution
    std::unordered_map<std::string, std::vector<std::string>> interface_implementations;
    std::unordered_map<std::string, std::unordered_set<std::string>> class_methods;

    void indent();
    std::string mapTypeToWASM(TypeNode* type);
    int countUINodes(ASTNode* node);
    bool isFloatExpression(ExprNode* expr);
    bool isStringExpression(ExprNode* expr);
    
    // Visitor methods for WASM Stack Machine
    void generateClass(ClassDeclNode* node);
    void generateFunction(FunctionNode* node);
    void generateStatement(ASTNode* stmt);
    void generateExpression(ExprNode* expr); // Generates WASM stack instructions

public:
    std::string generate(ProgramNode* program);
    std::string generateHTMLWrapper();
};

#endif // WASM_CODEGEN_H
