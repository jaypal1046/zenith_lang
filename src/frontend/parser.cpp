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

std::unique_ptr<ExprNode> Parser::parseLambda() {
    const Token& start_tok = current();
    expect(TokenType::PUNCT, "Expected '(' for lambda parameters");
    
    std::vector<std::unique_ptr<VarDeclNode>> parameters;
    if (current().type != TokenType::PUNCT || current().value != ")") {
        do {
            const Token& param_tok = current();
            std::string param_name(current().value);
            expect(TokenType::ID, "Expected lambda parameter name");
            
            std::unique_ptr<TypeNode> param_type = nullptr;
            if (match(TokenType::PUNCT, ":")) {
                param_type = parseType();
            } else {
                param_type = locate(std::make_unique<TypeNode>("Auto"), param_tok);
                param_type->is_inferred = true;
            }
            
            parameters.push_back(locate(std::make_unique<VarDeclNode>(std::move(param_type), param_name, nullptr), param_tok));
        } while (match(TokenType::PUNCT, ","));
    }
    expect(TokenType::PUNCT, "Expected ')' to close lambda parameters");
    
    // Expect "=>"
    expect(TokenType::OP, "=");
    expect(TokenType::OP, ">");
    
    std::vector<std::unique_ptr<ASTNode>> body;
    if (current().type == TokenType::PUNCT && current().value == "{") {
        body = parseBlock();
    } else {
        const Token& body_tok = current();
        auto expr = parseExpression();
        body.push_back(locate(std::make_unique<ReturnStmtNode>(std::move(expr)), body_tok));
    }
    
    auto ret_type = locate(std::make_unique<TypeNode>("Auto"), start_tok);
    ret_type->is_inferred = true;
    
    auto lambda_node = locate(std::make_unique<LambdaNode>(std::move(ret_type)), start_tok);
    lambda_node->parameters = std::move(parameters);
    lambda_node->body = std::move(body);
    
    return lambda_node;
}

