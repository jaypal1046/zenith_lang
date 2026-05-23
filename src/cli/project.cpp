#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <filesystem>
#include <vector>
#include <memory>
#include "../include/frontend/lexer.h"
#include "../include/frontend/parser.h"

std::string getDirectory(const std::string& filepath) {
    size_t last_slash = filepath.find_last_of("/\\");
    if (last_slash == std::string::npos) {
        return "";
    }
    return filepath.substr(0, last_slash + 1);
}

void resolveImports(ProgramNode* program, std::set<std::string>& loaded_files, const std::string& current_dir) {
    std::vector<std::unique_ptr<ASTNode>> new_statements;
    
    for (auto& stmt : program->statements) {
        if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            std::string mod = imp->module_name;
            if (mod.length() > 4 && mod.substr(mod.length() - 4) == ".zen") {
                std::string full_path = current_dir + mod;
                if (loaded_files.find(full_path) == loaded_files.end()) {
                    loaded_files.insert(full_path);
                    
                    std::ifstream file(full_path);
                    if (!file.is_open()) {
                        std::cerr << "Error: Could not open imported file: " << full_path << "\n";
                        exit(1);
                    }
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string code = buffer.str();
                    file.close();
                    
                    Lexer sub_lexer(code);
                    auto sub_tokens = sub_lexer.tokenize();
                    Parser sub_parser(sub_tokens);
                    auto sub_ast = sub_parser.parseProgram();
                    
                    resolveImports(sub_ast.get(), loaded_files, getDirectory(full_path));
                    
                    for (auto& sub_stmt : sub_ast->statements) {
                        new_statements.push_back(std::move(sub_stmt));
                    }
                }
                continue;
            }
        }
        new_statements.push_back(std::move(stmt));
    }
    
    program->statements = std::move(new_statements);
}
