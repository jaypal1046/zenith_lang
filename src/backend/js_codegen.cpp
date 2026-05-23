#include "../../include/backend/js_codegen.h"
#include <iostream>
#include <regex>

void JSCodeGenerator::indent() {
    for (int i = 0; i < indent_level * 4; ++i) {
        output << " ";
    }
}

bool JSCodeGenerator::containsAsyncCall(ASTNode* node, const std::unordered_set<std::string>& async_fns) {
    if (!node) return false;
    
    if (auto* call = dynamic_cast<MethodCallNode*>(node)) {
        if (async_fns.count(call->method_name)) return true;
        for (const auto& arg : call->arguments) {
            if (containsAsyncCall(arg.get(), async_fns)) return true;
        }
    }
    if (auto* ui = dynamic_cast<UIComponentNode*>(node)) {
        if (async_fns.count(ui->component_type)) return true;
        for (const auto& arg : ui->named_args) {
            if (containsAsyncCall(arg.second.get(), async_fns)) return true;
        }
        for (const auto& child : ui->children) {
            if (containsAsyncCall(child.get(), async_fns)) return true;
        }
    }
    if (auto* var_decl = dynamic_cast<VarDeclNode*>(node)) {
        if (containsAsyncCall(var_decl->initializer.get(), async_fns)) return true;
    }
    if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(node)) {
        if (containsAsyncCall(return_stmt->expression.get(), async_fns)) return true;
    }
    if (auto* if_stmt = dynamic_cast<IfStmtNode*>(node)) {
        if (containsAsyncCall(if_stmt->condition.get(), async_fns)) return true;
        for (const auto& s : if_stmt->then_branch) {
            if (containsAsyncCall(s.get(), async_fns)) return true;
        }
        for (const auto& s : if_stmt->else_branch) {
            if (containsAsyncCall(s.get(), async_fns)) return true;
        }
    }
    if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(node)) {
        if (containsAsyncCall(while_stmt->condition.get(), async_fns)) return true;
        for (const auto& s : while_stmt->body) {
            if (containsAsyncCall(s.get(), async_fns)) return true;
        }
    }
    if (auto* set_state = dynamic_cast<SetStateStmtNode*>(node)) {
        for (const auto& s : set_state->body) {
            if (containsAsyncCall(s.get(), async_fns)) return true;
        }
    }
    if (auto* binary = dynamic_cast<BinaryExprNode*>(node)) {
        return containsAsyncCall(binary->left.get(), async_fns) || containsAsyncCall(binary->right.get(), async_fns);
    }
    return false;
}

std::string JSCodeGenerator::generateExpression(ExprNode* expr) {
    if (!expr) return "";
    
    if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        if (id->name == "print") return "zenith.print";
        if (id->name == "println") return "zenith.println";
        if (is_inside_class_method && current_class_fields.count(id->name)) {
            return "this." + id->name;
        }
        if (is_inside_class_method && current_class_methods.count(id->name)) {
            return "this." + id->name + ".bind(this)";
        }
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
        return generateExpression(binary->left.get()) + " " + binary->op + " " + generateExpression(binary->right.get());
    }
    if (auto* list_lit = dynamic_cast<ListLiteralNode*>(expr)) {
        std::string res = "[";
        for (size_t i = 0; i < list_lit->elements.size(); ++i) {
            res += generateExpression(list_lit->elements[i].get());
            if (i < list_lit->elements.size() - 1) res += ", ";
        }
        res += "]";
        return res;
    }
    if (auto* map_lit = dynamic_cast<MapLiteralNode*>(expr)) {
        std::string res = "{";
        for (size_t i = 0; i < map_lit->entries.size(); ++i) {
            res += generateExpression(map_lit->entries[i].first.get()) + ": " + generateExpression(map_lit->entries[i].second.get());
            if (i < map_lit->entries.size() - 1) res += ", ";
        }
        res += "}";
        return res;
    }
    if (auto* prop = dynamic_cast<PropertyAccessNode*>(expr)) {
        return generateExpression(prop->object.get()) + "." + prop->property_name;
    }
    if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        bool is_async_call = async_functions.count(call->method_name) > 0;
        std::string res = (is_async_call ? "await " : "") + generateExpression(call->object.get()) + "." + call->method_name + "(";
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
        bool is_method = is_inside_class_method && current_class_methods.count(ui->component_type);
        bool is_custom = is_class || is_fn || is_method;
        
        if (is_custom) {
            if (is_class) {
                res = "new " + ui->component_type + "(";
            } else if (is_method) {
                bool is_async_fn = async_functions.count(ui->component_type) > 0;
                res = (is_async_fn ? "await this." : "this.") + ui->component_type + "(";
            } else {
                bool is_async_fn = async_functions.count(ui->component_type) > 0;
                res = (is_async_fn ? "await " : "") + ui->component_type + "(";
            }
        } else {
            res = "UI." + ui->component_type + "(";
        }
        
        // Children
        if (!is_custom) res += "[";
        for (size_t i = 0; i < ui->children.size(); ++i) {
            res += generateExpression(ui->children[i].get());
            if (i < ui->children.size() - 1) res += ", ";
        }
        if (!is_custom) res += "]";
        
        // Named arguments
        if (!is_custom) {
            res += ", {";
            for (size_t i = 0; i < ui->named_args.size(); ++i) {
                res += ui->named_args[i].first + ": " + generateExpression(ui->named_args[i].second.get());
                if (i < ui->named_args.size() - 1) res += ", ";
            }
            res += "}";
        }
        res += ")";
        return res;
    }
    return "";
}

