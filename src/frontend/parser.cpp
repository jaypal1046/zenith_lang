#include "../../include/frontend/parser.h"
#include <iostream>

Parser::Parser(std::vector<Token> t) : tokens(std::move(t)) {}

const Token& Parser::current() const {
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos];
}

const Token& Parser::prev() const {
    if (pos == 0) return tokens[0];
    return tokens[pos - 1];
}

const Token& Parser::peek(int offset) const {
    if (pos + offset >= tokens.size()) return tokens.back();
    return tokens[pos + offset];
}

void Parser::advance() {
    if (pos < tokens.size()) pos++;
}

bool Parser::match(TokenType type) {
    if (current().type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(TokenType type, std::string_view value) {
    if (current().type == type && current().value == value) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(TokenType type, const std::string& error_msg) {
    if (!match(type)) {
        std::cerr << "Parser Error: " << error_msg << " at line " << current().line << "\n";
        exit(1);
    }
}

// ---------------------------------------------------------
// Parsing Implementations
// ---------------------------------------------------------

std::unique_ptr<TypeNode> Parser::parseType() {
    const Token& start_tok = current();
    std::string type_name(current().value);
    if (current().type == TokenType::TYPE || (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])))) {
        advance();
    } else {
        std::cerr << "Parser Error: Expected a type (e.g., String, Int, or class name) at line " << current().line << "\n";
        exit(1);
    }
    
    auto node = locate(std::make_unique<TypeNode>(type_name), start_tok);
    
    // Check for Generics e.g., List<String>
    if (match(TokenType::OP, "<")) {
        node->generics.push_back(parseType());
        while (match(TokenType::PUNCT, ",")) {
            node->generics.push_back(parseType());
        }
        expect(TokenType::OP, "Expected '>' to close generics");
    }
    return node;
}

std::unique_ptr<ExprNode> Parser::parseListLiteral() {
    const Token& start_tok = current();
    auto list_node = locate(std::make_unique<ListLiteralNode>(), start_tok);
    expect(TokenType::PUNCT, "Expected '[' for list literal");
    
    if (current().type != TokenType::PUNCT || current().value != "]") {
        list_node->elements.push_back(parseExpression());
        while (match(TokenType::PUNCT, ",")) {
            list_node->elements.push_back(parseExpression());
        }
    }
    
    expect(TokenType::PUNCT, "Expected ']' to end list literal");
    return list_node;
}

std::unique_ptr<ExprNode> Parser::parseMapLiteral() {
    const Token& start_tok = current();
    auto map_node = locate(std::make_unique<MapLiteralNode>(), start_tok);
    expect(TokenType::PUNCT, "Expected '{' for map literal");
    
    if (current().type != TokenType::PUNCT || current().value != "}") {
        auto key = parseExpression();
        expect(TokenType::PUNCT, "Expected ':' after map key");
        auto value = parseExpression();
        map_node->entries.push_back({std::move(key), std::move(value)});
        
        while (match(TokenType::PUNCT, ",")) {
            auto k = parseExpression();
            expect(TokenType::PUNCT, "Expected ':' after map key");
            auto v = parseExpression();
            map_node->entries.push_back({std::move(k), std::move(v)});
        }
    }
    
    expect(TokenType::PUNCT, "Expected '}' to end map literal");
    return map_node;
}

std::unique_ptr<ExprNode> Parser::parseUIComponent() {
    const Token& start_tok = current();
    std::string comp_type(current().value);
    expect(TokenType::ID, "Expected UI component name");
    expect(TokenType::PUNCT, "Expected '(' for UI component");

    auto ui_node = locate(std::make_unique<UIComponentNode>(comp_type), start_tok);
    
    if (current().type != TokenType::PUNCT || current().value != ")") {
        do {
            if (current().type == TokenType::ID && peek().type == TokenType::PUNCT && peek().value == ":") {
                std::string arg_name(current().value);
                advance(); // consume ID
                advance(); // consume ':'
                auto val = parseExpression();
                ui_node->named_args.push_back({arg_name, std::move(val)});
            } else {
                auto child = parseExpression();
                ui_node->children.push_back(std::move(child));
            }
        } while (match(TokenType::PUNCT, ","));
    }
    
    expect(TokenType::PUNCT, "Expected ')' to close UI component");
    return ui_node;
}

