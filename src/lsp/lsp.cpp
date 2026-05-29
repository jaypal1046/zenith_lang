#include "../../include/lsp/lsp.h"
#include "../../include/frontend/lexer.h"
#include "../../include/frontend/parser.h"
#include "../../include/frontend/semantic.h"
#include "../../include/frontend/formatter.h"
#include "../../include/ast/ast.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

// ============================================================================
// LIGHTWEIGHT JSON PARSER FOR LSP MESSAGES
// ============================================================================

enum class JsonType { Null, Bool, Number, String, Array, Object };

struct JsonValue {
    JsonType type = JsonType::Null;
    bool bool_val = false;
    double num_val = 0.0;
    std::string str_val;
    std::vector<std::shared_ptr<JsonValue>> arr_val;
    std::unordered_map<std::string, std::shared_ptr<JsonValue>> obj_val;

    bool hasKey(const std::string& key) const {
        if (type != JsonType::Object) return false;
        return obj_val.find(key) != obj_val.end();
    }

    const JsonValue& operator[](const std::string& key) const {
        static JsonValue null_val;
        if (type != JsonType::Object) return null_val;
        auto it = obj_val.find(key);
        if (it != obj_val.end() && it->second) return *(it->second);
        return null_val;
    }

    const JsonValue& operator[](size_t index) const {
        static JsonValue null_val;
        if (type != JsonType::Array || index >= arr_val.size()) return null_val;
        if (arr_val[index]) return *(arr_val[index]);
        return null_val;
    }
};

class JsonParser {
private:
    std::string_view json;
    size_t pos = 0;

    void skipWhitespace() {
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n')) {
            pos++;
        }
    }

    char peek() {
        skipWhitespace();
        if (pos >= json.length()) return '\0';
        return json[pos];
    }

    char get() {
        skipWhitespace();
        if (pos >= json.length()) return '\0';
        return json[pos++];
    }

public:
    explicit JsonParser(std::string_view s) : json(s) {}

    JsonValue parseValue() {
        char c = peek();
        if (c == '"') return parseString();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (c == '-' || std::isdigit(c)) return parseNumber();
        return JsonValue{};
    }

    JsonValue parseNull() {
        get(); // n
        get(); // u
        get(); // l
        get(); // l
        return JsonValue{};
    }

    JsonValue parseBool() {
        char c = get();
        JsonValue val;
        val.type = JsonType::Bool;
        if (c == 't') {
            get(); // r
            get(); // u
            get(); // e
            val.bool_val = true;
        } else {
            get(); // a
            get(); // l
            get(); // s
            get(); // e
            val.bool_val = false;
        }
        return val;
    }

    JsonValue parseNumber() {
        size_t start = pos;
        if (json[pos] == '-') pos++;
        while (pos < json.length() && (std::isdigit(json[pos]) || json[pos] == '.' || json[pos] == 'e' || json[pos] == 'E' || json[pos] == '+' || json[pos] == '-')) {
            pos++;
        }
        std::string num_str(json.substr(start, pos - start));
        JsonValue val;
        val.type = JsonType::Number;
        try {
            val.num_val = std::stod(num_str);
        } catch (...) {
            val.num_val = 0;
        }
        return val;
    }

    JsonValue parseString() {
        get(); // consume '"'
        std::string res;
        while (pos < json.length() && json[pos] != '"') {
            if (json[pos] == '\\') {
                pos++;
                if (pos < json.length()) {
                    char esc = json[pos++];
                    if (esc == 'n') res += '\n';
                    else if (esc == 'r') res += '\r';
                    else if (esc == 't') res += '\t';
                    else if (esc == '\\') res += '\\';
                    else if (esc == '"') res += '"';
                    else res += esc;
                }
            } else {
                res += json[pos++];
            }
        }
        if (pos < json.length()) pos++; // consume '"'
        JsonValue val;
        val.type = JsonType::String;
        val.str_val = res;
        return val;
    }

    JsonValue parseArray() {
        get(); // consume '['
        JsonValue val;
        val.type = JsonType::Array;
        if (peek() == ']') {
            get(); // consume ']'
            return val;
        }
        while (true) {
            val.arr_val.push_back(std::make_shared<JsonValue>(parseValue()));
            char c = peek();
            if (c == ',') {
                get(); // consume ','
            } else if (c == ']') {
                get(); // consume ']'
                break;
            } else {
                break;
            }
        }
        return val;
    }

    JsonValue parseObject() {
        get(); // consume '{'
        JsonValue val;
        val.type = JsonType::Object;
        if (peek() == '}') {
            get(); // consume '}'
            return val;
        }
        while (true) {
            JsonValue key_val = parseString();
            std::string key = key_val.str_val;
            get(); // consume ':'
            val.obj_val[key] = std::make_shared<JsonValue>(parseValue());
            char c = peek();
            if (c == ',') {
                get(); // consume ','
            } else if (c == '}') {
                get(); // consume '}'
                break;
            } else {
                break;
            }
        }
        return val;
    }
};