void JSCodeGenerator::generateStatement(ASTNode* stmt) {
    if (!stmt) return;
    
    if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(stmt)) {
        indent();
        output << "return " << generateExpression(return_stmt->expression.get()) << ";\n";
    } else if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt)) {
        indent();
        output << "let " << var_decl->var_name;
        if (var_decl->initializer) {
            output << " = " << generateExpression(var_decl->initializer.get());
        }
        output << ";\n";
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
        output << "this.setState(() => {\n";
        indent_level++;
        for (const auto& s : set_state->body) {
            generateStatement(s.get());
        }
        indent_level--;
        indent();
        output << "});\n";
    } else if (auto* expr = dynamic_cast<ExprNode*>(stmt)) {
        indent();
        bool is_async_call = false;
        if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
            if (agentic_functions.count(call->method_name)) is_async_call = true;
        }
        output << (is_async_call ? "await " : "") << generateExpression(expr) << ";\n";
    }
}

void JSCodeGenerator::generateClass(ClassDeclNode* node) {
    current_class_fields.clear();
    for (const auto& param : node->primary_constructor_args) {
        current_class_fields.insert(param->var_name);
    }
    for (const auto& member : node->fields) {
        current_class_fields.insert(member->var_name);
    }
    current_class_methods.clear();
    for (const auto& method : node->methods) {
        current_class_methods.insert(method->function_name);
    }

    bool has_build = false;
    for (const auto& method : node->methods) {
        if (method->function_name == "build") has_build = true;
    }

    output << "class " << node->class_name << " {\n";
    indent_level++;
    
    // Constructor
    indent();
    output << "constructor(";
    for (size_t i = 0; i < node->primary_constructor_args.size(); ++i) {
        output << node->primary_constructor_args[i]->var_name;
        if (i < node->primary_constructor_args.size() - 1) output << ", ";
    }
    output << ") {\n";
    indent_level++;
    
    // Member variables inside constructor
    for (const auto& param : node->primary_constructor_args) {
        indent();
        output << "this." << param->var_name << " = " << param->var_name << ";\n";
    }
    for (const auto& member : node->fields) {
        indent();
        output << "this." << member->var_name << " = " 
               << (member->initializer ? generateExpression(member->initializer.get()) : "null") << ";\n";
    }
    if (has_build) {
        indent();
        output << "counterGlobalInstance = this;\n";
    }
    
    indent_level--;
    indent(); output << "}\n";
    
    // Methods
    is_inside_class_method = true;
    for (const auto& method : node->methods) {
        indent();
        auto* agentic = dynamic_cast<AgenticFunctionNode*>(method.get());
        if (agentic) {
            output << "async " << method->function_name << "(";
            for (size_t i = 0; i < method->parameters.size(); ++i) {
                output << method->parameters[i]->var_name;
                if (i < method->parameters.size() - 1) output << ", ";
            }
            output << ") {\n";
            indent_level++;
            
            std::string raw_prompt = agentic->prompt_template;
            std::string js_prompt = "`";
            size_t pos = 0;
            while (pos < raw_prompt.length()) {
                if (raw_prompt[pos] == '{') {
                    size_t end = raw_prompt.find('}', pos);
                    if (end != std::string::npos) {
                        std::string var_name = raw_prompt.substr(pos + 1, end - pos - 1);
                        bool is_param = false;
                        for (const auto& p : agentic->parameters) {
                            if (p->var_name == var_name) is_param = true;
                        }
                        if (is_param) {
                            js_prompt += "${" + var_name + "}";
                        } else {
                            js_prompt += "${this." + var_name + "}";
                        }
                        pos = end + 1;
                    } else {
                        js_prompt += "{";
                        pos++;
                    }
                } else {
                    js_prompt += raw_prompt[pos];
                    pos++;
                }
            }
            js_prompt += "`";
            
            std::string image_path_var = "null";
            if (agentic->is_multimodal) {
                for (const auto& param : agentic->parameters) {
                    std::string name = param->var_name;
                    if (name.find("image") != std::string::npos || name.find("img") != std::string::npos || name.find("file") != std::string::npos) {
                        image_path_var = name;
                        break;
                    }
                }
                if (image_path_var == "null" && !agentic->parameters.empty()) {
                    image_path_var = agentic->parameters.back()->var_name;
                }
            }
            
            indent();
            output << "return await zenith.llmPrompt(this.url || \"http://localhost:11434\", " << js_prompt << ", " 
                   << (agentic->is_streaming ? "true" : "false") << ", " << image_path_var << ");\n";
            
            indent_level--;
            indent(); output << "}\n";
        } else {
            if (async_functions.count(method->function_name)) {
                output << "async ";
            }
            output << method->function_name << "(";
            for (size_t i = 0; i < method->parameters.size(); ++i) {
                output << method->parameters[i]->var_name;
                if (i < method->parameters.size() - 1) output << ", ";
            }
            output << ") {\n";
            indent_level++;
            for (const auto& stmt : method->body) {
                generateStatement(stmt.get());
            }
            indent_level--;
            indent(); output << "}\n";
        }
    }
    
    // Special render method if it has build()
    has_build = false;
    for (const auto& method : node->methods) {
        if (method->function_name == "build") has_build = true;
    }
    if (has_build) {
        indent();
        output << "render() {\n";
        indent_level++;
        indent();
        output << "const newDom = this.build();\n";
        indent();
        output << "if (this.domElement && this.domElement.parentNode) {\n";
        indent_level++;
        indent();
        output << "this.domElement.parentNode.replaceChild(newDom, this.domElement);\n";
        indent_level--;
        indent();
        output << "} else {\n";
        indent_level++;
        indent();
        output << "const root = document.getElementById('zenith-ui-root');\n";
        indent();
        output << "root.innerHTML = '';\n";
        indent();
        output << "root.appendChild(newDom);\n";
        indent_level--;
        indent();
        output << "}\n";
        indent();
        output << "this.domElement = newDom;\n";
        indent();
        output << "return newDom;\n";
        indent_level--;
        indent();
        output << "}\n";

        // Generate setState helper
        indent();
        output << "setState(callback) {\n";
        indent_level++;
        indent();
        output << "callback();\n";
        indent();
        output << "this.render();\n";
        indent_level--;
        indent();
        output << "}\n";
    }
    
    is_inside_class_method = false;
    indent_level--;
    output << "}\n";
}