std::unique_ptr<ExprNode> Parser::parseExpression() {
    // If it is a lambda expression, parse it
    if (current().type == TokenType::PUNCT && current().value == "(") {
        size_t scan_pos = pos + 1;
        int paren_depth = 1;
        while (scan_pos < tokens.size() && paren_depth > 0) {
            if (tokens[scan_pos].type == TokenType::PUNCT && tokens[scan_pos].value == "(") {
                paren_depth++;
            } else if (tokens[scan_pos].type == TokenType::PUNCT && tokens[scan_pos].value == ")") {
                paren_depth--;
            }
            if (paren_depth == 0) {
                break;
            }
            scan_pos++;
        }
        if (paren_depth == 0 && scan_pos + 2 < tokens.size()) {
            if (tokens[scan_pos + 1].type == TokenType::OP && tokens[scan_pos + 1].value == "=" &&
                tokens[scan_pos + 2].type == TokenType::OP && tokens[scan_pos + 2].value == ">") {
                return parseLambda();
            }
        }
    }

    // If it's an await expression
    if (current().type == TokenType::KEYWORD && current().value == "await") {
        const Token& await_tok = current();
        advance(); // consume "await"
        auto expr = parseExpression();
        return locate(std::make_unique<AwaitExprNode>(std::move(expr)), await_tok);
    }
    // If it's a unary expression (e.g. !x, -5)
    if (current().type == TokenType::OP && (current().value == "!" || current().value == "-")) {
        const Token& op_tok = current();
        std::string op(current().value);
        advance(); // consume "!" or "-"
        auto expr = parseExpression();
        return locate(std::make_unique<UnaryExprNode>(op, std::move(expr)), op_tok);
    }
    std::unique_ptr<ExprNode> left_node;
    bool has_primary = false;

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
        left_node = parseUIComponent();
        has_primary = true;
    }

    // Handle TYPE tokens used as constructor expressions: Ref<Node>("val"), Weak<T>(ref), etc.
    // e.g. Ref<Node>("hello") — TYPE token + '<' generics + '(' args
    if (!has_primary && current().type == TokenType::TYPE &&
        (peek().type == TokenType::OP && peek().value == "<") ||
        (current().type == TokenType::TYPE && peek().type == TokenType::PUNCT && peek().value == "(")) {
        const Token& call_tok = current();
        // Parse the full generic type name: e.g. "Ref<Node>"
        std::string call_name(current().value);
        advance(); // consume TYPE token (e.g. "Ref")
        // Consume optional generic params <T>
        if (current().type == TokenType::OP && current().value == "<") {
            advance(); // consume '<'
            int depth = 1;
            while (depth > 0 && current().type != TokenType::END_OF_FILE) {
                if (current().type == TokenType::OP && current().value == "<") depth++;
                else if (current().type == TokenType::OP && current().value == ">") depth--;
                if (depth > 0) advance();
            }
            advance(); // consume final '>'
        }
        // Now parse argument list '(' args ')'
        auto call_node = locate(std::make_unique<FunctionCallNode>(call_name), call_tok);
        if (current().type == TokenType::PUNCT && current().value == "(") {
            advance(); // consume '('
            if (current().type != TokenType::PUNCT || current().value != ")") {
                call_node->arguments.push_back(parseExpression());
                while (match(TokenType::PUNCT, ",")) {
                    call_node->arguments.push_back(parseExpression());
                }
            }
            expect(TokenType::PUNCT, "Expected ')' to close constructor call");
        }
        left_node = std::move(call_node);
        has_primary = true;
    }

    if (!has_primary) {
        const Token& start_tok = current();
        if (current().type == TokenType::PUNCT && current().value == "(") {
            advance();
            left_node = parseExpression();
            expect(TokenType::PUNCT, "Expected ')' after grouped expression");
        } else if (current().type == TokenType::STRING) {
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
        } else if (match(TokenType::KEYWORD, "for")) {
            statements.push_back(parseForStatement());
        } else if (match(TokenType::KEYWORD, "setState")) {
            const Token& start_tok = prev();
            auto body = parseBlock();
            statements.push_back(locate(std::make_unique<SetStateStmtNode>(std::move(body)), start_tok));
        } else if (match(TokenType::KEYWORD, "break")) {
            const Token& start_tok = prev();
            expect(TokenType::PUNCT, "Expected ';'");
            statements.push_back(locate(std::make_unique<BreakStmtNode>(), start_tok));
        } else if (match(TokenType::KEYWORD, "continue")) {
            const Token& start_tok = prev();
            expect(TokenType::PUNCT, "Expected ';'");
            statements.push_back(locate(std::make_unique<ContinueStmtNode>(), start_tok));
        } else if (match(TokenType::KEYWORD, "return")) {
            const Token& start_tok = prev();
            auto expr = parseExpression();
            expect(TokenType::PUNCT, "Expected ';'");
            statements.push_back(locate(std::make_unique<ReturnStmtNode>(std::move(expr)), start_tok));
        } else if (match(TokenType::KEYWORD, "let")) {
            const Token& start_tok = prev();
            std::string var_name(current().value);
            expect(TokenType::ID, "Expected variable name");
            
            // Optional type annotation for let: let x: Int = 5
            std::unique_ptr<TypeNode> type = nullptr;
            if (match(TokenType::PUNCT, ":")) {
                type = parseType();
            }
            
            expect(TokenType::OP, "=");
            auto init = parseExpression();
            expect(TokenType::PUNCT, "Expected ';'");
            
            // If no type specified, mark for inference
            if (!type) {
                type = locate(std::make_unique<TypeNode>("Auto"), start_tok);
                type->is_inferred = true;
            }
            
            statements.push_back(locate(std::make_unique<VarDeclNode>(std::move(type), var_name, std::move(init)), start_tok));
        } else if (current().type == TokenType::TYPE &&
                   peek().type == TokenType::ID &&
                   peek(2).type == TokenType::PUNCT && peek(2).value == "(") {
            auto return_type = parseType();
            statements.push_back(parseFunction(false, false, std::move(return_type)));
        } else if (current().type == TokenType::TYPE ||
                   (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])) &&
                    peek().value != "." && peek().value != "(")) {
            // Standard typed variable declaration: TypeName varname = expr;
            // Also handles Ref<T> and Weak<T> which have peek() == "<" not "("
            const Token& start_tok = current();
            auto type = parseType();   // consumes e.g. "Ref<Node>" fully
            
            // After parseType(), if current is '(' this is a constructor call expression,
            // not a variable declaration — fall through to expression parsing
            if (current().type == TokenType::PUNCT && current().value == "(") {
                // This was a type-as-expression (e.g., constructing without assignment)
                // Treat as expression statement
                auto call_node = locate(std::make_unique<FunctionCallNode>(type->type_name), start_tok);
                advance(); // consume '('
                if (current().type != TokenType::PUNCT || current().value != ")") {
                    call_node->arguments.push_back(parseExpression());
                    while (match(TokenType::PUNCT, ",")) {
                        call_node->arguments.push_back(parseExpression());
                    }
                }
                expect(TokenType::PUNCT, "Expected ')' to close call");
                expect(TokenType::PUNCT, "Expected ';'");
                statements.push_back(std::move(call_node));
            } else {
                std::string var_name(current().value);
                expect(TokenType::ID, "Expected variable name");
                
                std::unique_ptr<ExprNode> init = nullptr;
                if (match(TokenType::OP, "=")) {
                    init = parseExpression();
                }
                expect(TokenType::PUNCT, "Expected ';'");
                statements.push_back(locate(std::make_unique<VarDeclNode>(std::move(type), var_name, std::move(init)), start_tok));
            }

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
        if (match(TokenType::KEYWORD, "if")) {
            if_node->else_branch.push_back(parseIfStatement());
        } else {
            if_node->else_branch = parseBlock();
        }
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

std::unique_ptr<ForStmtNode> Parser::parseForStatement() {
    const Token& start_tok = prev();
    expect(TokenType::PUNCT, "Expected '(' after 'for'");

    std::unique_ptr<ASTNode> initializer = nullptr;
    if (current().type != TokenType::PUNCT || current().value != ";") {
        if (match(TokenType::KEYWORD, "let")) {
            const Token& let_tok = prev();
            std::string var_name(current().value);
            expect(TokenType::ID, "Expected variable name");
            std::unique_ptr<TypeNode> type = nullptr;
            if (match(TokenType::PUNCT, ":")) {
                type = parseType();
            }
            std::unique_ptr<ExprNode> init = nullptr;
            if (match(TokenType::OP, "=")) {
                init = parseExpression();
            }
            if (!type) {
                type = locate(std::make_unique<TypeNode>("Auto"), let_tok);
                type->is_inferred = true;
            }
            initializer = locate(std::make_unique<VarDeclNode>(std::move(type), var_name, std::move(init)), let_tok);
        } else {
            initializer = parseExpression();
        }
    }
    expect(TokenType::PUNCT, "Expected ';' after initializer");

    std::unique_ptr<ExprNode> condition = nullptr;
    if (current().type != TokenType::PUNCT || current().value != ";") {
        condition = parseExpression();
    }
    expect(TokenType::PUNCT, "Expected ';' after condition");

    std::unique_ptr<ExprNode> update = nullptr;
    if (current().type != TokenType::PUNCT || current().value != ")") {
        update = parseExpression();
    }
    expect(TokenType::PUNCT, "Expected ')' after update");

    auto for_node = locate(std::make_unique<ForStmtNode>(std::move(initializer), std::move(condition), std::move(update)), start_tok);
    for_node->body = parseBlock();
    return for_node;
}

std::unique_ptr<VarDeclNode> Parser::parseParameter() {
    const Token& start_tok = current();
    
    // Support type inference for parameters with default values
    // Syntax: fn name(param: Int = 5) or fn name(param = 5) [inferred]
    std::unique_ptr<TypeNode> param_type = nullptr;
    std::string param_name;
    std::unique_ptr<ExprNode> default_value = nullptr;
    
    // Check if parameter starts with identifier (could be name-only for inference)
    if (current().type == TokenType::ID) {
        param_name = std::string(current().value);
        advance();
        
        // Check for optional type annotation
        if (match(TokenType::PUNCT, ":")) {
            param_type = parseType();
        }
        
        // Check for default value
        if (match(TokenType::OP, "=")) {
            default_value = parseExpression();
            
            // If no type specified but has default value, mark for inference
            if (!param_type && default_value) {
                param_type = locate(std::make_unique<TypeNode>("Auto"), start_tok);
                param_type->is_inferred = true;
            }
        }
        
        // Require type if no default value
        if (!param_type && !default_value) {
            std::cerr << "Parser Error: Parameter '" << param_name << "' must have a type annotation or default value at line " << start_tok.line << "\n";
            exit(1);
        }
    } else {
        // Traditional syntax: Type name
        param_type = parseType();
        param_name = std::string(current().value);
        expect(TokenType::ID, "Expected parameter name");
    }
    
    auto param_node = locate(std::make_unique<VarDeclNode>(std::move(param_type), param_name, std::move(default_value)), start_tok);
    return param_node;
}

std::unique_ptr<FunctionNode> Parser::parseFunction(bool is_agentic, bool is_async, std::unique_ptr<TypeNode> return_type) {
    const Token& start_tok = current();
    std::string fn_name(current().value);
    expect(TokenType::ID, "Expected function name");
    
    // Parse optional generic parameters: <T, U, ...>
    std::vector<std::string> generic_params;
    if (match(TokenType::OP, "<")) {
        do {
            if (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0]))) {
                generic_params.push_back(std::string(current().value));
                advance();
            } else {
                std::cerr << "Parser Error: Expected generic type parameter (e.g., T, U) at line " << current().line << "\n";
                exit(1);
            }
        } while (match(TokenType::PUNCT, ","));
        expect(TokenType::OP, "Expected '>' to close generic parameters");
    }
    
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
        std::string prompt = "";
        bool streaming = false;
        bool multimodal = false;
        std::string model = "llama3";
        float temp = 0.7f;
        int max_toks = 1024;
        std::vector<std::string> tools;

        while (current().type != TokenType::PUNCT || current().value != "}") {
            if (current().type == TokenType::ID) {
                std::string key(current().value);
                advance();
                expect(TokenType::PUNCT, "Expected ':' after agentic parameter");
                
                if (key == "prompt") {
                    prompt = current().value;
                    expect(TokenType::STRING, "Expected string literal for prompt");
                } else if (key == "streaming" || key == "stream") {
                    if ((current().type == TokenType::KEYWORD || current().type == TokenType::ID) && current().value == "true") {
                        streaming = true;
                        advance();
                    } else if ((current().type == TokenType::KEYWORD || current().type == TokenType::ID) && current().value == "false") {
                        streaming = false;
                        advance();
                    } else {
                        std::cerr << "Parser Error: Expected boolean for streaming at line " << current().line << "\n";
                        exit(1);
                    }
                } else if (key == "multimodal") {
                    if ((current().type == TokenType::KEYWORD || current().type == TokenType::ID) && current().value == "true") {
                        multimodal = true;
                        advance();
                    } else if ((current().type == TokenType::KEYWORD || current().type == TokenType::ID) && current().value == "false") {
                        multimodal = false;
                        advance();
                    } else {
                        std::cerr << "Parser Error: Expected boolean for multimodal at line " << current().line << "\n";
                        exit(1);
                    }
                } else if (key == "model") {
                    model = current().value;
                    expect(TokenType::STRING, "Expected string literal for model");
                } else if (key == "temperature") {
                    if (current().type == TokenType::FLOAT || current().type == TokenType::INT) {
                        temp = std::stof(std::string(current().value));
                        advance();
                    } else {
                        std::cerr << "Parser Error: Expected number for temperature at line " << current().line << "\n";
                        exit(1);
                    }
                } else if (key == "max_tokens" || key == "maxTokens") {
                    if (current().type == TokenType::INT) {
                        max_toks = std::stoi(std::string(current().value));
                        advance();
                    } else {
                        std::cerr << "Parser Error: Expected integer for max_tokens at line " << current().line << "\n";
                        exit(1);
                    }
                } else if (key == "tools") {
                    expect(TokenType::PUNCT, "Expected '[' for tools list");
                    while (current().type != TokenType::PUNCT || current().value != "]") {
                        if (current().type == TokenType::STRING) {
                            tools.push_back(std::string(current().value));
                            advance();
                        } else {
                            std::cerr << "Parser Error: Expected tool name string at line " << current().line << "\n";
                            exit(1);
                        }
                        if (current().type == TokenType::PUNCT && current().value == ",") {
                            advance();
                        }
                    }
                    expect(TokenType::PUNCT, "Expected ']' to end tools list");
                } else {
                    std::cerr << "Parser Error: Unknown agentic parameter '" << key << "' at line " << current().line << "\n";
                    exit(1);
                }
            } else {
                advance();
            }
            if (current().type == TokenType::PUNCT && current().value == ",") {
                advance();
            }
        }
        expect(TokenType::PUNCT, "Expected '}' to close agentic body");

        auto agentic_fn = locate(std::make_unique<AgenticFunctionNode>(std::move(return_type), fn_name, prompt), start_tok);
        agentic_fn->parameters = std::move(parameters);
        agentic_fn->generic_params = std::move(generic_params);
        agentic_fn->is_async = is_async;
        agentic_fn->is_streaming = streaming;
        agentic_fn->is_multimodal = multimodal;
        agentic_fn->model_name = model;
        agentic_fn->temperature = temp;
        agentic_fn->max_tokens = max_toks;
        agentic_fn->tools = tools;
        
        return agentic_fn;
    } else {
        auto fn = locate(std::make_unique<FunctionNode>(std::move(return_type), fn_name), start_tok);
        fn->parameters = std::move(parameters);
        fn->generic_params = std::move(generic_params);
        fn->is_async = is_async;
        
        fn->body = parseBlock();
        return fn;
    }
}