std::unique_ptr<ExprNode> Parser::parseExpression() {
    // If it's a list literal
    if (current().type == TokenType::PUNCT && current().value == "[") {
        return parseListLiteral();
    }
    // If it's a map literal
    if (current().type == TokenType::PUNCT && current().value == "{") {
        return parseMapLiteral();
    }
    // If it's a function call or UI component (identifier followed by '(')
    if (current().type == TokenType::ID && peek().type == TokenType::PUNCT && peek().value == "(") {
        return parseUIComponent();
    }

    std::unique_ptr<ExprNode> left_node;
    const Token& start_tok = current();
    if (current().type == TokenType::STRING) {
        left_node = locate(std::make_unique<StringLiteralNode>(std::string(current().value)), start_tok);
        advance();
    } else if (current().type == TokenType::INT) {
        left_node = locate(std::make_unique<NumberLiteralNode>(std::string(current().value), false), start_tok);
        advance();
    } else if (current().type == TokenType::FLOAT) {
        left_node = locate(std::make_unique<NumberLiteralNode>(std::string(current().value), true), start_tok);
        advance();
    } else if (current().value == "true" || current().value == "false") {
        left_node = locate(std::make_unique<BoolLiteralNode>(current().value == "true"), start_tok);
        advance();
    } else {
        std::string left_val(current().value);
        left_node = locate(std::make_unique<IdentifierNode>(left_val), start_tok);
        advance(); // Consume identifier
    }

    // Parse Dot Notation (OOP)
    while (current().type == TokenType::PUNCT && current().value == ".") {
        const Token& dot_tok = current();
        advance(); // consume '.'
        std::string prop_name(current().value);
        expect(TokenType::ID, "Expected property or method name after '.'");
        
        if (match(TokenType::PUNCT, "(")) {
            // It's a method call!
            auto method_node = locate(std::make_unique<MethodCallNode>(std::move(left_node), prop_name), dot_tok);
            if (current().type != TokenType::PUNCT || current().value != ")") {
                method_node->arguments.push_back(parseExpression());
                while (match(TokenType::PUNCT, ",")) {
                    method_node->arguments.push_back(parseExpression());
                }
            }
            expect(TokenType::PUNCT, "Expected ')' to close method call");
            left_node = std::move(method_node);
        } else {
            // It's a property access
            left_node = locate(std::make_unique<PropertyAccessNode>(std::move(left_node), prop_name), dot_tok);
        }
    }

    // Parse Binary Expression
    if (current().type == TokenType::OP) {
        const Token& op_tok = current();
        std::string op(current().value);
        advance();
        auto right_node = parseExpression();
        return locate(std::make_unique<BinaryExprNode>(std::move(left_node), op, std::move(right_node)), op_tok);
    }
    return left_node;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseBlock() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    expect(TokenType::PUNCT, "Expected '{' to start block");
    
    while (current().type != TokenType::PUNCT || current().value != "}") {
        if (match(TokenType::KEYWORD, "if")) {
            statements.push_back(parseIfStatement());
        } else if (match(TokenType::KEYWORD, "while")) {
            statements.push_back(parseWhileStatement());
        } else if (match(TokenType::KEYWORD, "setState")) {
            const Token& start_tok = prev();
            auto body = parseBlock();
            statements.push_back(locate(std::make_unique<SetStateStmtNode>(std::move(body)), start_tok));
        } else if (match(TokenType::KEYWORD, "return")) {
            const Token& start_tok = prev();
            auto expr = parseExpression();
            expect(TokenType::PUNCT, "Expected ';'");
            statements.push_back(locate(std::make_unique<ReturnStmtNode>(std::move(expr)), start_tok));
        } else if (current().type == TokenType::TYPE || 
                   (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])) && 
                    peek().value != "." && peek().value != "(")) {
            const Token& start_tok = current();
            auto type = parseType();
            std::string var_name(current().value);
            expect(TokenType::ID, "Expected variable name");
            
            std::unique_ptr<ExprNode> init = nullptr;
            if (match(TokenType::OP, "=")) {
                init = parseExpression();
            }
            expect(TokenType::PUNCT, "Expected ';'");
            statements.push_back(locate(std::make_unique<VarDeclNode>(std::move(type), var_name, std::move(init)), start_tok));
        } else if (current().type == TokenType::ID) {
            statements.push_back(parseExpression());
            expect(TokenType::PUNCT, "Expected ';'");
        } else {
            advance();
        }
    }
    expect(TokenType::PUNCT, "Expected '}' to end block");
    return statements;
}