void JSCodeGenerator::generateInterface(InterfaceDeclNode* node) {
    // Interfaces are parsed and verified at compile-time by Semantic Analyzer.
    // They are no-ops in dynamic JS.
}

void JSCodeGenerator::generateFunction(FunctionNode* node) {
    indent();
    bool is_async = async_functions.count(node->function_name) > 0;
    if (node->function_name == "main") {
        output << "async function main() {\n";
    } else {
        output << (is_async ? "async " : "") << "function " << node->function_name << "(";
        for (size_t i = 0; i < node->parameters.size(); ++i) {
            output << node->parameters[i]->var_name;
            if (i < node->parameters.size() - 1) output << ", ";
        }
        output << ") {\n";
    }
    indent_level++;
    for (const auto& stmt : node->body) {
        generateStatement(stmt.get());
    }
    indent_level--;
    indent(); output << "}\n";
}

void JSCodeGenerator::generateAgenticFunction(AgenticFunctionNode* node) {
    indent();
    output << "async function " << node->function_name << "(";
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        output << node->parameters[i]->var_name;
        if (i < node->parameters.size() - 1) output << ", ";
    }
    output << ") {\n";
    indent_level++;
    
    std::string raw_prompt = node->prompt_template;
    std::string js_prompt = "`";
    size_t pos = 0;
    while (pos < raw_prompt.length()) {
        if (raw_prompt[pos] == '{') {
            size_t end = raw_prompt.find('}', pos);
            if (end != std::string::npos) {
                std::string var_name = raw_prompt.substr(pos + 1, end - pos - 1);
                bool is_param = false;
                for (const auto& p : node->parameters) {
                    if (p->var_name == var_name) is_param = true;
                }
                if (is_param) {
                    js_prompt += "${" + var_name + "}";
                } else {
                    js_prompt += "${this." + var_name + "}";
                }
                pos = end + 1;
            } else {
                js_prompt += "{";
                pos++;
            }
        } else {
            js_prompt += raw_prompt[pos];
            pos++;
        }
    }
    js_prompt += "`";
    
    std::string image_path_var = "null";
    if (node->is_multimodal) {
        for (const auto& param : node->parameters) {
            std::string name = param->var_name;
            if (name.find("image") != std::string::npos || name.find("img") != std::string::npos || name.find("file") != std::string::npos) {
                image_path_var = name;
                break;
            }
        }
        if (image_path_var == "null" && !node->parameters.empty()) {
            image_path_var = node->parameters.back()->var_name;
        }
    }
    
    indent();
    output << "return await zenith.llmPrompt(typeof this !== 'undefined' && this && this.url ? this.url : \"http://localhost:11434\", " 
           << js_prompt << ", " 
           << (node->is_streaming ? "true" : "false") << ", " 
           << image_path_var << ");\n";
    
    indent_level--;
    indent(); output << "}\n";
}

void JSCodeGenerator::generateOrchestration(AgentOrchestrationNode* node) {
    indent();
    output << "async function " << node->orchestration_name << "(input) {\n";
    indent_level++;
    
    if (node->strategy == "sequential") {
        indent(); output << "let current_val = input;\n";
        for (const auto& agent : node->agents) {
            indent(); output << "current_val = await " << agent << "(current_val);\n";
        }
        indent(); output << "return current_val;\n";
    } else {
        indent(); output << "return await Promise.all([\n";
        indent_level++;
        for (size_t i = 0; i < node->agents.size(); ++i) {
            indent(); output << node->agents[i] << "(input)";
            if (i < node->agents.size() - 1) output << ",";
            output << "\n";
        }
        indent_level--;
        indent(); output << "]);\n";
    }
    
    indent_level--;
    indent(); output << "}\n";
}

