#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "lexer.h"
#include "../ast/ast.h"

class Parser {
private:
    std::vector<Token> tokens;
    size_t pos = 0;

    const Token& current() const;
    const Token& prev() const;
    const Token& peek(int offset = 1) const;
    void advance();
    bool match(TokenType type);
    bool match(TokenType type, std::string_view value);
    void expect(TokenType type, const std::string& error_msg);

    // Parsing methods
    std::unique_ptr<TypeNode> parseType();
    std::unique_ptr<VarDeclNode> parseParameter();
    std::unique_ptr<ExprNode> parseExpression(); // Basic math/comparisons
    std::unique_ptr<ExprNode> parseListLiteral(); // Parses [1, 2, 3]
    std::unique_ptr<ExprNode> parseMapLiteral(); // Parses {"key": "val"}
    std::unique_ptr<ExprNode> parseUIComponent(); // Parses Column(...) etc
    std::unique_ptr<ExprNode> parseLambda(); // Parses lambda expression (params) => { body }
    
    // Control Flow
    std::vector<std::unique_ptr<ASTNode>> parseBlock();
    std::unique_ptr<IfStmtNode> parseIfStatement();
    std::unique_ptr<WhileStmtNode> parseWhileStatement();

    std::unique_ptr<FunctionNode> parseFunction(bool is_agentic, bool is_async, std::unique_ptr<TypeNode> return_type);
    std::unique_ptr<ClassDeclNode> parseClass(bool is_managed = false);
    std::unique_ptr<InterfaceDeclNode> parseInterface();
    std::unique_ptr<AgentOrchestrationNode> parseOrchestration();

    template<typename T>
    std::unique_ptr<T> locate(std::unique_ptr<T> node, const Token& tok) {
        if (node) {
            node->line = tok.line;
            node->column = tok.column;
        }
        return node;
    }

public:
    explicit Parser(std::vector<Token> t);
    std::unique_ptr<ProgramNode> parseProgram();
};

#endif // PARSER_H