// ============================================================================
// LSP AST HOVER SEARCH ENGINE
// ============================================================================

void findNodeAt(ASTNode* node, int line, int col, ASTNode*& best) {
    if (!node) return;
    
    // Check if this node starts on the targeted line and at/before the targeted column
    if (node->line == line && node->column <= col) {
        if (!best || node->column > best->column) {
            best = node;
        }
    }
    
    // Recurse into child nodes based on concrete types
    if (auto* prog = dynamic_cast<ProgramNode*>(node)) {
        for (const auto& s : prog->statements) findNodeAt(s.get(), line, col, best);
    }
    else if (auto* var = dynamic_cast<VarDeclNode*>(node)) {
        findNodeAt(var->initializer.get(), line, col, best);
    }
    else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(node)) {
        for (const auto& arg : class_decl->primary_constructor_args) findNodeAt(arg.get(), line, col, best);
        for (const auto& f : class_decl->fields) findNodeAt(f.get(), line, col, best);
        for (const auto& m : class_decl->methods) findNodeAt(m.get(), line, col, best);
    }
    else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(node)) {
        for (const auto& m : interface_decl->methods) findNodeAt(m.get(), line, col, best);
    }
    else if (auto* fn = dynamic_cast<FunctionNode*>(node)) {
        for (const auto& p : fn->parameters) findNodeAt(p.get(), line, col, best);
        for (const auto& s : fn->body) findNodeAt(s.get(), line, col, best);
    }
    else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(node)) {
        findNodeAt(if_stmt->condition.get(), line, col, best);
        for (const auto& s : if_stmt->then_branch) findNodeAt(s.get(), line, col, best);
        for (const auto& s : if_stmt->else_branch) findNodeAt(s.get(), line, col, best);
    }
    else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(node)) {
        findNodeAt(while_stmt->condition.get(), line, col, best);
        for (const auto& s : while_stmt->body) findNodeAt(s.get(), line, col, best);
    }
    else if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(node)) {
        findNodeAt(return_stmt->expression.get(), line, col, best);
    }
    else if (auto* set_state = dynamic_cast<SetStateStmtNode*>(node)) {
        for (const auto& s : set_state->body) findNodeAt(s.get(), line, col, best);
    }
    else if (auto* await_expr = dynamic_cast<AwaitExprNode*>(node)) {
        findNodeAt(await_expr->expression.get(), line, col, best);
    }
    else if (auto* try_expr = dynamic_cast<TryExprNode*>(node)) {
        findNodeAt(try_expr->expression.get(), line, col, best);
    }
    else if (auto* match_expr = dynamic_cast<MatchExprNode*>(node)) {
        findNodeAt(match_expr->subject.get(), line, col, best);
        for (const auto& arm : match_expr->arms) {
            findNodeAt(arm.second.get(), line, col, best);
        }
    }
    else if (auto* lambda = dynamic_cast<LambdaNode*>(node)) {
        for (const auto& p : lambda->parameters) findNodeAt(p.get(), line, col, best);
        for (const auto& s : lambda->body) findNodeAt(s.get(), line, col, best);
    }
    else if (auto* ui = dynamic_cast<UIComponentNode*>(node)) {
        for (const auto& child : ui->children) findNodeAt(child.get(), line, col, best);
        for (const auto& arg : ui->named_args) findNodeAt(arg.second.get(), line, col, best);
    }
    else if (auto* binary = dynamic_cast<BinaryExprNode*>(node)) {
        findNodeAt(binary->left.get(), line, col, best);
        findNodeAt(binary->right.get(), line, col, best);
    }
    else if (auto* opt = dynamic_cast<OptionExprNode*>(node)) {
        findNodeAt(opt->value.get(), line, col, best);
    }
    else if (auto* res = dynamic_cast<ResultExprNode*>(node)) {
        findNodeAt(res->value.get(), line, col, best);
    }
    else if (auto* list_lit = dynamic_cast<ListLiteralNode*>(node)) {
        for (const auto& elem : list_lit->elements) findNodeAt(elem.get(), line, col, best);
    }
    else if (auto* map_lit = dynamic_cast<MapLiteralNode*>(node)) {
        for (const auto& entry : map_lit->entries) {
            findNodeAt(entry.first.get(), line, col, best);
            findNodeAt(entry.second.get(), line, col, best);
        }
    }
    else if (auto* prop = dynamic_cast<PropertyAccessNode*>(node)) {
        findNodeAt(prop->object.get(), line, col, best);
    }
    else if (auto* call = dynamic_cast<FunctionCallNode*>(node)) {
        for (const auto& arg : call->arguments) findNodeAt(arg.get(), line, col, best);
    }
    else if (auto* call = dynamic_cast<MethodCallNode*>(node)) {
        findNodeAt(call->object.get(), line, col, best);
        for (const auto& arg : call->arguments) findNodeAt(arg.get(), line, col, best);
    }
}

