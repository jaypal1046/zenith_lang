#include "../../include/frontend/lexer.h"
#include <cctype>

Lexer::Lexer(std::string_view src) : source(src) {}

void Lexer::advance(int n) {
    for (int i = 0; i < n; ++i) {
        if (pos < source.length()) {
            if (source[pos] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            pos++;
        }
    }
}

char Lexer::peek(int offset) const {
    if (pos + offset >= source.length()) return '\0';
    return source[pos + offset];
}

void Lexer::skipWhitespaceAndComments() {
    while (pos < source.length()) {
        char c = peek();
        if (std::isspace(c)) {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            while (pos < source.length() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') {
            advance(2);
            while (pos < source.length() && !(peek() == '*' && peek(1) == '/')) advance();
            advance(2);
        } else {
            break;
        }
    }
}

bool Lexer::isKeyword(std::string_view text) const {
    static const std::vector<std::string_view> keywords = {
        "agentic", "async", "if", "else", "return", "class", "while", "for", "struct", "import", "await", "setState", "interface", "implements", "let", "orchestration", "foreign", "break", "continue"
    };
    for (auto k : keywords) if (text == k) return true;
    return false;
}

bool Lexer::isType(std::string_view text) const {
    static const std::vector<std::string_view> types = {
        "Int", "Float", "Bool", "String", "Void", "UI", "List", "Map",
        "Ref", "Weak"  // Memory management smart pointer types
    };
    for (auto t : types) if (text == t) return true;
    return false;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (pos < source.length()) {
        skipWhitespaceAndComments();
        if (pos >= source.length()) break;

        int start_col = column;
        int start_line = line;
        size_t start_pos = pos;
        char c = peek();

        if (std::isalpha(c) || c == '_') {
            while (std::isalnum(peek()) || peek() == '_') advance();
            std::string_view text = source.substr(start_pos, pos - start_pos);
            
            TokenType type = TokenType::ID;
            if (isKeyword(text)) type = TokenType::KEYWORD;
            else if (isType(text)) type = TokenType::TYPE;
            
            tokens.push_back({type, text, start_line, start_col});
            continue;
        }

        // @annotation tokens: @managed, @weak, @gc_root
        if (c == '@') {
            advance(); // consume '@'
            size_t ann_start = pos;
            while (std::isalnum(peek()) || peek() == '_') advance();
            std::string_view ann_text = source.substr(ann_start, pos - ann_start);
            // Store as KEYWORD with the full @name for easy parser matching
            std::string full_ann = "@" + std::string(ann_text);
            // We need to store this - use a persistent string storage trick via the source
            // We emit as a KEYWORD token with value stored in a side table
            // Simplest: emit as ID type with leading @ preserved in value
            // We'll use TokenType::KEYWORD for @ annotations
            tokens.push_back({TokenType::KEYWORD,
                              source.substr(start_pos, pos - start_pos),
                              start_line, start_col});
            continue;
        }

        if (std::isdigit(c)) {
            bool isFloat = false;
            while (std::isdigit(peek()) || (peek() == '.' && !isFloat)) {
                if (peek() == '.') isFloat = true;
                advance();
            }
            if (peek() == 'e' || peek() == 'E') {
                isFloat = true;
                advance();
                if (peek() == '+' || peek() == '-') {
                    advance();
                }
                while (std::isdigit(peek())) {
                    advance();
                }
            }
            tokens.push_back({isFloat ? TokenType::FLOAT : TokenType::INT, 
                              source.substr(start_pos, pos - start_pos), start_line, start_col});
            continue;
        }

        if (c == '"') {
            advance();
            start_pos = pos;
            while (pos < source.length() && peek() != '"') {
                if (peek() == '\\') advance(2);
                else advance();
            }
            std::string_view text = source.substr(start_pos, pos - start_pos);
            advance();
            tokens.push_back({TokenType::STRING, text, start_line, start_col});
            continue;
        }

        if (std::string("+-*/=<>!&|").find(c) != std::string::npos) {
            advance();
            if ((c == '=' || c == '!' || c == '<' || c == '>') && peek() == '=') {
                advance();
            } else if ((c == '+' || c == '-' || c == '*' || c == '/') && peek() == '=') {
                advance();
            } else if ((c == '&' || c == '|') && peek() == c) {
                advance();
            }
            tokens.push_back({TokenType::OP, source.substr(start_pos, pos - start_pos), start_line, start_col});
            continue;
        }

        if (std::string("{}()[];.,:").find(c) != std::string::npos) {
            advance();
            tokens.push_back({TokenType::PUNCT, source.substr(start_pos, pos - start_pos), start_line, start_col});
            continue;
        }

        advance();
        tokens.push_back({TokenType::UNKNOWN, source.substr(start_pos, pos - start_pos), start_line, start_col});
    }
    tokens.push_back({TokenType::END_OF_FILE, "", line, column});
    return tokens;
}

const char* typeToString(TokenType t) {
    switch (t) {
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::TYPE: return "TYPE";
        case TokenType::ID: return "ID";
        case TokenType::INT: return "INT";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::OP: return "OP";
        case TokenType::PUNCT: return "PUNCT";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}
