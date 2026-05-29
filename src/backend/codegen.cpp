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
    else if (base == "Future") base = "zenith::stdlib::Future";
    else if (base == "Promise") base = "zenith::stdlib::Promise";
    else if (base == "Ref") base = "zenith::mem::Ref";
    else if (base == "Weak") base = "zenith::mem::Weak";
    else if (base == "Function") {
        if (type->generics.empty()) return "std::function<void()>";
        std::string ret = mapType(type->generics.back().get());
        std::string args;
        for (size_t i = 0; i < type->generics.size() - 1; ++i) {
            args += mapType(type->generics[i].get());
            if (i < type->generics.size() - 2) args += ", ";
        }
        return "std::function<" + ret + "(" + args + ")>";
    }

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

std::string CodeGenerator::mapTypeForCFFI(TypeNode* type, bool is_return) {
    if (!type) return "void";
    std::string base = type->type_name;
    // Map String to const char* for parameters, char* for return (C ABI compatibility)
    if (base == "String") {
        return is_return ? "char*" : "const char*";
    }
    else if (base == "Int") base = "int";
    else if (base == "Float") base = "float";
    else if (base == "Bool") base = "bool";
    else if (base == "Void") base = "void";
    else if (base == "UI") base = "zenith::UIElement";
    else if (base == "List") base = "std::vector";
    else if (base == "Map") base = "std::unordered_map";
    else if (base == "Future") base = "zenith::stdlib::Future";
    else if (base == "Promise") base = "zenith::stdlib::Promise";
    else if (base == "Ref") base = "zenith::mem::Ref";
    else if (base == "Weak") base = "zenith::mem::Weak";
    else if (base == "Function") {
        if (type->generics.empty()) return "std::function<void()>";
        std::string ret = mapTypeForCFFI(type->generics.back().get(), true);
        std::string args;
        for (size_t i = 0; i < type->generics.size() - 1; ++i) {
            args += mapTypeForCFFI(type->generics[i].get(), false);
            if (i < type->generics.size() - 2) args += ", ";
        }
        return "std::function<" + ret + "(" + args + ")>";
    }

    if (!type->generics.empty()) {
        base += "<";
        for (size_t i = 0; i < type->generics.size(); ++i) {
            base += mapTypeForCFFI(type->generics[i].get(), false);
            if (i < type->generics.size() - 1) base += ", ";
        }
        base += ">";
    }
    return base;
}