// ============================================================================
// LSP STATE STORAGE AND DRIVER
// ============================================================================

static std::unordered_map<std::string, std::shared_ptr<ProgramNode>> ast_cache;
static std::unordered_map<std::string, std::string> text_cache;

void sendResponse(const std::string& response_json) {
    std::cout << "Content-Length: " << response_json.length() << "\r\n\r\n" << response_json << std::flush;
}

void publishDiagnostics(const std::string& uri, const std::string& content) {
    // Intercept std::cerr errors
    std::streambuf* old_cerr = std::cerr.rdbuf();
    std::stringstream error_ss;
    std::cerr.rdbuf(error_ss.rdbuf());

    // Tokenize and Parse
    Lexer lexer(content);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parseProgram();

    // Run Semantic Analysis
    if (ast) {
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast.get());
        // Cache AST for hover support
        ast_cache[uri] = std::shared_ptr<ProgramNode>(ast.release());
        // Cache raw text for formatting
        text_cache[uri] = content;
    }

    // Restore std::cerr
    std::cerr.rdbuf(old_cerr);
    
    std::string errors = error_ss.str();
    std::stringstream err_stream(errors);
    std::string line_str;
    
    std::string diagnostics_json = "[";
    bool first_diag = true;

    while (std::getline(err_stream, line_str)) {
        if (line_str.empty()) continue;

        int line_num = 1;
        int col_num = 1;
        std::string message;

        // Example: [Semantic Error] main.zen:12:15: Variable 'x' is not declared
        size_t sem_err = line_str.find("[Semantic Error]");
        if (sem_err != std::string::npos) {
            size_t colon1 = line_str.find(':', sem_err);
            if (colon1 != std::string::npos) {
                size_t colon2 = line_str.find(':', colon1 + 1);
                if (colon2 != std::string::npos) {
                    size_t colon3 = line_str.find(':', colon2 + 1);
                    if (colon3 != std::string::npos) {
                        try {
                            line_num = std::stoi(line_str.substr(colon1 + 1, colon2 - colon1 - 1));
                            col_num = std::stoi(line_str.substr(colon2 + 1, colon3 - colon2 - 1));
                        } catch (...) {}
                        message = line_str.substr(colon3 + 1);
                    }
                }
            }
        }
        else {
            // Example: Parser Error: Expected a type at line 12
            size_t parser_err = line_str.find("Parser Error: ");
            if (parser_err != std::string::npos) {
                size_t at_line = line_str.rfind(" at line ");
                if (at_line != std::string::npos) {
                    try {
                        line_num = std::stoi(line_str.substr(at_line + 9));
                    } catch (...) {}
                    col_num = 1;
                    message = line_str.substr(parser_err + 14, at_line - parser_err - 14);
                } else {
                    message = line_str.substr(parser_err + 14);
                }
            }
        }

        if (message.empty()) {
            message = line_str;
        }

        // Clean up message spacing / escapes
        // Escape quotes for JSON
        std::string escaped_message;
        for (char c : message) {
            if (c == '"') escaped_message += "\\\"";
            else if (c == '\\') escaped_message += "\\\\";
            else if (c == '\n') escaped_message += "\\n";
            else if (c == '\r') escaped_message += "\\r";
            else if (c == '\t') escaped_message += "\\t";
            else escaped_message += c;
        }
        
        // Remove leading spaces
        if (!escaped_message.empty() && escaped_message[0] == ' ') {
            escaped_message = escaped_message.substr(1);
        }

        if (!first_diag) diagnostics_json += ",";
        first_diag = false;

        // Convert 1-indexed to 0-indexed for LSP
        int lsp_line = std::max(0, line_num - 1);
        int lsp_col = std::max(0, col_num - 1);

        diagnostics_json += "{";
        diagnostics_json += "\"range\":{\"start\":{\"line\":" + std::to_string(lsp_line) + ",\"character\":" + std::to_string(lsp_col) + "},";
        diagnostics_json += "\"end\":{\"line\":" + std::to_string(lsp_line) + ",\"character\":" + std::to_string(lsp_col + 5) + "}},";
        diagnostics_json += "\"severity\":1,"; // 1 = Error
        diagnostics_json += "\"message\":\"" + escaped_message + "\"";
        diagnostics_json += "}";
    }
    diagnostics_json += "]";

    std::string notif_json = "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",\"params\":{";
    notif_json += "\"uri\":\"" + uri + "\",";
    notif_json += "\"diagnostics\":" + diagnostics_json;
    notif_json += "}}";
    
    sendResponse(notif_json);
}

