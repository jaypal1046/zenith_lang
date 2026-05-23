#include "../../include/lsp/lsp.h"
#include "../../include/frontend/lexer.h"
#include "../../include/frontend/parser.h"
#include "../../include/frontend/semantic.h"
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
            resp += "\"textDocumentSync\":1,"; // Full sync
            resp += "\"hoverProvider\":true";
            resp += "}}}";
            sendResponse(resp);
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
        else if (method == "textDocument/hover") {
            std::string uri = val["params"]["textDocument"]["uri"].str_val;
            int hover_line = (int)val["params"]["position"]["line"].num_val;
            int hover_char = (int)val["params"]["position"]["character"].num_val;
            handleHover(id_str, uri, hover_line, hover_char);
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