std::string CodeGenerator::generateExpression(ExprNode* expr) {
    if (!expr) return "";
    
    if (auto* await_expr = dynamic_cast<AwaitExprNode*>(expr)) {
        return "zenith::await_val(" + generateExpression(await_expr->expression.get()) + ")";
    }
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
        if (prop->property_name == "size") {
            return generateExpression(prop->object.get()) + ".size()";
        }
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
        if (call->method_name == "map" && !call->arguments.empty()) {
            std::string list_expr = generateExpression(call->object.get());
            std::string callback_expr = generateExpression(call->arguments[0].get());
            std::string res = "[&]() {\n";
            res += "            std::vector<zenith::UIElement> _mapped_res;\n";
            res += "            for (auto&& _item : " + list_expr + ") {\n";
            res += "                _mapped_res.push_back(" + callback_expr + "(_item));\n";
            res += "            }\n";
            res += "            return _mapped_res;\n";
            res += "        }()";
            return res;
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
    if (auto* fcall = dynamic_cast<FunctionCallNode*>(expr)) {
        std::string fname = fcall->function_name;
        // Map Ref<T>(...) constructor to zenith::mem::make_ref<T>(...)
        // The actual type was stored as just "Ref" in function_name
        if (fname == "Ref") {
            // Handled at VarDecl level via make_ref<T>() — fallback generic call
            std::string res = fname + "(";
            for (size_t i = 0; i < fcall->arguments.size(); ++i) {
                res += generateExpression(fcall->arguments[i].get());
                if (i < fcall->arguments.size() - 1) res += ", ";
            }
            res += ")";
            return res;
        }
        if (fname == "Weak") {
            std::string res = "zenith::mem::Weak<zenith::mem::Managed>(";
            if (!fcall->arguments.empty()) res += generateExpression(fcall->arguments[0].get());
            res += ")";
            return res;
        }
        // Generic FunctionCallNode — emit as regular C++ function call
        std::string res = fname + "(";
        for (size_t i = 0; i < fcall->arguments.size(); ++i) {
            res += generateExpression(fcall->arguments[i].get());
            if (i < fcall->arguments.size() - 1) res += ", ";
        }
        res += ")";
        return res;
    }
    if (auto* lambda = dynamic_cast<LambdaNode*>(expr)) {
        std::string res = "[=](";
        for (size_t i = 0; i < lambda->parameters.size(); ++i) {
            res += mapType(lambda->parameters[i]->type.get()) + " " + lambda->parameters[i]->var_name;
            if (i < lambda->parameters.size() - 1) res += ", ";
        }
        res += ") {\n";
        
        std::stringstream ss;
        std::streambuf* old_buf = static_cast<std::ostream&>(output).rdbuf(ss.rdbuf());
        
        indent_level++;
        for (const auto& s : lambda->body) {
            generateStatement(s.get());
        }
        indent_level--;
        
        static_cast<std::ostream&>(output).rdbuf(old_buf);
        res += ss.str();
        
        for (int i = 0; i < indent_level * 4; ++i) {
            res += " ";
        }
        res += "}";
        return res;
    }
    if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
        std::string res;
        bool is_class = class_names.count(ui->component_type) > 0;
        bool is_fn = function_names.count(ui->component_type) > 0;
        bool is_variable_call = !ui->component_type.empty() && std::islower(static_cast<unsigned char>(ui->component_type[0]));
        bool is_custom = is_class || is_fn || is_variable_call;
        // Widgets with NO first positional argument at all (attrs only — no label, no children)
        bool is_no_first_param = (!is_custom && ui->component_type == "Slider");
        
        if (is_custom) {
            res = ui->component_type + "(";
        } else {
            res = "zenith::UI::" + ui->component_type + "(";
        }
        
        // Children / Positional arguments
        if (!is_custom) {
            bool is_string_param = (
                ui->component_type == "Text" ||
                ui->component_type == "Button" ||
                ui->component_type == "TextField" ||
                ui->component_type == "Image" ||
                ui->component_type == "Video" ||
                ui->component_type == "Checkbox" ||
                ui->component_type == "Toggle" ||
                ui->component_type == "Dropdown"
            );
            if (is_no_first_param) {
                // No leading arg — attrs follow directly with no preceding comma
            } else if (is_string_param) {
                if (!ui->children.empty()) {
                    res += generateExpression(ui->children[0].get());
                } else {
                    res += "\"\"";
                }
            } else {
                if (!ui->children.empty()) {
                    res += "zenith::make_children(";
                    for (size_t i = 0; i < ui->children.size(); ++i) {
                        res += generateExpression(ui->children[i].get());
                        if (i < ui->children.size() - 1) res += ", ";
                    }
                    res += ")";
                } else {
                    res += "zenith::make_children()";
                }
            }
        } else {
            if (!ui->children.empty()) {
                for (size_t i = 0; i < ui->children.size(); ++i) {
                    res += generateExpression(ui->children[i].get());
                    if (i < ui->children.size() - 1) res += ", ";
                }
            }
        }
        
        // Named arguments (attributes) - only for built-in UI components
        if (!is_custom) {
            // is_no_first_param widgets have no leading positional arg, so no leading comma
            res += (is_no_first_param ? "{" : ", {");
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
        if (is_generating_async_function) {
            if (return_stmt->expression) {
                output << current_async_promise_name << "->set_value(" << generateExpression(return_stmt->expression.get()) << ");\n";
            } else {
                output << current_async_promise_name << "->set_value();\n";
            }
            indent();
            output << "return;\n";
        } else {
            output << "return " << generateExpression(return_stmt->expression.get()) << ";\n";
        }
    } else if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt)) {
        indent();
        std::string type_name = var_decl->type->type_name;
        
        // Handle type inference - use 'auto' for inferred types
        bool use_auto = var_decl->type->is_inferred && var_decl->initializer != nullptr;
        
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
            if (use_auto) {
                // Use C++ auto keyword for type inference
                output << "auto " << var_decl->var_name;
                if (var_decl->initializer) {
                    output << " = " << generateExpression(var_decl->initializer.get());
                }
                output << ";\n";
            } else if (var_decl->type && (var_decl->type->type_name == "Ref" || var_decl->type->type_name == "Weak")
                       && !var_decl->type->generics.empty()) {
                // RC/GC smart pointer variable declaration
                std::string inner_type = mapType(var_decl->type->generics[0].get());
                std::string cpp_type   = mapType(var_decl->type.get());
                output << cpp_type << " " << var_decl->var_name;
                if (var_decl->initializer) {
                    if (var_decl->type->type_name == "Ref") {
                        // Ref<T> varname = Ref<T>(args...) → make_ref<T>(args...)
                        auto* fcall = dynamic_cast<FunctionCallNode*>(var_decl->initializer.get());
                        if (fcall) {
                            output << " = zenith::mem::make_ref<" << inner_type << ">(";
                            for (size_t i = 0; i < fcall->arguments.size(); ++i) {
                                output << generateExpression(fcall->arguments[i].get());
                                if (i < fcall->arguments.size() - 1) output << ", ";
                            }
                            output << ")";
                        } else {
                            // Ref from another Ref (copy)
                            output << " = " << generateExpression(var_decl->initializer.get());
                        }
                    } else {
                        // Weak<T> varname = someRef → zenith::mem::Weak<T>(someRef)
                        output << " = zenith::mem::Weak<" << inner_type << ">(";
                        output << generateExpression(var_decl->initializer.get());
                        output << ")";
                    }
                }
                output << ";\n";
            } else {
                output << mapType(var_decl->type.get()) << " " << var_decl->var_name;
                if (var_decl->initializer) {
                    output << " = " << generateExpression(var_decl->initializer.get());
                }
                output << ";\n";
            }
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

    std::string image_path_var = "";
    if (node->is_multimodal) {
        for (const auto& param : node->parameters) {
            std::string name = param->var_name;
            if (name.find("image") != std::string::npos || name.find("img") != std::string::npos || name.find("file") != std::string::npos) {
                image_path_var = name;
                break;
            }
        }
        if (image_path_var.empty() && !node->parameters.empty()) {
            image_path_var = node->parameters.back()->var_name;
        }
    }

    indent(); output << "zenith::LLMClient client(\"http://localhost:11434/api/generate\");\n";
    if (node->is_streaming) {
        indent(); output << "std::string response = client.promptStream(prompt, [](const std::string& chunk) { std::cout << chunk << std::flush; }";
        if (!image_path_var.empty()) {
            output << ", " << image_path_var;
        }
        output << ");\n";
    } else {
        indent(); output << "std::string response = client.prompt(prompt";
        if (!image_path_var.empty()) {
            output << ", " << image_path_var;
        }
        output << ");\n";
    }
    indent(); output << "return response;\n";
    
    indent_level--;
    indent(); output << "}\n\n";
}