void handleHover(const std::string& id_str, const std::string& uri, int line_0, int col_0) {
    int target_line = line_0 + 1;
    int target_col = col_0 + 1;
    
    std::string hover_text = "";
    
    auto it = ast_cache.find(uri);
    if (it != ast_cache.end() && it->second) {
        ASTNode* best = nullptr;
        findNodeAt(it->second.get(), target_line, target_col, best);
        
        if (best) {
            if (auto* ident = dynamic_cast<IdentifierNode*>(best)) {
                std::string type = ident->type_hint;
                if (type.empty()) type = "Unknown";
                hover_text = "(variable) " + ident->name + ": " + type;
            }
            else if (auto* var = dynamic_cast<VarDeclNode*>(best)) {
                std::string type = (var->type ? var->type->type_name : "Auto");
                hover_text = "(variable) " + var->var_name + ": " + type;
            }
            else if (auto* fn = dynamic_cast<FunctionNode*>(best)) {
                std::string type = (fn->return_type ? fn->return_type->type_name : "Void");
                hover_text = "(function) " + fn->function_name + "(";
                for (size_t i = 0; i < fn->parameters.size(); ++i) {
                    auto* p = fn->parameters[i].get();
                    hover_text += p->var_name + ": " + (p->type ? p->type->type_name : "Auto");
                    if (i < fn->parameters.size() - 1) hover_text += ", ";
                }
                hover_text += ") -> " + type;
            }
            else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(best)) {
                hover_text = "class " + class_decl->class_name;
            }
            else if (auto* ui = dynamic_cast<UIComponentNode*>(best)) {
                hover_text = "UI Component " + ui->component_type;
            }
        }
    }
    
    // Format JSON response
    std::string resp;
    if (hover_text.empty()) {
        resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":null}";
    } else {
        // Escape text
        std::string escaped;
        for (char c : hover_text) {
            if (c == '"') escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else if (c == '\n') escaped += "\\n";
            else escaped += c;
        }
        resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":{\"contents\":{\"kind\":\"markdown\",\"value\":\"" + escaped + "\"}}}";
    }
    sendResponse(resp);
}

// ============================================================================
// LSP GO-TO-DEFINITION ENGINE
// ============================================================================

struct DefinitionLocation {
    std::string uri;
    int line = -1;   // 0-indexed, -1 means not found
    int col = -1;    // 0-indexed, -1 means not found
};

class SymbolDefinitionFinder {
private:
    std::string target_name;
    int target_line; // 1-indexed
    int target_col;  // 1-indexed

    // Scopes tracking
    std::vector<std::vector<VarDeclNode*>> scopes;
    ClassDeclNode* current_class = nullptr;
    
    ASTNode* resolved_decl = nullptr;
    bool ref_found = false;

    void pushScope() {
        scopes.push_back({});
    }

    void popScope() {
        if (!scopes.empty()) scopes.pop_back();
    }

    void addLocal(VarDeclNode* var) {
        if (!scopes.empty()) {
            scopes.back().push_back(var);
        }
    }

    // Check if the node matches the target symbol name and cursor line
    bool isTargetRef(ASTNode* node) {
        if (!node) return false;
        // Check if the node is on the target line and cursor is within or after start col
        if (node->line == target_line && node->column <= target_col) {
            return true;
        }
        return false;
    }

    void resolve() {
        // 1. Search local variables & parameters (most nested to least nested)
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            for (auto* var : *it) {
                if (var->var_name == target_name) {
                    resolved_decl = var;
                    return;
                }
            }
        }

