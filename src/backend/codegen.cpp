#include "../../include/backend/codegen.h"
#include <iostream>
#include <regex>

void CodeGenerator::indent() {
    for (int i = 0; i < indent_level * 4; ++i) {
        output << " ";
    }
}

std::string CodeGenerator::mapType(TypeNode* type) {
    if (!type) return "void";
    std::string base = type->type_name;
    if (base == "String") base = "std::string";
    else if (base == "Int") base = "int";
    else if (base == "Float") base = "float";
    else if (base == "Bool") base = "bool";
    else if (base == "Void") base = "void";
    else if (base == "UI") base = "zenith::UIElement";
    else if (base == "List") base = "std::vector";
    else if (base == "Map") base = "std::unordered_map";

    if (!type->generics.empty()) {
        base += "<";
        for (size_t i = 0; i < type->generics.size(); ++i) {
            base += mapType(type->generics[i].get());
            if (i < type->generics.size() - 1) base += ", ";
        }
        base += ">";
    }
    return base;
}

std::string CodeGenerator::generateExpression(ExprNode* expr) {
    if (!expr) return "";
    
    if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        return id->name;
    }
    if (auto* str = dynamic_cast<StringLiteralNode*>(expr)) {
        return "\"" + str->value + "\"";
    }
    if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
        return num->value;
    }
    if (auto* b = dynamic_cast<BoolLiteralNode*>(expr)) {
        return b->value ? "true" : "false";
    }
    if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        if (binary->op == "+" && binary->is_string_concat) {
            return "zenith::concat(" + generateExpression(binary->left.get()) + ", " + generateExpression(binary->right.get()) + ")";
        }
        return generateExpression(binary->left.get()) + " " + binary->op + " " + generateExpression(binary->right.get());
    }
    if (auto* list_lit = dynamic_cast<ListLiteralNode*>(expr)) {
        std::string res = "{";
        for (size_t i = 0; i < list_lit->elements.size(); ++i) {
            res += generateExpression(list_lit->elements[i].get());
            if (i < list_lit->elements.size() - 1) res += ", ";
        }
        res += "}";
        return res;
    }
    if (auto* map_lit = dynamic_cast<MapLiteralNode*>(expr)) {
        std::string res = "{";
        for (size_t i = 0; i < map_lit->entries.size(); ++i) {
            res += "{" + generateExpression(map_lit->entries[i].first.get()) + ", " + generateExpression(map_lit->entries[i].second.get()) + "}";
            if (i < map_lit->entries.size() - 1) res += ", ";
        }
        res += "}";
        return res;
    }
    if (auto* prop = dynamic_cast<PropertyAccessNode*>(expr)) {
        return generateExpression(prop->object.get()) + "." + prop->property_name;
    }
    if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        if (call->method_name == "render") {
            if (auto* inner_call = dynamic_cast<MethodCallNode*>(call->object.get())) {
                if (inner_call->method_name == "build") {
                    return "zenith::runInteractiveLoop(" + generateExpression(inner_call->object.get()) + ")";
                }
            }
        }
        std::string m_name = call->method_name;
        if (m_name == "push") m_name = "push_back";
        
        std::string res = generateExpression(call->object.get()) + "." + m_name + "(";
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            res += generateExpression(call->arguments[i].get());
            if (i < call->arguments.size() - 1) res += ", ";
        }
        res += ")";
        return res;
    }
    if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
        std::string res;
        bool is_class = class_names.count(ui->component_type) > 0;
        bool is_fn = function_names.count(ui->component_type) > 0;
        bool is_custom = is_class || is_fn;
        
        if (is_custom) {
            res = ui->component_type + "(";
        } else {
            res = "zenith::UI::" + ui->component_type + "(";
        }
        
        // Children / Positional arguments
        if (!ui->children.empty()) {
            if (!is_custom) res += "{";
            for (size_t i = 0; i < ui->children.size(); ++i) {
                res += generateExpression(ui->children[i].get());
                if (i < ui->children.size() - 1) res += ", ";
            }
            if (!is_custom) res += "}";
        } else {
            if (!is_custom) res += "{}";
        }
        
        // Named arguments (attributes) - only for built-in UI components
        if (!is_custom) {
            res += ", {";
            for (size_t i = 0; i < ui->named_args.size(); ++i) {
                std::string key = ui->named_args[i].first;
                std::string val = generateExpression(ui->named_args[i].second.get());
                if (key.rfind("on", 0) == 0) { // starts with "on"
                    if (val.front() != '"') {
                        val = "\"" + val + "\"";
                    }
                }
                res += "{\"" + key + "\", zenith::toString(" + val + ")}";
                if (i < ui->named_args.size() - 1) res += ", ";
            }
            res += "}";
        }
        res += ")";
        return res;
    }
    return "";
}

