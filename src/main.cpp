#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include "../include/frontend/lexer.h"
#include "../include/frontend/parser.h"
#include "../include/frontend/semantic.h"
#include "../include/backend/codegen.h"
#include "../include/backend/js_codegen.h"
#include "../include/backend/wasm_codegen.h"

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

int main(int argc, char* argv[]) {
    std::string target = "cpp";
    std::string filename = "";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-target") {
            if (i + 1 < argc) {
                target = argv[++i];
            } else {
                std::cerr << "Error: -target option requires a value (cpp, web, or wasm)\n";
                return 1;
            }
        } else {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << "Usage: zenith <filename.zen> [-target <cpp|web|wasm>]\n";
        return 1;
    }

    if (filename.length() < 5 || filename.substr(filename.length() - 4) != ".zen") {
        std::cerr << "Error: Input file must have .zen extension\n";
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    file.close();

    std::cout << "--- Zenith Compiler v0.2.0 ---\n";
    std::cout << "1. Lexing source: " << filename << "...\n";
    
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    std::cout << "   [OK] Generated " << tokens.size() << " tokens.\n";

    std::cout << "2. Parsing AST...\n";
    Parser parser(tokens);
    auto ast = parser.parseProgram();
    std::cout << "   [OK] AST Parsed Successfully.\n";

    // Resolve custom imports recursively relative to the main file's path
    std::set<std::string> loaded_files;
    loaded_files.insert(filename);
    resolveImports(ast.get(), loaded_files, getDirectory(filename));

    std::cout << "3. Running Semantic Analysis...\n";
    SemanticAnalyzer analyzer;
    if (!analyzer.analyze(ast.get())) {
        return 1;
    }

    std::cout << "4. Running Code Generator (Target: " << target << ")...\n";
    std::string transpiled_code;
    std::string out_filename;

    if (target == "web") {
        JSCodeGenerator codegen;
        transpiled_code = codegen.generate(ast.get());
        out_filename = filename.substr(0, filename.length() - 4) + ".html";
    } else if (target == "wasm") {
        WASMCodeGenerator codegen;
        std::string wat_code = codegen.generate(ast.get());
        std::string html_wrapper = codegen.generateHTMLWrapper();
        
        std::string wat_filename = filename.substr(0, filename.length() - 4) + ".wat";
        std::ofstream wat_file(wat_filename);
        wat_file << wat_code;
        wat_file.close();
        
        std::string html_filename = filename.substr(0, filename.length() - 4) + "_wasm.html";
        std::ofstream html_file(html_filename);
        html_file << html_wrapper;
        html_file.close();
        
        std::cout << "   [OK] Written WAT output to: " << wat_filename << "\n";
        std::cout << "   [OK] Written HTML loader to: " << html_filename << "\n";
        return 0;
    } else {
        CodeGenerator codegen;
        transpiled_code = codegen.generate(ast.get());
        out_filename = filename.substr(0, filename.length() - 4) + ".cpp";
    }

    std::ofstream out_file(out_filename);
    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open output file " << out_filename << "\n";
        return 1;
    }
    out_file << transpiled_code;
    out_file.close();
    std::cout << "   [OK] Written " << (target == "web" ? "HTML" : "C++") << " transpiled output to: " << out_filename << "\n";

    return 0;
}