std::unique_ptr<ClassDeclNode> Parser::parseClass(bool is_managed) {
    const Token& start_tok = current();
    expect(TokenType::KEYWORD, "Expected 'class'");
    std::string class_name(current().value);
    expect(TokenType::ID, "Expected class name");
    
    auto class_node = locate(std::make_unique<ClassDeclNode>(class_name), start_tok);
    class_node->is_managed = is_managed;  // Set from @managed annotation

    // Parse optional generic parameters: <T, U, ...>
    if (match(TokenType::OP, "<")) {
        do {
            if (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0]))) {
                class_node->generic_params.push_back(std::string(current().value));
                advance();
            } else {
                std::cerr << "Parser Error: Expected generic type parameter (e.g., T, U) at line " << current().line << "\n";
                exit(1);
            }
        } while (match(TokenType::PUNCT, ","));
        expect(TokenType::OP, "Expected '>' to close generic parameters");
    }

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
        bool is_async = false;
        while (current().type == TokenType::KEYWORD && (current().value == "agentic" || current().value == "async")) {
            if (current().value == "agentic") {
                is_agentic = true;
            } else if (current().value == "async") {
                is_async = true;
            }
            advance();
        }
        
        bool is_let = false;
        if (current().type == TokenType::KEYWORD && current().value == "let") {
            is_let = true;
            advance();
        }
        
        if (is_let) {
            const Token& type_tok = prev();
            std::string field_name(current().value);
            expect(TokenType::ID, "Expected field name");
            expect(TokenType::OP, "=");
            auto init = parseExpression();
            expect(TokenType::PUNCT, "Expected ';'");
            auto return_type = locate(std::make_unique<TypeNode>("Auto"), type_tok);
            class_node->fields.push_back(locate(std::make_unique<VarDeclNode>(std::move(return_type), field_name, std::move(init)), type_tok));
        } else if (current().type == TokenType::TYPE || (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])))) {
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
                class_node->methods.push_back(parseFunction(is_agentic, is_async, std::move(return_type)));
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
        bool is_async = false;
        while (current().type == TokenType::KEYWORD && (current().value == "agentic" || current().value == "async")) {
            if (current().value == "agentic") {
                is_agentic = true;
            } else if (current().value == "async") {
                is_async = true;
            }
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
            fn->is_async = is_async;
            interface_node->methods.push_back(std::move(fn));
        } else {
            std::cerr << "Parser Error: Unexpected token in interface body at line " << current().line << "\n";
            exit(1);
        }
    }
    
    expect(TokenType::PUNCT, "Expected '}' to end interface body");
    return interface_node;
}

