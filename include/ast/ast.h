#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

// Base Node
class ASTNode {
public:
    int line = 0;
    int column = 0;
    virtual ~ASTNode() = default;
};

// Type Node
class TypeNode : public ASTNode {
public:
    std::string type_name;
    std::vector<std::unique_ptr<TypeNode>> generics;
    TypeNode(std::string name) : type_name(std::move(name)) {}
};

// Expressions
class ExprNode : public ASTNode {};

// Dummy expression to hold logic like 'x > 10' for now
class BinaryExprNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> left;
    std::string op;
    std::unique_ptr<ExprNode> right;
    bool is_string_concat = false;
    BinaryExprNode(std::unique_ptr<ExprNode> l, std::string o, std::unique_ptr<ExprNode> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

class IdentifierNode : public ExprNode {
public:
    std::string name;
    IdentifierNode(std::string n) : name(std::move(n)) {}
};

class StringLiteralNode : public ExprNode {
public:
    std::string value;
    StringLiteralNode(std::string v) : value(std::move(v)) {}
};

class NumberLiteralNode : public ExprNode {
public:
    std::string value;
    bool is_float;
    NumberLiteralNode(std::string v, bool f) : value(std::move(v)), is_float(f) {}
};

class BoolLiteralNode : public ExprNode {
public:
    bool value;
    BoolLiteralNode(bool v) : value(v) {}
};

class ListLiteralNode : public ExprNode {
public:
    std::vector<std::unique_ptr<ExprNode>> elements;
};

class MapLiteralNode : public ExprNode {
public:
    std::vector<std::pair<std::unique_ptr<ExprNode>, std::unique_ptr<ExprNode>>> entries;
};

class PropertyAccessNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> object;
    std::string property_name;
    PropertyAccessNode(std::unique_ptr<ExprNode> obj, std::string prop)
        : object(std::move(obj)), property_name(std::move(prop)) {}
};

class MethodCallNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> object;
    std::string method_name;
    std::vector<std::unique_ptr<ExprNode>> arguments;
    
    MethodCallNode(std::unique_ptr<ExprNode> obj, std::string method)
        : object(std::move(obj)), method_name(std::move(method)) {}
};

class UIComponentNode : public ExprNode {
public:
    std::string component_type; // e.g. Column, Text
    std::vector<std::pair<std::string, std::unique_ptr<ExprNode>>> named_args;
    std::vector<std::unique_ptr<ExprNode>> children;
    
    UIComponentNode(std::string type) : component_type(std::move(type)) {}
};

// Variable Declaration (let x: String = "val" or String x = "val")
class VarDeclNode : public ASTNode {
public:
    std::string var_name;
    std::unique_ptr<TypeNode> type;
    std::unique_ptr<ExprNode> initializer;
    
    VarDeclNode(std::unique_ptr<TypeNode> t, std::string name, std::unique_ptr<ExprNode> init = nullptr)
        : type(std::move(t)), var_name(std::move(name)), initializer(std::move(init)) {}
};

// ==========================================
// CONTROL FLOW
// ==========================================
class IfStmtNode : public ASTNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<ASTNode>> then_branch;
    std::vector<std::unique_ptr<ASTNode>> else_branch; // Can be empty
    
    IfStmtNode(std::unique_ptr<ExprNode> cond) : condition(std::move(cond)) {}
};

class WhileStmtNode : public ASTNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
    
    WhileStmtNode(std::unique_ptr<ExprNode> cond) : condition(std::move(cond)) {}
};

class ReturnStmtNode : public ASTNode {
public:
    std::unique_ptr<ExprNode> expression;
    ReturnStmtNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
};

class SetStateStmtNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> body;
    SetStateStmtNode(std::vector<std::unique_ptr<ASTNode>> body_stmts) : body(std::move(body_stmts)) {}
};

// Functions
class FunctionNode : public ASTNode {
public:
    std::string function_name;
    std::unique_ptr<TypeNode> return_type;
    std::vector<std::unique_ptr<VarDeclNode>> parameters;
    std::vector<std::unique_ptr<ASTNode>> body;

    FunctionNode(std::unique_ptr<TypeNode> ret_type, std::string name)
        : return_type(std::move(ret_type)), function_name(std::move(name)) {}
};

// Agentic Functions
class AgenticFunctionNode : public FunctionNode {
public:
    std::string prompt_template;

    AgenticFunctionNode(std::unique_ptr<TypeNode> ret_type, std::string name, std::string prompt)
        : FunctionNode(std::move(ret_type), std::move(name)), prompt_template(std::move(prompt)) {}
};

// Classes with Primary Constructors (Kotlin style)
class ClassDeclNode : public ASTNode {
public:
    std::string class_name;
    std::vector<std::string> implemented_interfaces;
    std::vector<std::unique_ptr<VarDeclNode>> primary_constructor_args;
    std::vector<std::unique_ptr<VarDeclNode>> fields;
    std::vector<std::unique_ptr<FunctionNode>> methods;
    
    ClassDeclNode(std::string name) : class_name(std::move(name)) {}
};

class InterfaceDeclNode : public ASTNode {
public:
    std::string interface_name;
    std::vector<std::unique_ptr<FunctionNode>> methods;
    InterfaceDeclNode(std::string name) : interface_name(std::move(name)) {}
};

class ImportNode : public ASTNode {
public:
    std::string module_name;
    ImportNode(std::string name) : module_name(std::move(name)) {}
};

// Program Root
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
};

#endif // AST_H