void CodeGenerator::generateStatement(ASTNode* stmt) {
    if (!stmt) return;
    
    if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(stmt)) {
        indent();
        output << "return " << generateExpression(return_stmt->expression.get()) << ";\n";
    } else if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt)) {
        indent();
        std::string type_name = var_decl->type->type_name;
        if (interface_names.count(type_name)) {
            if (var_decl->initializer) {
                if (auto* id = dynamic_cast<IdentifierNode*>(var_decl->initializer.get())) {
                    output << type_name << "& " << var_decl->var_name << " = " << id->name << ";\n";
                } else {
                    output << "auto temp_" << var_decl->var_name << " = " << generateExpression(var_decl->initializer.get()) << ";\n";
                    indent();
                    output << type_name << "& " << var_decl->var_name << " = temp_" << var_decl->var_name << ";\n";
                }
            } else {
                output << type_name << "* " << var_decl->var_name << " = nullptr;\n";
            }
        } else {
            output << mapType(var_decl->type.get()) << " " << var_decl->var_name;
            if (var_decl->initializer) {
                output << " = " << generateExpression(var_decl->initializer.get());
            }
            output << ";\n";
        }
    } else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(stmt)) {
        indent();
        output << "if (" << generateExpression(if_stmt->condition.get()) << ") {\n";
        indent_level++;
        for (const auto& s : if_stmt->then_branch) generateStatement(s.get());
        indent_level--;
        indent(); output << "}";
        if (!if_stmt->else_branch.empty()) {
            output << " else {\n";
            indent_level++;
            for (const auto& s : if_stmt->else_branch) generateStatement(s.get());
            indent_level--;
            indent(); output << "}\n";
        } else {
            output << "\n";
        }
    } else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(stmt)) {
        indent();
        output << "while (" << generateExpression(while_stmt->condition.get()) << ") {\n";
        indent_level++;
        for (const auto& s : while_stmt->body) generateStatement(s.get());
        indent_level--;
        indent(); output << "}\n";
    } else if (auto* set_state = dynamic_cast<SetStateStmtNode*>(stmt)) {
        indent();
        output << "{\n";
        indent_level++;
        for (const auto& s : set_state->body) {
            generateStatement(s.get());
        }
        indent();
        output << "std::cout << \"\\n[Runtime] setState: Re-rendering UI Layout...\\n\";\n";
        if (is_inside_class_method) {
            indent();
            output << "this->build().render();\n";
        }
        indent_level--;
        indent();
        output << "}\n";
    } else if (auto* expr = dynamic_cast<ExprNode*>(stmt)) {
        indent();
        output << generateExpression(expr) << ";\n";
    }
}

void CodeGenerator::generateAgenticFunction(AgenticFunctionNode* node) {
    indent();
    output << mapType(node->return_type.get()) << " " << node->function_name << "(";
    
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        output << mapType(node->parameters[i]->type.get()) << " " << node->parameters[i]->var_name;
        if (i < node->parameters.size() - 1) output << ", ";
    }
    output << ") {\n";
    indent_level++;

    indent(); output << "// --- AUTO-GENERATED AGENTIC BINDING ---\n";
    indent(); output << "std::string prompt = R\"(" << node->prompt_template << ")\";\n";
    
    // Extract and replace all placeholders in the prompt template
    std::regex var_regex("\\{([a-zA-Z_][a-zA-Z0-9_]*)\\}");
    auto words_begin = std::sregex_iterator(node->prompt_template.begin(), node->prompt_template.end(), var_regex);
    auto words_end = std::sregex_iterator();
    std::unordered_set<std::string> placeholders;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        placeholders.insert((*i)[1].str());
    }

    for (const auto& placeholder : placeholders) {
        indent(); output << "prompt = std::regex_replace(prompt, std::regex(\"\\\\{" << placeholder << "\\\\}\"), " << placeholder << ");\n";
    }

    indent(); output << "zenith::LLMClient client(\"http://localhost:11434/api/generate\");\n";
    indent(); output << "std::string response = client.prompt(prompt);\n";
    indent(); output << "return response;\n";
    
    indent_level--;
    indent(); output << "}\n\n";
}

void CodeGenerator::generateFunction(FunctionNode* node) {
    indent();
    if (node->function_name == "main") {
        output << "int main(";
    } else {
        output << mapType(node->return_type.get()) << " " << node->function_name << "(";
    }
    
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        output << mapType(node->parameters[i]->type.get()) << " " << node->parameters[i]->var_name;
        if (i < node->parameters.size() - 1) output << ", ";
    }
    output << ") {\n";
    
    indent_level++;
    for (const auto& stmt : node->body) {
        generateStatement(stmt.get());
    }
    indent_level--;
    
    indent(); output << "}\n\n";
}