void CodeGenerator::generateOrchestration(AgentOrchestrationNode* node) {
    indent();
    if (node->strategy == "sequential") {
        output << "std::string " << node->orchestration_name << "(std::string input) {\n";
        indent_level++;
        indent(); output << "std::string current_val = input;\n";
        for (const auto& agent : node->agents) {
            indent(); output << "current_val = " << agent << "(current_val);\n";
        }
        indent(); output << "return current_val;\n";
    } else {
        output << "std::vector<std::string> " << node->orchestration_name << "(std::string input) {\n";
        indent_level++;
        for (size_t i = 0; i < node->agents.size(); ++i) {
            indent(); output << "auto f" << i << " = std::async(std::launch::async, " << node->agents[i] << ", input);\n";
        }
        indent(); output << "std::vector<std::string> results;\n";
        for (size_t i = 0; i < node->agents.size(); ++i) {
            indent(); output << "results.push_back(f" << i << ".get());\n";
        }
        indent(); output << "return results;\n";
    }
    indent_level--;
    indent(); output << "}\n\n";
}

void CodeGenerator::generateFunction(FunctionNode* node) {
    if (node->is_foreign) {
        if (node->foreign_abi == "C") {
            indent();
            output << "extern \"C\" " << mapTypeForCFFI(node->return_type.get(), true) << " " << node->function_name << "(";
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << mapTypeForCFFI(node->parameters[i]->type.get(), false) << " " << node->parameters[i]->var_name;
                if (i < node->parameters.size() - 1) output << ", ";
            }
            output << ");\n\n";
            return;
        }
        else if (node->foreign_abi == "python") {
            // Use embedded Python runtime via PythonFFIBridge
            indent();
            output << mapTypeForCFFI(node->return_type.get(), true) << " " << node->function_name << "(";
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << mapTypeForCFFI(node->parameters[i]->type.get(), false) << " " << node->parameters[i]->var_name;
                if (i < node->parameters.size() - 1) output << ", ";
            }
            output << ") {\n";
            indent_level++;

            // Generate typed call based on return type
            std::string ret_type = mapType(node->return_type.get());
            std::string module_name = "bridge";  // Default module name

            // Check for custom module in attributes
            if (node->attributes.count("python_module")) {
                module_name = node->attributes.at("python_module");
            }

            if (ret_type == "int" || ret_type == "Int") {
                indent(); output << "return zenith::ffi::PythonFFIBridge::callInt(\"" << module_name << "\", \"" << node->function_name << "\"";
            } else if (ret_type == "double" || ret_type == "float" || ret_type == "Float") {
                indent(); output << "return zenith::ffi::PythonFFIBridge::callDouble(\"" << module_name << "\", \"" << node->function_name << "\"";
            } else if (ret_type == "bool" || ret_type == "Bool") {
                indent(); output << "return zenith::ffi::PythonFFIBridge::callBool(\"" << module_name << "\", \"" << node->function_name << "\"";
            } else if (ret_type == "std::string" || ret_type == "String") {
                indent(); output << "return zenith::ffi::PythonFFIBridge::callString(\"" << module_name << "\", \"" << node->function_name << "\"";
            } else {
                indent(); output << "zenith::ffi::PythonFFIBridge::callFunction(\"" << module_name << "\", \"" << node->function_name << "\"";
            }

            // Pass arguments
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << ", " << node->parameters[i]->var_name;
            }
            output << ");\n";

            indent_level--;
            indent(); output << "}\n\n";
            return;
        }
        else { // "js"
            indent();
            output << mapType(node->return_type.get()) << " " << node->function_name << "(";
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << mapType(node->parameters[i]->type.get()) << " " << node->parameters[i]->var_name;
                if (i < node->parameters.size() - 1) output << ", ";
            }
            output << ") {\n";
            indent_level++;
            indent(); output << "std::cerr << \"[Warning] JS interop function '" << node->function_name << "' is not supported on desktop platform.\" << std::endl;\n";
            std::string ret_type = mapType(node->return_type.get());
            if (ret_type == "int" || ret_type == "Int") {
                indent(); output << "return 0;\n";
            } else if (ret_type == "double" || ret_type == "float" || ret_type == "Float") {
                indent(); output << "return 0.0;\n";
            } else if (ret_type == "bool" || ret_type == "Bool") {
                indent(); output << "return false;\n";
            } else if (ret_type == "std::string" || ret_type == "String") {
                indent(); output << "return \"\";\n";
            } else {
                indent(); output << "return;\n";
            }
            indent_level--;
            indent(); output << "}\n\n";
            return;
        }
    }

    indent();
    if (node->is_exported) {
        output << "#ifdef _WIN32\n";
        output << "extern \"C\" __declspec(dllexport)\n";
        output << "#else\n";
        output << "extern \"C\"\n";
        output << "#endif\n";
    }
    if (node->function_name == "main") {
        output << "int main(";
    } else {
        if (node->is_async) {
            output << "zenith::stdlib::Future<" << mapType(node->return_type.get()) << "> " << node->function_name << "(";
        } else {
            output << mapType(node->return_type.get()) << " " << node->function_name << "(";
        }
    }
    
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        auto& param = node->parameters[i];
        
        // Support type inference for parameters with default values
        // For C++17 compatibility, we infer the actual type from the default value instead of using 'auto'
        bool use_inferred = param->type->is_inferred && param->initializer != nullptr;
        
        if (use_inferred) {
            // Infer type from the initializer expression
            std::string inferred_type = "std::string"; // Default for string literals
            
            if (auto* num_lit = dynamic_cast<NumberLiteralNode*>(param->initializer.get())) {
                inferred_type = num_lit->value.find('.') != std::string::npos ? "double" : "int";
            } else if (auto* bool_lit = dynamic_cast<BoolLiteralNode*>(param->initializer.get())) {
                inferred_type = "bool";
            } else if (auto* str_lit = dynamic_cast<StringLiteralNode*>(param->initializer.get())) {
                inferred_type = "std::string";
            }
            
            output << inferred_type << " " << param->var_name;
            if (param->initializer) {
                output << " = " << generateExpression(param->initializer.get());
            }
        } else {
            output << mapType(param->type.get()) << " " << param->var_name;
            if (param->initializer) {
                output << " = " << generateExpression(param->initializer.get());
            }
        }
        
        if (i < node->parameters.size() - 1) output << ", ";
    }
    output << ") {\n";
    
    indent_level++;

    // For main(): inject GC lifecycle management at top
    bool is_main = (node->function_name == "main");
    if (is_main) {
        indent(); output << "// --- Zenith RC+GC Memory Manager: Start background cycle collector ---\n";
        indent(); output << "zenith::mem::GcHeap::instance().start_background_gc(5000);\n\n";
    }
    
    if (node->is_async) {
        std::string cpp_ret_type = mapType(node->return_type.get());
        indent();
        output << "auto _promise = std::make_shared<zenith::stdlib::Promise<" << cpp_ret_type << ">>();\n";
        indent();
        output << "std::thread([_promise";
        if (is_inside_class_method) {
            output << ", this";
        }
        for (const auto& param : node->parameters) {
            output << ", " << param->var_name;
        }
        output << "]() mutable {\n";
        indent_level++;
        indent();
        output << "try {\n";
        indent_level++;
        
        is_generating_async_function = true;
        current_async_promise_name = "_promise";
    }
    
    for (const auto& stmt : node->body) {
        generateStatement(stmt.get());
    }
    
    if (node->is_async) {
        is_generating_async_function = false;
        
        if (mapType(node->return_type.get()) == "void") {
            indent();
            output << "_promise->set_value();\n";
        }
        
        indent_level--;
        indent(); output << "} catch (...) {\n";
        indent_level++;
        indent(); output << "_promise->set_exception(std::current_exception());\n";
        indent_level--;
        indent(); output << "}\n";
        
        indent_level--;
        indent(); output << "}).detach();\n";
        indent(); output << "return _promise->get_future();\n";
    }
    
    indent_level--;
    
    // For main(): inject GC epilogue — stop background thread, do final sweep, print stats
    if (is_main) {
        indent(); output << "\n";
        indent(); output << "// --- Zenith RC+GC Memory Manager: Shutdown ---\n";
        indent(); output << "zenith::mem::GcHeap::instance().stop_background_gc();\n";
        indent(); output << "zenith::mem::GcHeap::instance().collect(); // Final cycle sweep\n";
        output << "#ifdef ZENITH_GC_STATS\n";
        indent(); output << "std::cout << zenith::mem::gcStatsString() << std::endl;\n";
        output << "#endif\n";
    }

    indent(); output << "}\n\n";
}