std::unique_ptr<IfStmtNode> Parser::parseIfStatement() {
    const Token& start_tok = prev();
    expect(TokenType::PUNCT, "Expected '(' after 'if'");
    auto condition = parseExpression();
    expect(TokenType::PUNCT, "Expected ')' after condition");
    
    auto if_node = locate(std::make_unique<IfStmtNode>(std::move(condition)), start_tok);
    if_node->then_branch = parseBlock();

    if (match(TokenType::KEYWORD, "else")) {
        if_node->else_branch = parseBlock();
    }
    return if_node;
}

std::unique_ptr<WhileStmtNode> Parser::parseWhileStatement() {
    const Token& start_tok = prev();
    expect(TokenType::PUNCT, "Expected '(' after 'while'");
    auto condition = parseExpression();
    expect(TokenType::PUNCT, "Expected ')' after condition");
    
    auto while_node = locate(std::make_unique<WhileStmtNode>(std::move(condition)), start_tok);
    while_node->body = parseBlock();
    return while_node;
}

std::unique_ptr<VarDeclNode> Parser::parseParameter() {
    const Token& start_tok = current();
    auto param_type = parseType();
    std::string param_name(current().value);
    expect(TokenType::ID, "Expected parameter name");
    return locate(std::make_unique<VarDeclNode>(std::move(param_type), param_name), start_tok);
}

std::unique_ptr<FunctionNode> Parser::parseFunction(bool is_agentic, std::unique_ptr<TypeNode> return_type) {
    const Token& start_tok = current();
    std::string fn_name(current().value);
    expect(TokenType::ID, "Expected function name");
    
    expect(TokenType::PUNCT, "Expected '(' for function parameters");
    
    std::vector<std::unique_ptr<VarDeclNode>> parameters;
    if (current().type != TokenType::PUNCT || current().value != ")") {
        parameters.push_back(parseParameter());
        while (match(TokenType::PUNCT, ",")) {
            parameters.push_back(parseParameter());
        }
    }
    expect(TokenType::PUNCT, "Expected ')' after parameters");
    
    if (is_agentic) {
        expect(TokenType::PUNCT, "Expected '{' to start agentic body");
        expect(TokenType::ID, "Expected 'prompt' block in agentic function");
        expect(TokenType::PUNCT, "Expected ':'");
        std::string prompt(current().value);
        expect(TokenType::STRING, "Expected string literal for prompt");
        
        auto agentic_fn = locate(std::make_unique<AgenticFunctionNode>(std::move(return_type), fn_name, prompt), start_tok);
        agentic_fn->parameters = std::move(parameters);
        
        while (!match(TokenType::PUNCT, "}")) advance();
        return agentic_fn;
    } else {
        auto fn = locate(std::make_unique<FunctionNode>(std::move(return_type), fn_name), start_tok);
        fn->parameters = std::move(parameters);
        
        fn->body = parseBlock();
        return fn;
    }
}

std::unique_ptr<ClassDeclNode> Parser::parseClass() {
    const Token& start_tok = current();
    expect(TokenType::KEYWORD, "Expected 'class'");
    std::string class_name(current().value);
    expect(TokenType::ID, "Expected class name");
    
    auto class_node = locate(std::make_unique<ClassDeclNode>(class_name), start_tok);

    if (match(TokenType::PUNCT, "(")) {
        if (current().type != TokenType::PUNCT || current().value != ")") {
            class_node->primary_constructor_args.push_back(parseParameter());
            while (match(TokenType::PUNCT, ",")) {
                class_node->primary_constructor_args.push_back(parseParameter());
            }
        }
        expect(TokenType::PUNCT, "Expected ')' after primary constructor parameters");
    }

    if (match(TokenType::KEYWORD, "implements")) {
        std::string interface_name(current().value);
        expect(TokenType::ID, "Expected interface name");
        class_node->implemented_interfaces.push_back(interface_name);
        while (match(TokenType::PUNCT, ",")) {
            std::string next_name(current().value);
            expect(TokenType::ID, "Expected interface name");
            class_node->implemented_interfaces.push_back(next_name);
        }
    }

    expect(TokenType::PUNCT, "Expected '{' to start class body");
    
    while (current().type != TokenType::PUNCT || current().value != "}") {
        bool is_agentic = false;
        if (current().type == TokenType::KEYWORD && current().value == "agentic") {
            is_agentic = true;
            advance();
        }
        
        if (current().type == TokenType::TYPE || (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])))) {
            const Token& type_tok = current();
            auto return_type = parseType();
            if (current().type == TokenType::ID && peek().value != "(") {
                std::string field_name(current().value);
                expect(TokenType::ID, "Expected field name");
                std::unique_ptr<ExprNode> init = nullptr;
                if (match(TokenType::OP, "=")) {
                    init = parseExpression();
                }
                expect(TokenType::PUNCT, "Expected ';'");
                class_node->fields.push_back(locate(std::make_unique<VarDeclNode>(std::move(return_type), field_name, std::move(init)), type_tok));
            } else {
                class_node->methods.push_back(parseFunction(is_agentic, std::move(return_type)));
            }
        } else {
            std::cerr << "Parser Error: Unexpected token in class body at line " << current().line << "\n";
            exit(1);
        }
    }
    
    expect(TokenType::PUNCT, "Expected '}' to end class body");
    return class_node;
}