std::unique_ptr<AgentOrchestrationNode> Parser::parseOrchestration() {
    const Token& start_tok = current();
    expect(TokenType::KEYWORD, "orchestration");
    
    std::string name(current().value);
    expect(TokenType::ID, "Expected orchestration name");
    
    expect(TokenType::PUNCT, "{");
    
    auto orch_node = locate(std::make_unique<AgentOrchestrationNode>(name), start_tok);
    
    while (current().type != TokenType::PUNCT || current().value != "}") {
        if (current().type == TokenType::ID) {
            std::string key(current().value);
            advance();
            expect(TokenType::PUNCT, "Expected ':' after orchestration parameter");
            
            if (key == "agents") {
                expect(TokenType::PUNCT, "Expected '[' for agents list");
                while (current().type != TokenType::PUNCT || current().value != "]") {
                    if (current().type == TokenType::ID) {
                        orch_node->agents.push_back(std::string(current().value));
                        advance();
                    } else {
                        std::cerr << "Parser Error: Expected agent function identifier at line " << current().line << "\n";
                        exit(1);
                    }
                    if (current().type == TokenType::PUNCT && current().value == ",") {
                        advance();
                    }
                }
                expect(TokenType::PUNCT, "Expected ']' to end agents list");
            } else if (key == "strategy") {
                orch_node->strategy = current().value;
                expect(TokenType::STRING, "Expected strategy string literal");
            } else {
                std::cerr << "Parser Error: Unknown orchestration key '" << key << "' at line " << current().line << "\n";
                exit(1);
            }
        } else {
            advance();
        }
        if (current().type == TokenType::PUNCT && current().value == ",") {
            advance();
        }
    }
    expect(TokenType::PUNCT, "Expected '}' to close orchestration block");
    return orch_node;
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    const Token& start_tok = current();
    auto program = locate(std::make_unique<ProgramNode>(), start_tok);
    
    while (current().type != TokenType::END_OF_FILE) {
        // --- Handle @annotation decorators (e.g. @managed, @weak, @gc_root) ---
        bool has_managed   = false;
        bool has_weak      = false;
        bool has_gc_root   = false;
        bool has_export    = false;
        std::string route_path = "";
        std::string library_name = "";
        while (current().type == TokenType::KEYWORD &&
               !current().value.empty() && current().value[0] == '@') {
            std::string ann(current().value);  // e.g. "@managed"
            advance();
            if (ann == "@managed")        has_managed = true;
            else if (ann == "@weak")      has_weak    = true;
            else if (ann == "@gc_root")   has_gc_root = true;
            else if (ann == "@export")    has_export  = true;
            else if (ann == "@route") {
                expect(TokenType::PUNCT, "(");
                route_path = std::string(current().value);
                expect(TokenType::STRING, "Expected string literal for route path");
                expect(TokenType::PUNCT, ")");
            }
            else if (ann == "@library") {
                expect(TokenType::PUNCT, "(");
                library_name = std::string(current().value);
                expect(TokenType::STRING, "Expected string literal for library name");
                expect(TokenType::PUNCT, ")");
            }
        }

        if (current().type == TokenType::KEYWORD && current().value == "class") {
            auto class_node = parseClass(has_managed);
            if (!route_path.empty()) {
                class_node->is_routed = true;
                class_node->route_path = route_path;
            }
            program->statements.push_back(std::move(class_node));
        } 
        else if (current().type == TokenType::KEYWORD && current().value == "interface") {
            program->statements.push_back(parseInterface());
        }
        else if (current().type == TokenType::KEYWORD && current().value == "orchestration") {
            program->statements.push_back(parseOrchestration());
        }
        else if (current().type == TokenType::KEYWORD && current().value == "foreign") {
            const Token& start_tok = current();
            advance(); // consume "foreign"
            
            std::string abi;
            if (current().type == TokenType::STRING) {
                abi = std::string(current().value);
                advance();
            } else {
                std::cerr << "Parser Error: Expected ABI string after 'foreign' keyword at line " << current().line << "\n";
                exit(1);
            }
            
            expect(TokenType::PUNCT, "Expected '{' to start foreign block");
            while (current().type != TokenType::PUNCT || current().value != "}") {
                bool is_async = false;
                if (current().type == TokenType::KEYWORD && current().value == "async") {
                    is_async = true;
                    advance();
                }
                
                auto ret_type = parseType();
                
                std::string fn_name(current().value);
                expect(TokenType::ID, "Expected function name");
                
                expect(TokenType::PUNCT, "Expected '(' for parameters");
                std::vector<std::unique_ptr<VarDeclNode>> parameters;
                if (current().type != TokenType::PUNCT || current().value != ")") {
                    parameters.push_back(parseParameter());
                    while (match(TokenType::PUNCT, ",")) {
                        parameters.push_back(parseParameter());
                    }
                }
                expect(TokenType::PUNCT, "Expected ')' after parameters");
                expect(TokenType::PUNCT, "Expected ';' after foreign function declaration");
                
                auto fn = locate(std::make_unique<FunctionNode>(std::move(ret_type), fn_name), start_tok);
                fn->parameters = std::move(parameters);
                fn->is_async = is_async;
                fn->is_foreign = true;
                fn->foreign_abi = abi;
                if (!library_name.empty()) {
                    fn->attributes["library"] = library_name;
                }
                
                program->statements.push_back(std::move(fn));
            }
            expect(TokenType::PUNCT, "Expected '}' to close foreign block");
        }
        else if (match(TokenType::KEYWORD, "let")) {
            const Token& let_tok = prev();
            std::string var_name(current().value);
            expect(TokenType::ID, "Expected variable name");

            std::unique_ptr<TypeNode> type = nullptr;
            if (match(TokenType::PUNCT, ":")) {
                type = parseType();
            }

            expect(TokenType::OP, "=");
            auto init = parseExpression();
            expect(TokenType::PUNCT, "Expected ';'");

            if (!type) {
                type = locate(std::make_unique<TypeNode>("Auto"), let_tok);
                type->is_inferred = true;
            }

            program->statements.push_back(
                locate(std::make_unique<VarDeclNode>(std::move(type), var_name, std::move(init)), let_tok));
        }
        else if ((current().type == TokenType::KEYWORD && (current().value == "agentic" || current().value == "async")) ||
                 current().type == TokenType::TYPE ||
                 (current().type == TokenType::ID && std::isupper(static_cast<unsigned char>(current().value[0])))) {
            bool is_agentic = false;
            bool is_async = false;
            while (current().type == TokenType::KEYWORD && (current().value == "agentic" || current().value == "async")) {
                if (current().value == "agentic") {
                    is_agentic = true;
                } else if (current().value == "async") {
                    is_async = true;
                }
                advance();
            }
            auto ret_type = parseType();
            auto fn = parseFunction(is_agentic, is_async, std::move(ret_type));
            fn->is_exported = has_export;
            program->statements.push_back(std::move(fn));
        } 
        else if (current().type == TokenType::KEYWORD && current().value == "import") {
            const Token& imp_tok = current();
            advance(); // consume "import"

            // Check for: import cdn "url"  OR  import npm "package"  OR  import native "path"  OR  import zen "package"
            if (current().type == TokenType::ID && (current().value == "cdn" || current().value == "npm" || current().value == "native" || current().value == "zen")) {
                std::string kind_str(current().value);
                advance(); // consume kind keyword

                std::string pkg_or_url;
                if (current().type == TokenType::STRING) {
                    pkg_or_url = std::string(current().value);
                    advance();
                } else {
                    std::cerr << "Parser Error: Expected URL/package string after 'import " << kind_str << "' at line " << current().line << "\n";
                    exit(1);
                }

                std::string target_filter = "";
                if (current().type == TokenType::KEYWORD && current().value == "for") {
                    advance(); // consume "for"
                    if (current().type == TokenType::STRING || current().type == TokenType::ID) {
                        target_filter = std::string(current().value);
                        advance();
                    } else {
                        std::cerr << "Parser Error: Expected target identifier or string after 'for' at line " << current().line << "\n";
                        exit(1);
                    }
                }

                // Optional semicolon
                if (current().type == TokenType::PUNCT && current().value == ";") advance();

                ImportNode::ImportKind kind = ImportNode::ImportKind::Module;
                std::string cdn_url = pkg_or_url;

                if (kind_str == "cdn") {
                    kind = ImportNode::ImportKind::Cdn;
                } else if (kind_str == "npm") {
                    kind = ImportNode::ImportKind::Npm;
                    cdn_url = "https://cdn.jsdelivr.net/npm/" + pkg_or_url;
                } else if (kind_str == "native") {
                    kind = ImportNode::ImportKind::Native;
                } else if (kind_str == "zen") {
                    kind = ImportNode::ImportKind::Zen;
                }

                program->statements.push_back(
                    locate(std::make_unique<ImportNode>(pkg_or_url, kind, cdn_url, target_filter), imp_tok));
            } else {
                // Classic: import std.io or import "path"
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
                if (current().type == TokenType::PUNCT && current().value == ";") advance();
                program->statements.push_back(
                    locate(std::make_unique<ImportNode>(mod, ImportNode::ImportKind::Module), imp_tok));
            }
        }
        else {
            advance();
        }
    }
    
    return program;
}