        // 2. Search class fields and methods
        if (current_class) {
            for (const auto& f : current_class->fields) {
                if (f->var_name == target_name) {
                    resolved_decl = f.get();
                    return;
                }
            }
            for (const auto& m : current_class->methods) {
                if (m->function_name == target_name) {
                    resolved_decl = m.get();
                    return;
                }
            }
        }
    }

public:
    SymbolDefinitionFinder(std::string name, int l, int c)
        : target_name(std::move(name)), target_line(l), target_col(c) {}

    ASTNode* getResolved() const { return resolved_decl; }

    void walk(ASTNode* node) {
        if (!node || ref_found) return;

        if (isTargetRef(node)) {
            if (auto* ident = dynamic_cast<IdentifierNode*>(node)) {
                if (ident->name == target_name) {
                    resolve();
                    ref_found = true;
                    return;
                }
            }
        }

        if (auto* prog = dynamic_cast<ProgramNode*>(node)) {
            for (const auto& s : prog->statements) walk(s.get());
        }
        else if (auto* var = dynamic_cast<VarDeclNode*>(node)) {
            addLocal(var);
            walk(var->initializer.get());
        }
        else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(node)) {
            ClassDeclNode* prev_class = current_class;
            current_class = class_decl;
            pushScope();
            
            for (const auto& arg : class_decl->primary_constructor_args) {
                addLocal(arg.get());
            }
            for (const auto& f : class_decl->fields) {
                walk(f.get());
            }
            for (const auto& m : class_decl->methods) {
                walk(m.get());
            }
            
            popScope();
            current_class = prev_class;
        }
        else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(node)) {
            for (const auto& m : interface_decl->methods) walk(m.get());
        }
        else if (auto* fn = dynamic_cast<FunctionNode*>(node)) {
            pushScope();
            for (const auto& p : fn->parameters) {
                addLocal(p.get());
            }
            for (const auto& s : fn->body) {
                walk(s.get());
            }
            popScope();
        }
        else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(node)) {
            walk(if_stmt->condition.get());
            pushScope();
            for (const auto& s : if_stmt->then_branch) walk(s.get());
            popScope();
            pushScope();
            for (const auto& s : if_stmt->else_branch) walk(s.get());
            popScope();
        }
        else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(node)) {
            walk(while_stmt->condition.get());
            pushScope();
            for (const auto& s : while_stmt->body) walk(s.get());
            popScope();
        }
        else if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(node)) {
            walk(return_stmt->expression.get());
        }
        else if (auto* set_state = dynamic_cast<SetStateStmtNode*>(node)) {
            pushScope();
            for (const auto& s : set_state->body) walk(s.get());
            popScope();
        }
        else if (auto* await_expr = dynamic_cast<AwaitExprNode*>(node)) {
            walk(await_expr->expression.get());
        }
        else if (auto* try_expr = dynamic_cast<TryExprNode*>(node)) {
            walk(try_expr->expression.get());
        }
        else if (auto* match_expr = dynamic_cast<MatchExprNode*>(node)) {
            walk(match_expr->subject.get());
            for (const auto& arm : match_expr->arms) {
                pushScope();
                walk(arm.second.get());
                popScope();
            }
        }
        else if (auto* lambda = dynamic_cast<LambdaNode*>(node)) {
            pushScope();
            for (const auto& p : lambda->parameters) {
                addLocal(p.get());
            }
            for (const auto& s : lambda->body) {
                walk(s.get());
            }
            popScope();
        }
        else if (auto* ui = dynamic_cast<UIComponentNode*>(node)) {
            for (const auto& child : ui->children) walk(child.get());
            for (const auto& arg : ui->named_args) walk(arg.second.get());
        }
        else if (auto* binary = dynamic_cast<BinaryExprNode*>(node)) {
            walk(binary->left.get());
            walk(binary->right.get());
        }
        else if (auto* opt = dynamic_cast<OptionExprNode*>(node)) {
            walk(opt->value.get());
        }
        else if (auto* res = dynamic_cast<ResultExprNode*>(node)) {
            walk(res->value.get());
        }
        else if (auto* list_lit = dynamic_cast<ListLiteralNode*>(node)) {
            for (const auto& elem : list_lit->elements) walk(elem.get());
        }
        else if (auto* map_lit = dynamic_cast<MapLiteralNode*>(node)) {
            for (const auto& entry : map_lit->entries) {
                walk(entry.first.get());
                walk(entry.second.get());
            }
        }
        else if (auto* prop = dynamic_cast<PropertyAccessNode*>(node)) {
            walk(prop->object.get());
        }
        else if (auto* call = dynamic_cast<FunctionCallNode*>(node)) {
            for (const auto& arg : call->arguments) walk(arg.get());
        }
        else if (auto* call = dynamic_cast<MethodCallNode*>(node)) {
            walk(call->object.get());
            for (const auto& arg : call->arguments) walk(arg.get());
        }
    }
};