void CodeGenerator::generateClass(ClassDeclNode* node) {
    indent();
    output << "class " << node->class_name;
    if (!node->implemented_interfaces.empty()) {
        output << " : ";
        for (size_t i = 0; i < node->implemented_interfaces.size(); ++i) {
            output << "public " << node->implemented_interfaces[i];
            if (i < node->implemented_interfaces.size() - 1) output << ", ";
        }
    }
    output << " {\n";
    output << "private:\n";
    
    indent_level++;
    for (const auto& arg : node->primary_constructor_args) {
        indent(); output << mapType(arg->type.get()) << " " << arg->var_name << ";\n";
    }
    indent_level--;

    output << "public:\n";
    indent_level++;
    
    // Custom fields
    for (const auto& field : node->fields) {
        indent(); output << mapType(field->type.get()) << " " << field->var_name;
        if (field->initializer) {
            output << " = " << generateExpression(field->initializer.get());
        }
        output << ";\n";
    }
    output << "\n";
    
    indent(); output << node->class_name << "(";
    for (size_t i = 0; i < node->primary_constructor_args.size(); ++i) {
        output << mapType(node->primary_constructor_args[i]->type.get()) << " " << node->primary_constructor_args[i]->var_name;
        if (i < node->primary_constructor_args.size() - 1) output << ", ";
    }
    output << ") ";
    
    if (!node->primary_constructor_args.empty()) {
        output << ": ";
        for (size_t i = 0; i < node->primary_constructor_args.size(); ++i) {
            output << node->primary_constructor_args[i]->var_name << "(" << node->primary_constructor_args[i]->var_name << ")";
            if (i < node->primary_constructor_args.size() - 1) output << ", ";
        }
    }
    output << " {}\n\n";

    for (const auto& method : node->methods) {
        is_inside_class_method = true;
        if (auto* agentic = dynamic_cast<AgenticFunctionNode*>(method.get())) {
            generateAgenticFunction(agentic);
        } else {
            generateFunction(method.get());
        }
        is_inside_class_method = false;
    }

    // Generate triggerCallback dispatcher
    indent(); output << "void triggerCallback(std::string name) {\n";
    indent_level++;
    for (const auto& method : node->methods) {
        if (method->function_name != "build" && method->parameters.empty()) {
            indent();
            output << "if (name == \"" << method->function_name << "\") { this->" << method->function_name << "(); return; }\n";
        }
    }
    indent_level--;
    indent(); output << "}\n\n";
    
    indent_level--;
    indent(); output << "};\n\n";
}

void CodeGenerator::generateInterface(InterfaceDeclNode* node) {
    indent();
    output << "class " << node->interface_name << " {\n";
    output << "public:\n";
    indent_level++;
    indent(); output << "virtual ~" << node->interface_name << "() = default;\n";
    for (const auto& method : node->methods) {
        indent();
        output << "virtual " << mapType(method->return_type.get()) << " " << method->function_name << "(";
        for (size_t i = 0; i < method->parameters.size(); ++i) {
            output << mapType(method->parameters[i]->type.get()) << " " << method->parameters[i]->var_name;
            if (i < method->parameters.size() - 1) output << ", ";
        }
        output << ") = 0;\n";
    }
    indent_level--;
    indent(); output << "};\n\n";
}

std::string CodeGenerator::generate(ProgramNode* program) {
    std::cout << "4. Running Code Generator (Transpiling to C++17)...\n";
    
    // Clear and populate class, interface, and function names
    class_names.clear();
    interface_names.clear();
    function_names.clear();
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            class_names.insert(class_decl->class_name);
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            interface_names.insert(interface_decl->interface_name);
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            function_names.insert(fn_decl->function_name);
        } else if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            if (imp->module_name == "std.io") {
                function_names.insert("print");
                function_names.insert("println");
                function_names.insert("httpGet");
                function_names.insert("httpPost");
            }
        }
    }
    
    output << "#include <string>\n";
    output << "#include <vector>\n";
    output << "#include <unordered_map>\n";
    output << "#include <regex>\n";
    output << "#include \"zenith_runtime.h\"\n\n";

    bool has_std_io = false;
    for (const auto& stmt : program->statements) {
        if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            if (imp->module_name == "std.io") {
                has_std_io = true;
            }
        }
    }

    if (has_std_io) {
        output << "inline void print(std::string msg) { std::cout << msg; }\n";
        output << "inline void println(std::string msg) { std::cout << msg << std::endl; }\n";
        output << "inline std::string httpGet(std::string url) { return zenith::httpGet(url); }\n";
        output << "inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }\n\n";
    }

    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            generateClass(class_decl);
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            generateInterface(interface_decl);
        } else if (auto* agentic_fn = dynamic_cast<AgenticFunctionNode*>(stmt.get())) {
            generateAgenticFunction(agentic_fn);
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            generateFunction(fn_decl);
        }
    }
    
    std::cout << "   [OK] Code Generation Complete.\n";
    return output.str();
}