std::string JSCodeGenerator::generate(ProgramNode* program) {
    output.str("");
    output.clear();
    
    // Register all globals
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            class_names.insert(class_decl->class_name);
            for (const auto& method : class_decl->methods) {
                if (dynamic_cast<AgenticFunctionNode*>(method.get())) {
                    agentic_functions.insert(method->function_name);
                }
            }
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            interface_names.insert(interface_decl->interface_name);
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            function_names.insert(fn_decl->function_name);
            if (dynamic_cast<AgenticFunctionNode*>(fn_decl)) {
                agentic_functions.insert(fn_decl->function_name);
            }
        } else if (auto* orch_decl = dynamic_cast<AgentOrchestrationNode*>(stmt.get())) {
            function_names.insert(orch_decl->orchestration_name);
            async_functions.insert(orch_decl->orchestration_name);
        }
    }

    // Initialize async_functions with agentic_functions
    async_functions = agentic_functions;
    function_names.insert("httpGet");
    function_names.insert("httpPost");
    async_functions.insert("httpGet");
    async_functions.insert("httpPost");
    
    // Fixed-point async propagation analysis
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& stmt : program->statements) {
            if (auto* fn = dynamic_cast<FunctionNode*>(stmt.get())) {
                if (async_functions.count(fn->function_name) == 0) {
                    bool is_async = false;
                    for (const auto& s : fn->body) {
                        if (containsAsyncCall(s.get(), async_functions)) {
                            is_async = true;
                            break;
                        }
                    }
                    if (is_async) {
                        async_functions.insert(fn->function_name);
                        changed = true;
                    }
                }
            } else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
                for (const auto& method : class_decl->methods) {
                    if (async_functions.count(method->function_name) == 0) {
                        bool is_async = false;
                        for (const auto& s : method->body) {
                            if (containsAsyncCall(s.get(), async_functions)) {
                                is_async = true;
                                break;
                            }
                        }
                        if (is_async) {
                            async_functions.insert(method->function_name);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    
    // HTML Wrapper Shell
    output << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
    output << "    <meta charset=\"UTF-8\">\n    <title>Zenith Web Application</title>\n";
    output << "    <link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600;700&family=Fira+Code:wght@400;500&display=swap\" rel=\"stylesheet\">\n";
    output << "    <style>\n";
    output << "        body {\n";
    output << "            background: radial-gradient(circle at top right, #1a1a2e, #0a0a0f);\n";
    output << "            color: #e2e8f0;\n";
    output << "            font-family: 'Inter', sans-serif;\n";
    output << "            margin: 0;\n";
    output << "            padding: 24px;\n";
    output << "            display: flex;\n";
    output << "            flex-direction: column;\n";
    output << "            align-items: center;\n";
    output << "            min-height: 100vh;\n";
    output << "        }\n";
    output << "        .app-header {\n";
    output << "            margin-bottom: 24px;\n";
    output << "            text-align: center;\n";
    output << "        }\n";
    output << "        .app-header h1 {\n";
    output << "            margin: 0;\n";
    output << "            font-size: 2.5rem;\n";
    output << "            background: linear-gradient(135deg, #00f2fe, #4facfe);\n";
    output << "            -webkit-background-clip: text;\n";
    output << "            -webkit-text-fill-color: transparent;\n";
    output << "            font-weight: 700;\n";
    output << "        }\n";
    output << "        .app-header p {\n";
    output << "            color: #94a3b8;\n";
    output << "            margin-top: 8px;\n";
    output << "        }\n";
    output << "        .main-container {\n";
    output << "            display: flex;\n";
    output << "            width: 100%;\n";
    output << "            max-width: 1200px;\n";
    output << "            gap: 24px;\n";
    output << "            flex-wrap: wrap;\n";
    output << "        }\n";
    output << "        .panel {\n";
    output << "            flex: 1;\n";
    output << "            min-width: 300px;\n";
    output << "            background: rgba(30, 41, 59, 0.4);\n";
    output << "            backdrop-filter: blur(16px);\n";
    output << "            border: 1px solid rgba(255, 255, 255, 0.1);\n";
    output << "            border-radius: 16px;\n";
    output << "            padding: 24px;\n";
    output << "            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.25);\n";
    output << "            display: flex;\n";
    output << "            flex-direction: column;\n";
    output << "        }\n";
    output << "        .panel-title {\n";
    output << "            font-size: 1.25rem;\n";
    output << "            font-weight: 600;\n";
    output << "            margin-bottom: 16px;\n";
    output << "            border-bottom: 1px solid rgba(255, 255, 255, 0.1);\n";
    output << "            padding-bottom: 8px;\n";
    output << "            color: #38bdf8;\n";
    output << "            display: flex;\n";
    output << "            justify-content: space-between;\n";
    output << "            align-items: center;\n";
    output << "        }\n";
    output << "        #zenith-ui-root {\n";
    output << "            display: flex;\n";
    output << "            justify-content: center;\n";
    output << "            align-items: center;\n";
    output << "            min-height: 250px;\n";
    output << "            width: 100%;\n";
    output << "        }\n";
    output << "        .zenith-column {\n";
    output << "            display: flex;\n";
    output << "            flex-direction: column;\n";
    output << "            gap: 16px;\n";
    output << "            width: 100%;\n";
    output << "            padding: 16px;\n";
    output << "            border: 1px solid rgba(0, 242, 254, 0.3);\n";
    output << "            border-radius: 12px;\n";
    output << "            background: rgba(0, 242, 254, 0.05);\n";
    output << "        }\n";
    output << "        .zenith-row {\n";
    output << "            display: flex;\n";
    output << "            flex-direction: row;\n";
    output << "            gap: 16px;\n";
    output << "            align-items: center;\n";
    output << "            width: 100%;\n";
    output << "            padding: 12px;\n";
    output << "            border: 1px solid rgba(240, 70, 250, 0.3);\n";
    output << "            border-radius: 8px;\n";
    output << "            background: rgba(240, 70, 250, 0.05);\n";
    output << "        }\n";
    output << "        .zenith-text {\n";
    output << "            font-size: 1rem;\n";
    output << "            line-height: 1.5;\n";
    output << "        }\n";
    output << "        .terminal-panel {\n";
    output << "            background: #090d16;\n";
    output << "            border: 1px solid #1e293b;\n";
    output << "        }\n";
    output << "        .terminal-body {\n";
    output << "            font-family: 'Fira Code', monospace;\n";
    output << "            font-size: 0.9rem;\n";
    output << "            color: #10b981;\n";
    output << "            white-space: pre-wrap;\n";
    output << "            overflow-y: auto;\n";
    output << "            height: 350px;\n";
    output << "            padding: 12px;\n";
    output << "            background: #020617;\n";
    output << "            border-radius: 8px;\n";
    output << "            box-shadow: inset 0 2px 8px rgba(0, 0, 0, 0.8);\n";
    output << "        }\n";
    output << "        .zenith-input {\n";
    output << "            background: rgba(15, 23, 42, 0.6);\n";
    output << "            border: 1px solid rgba(0, 242, 254, 0.3);\n";
    output << "            border-radius: 8px;\n";
    output << "            padding: 8px 12px;\n";
    output << "            color: #f8fafc;\n";
    output << "            font-family: inherit;\n";
    output << "            font-size: 0.95rem;\n";
    output << "            outline: none;\n";
    output << "            transition: all 0.3s ease;\n";
    output << "            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);\n";
    output << "        }\n";
    output << "        .zenith-input:focus {\n";
    output << "            border-color: #00f2fe;\n";
    output << "            box-shadow: 0 0 10px rgba(0, 242, 254, 0.4);\n";
    output << "            background: rgba(15, 23, 42, 0.8);\n";
    output << "        }\n";
    output << "        .interactive-btn {\n";
    output << "            background: linear-gradient(135deg, #059669, #10b981);\n";
    output << "            color: white;\n";
    output << "            border: none;\n";
    output << "            padding: 8px 16px;\n";
    output << "            border-radius: 6px;\n";
    output << "            font-weight: 600;\n";
    output << "            cursor: pointer;\n";
    output << "            transition: all 0.2s;\n";
    output << "        }\n";
    output << "        .interactive-btn:hover {\n";
    output << "            transform: translateY(-1px);\n";
    output << "            box-shadow: 0 4px 12px rgba(16, 185, 129, 0.3);\n";
    output << "        }\n";
    output << "    </style>\n</head>\n<body>\n";
    output << "    <div class=\"app-header\">\n";
    output << "        <h1>Zenith Live Web Target</h1>\n";
    output << "        <p>Statically compiled Zenith layout engine & LLM bindings running live in browser</p>\n";
    output << "    </div>\n";
    output << "    <div class=\"main-container\">\n";
    output << "        <div class=\"panel\">\n";
    output << "            <div class=\"panel-title\">\n";
    output << "                Render Canvas\n";
    output << "                <button class=\"interactive-btn\" onclick=\"triggerIncrement()\">setState Trigger</button>\n";
    output << "            </div>\n";
    output << "            <div id=\"zenith-ui-root\"></div>\n";
    output << "        </div>\n";
    output << "        <div class=\"panel terminal-panel\">\n";
    output << "            <div class=\"panel-title\" style=\"color: #10b981;\">Output Console</div>\n";
    output << "            <div class=\"terminal-body\" id=\"zenith-console\"></div>\n";
    output << "        </div>\n";
    output << "    </div>\n\n";
    
    output << "    <script>\n";
    output << "        const zenith = {\n";
    output << "            consoleElement: null,\n";
    output << "            print: function(msg) {\n";
    output << "                if (!this.consoleElement) this.consoleElement = document.getElementById('zenith-console');\n";
    output << "                this.consoleElement.textContent += msg;\n";
    output << "                this.consoleElement.scrollTop = this.consoleElement.scrollHeight;\n";
    output << "            },\n";
    output << "            println: function(msg) {\n";
    output << "                this.print(msg + '\\n');\n";
    output << "            },\n";
    output << "            encodeImageToBase64: async function(pathOrUrl) {\n";
    output << "                if (!pathOrUrl) return \"\";\n";
    output << "                if (pathOrUrl.startsWith(\"data:\")) return pathOrUrl.split(\",\")[1];\n";
    output << "                try {\n";
    output << "                    const response = await fetch(pathOrUrl);\n";
    output << "                    const blob = await response.blob();\n";
    output << "                    return new Promise((resolve) => {\n";
    output << "                        const reader = new FileReader();\n";
    output << "                        reader.onloadend = () => {\n";
    output << "                            const base64String = reader.result.split(',')[1];\n";
    output << "                            resolve(base64String);\n";
    output << "                        };\n";
    output << "                        reader.readAsDataURL(blob);\n";
    output << "                    });\n";
    output << "                } catch (e) {\n";
    output << "                    return \"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==\";\n";
    output << "                }\n";
    output << "            },\n";
    output << "            llmPrompt: async function(url, promptStr, isStreaming, imagePath) {\n";
    output << "                let imageBase64 = \"\";\n";
    output << "                if (imagePath) {\n";
    output << "                    this.println('[Runtime] Loading and encoding image: ' + imagePath);\n";
    output << "                    imageBase64 = await this.encodeImageToBase64(imagePath);\n";
    output << "                }\n";
    output << "                this.println('\\n[Runtime] LLMClient sending prompt to local backend (' + url + '): \"' + promptStr + '\"');\n";
    output << "                if (imageBase64) {\n";
    output << "                    this.println('[Runtime] Image attached (Base64 length: ' + imageBase64.length + ' chars)');\n";
    output << "                }\n";
    output << "                try {\n";
    output << "                    const response = await fetch(url + '/api/generate', {\n";
    output << "                        method: 'POST',\n";
    output << "                        headers: { 'Content-Type': 'application/json' },\n";
    output << "                        body: JSON.stringify({\n";
    output << "                            model: 'llama3',\n";
    output << "                            prompt: promptStr,\n";
    output << "                            stream: !!isStreaming,\n";
    output << "                            images: imageBase64 ? [imageBase64] : undefined\n";
    output << "                        })\n";
    output << "                    });\n";
    output << "                    if (response.ok) {\n";
    output << "                        if (isStreaming) {\n";
    output << "                            const reader = response.body.getReader();\n";
    output << "                            const decoder = new TextDecoder(\"utf-8\");\n";
    output << "                            let buffer = \"\";\n";
    output << "                            let fullResponse = \"\";\n";
    output << "                            while (true) {\n";
    output << "                                const { value, done } = await reader.read();\n";
    output << "                                if (done) break;\n";
    output << "                                buffer += decoder.decode(value, { stream: true });\n";
    output << "                                const lines = buffer.split(\"\\n\");\n";
    output << "                                buffer = lines.pop();\n";
    output << "                                for (const line of lines) {\n";
    output << "                                    if (line.trim() === \"\") continue;\n";
    output << "                                    try {\n";
    output << "                                        const json = JSON.parse(line);\n";
    output << "                                        if (json.response) {\n";
    output << "                                            fullResponse += json.response;\n";
    output << "                                            this.print(json.response);\n";
    output << "                                        }\n";
    output << "                                    } catch (err) {}\n";
    output << "                                }\n";
    output << "                            }\n";
    output << "                            if (buffer.trim() !== \"\") {\n";
    output << "                                try {\n";
    output << "                                    const json = JSON.parse(buffer);\n";
    output << "                                    if (json.response) {\n";
    output << "                                        fullResponse += json.response;\n";
    output << "                                        this.print(json.response);\n";
    output << "                                    }\n";
    output << "                                } catch (err) {}\n";
    output << "                            }\n";
    output << "                            this.println(\"\");\n";
    output << "                            return fullResponse;\n";
    output << "                        } else {\n";
    output << "                            const json = await response.json();\n";
    output << "                            return json.response;\n";
    output << "                        }\n";
    output << "                    }\n";
    output << "                } catch(e) {}\n";
    output << "                this.println('[Runtime Warning] Ollama backend not reachable. Falling back to simulated completion.');\n";
    output << "                let simulated = \"Simulated response for prompt: '\" + promptStr + \"'\";\n";
    output << "                if (imageBase64) {\n";
    output << "                    simulated += \" with image '\" + imagePath + \"'\";\n";
    output << "                }\n";
    output << "                if (isStreaming) {\n";
    output << "                    const tokens = simulated.split(\" \");\n";
    output << "                    let fullText = \"\";\n";
    output << "                    for (const tok of tokens) {\n";
    output << "                        this.print(tok + \" \");\n";
    output << "                        await new Promise(r => setTimeout(r, 50));\n";
    output << "                        fullText += tok + \" \";\n";
    output << "                    }\n";
    output << "                    this.println(\"\");\n";
    output << "                    return fullText;\n";
    output << "                } else {\n";
    output << "                    return simulated;\n";
    output << "                }\n";
    output << "            }\n";
    output << "        };\n\n";
    output << "        async function httpGet(url) {\n";
    output << "            zenith.println('[Network] httpGet: Fetching ' + url + '...');\n";
    output << "            try {\n";
    output << "                const response = await fetch(url);\n";
    output << "                if (response.ok) {\n";
    output << "                    return await response.text();\n";
    output << "                }\n";
    output << "            } catch(e) {}\n";
    output << "            zenith.println('[Network Error/CORS] Using mock response for ' + url);\n";
    output << "            if (url.includes('users')) {\n";
    output << "                return JSON.stringify({users: [\"Sam\", \"Jay\", \"Alex\"], status: \"active\"});\n";
    output << "            }\n";
    output << "            return JSON.stringify({message: \"Hello from Zenith JS mock endpoint!\", status: \"success\"});\n";
    output << "        }\n\n";
    output << "        async function httpPost(url, body) {\n";
    output << "            zenith.println('[Network] httpPost: Posting to ' + url + '...');\n";
    output << "            try {\n";
    output << "                const response = await fetch(url, {\n";
    output << "                    method: 'POST',\n";
    output << "                    headers: { 'Content-Type': 'application/json' },\n";
    output << "                    body: body\n";
    output << "                });\n";
    output << "                if (response.ok) {\n";
    output << "                    return await response.text();\n";
    output << "                }\n";
    output << "            } catch(e) {}\n";
    output << "            zenith.println('[Network Error/CORS] Using mock response for POST ' + url);\n";
    output << "            return JSON.stringify({status: \"posted\", received: JSON.parse(body)});\n";
    output << "        }\n\n";
    output << "        const UI = {\n";
    output << "            print: function(msg) { zenith.print(Array.isArray(msg) ? msg.join('') : msg); },\n";
    output << "            println: function(msg) { zenith.println(Array.isArray(msg) ? msg.join('') : msg); },\n";
    output << "            applyStyles: function(el, attrs = {}) {\n";
    output << "                if (!attrs) return;\n";
    output << "                if (attrs.color) el.style.color = attrs.color;\n";
    output << "                if (attrs.backgroundColor) el.style.backgroundColor = attrs.backgroundColor;\n";
    output << "                if (attrs.padding) el.style.padding = typeof attrs.padding === 'number' ? attrs.padding + 'px' : attrs.padding;\n";
    output << "                if (attrs.margin) el.style.margin = typeof attrs.margin === 'number' ? attrs.margin + 'px' : attrs.margin;\n";
    output << "                if (attrs.width) el.style.width = typeof attrs.width === 'number' ? attrs.width + 'px' : attrs.width;\n";
    output << "                if (attrs.height) el.style.height = typeof attrs.height === 'number' ? attrs.height + 'px' : attrs.height;\n";
    output << "                if (attrs.fontWeight) el.style.fontWeight = attrs.fontWeight;\n";
    output << "                if (attrs.flexDirection) el.style.flexDirection = attrs.flexDirection;\n";
    output << "                if (attrs.justifyContent) el.style.justifyContent = attrs.justifyContent;\n";
    output << "                if (attrs.alignItems) el.style.alignItems = attrs.alignItems;\n";
    output << "                if (attrs.flexGrow !== undefined) el.style.flexGrow = attrs.flexGrow;\n";
    output << "                if (attrs.gap) el.style.gap = typeof attrs.gap === 'number' ? attrs.gap + 'px' : attrs.gap;\n";
    output << "                if (attrs.onClick && typeof attrs.onClick === 'function') {\n";
    output << "                    el.onclick = attrs.onClick;\n";
    output << "                }\n";
    output << "                if (attrs.onChange && typeof attrs.onChange === 'function') {\n";
    output << "                    el.oninput = function() { attrs.onChange(el.value); };\n";
    output << "                }\n";
    output << "            },\n";
    output << "            Column: function(children, attrs = {}) {\n";
    output << "                const el = document.createElement('div');\n";
    output << "                el.className = 'zenith-column';\n";
    output << "                const flatChildren = Array.isArray(children) ? children.flat() : [];\n";
    output << "                for (const child of flatChildren) {\n";
    output << "                    if (child) el.appendChild(child);\n";
    output << "                }\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Row: function(children, attrs = {}) {\n";
    output << "                const el = document.createElement('div');\n";
    output << "                el.className = 'zenith-row';\n";
    output << "                const flatChildren = Array.isArray(children) ? children.flat() : [];\n";
    output << "                for (const child of flatChildren) {\n";
    output << "                    if (child) el.appendChild(child);\n";
    output << "                }\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Text: function(text, attrs = {}) {\n";
    output << "                const el = document.createElement('span');\n";
    output << "                el.className = 'zenith-text';\n";
    output << "                el.textContent = Array.isArray(text) ? text.join('') : text;\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Button: function(label, attrs = {}) {\n";
    output << "                const el = document.createElement('button');\n";
    output << "                el.className = 'interactive-btn';\n";
    output << "                el.textContent = Array.isArray(label) ? label.join('') : label;\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            TextField: function(placeholder, attrs = {}) {\n";
    output << "                const el = document.createElement('input');\n";
    output << "                el.type = 'text';\n";
    output << "                el.placeholder = Array.isArray(placeholder) ? placeholder.join('') : placeholder;\n";
    output << "                el.className = 'zenith-input';\n";
    output << "                if (attrs.value) el.value = attrs.value;\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Image: function(url, attrs = {}) {\n";
    output << "                const el = document.createElement('img');\n";
    output << "                el.src = Array.isArray(url) ? url.join('') : url;\n";
    output << "                el.style.maxWidth = '100%';\n";
    output << "                el.style.borderRadius = '12px';\n";
    output << "                el.style.boxShadow = '0 8px 30px rgba(0, 0, 0, 0.5)';\n";
    output << "                el.style.border = '1px solid rgba(255, 255, 255, 0.1)';\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Video: function(url, attrs = {}) {\n";
    output << "                const el = document.createElement('video');\n";
    output << "                el.src = Array.isArray(url) ? url.join('') : url;\n";
    output << "                el.controls = true;\n";
    output << "                el.style.maxWidth = '100%';\n";
    output << "                el.style.borderRadius = '12px';\n";
    output << "                el.style.boxShadow = '0 8px 30px rgba(0, 0, 0, 0.5)';\n";
    output << "                el.style.border = '1px solid rgba(255, 255, 255, 0.1)';\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Scrolling: function(children, attrs = {}) {\n";
    output << "                const el = document.createElement('div');\n";
    output << "                el.style.display = 'flex';\n";
    output << "                el.style.flexDirection = 'column';\n";
    output << "                el.style.overflowY = 'auto';\n";
    output << "                el.style.maxHeight = '300px';\n";
    output << "                el.style.border = '1px solid rgba(255, 255, 255, 0.1)';\n";
    output << "                el.style.padding = '12px';\n";
    output << "                el.style.borderRadius = '12px';\n";
    output << "                el.style.background = 'rgba(0, 0, 0, 0.3)';\n";
    output << "                const flatChildren = Array.isArray(children) ? children.flat() : [];\n";
    output << "                for (const child of flatChildren) {\n";
    output << "                    if (child) el.appendChild(child);\n";
    output << "                }\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Card: function(children, attrs = {}) {\n";
    output << "                const el = document.createElement('div');\n";
    output << "                el.style.display = 'flex';\n";
    output << "                el.style.flexDirection = 'column';\n";
    output << "                el.style.background = 'rgba(30, 41, 59, 0.6)';\n";
    output << "                el.style.backdropFilter = 'blur(12px)';\n";
    output << "                el.style.border = '1px solid rgba(255, 255, 255, 0.1)';\n";
    output << "                el.style.borderRadius = '16px';\n";
    output << "                el.style.padding = '20px';\n";
    output << "                el.style.margin = '10px 0';\n";
    output << "                el.style.boxShadow = '0 20px 40px rgba(0, 0, 0, 0.4)';\n";
    output << "                const flatChildren = Array.isArray(children) ? children.flat() : [];\n";
    output << "                for (const child of flatChildren) {\n";
    output << "                    if (child) el.appendChild(child);\n";
    output << "                }\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Container: function(children, attrs = {}) {\n";
    output << "                const el = document.createElement('div');\n";
    output << "                el.style.display = 'flex';\n";
    output << "                el.style.flexDirection = 'column';\n";
    output << "                el.style.border = '1px solid rgba(255, 255, 255, 0.05)';\n";
    output << "                el.style.borderRadius = '12px';\n";
    output << "                el.style.padding = '16px';\n";
    output << "                el.style.background = 'rgba(255, 255, 255, 0.02)';\n";
    output << "                const flatChildren = Array.isArray(children) ? children.flat() : [];\n";
    output << "                for (const child of flatChildren) {\n";
    output << "                    if (child) el.appendChild(child);\n";
    output << "                }\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            render: function(element) {\n";
    output << "                const root = document.getElementById('zenith-ui-root');\n";
    output << "                root.innerHTML = '';\n";
    output << "                root.appendChild(element);\n";
    output << "            }\n";
    output << "        };\n";
    output << "    </script>\n";
    
    output << "    <script>\n";
    
    // Generate JS content
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            generateClass(class_decl);
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            generateInterface(interface_decl);
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            if (auto* agentic = dynamic_cast<AgenticFunctionNode*>(fn_decl)) {
                generateAgenticFunction(agentic);
            } else {
                generateFunction(fn_decl);
            }
        } else if (auto* orch_decl = dynamic_cast<AgentOrchestrationNode*>(stmt.get())) {
            generateOrchestration(orch_decl);
        } else {
            generateStatement(stmt.get());
        }
    }
    
    // Add global trigger handles and execute main
    output << "        let counterGlobalInstance = null;\n";
    output << "        function triggerIncrement() {\n";
    output << "            if (counterGlobalInstance) {\n";
    output << "                if (typeof counterGlobalInstance.increment === 'function') {\n";
    output << "                    counterGlobalInstance.increment();\n";
    output << "                } else if (typeof counterGlobalInstance.handleIncrement === 'function') {\n";
    output << "                    counterGlobalInstance.handleIncrement();\n";
    output << "                }\n";
    output << "            }\n";
    output << "        }\n\n";
    
    output << "        window.onload = () => {\n";
    output << "            main();\n";
    output << "        };\n";
    output << "    </script>\n";
    output << "</body>\n</html>\n";
    
    return output.str();
}