DefinitionLocation findDefinitionForSymbol(const std::string& symbol_name, ProgramNode* program, int target_line, int target_col) {
    DefinitionLocation result;
    
    // 1. Recursive scope check (identifies local variables, parameters, fields, and methods)
    SymbolDefinitionFinder finder(symbol_name, target_line, target_col);
    finder.walk(program);
    ASTNode* resolved = finder.getResolved();
    
    if (resolved) {
        result.line = resolved->line - 1;  // Convert to 0-indexed
        result.col = resolved->column;
        result.uri = "current";
        return result;
    }
    
    // 2. Fallback to top-level statements (global classes, functions, interfaces, variables)
    for (const auto& stmt : program->statements) {
        if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt.get())) {
            if (var_decl->var_name == symbol_name) {
                result.line = var_decl->line - 1;
                result.col = var_decl->column;
                result.uri = "current";
                return result;
            }
        }
        else if (auto* fn = dynamic_cast<FunctionNode*>(stmt.get())) {
            if (fn->function_name == symbol_name) {
                result.line = fn->line - 1;
                result.col = fn->column;
                result.uri = "current";
                return result;
            }
        }
        else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            if (class_decl->class_name == symbol_name) {
                result.line = class_decl->line - 1;
                result.col = class_decl->column;
                result.uri = "current";
                return result;
            }
        }
        else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            if (interface_decl->interface_name == symbol_name) {
                result.line = interface_decl->line - 1;
                result.col = interface_decl->column;
                result.uri = "current";
                return result;
            }
        }
    }
    
    return result;
}

void handleDefinition(const std::string& id_str, const std::string& uri, int line_0, int col_0) {
    int target_line = line_0 + 1;
    int target_col = col_0 + 1;
    
    auto it = ast_cache.find(uri);
    if (it == ast_cache.end() || !it->second) {
        std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":null}";
        sendResponse(resp);
        return;
    }
    
    ASTNode* best = nullptr;
    findNodeAt(it->second.get(), target_line, target_col, best);
    
    std::string symbol_name;
    if (auto* ident = dynamic_cast<IdentifierNode*>(best)) {
        symbol_name = ident->name;
    }
    
    if (symbol_name.empty()) {
        std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":null}";
        sendResponse(resp);
        return;
    }
    
    DefinitionLocation def_loc = findDefinitionForSymbol(symbol_name, it->second.get(), target_line, target_col);
    
    std::string resp;
    if (def_loc.line == -1 || def_loc.col == -1 || def_loc.uri.empty()) {
        // No definition found in current file - could be builtin or external
        resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":null}";
    } else {
        resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":[";
        resp += "{\"uri\":\"" + uri + "\",";
        resp += "\"range\":{\"start\":{\"line\":" + std::to_string(def_loc.line) + ",\"character\":" + std::to_string(def_loc.col) + "},";
        resp += "\"end\":{\"line\":" + std::to_string(def_loc.line) + ",\"character\":" + std::to_string(def_loc.col + symbol_name.length()) + "}}";
        resp += "}]}";
    }
    sendResponse(resp);
}

// ============================================================================
// LSP DOCUMENT FORMATTING ENGINE
// ============================================================================

void handleFormatting(const std::string& id_str, const std::string& uri) {
    auto text_it = text_cache.find(uri);
    auto ast_it = ast_cache.find(uri);
    
    if (text_it == text_cache.end() || ast_it == ast_cache.end() || !ast_it->second) {
        std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":null}";
        sendResponse(resp);
        return;
    }
    
    std::string original_text = text_it->second;
    ProgramNode* ast = ast_it->second.get();
    
    // Use the existing Formatter to format the AST
    Formatter formatter;
    std::string formatted_text = formatter.format(ast);
    
    // If no changes needed, return empty edits
    if (formatted_text == original_text) {
        std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":[]}";
        sendResponse(resp);
        return;
    }
    
    // Calculate the full document range
    size_t line_count = 0;
    for (char c : original_text) {
        if (c == '\n') {
            line_count++;
        }
    }
    // Account for the last line (may not end with newline)
    if (!original_text.empty() && original_text.back() != '\n') {
        line_count++;
    }
    
    // Return a single text edit that replaces the entire document
    std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":[";
    resp += "{\"range\":{\"start\":{\"line\":0,\"character\":0},";
    resp += "\"end\":{\"line\":" + std::to_string(line_count) + ",\"character\":0},";
    resp += "\"newText\":\"";
    
    // Escape the formatted text for JSON
    for (char c : formatted_text) {
        if (c == '"') resp += "\\\"";
        else if (c == '\\') resp += "\\\\";
        else if (c == '\n') resp += "\\n";
        else if (c == '\r') resp += "\\r";
        else if (c == '\t') resp += "\\t";
        else resp += c;
    }
    resp += "\"}]}";
    
    sendResponse(resp);
}

