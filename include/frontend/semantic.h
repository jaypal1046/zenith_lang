#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../ast/ast.h"
#include "type_inference.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <regex>

// A simple symbol table to track variables in scope
class SymbolTable {
private:
    std::unordered_map<std::string, std::string> symbols; // name -> type
    SymbolTable* parent = nullptr;

public:
    SymbolTable(SymbolTable* p = nullptr) : parent(p) {}

    void define(const std::string& name, const std::string& type) {
        symbols[name] = type;
    }

    bool lookup(const std::string& name) {
        if (symbols.find(name) != symbols.end()) return true;
        if (parent != nullptr) return parent->lookup(name);
        return false;
    }

    std::string getType(const std::string& name) {
        if (symbols.find(name) != symbols.end()) return symbols[name];
        if (parent != nullptr) return parent->getType(name);
        return "";
    }

    SymbolTable* getParent() const { return parent; }
};

class SemanticAnalyzer {
private:
    SymbolTable* current_scope;
    bool has_errors = false;
    
    // Type inference support
    TypeInferencer type_inferencer;
    
    // Global tracking of structures and routines
    std::unordered_map<std::string, ClassDeclNode*> classes;
    std::unordered_map<std::string, InterfaceDeclNode*> interfaces;
    std::unordered_map<std::string, FunctionNode*> functions;
    std::unordered_map<std::string, AgentOrchestrationNode*> orchestrations;

    void error(const std::string& msg, ASTNode* node = nullptr) {
        std::cerr << "[Semantic Error]";
        if (node && node->line > 0) {
            std::cerr << " main.zen:" << node->line << ":" << node->column << ":";
        }
        std::cerr << " " << msg << "\n";
        has_errors = true;
    }

    void analyzeAgenticFunction(AgenticFunctionNode* node);
    void analyzeOrchestration(AgentOrchestrationNode* node);
    void analyzeFunction(FunctionNode* node);
    void analyzeStatement(ASTNode* stmt);
    void analyzeBlock(const std::vector<std::unique_ptr<ASTNode>>& block);
    std::string typeCheckExpression(ExprNode* expr, const std::string& expected_type = "");
    
    // Type inference helpers
    std::string inferAndValidateVarDecl(VarDeclNode* var_decl);
    std::string inferFunctionReturnType(FunctionNode* func);
    std::string inferBlockReturnType(const std::vector<std::unique_ptr<ASTNode>>& block);
    
    std::string current_fn_return_type;
    std::vector<std::unique_ptr<FunctionNode>> builtin_fns;
    bool isAssignable(const std::string& source, const std::string& target);

public:
    SemanticAnalyzer() : current_scope(new SymbolTable()) {}
    ~SemanticAnalyzer() { delete current_scope; }

    bool analyze(ProgramNode* program);
};

#endif // SEMANTIC_H
