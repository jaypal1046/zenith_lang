#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <string_view>

enum class TokenType {
    KEYWORD, TYPE, ID, INT, FLOAT, STRING, OP, PUNCT, END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType type;
    std::string_view value;
    int line;
    int column;
};

class Lexer {
private:
    std::string_view source;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    void advance(int n = 1);
    char peek(int offset = 0) const;
    void skipWhitespaceAndComments();
    bool isKeyword(std::string_view text) const;
    bool isType(std::string_view text) const;

public:
    explicit Lexer(std::string_view src);
    std::vector<Token> tokenize();
};

const char* typeToString(TokenType t);

#endif // LEXER_H