// ============================================================================
// LSP COMPLETION ENGINE
// ============================================================================

void handleCompletion(const std::string& id_str, const std::string& uri, int line_0, int col_0) {
    (void)line_0; (void)col_0;

    // Static Zenith keyword completions
    static const std::vector<std::pair<std::string,std::string>> keywords = {
        {"class",      "keyword"},  {"interface",  "keyword"},
        {"import",     "keyword"},  {"return",     "keyword"},
        {"if",         "keyword"},  {"else",       "keyword"},
        {"while",      "keyword"},  {"for",        "keyword"},
        {"setState",   "keyword"},  {"await",      "keyword"},
        {"async",      "keyword"},  {"match",      "keyword"},
        {"try",        "keyword"},  {"catch",      "keyword"},
        // Built-in types
        {"String",     "type"},     {"Int",        "type"},
        {"Float",      "type"},     {"Bool",       "type"},
        {"Void",       "type"},     {"List",       "type"},
        {"Map",        "type"},     {"Future",     "type"},
        {"Option",     "type"},     {"Result",     "type"},
        // Stdlib
        {"println",    "function"}, {"print",      "function"},
        {"httpGet",    "function"}, {"httpPost",   "function"},
        {"gcStats",    "function"},
        // UI components
        {"Column",     "component"},{"Row",        "component"},
        {"Text",       "component"},{"Button",     "component"},
        {"Card",       "component"},{"Container",  "component"},
        {"TextField",  "component"},{"Image",      "component"},
        {"Video",      "component"},{"Scrolling",  "component"},
        {"Checkbox",   "component"},{"Slider",     "component"},
        {"Toggle",     "component"},{"Dropdown",   "component"},
    };

    // Item kind mapping for LSP protocol:
    // 1=Text 2=Method 3=Function 6=Variable 9=Module 14=Keyword 7=Constructor 15=Snippet
    auto kindOf = [](const std::string& k) -> int {
        if (k == "keyword")   return 14;
        if (k == "type")      return 7;
        if (k == "function")  return 3;
        if (k == "component") return 9;
        return 6;
    };

    std::string items_json = "[";
    bool first = true;

    auto emit = [&](const std::string& label, const std::string& cat, const std::string& detail) {
        if (!first) items_json += ",";
        first = false;
        items_json += "{";
        items_json += "\"label\":\"" + label + "\",";
        items_json += "\"kind\":" + std::to_string(kindOf(cat)) + ",";
        items_json += "\"detail\":\"" + detail + "\",";
        items_json += "\"insertText\":\"" + label + "\"";
        items_json += "}";
    };

    for (const auto& kw : keywords) {
        emit(kw.first, kw.second, kw.second);
    }

    // Add live symbols from AST cache
    auto it = ast_cache.find(uri);
    if (it != ast_cache.end() && it->second) {
        for (const auto& stmt : it->second->statements) {
            if (auto* cls = dynamic_cast<ClassDeclNode*>(stmt.get())) {
                emit(cls->class_name, "type", "class " + cls->class_name);
                for (const auto& m : cls->methods) {
                    emit(m->function_name, "function", "method " + m->function_name);
                }
            } else if (auto* fn = dynamic_cast<FunctionNode*>(stmt.get())) {
                emit(fn->function_name, "function", "function " + fn->function_name);
            } else if (auto* var = dynamic_cast<VarDeclNode*>(stmt.get())) {
                emit(var->var_name, "function", "variable " + var->var_name);
            }
        }
    }

    items_json += "]";

    std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":{";
    resp += "\"isIncomplete\":false,";
    resp += "\"items\":" + items_json;
    resp += "}}";
    sendResponse(resp);
}


    std::string formatted_text = formatter.format(ast_it->second.get());
    
    if (formatted_text.empty()) {
        std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":null}";
        sendResponse(resp);
        return;
    }
    
    // Create a text edit that replaces the entire document
    std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":[";
    resp += "{\"range\":{";
    resp += "\"start\":{\"line\":0,\"character\":0},";
    
    // Calculate end line and character from original text
    const std::string& original_text = text_it->second;
    size_t line_count = 1;
    size_t last_line_start = 0;
    for (size_t i = 0; i < original_text.length(); ++i) {
        if (original_text[i] == '\n') {
            line_count++;
            last_line_start = i + 1;
        }
    }
    size_t last_line_length = original_text.length() - last_line_start;
    
    resp += "\"end\":{\"line\":" + std::to_string(line_count - 1) + ",\"character\":" + std::to_string(last_line_length) + "}";
    resp += "},";
    
    // Escape the formatted text for JSON
    std::string escaped_formatted;
    for (char c : formatted_text) {
        if (c == '"') escaped_formatted += "\\\"";
        else if (c == '\\') escaped_formatted += "\\\\";
        else if (c == '\n') escaped_formatted += "\\n";
        else if (c == '\r') escaped_formatted += "\\r";
        else if (c == '\t') escaped_formatted += "\\t";
        else escaped_formatted += c;
    }
    
    resp += "\"newText\":\"" + escaped_formatted + "\"";
    resp += "}]";
    sendResponse(resp);
}

