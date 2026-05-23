#ifndef ZENITH_PROJECT_H
#define ZENITH_PROJECT_H

#include <string>
#include <set>
#include <vector>
#include <memory>
#include "../include/frontend/lexer.h"
#include "../include/frontend/parser.h"

std::string getDirectory(const std::string& filepath);
void resolveImports(ProgramNode* program, std::set<std::string>& loaded_files, const std::string& current_dir);
int runCreateProject(const std::string& project_name, const std::string& argv0);
int runPlatformProject(const std::string& platform, const std::string& argv0);

#endif // ZENITH_PROJECT_H