std::unique_ptr<InterfaceDeclNode> Parser::parseInterface() {
    const Token& start_tok = current();
    expect(TokenType::KEYWORD, "Expected 'interface'");
    std::string interface_name(current().value);
    expect(TokenType::ID, "Expected interface name");
    
    auto interface_node = locate(std::make_unique<InterfaceDeclNode>(interface_name), start_tok);
    
    expect(TokenType::PUNCT, "Expected '{' to start interface body");
    
    while (current().type != TokenType::PUNCT || current().value != "}") {
        bool is_agentic = false;
        if (current().type == TokenType::KEYWORD && current().value == "agentic") {
            is_agentic = true;
            advance();
        }
        
        if (current().type == TokenType::TYPE || (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])))) {
            const Token& type_tok = current();
            auto return_type = parseType();
            
            std::string fn_name(current().value);
            expect(TokenType::ID, "Expected function name");
            
            expect(TokenType::PUNCT, "Expected '('");
            std::vector<std::unique_ptr<VarDeclNode>> parameters;
            if (current().type != TokenType::PUNCT || current().value != ")") {
                parameters.push_back(parseParameter());
                while (match(TokenType::PUNCT, ",")) {
                    parameters.push_back(parseParameter());
                }
            }
            expect(TokenType::PUNCT, "Expected ')' after parameters");
            expect(TokenType::PUNCT, "Expected ';' after interface function declaration");
            
            auto fn = locate(std::make_unique<FunctionNode>(std::move(return_type), fn_name), type_tok);
            fn->parameters = std::move(parameters);
            interface_node->methods.push_back(std::move(fn));
        } else {
            std::cerr << "Parser Error: Unexpected token in interface body at line " << current().line << "\n";
            exit(1);
        }
    }
    
    expect(TokenType::PUNCT, "Expected '}' to end interface body");
    return interface_node;
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    const Token& start_tok = current();
    auto program = locate(std::make_unique<ProgramNode>(), start_tok);
    
    while (current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::KEYWORD && current().value == "class") {
            program->statements.push_back(parseClass());
        } 
        else if (current().type == TokenType::KEYWORD && current().value == "interface") {
            program->statements.push_back(parseInterface());
        }
        else if (current().type == TokenType::KEYWORD && current().value == "agentic") {
            advance();
            auto ret_type = parseType();
            program->statements.push_back(parseFunction(true, std::move(ret_type)));
        } 
        else if (current().type == TokenType::TYPE || (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])))) {
            auto ret_type = parseType();
            program->statements.push_back(parseFunction(false, std::move(ret_type)));
        } 
        else if (current().type == TokenType::KEYWORD && current().value == "import") {
            const Token& imp_tok = current();
            advance();
            std::string mod;
            if (current().type == TokenType::STRING) {
                mod = std::string(current().value);
                advance();
            } else {
                while (current().type == TokenType::ID || (current().type == TokenType::PUNCT && current().value == ".")) {
                    mod += current().value;
                    advance();
                }
            }
            expect(TokenType::PUNCT, "Expected ';' after import statement");
            program->statements.push_back(locate(std::make_unique<ImportNode>(mod), imp_tok));
        }
        else {
            advance();
        }
    }
    
    return program;
}