void runLspServer() {
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stdin), _O_BINARY);
#endif
    std::string line;
    while (true) {
        int content_length = 0;
        
        // Read headers
        while (std::getline(std::cin, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                break;
            }
            if (line.rfind("Content-Length:", 0) == 0) {
                try {
                    content_length = std::stoi(line.substr(15));
                } catch (...) {
                    content_length = 0;
                }
            }
        }
        
        if (content_length <= 0) {
            if (std::cin.eof()) {
                break;
            }
            continue;
        }
        
        // Read JSON body
        std::string body(content_length, '\0');
        std::cin.read(&body[0], content_length);
        
        // Parse JSON message
        JsonParser parser(body);
        JsonValue val = parser.parseValue();
        
        if (val.type != JsonType::Object) {
            continue;
        }
        
        std::string id_str = "null";
        if (val.hasKey("id")) {
            const auto& id_val = val["id"];
            if (id_val.type == JsonType::Number) {
                id_str = std::to_string((int)id_val.num_val);
            } else if (id_val.type == JsonType::String) {
                id_str = "\"" + id_val.str_val + "\"";
            }
        }
        
        std::string method = val["method"].str_val;
        
        if (method == "initialize") {
            std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":{\"capabilities\":{";
            resp += "\"textDocumentSync\":1,";        // Full sync
            resp += "\"hoverProvider\":true,";
            resp += "\"completionProvider\":{\"triggerCharacters\":[\".\",\" \",\"(\",\":\"]},";
            resp += "\"definitionProvider\":true,";
            resp += "\"documentFormattingProvider\":true";
            resp += "}}}";
            sendResponse(resp);
        }
        else if (method == "initialized") {
            // Client acknowledgement — no response needed
        }
        else if (method == "textDocument/didOpen") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            std::string text = val["params"]["textDocument"]["text"].str_val;
            publishDiagnostics(uri, text);
        }
        else if (method == "textDocument/didChange") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            std::string text = val["params"]["contentChanges"][0]["text"].str_val;
            publishDiagnostics(uri, text);
        }
        else if (method == "textDocument/didSave") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            // Re-run diagnostics on save
            if (val["params"].hasKey("text")) {
                std::string text = val["params"]["text"].str_val;
                publishDiagnostics(uri, text);
            }
        }
        else if (method == "textDocument/hover") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            int hover_line = (int)val["params"]["position"]["line"].num_val;
            int hover_char = (int)val["params"]["position"]["character"].num_val;
            handleHover(id_str, uri, hover_line, hover_char);
        }
        else if (method == "textDocument/definition") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            int def_line = (int)val["params"]["position"]["line"].num_val;
            int def_char = (int)val["params"]["position"]["character"].num_val;
            handleDefinition(id_str, uri, def_line, def_char);
        }
        else if (method == "textDocument/formatting") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            handleFormatting(id_str, uri);
        }
        else if (method == "textDocument/completion") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            int comp_line = (int)val["params"]["position"]["line"].num_val;
            int comp_char = (int)val["params"]["position"]["character"].num_val;
            handleCompletion(id_str, uri, comp_line, comp_char);
        }
        else if (method == "shutdown") {
            std::string resp = "{\"jsonrpc\":\"2.0\",\"id\":" + id_str + ",\"result\":null}";
            sendResponse(resp);
        }
        else if (method == "exit") {
            break;
        }
    }
}