void CodeGenerator::generateClass(ClassDeclNode* node) {
    indent();
    output << "class " << node->class_name;

    // Inheritance: @managed → zenith::mem::Managed; interfaces follow
    bool has_base = node->is_managed || !node->implemented_interfaces.empty();
    if (has_base) {
        output << " : ";
        bool first = true;
        if (node->is_managed) {
            output << "public zenith::mem::Managed";
            first = false;
        }
        for (size_t i = 0; i < node->implemented_interfaces.size(); ++i) {
            if (!first) output << ", ";
            output << "public " << node->implemented_interfaces[i];
            first = false;
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
    output << " {}\n";

    // @managed: generate GC child enumeration override
    if (node->is_managed) {
        output << "\n";
        indent(); output << "void __gc_enumerate(std::vector<zenith::mem::RcBlock*>& out) override {\n";
        indent_level++;
        for (const auto& field : node->fields) {
            std::string ft = field->type ? field->type->type_name : "";
            if (ft == "Ref" || ft == "Weak") {
                // Access the block via the smart pointer's internal block
                indent(); output << "// GC trace field: " << field->var_name << "\n";
            }
        }
        indent_level--;
        indent(); output << "}\n";
    }
    output << "\n";

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
    indent(); output << "void triggerCallback(std::string name, std::string val = \"\") {\n";
    indent_level++;
    for (const auto& method : node->methods) {
        if (method->function_name != "build") {
            if (method->parameters.empty()) {
                indent();
                output << "if (name == \"" << method->function_name << "\") { this->" << method->function_name << "(); return; }\n";
            } else if (method->parameters.size() == 1) {
                std::string param_type = mapType(method->parameters[0]->type.get());
                indent();
                if (param_type == "std::string") {
                    output << "if (name == \"" << method->function_name << "\") { this->" << method->function_name << "(val); return; }\n";
                } else if (param_type == "bool") {
                    output << "if (name == \"" << method->function_name << "\") { this->" << method->function_name << "(val == \"true\"); return; }\n";
                } else if (param_type == "int") {
                    output << "if (name == \"" << method->function_name << "\") { try { this->" << method->function_name << "(std::stoi(val)); } catch(...) {} return; }\n";
                } else if (param_type == "float") {
                    output << "if (name == \"" << method->function_name << "\") { try { this->" << method->function_name << "(std::stof(val)); } catch(...) {} return; }\n";
                }
            }
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
        output << "virtual ";
        if (method->is_async) {
            output << "zenith::stdlib::Future<" << mapType(method->return_type.get()) << "> ";
        } else {
            output << mapType(method->return_type.get()) << " ";
        }
        output << method->function_name << "(";
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
        } else if (auto* orch_decl = dynamic_cast<AgentOrchestrationNode*>(stmt.get())) {
            function_names.insert(orch_decl->orchestration_name);
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
    output << "#include <future>\n";
    output << "#include <iostream>\n";
    output << "#include <functional>\n";
    output << "#include \"zenith_runtime.h\"\n";
    output << "#include \"zenith/std/concurrency.hpp\"\n\n";

    // Emit native target-scoped imports for C++ target
    for (const auto& stmt : program->statements) {
        if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            if (imp->isActiveFor("cpp") && imp->kind == ImportNode::ImportKind::Native) {
                std::string url = imp->cdn_url;
                bool is_binary_lib = false;
                if (url.length() >= 3) {
                    std::string ext3 = url.substr(url.length() - 3);
                    if (ext3 == ".so" || ext3 == ".a") {
                        is_binary_lib = true;
                    }
                }
                if (url.length() >= 4) {
                    std::string ext4 = url.substr(url.length() - 4);
                    if (ext4 == ".dll" || ext4 == ".lib") {
                        is_binary_lib = true;
                    }
                }
                if (url.length() >= 6) {
                    std::string ext6 = url.substr(url.length() - 6);
                    if (ext6 == ".dylib") {
                        is_binary_lib = true;
                    }
                }
                if (!is_binary_lib) {
                    output << "#include \"" << url << "\"\n";
                }
            }
        }
    }
    output << "\n";

    // Target Platform Detection Constants
    output << "#ifdef __ANDROID__\n";
    output << "const bool isAndroid = true;\n";
    output << "#else\n";
    output << "const bool isAndroid = false;\n";
    output << "#endif\n\n";

    output << "#ifdef __APPLE__\n";
    output << "  #include <TargetConditionals.h>\n";
    output << "  #if TARGET_OS_IPHONE\n";
    output << "    const bool isIos = true;\n";
    output << "    const bool isMac = false;\n";
    output << "  #else\n";
    output << "    const bool isIos = false;\n";
    output << "    const bool isMac = true;\n";
    output << "  #endif\n";
    output << "#else\n";
    output << "  const bool isIos = false;\n";
    output << "  const bool isMac = false;\n";
    output << "#endif\n\n";

    output << "#ifdef __linux__\n";
    output << "  #ifndef __ANDROID__\n";
    output << "    const bool isLinux = true;\n";
    output << "  #else\n";
    output << "    const bool isLinux = false;\n";
    output << "  #endif\n";
    output << "#else\n";
    output << "  const bool isLinux = false;\n";
    output << "#endif\n\n";

    output << "#ifdef _WIN32\n";
    output << "const bool isWindows = true;\n";
    output << "#else\n";
    output << "const bool isWindows = false;\n";
    output << "#endif\n\n";

    output << "const bool isWeb = false;\n\n";

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
        output << "inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }\n";
        output << "inline std::string gcStats() { return zenith::mem::gcStatsString(); }\n\n";
        // Register gcStats as a known function so it's not treated as a UI component
        function_names.insert("gcStats");
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
        } else if (auto* orch_decl = dynamic_cast<AgentOrchestrationNode*>(stmt.get())) {
            generateOrchestration(orch_decl);
        }
    }
    
    std::cout << "   [OK] Code Generation Complete.\n";
    return output.str();
}
