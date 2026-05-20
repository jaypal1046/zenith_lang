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
        bool is_custom = is_class || is_fn;
        
        if (is_custom) {
            if (is_class) {
                res = "new " + ui->component_type + "(";
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
    
    indent_level--;
    indent(); output << "}\n";
    
    // Methods
    is_inside_class_method = true;
    for (const auto& method : node->methods) {
        indent();
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
    
    // Special render method if it has build()
    bool has_build = false;
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
    output << "async " << node->function_name << "(";
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
    
    indent();
    output << "return await zenith.llmPrompt(this.url, " << js_prompt << ");\n";
    
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
        }
    }

    // Initialize async_functions with agentic_functions
    async_functions = agentic_functions;
    
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
    output << "            llmPrompt: async function(url, promptStr) {\n";
    output << "                this.println('\\n[Runtime] LLMClient sending prompt to local backend (' + url + '): \"' + promptStr + '\"');\n";
    output << "                try {\n";
    output << "                    const response = await fetch(url + '/api/generate', {\n";
    output << "                        method: 'POST',\n";
    output << "                        headers: { 'Content-Type': 'application/json' },\n";
    output << "                        body: JSON.stringify({\n";
    output << "                            model: 'llama3',\n";
    output << "                            prompt: promptStr,\n";
    output << "                            stream: false\n";
    output << "                        })\n";
    output << "                    });\n";
    output << "                    if (response.ok) {\n";
    output << "                        const json = await response.json();\n";
    output << "                        return json.response;\n";
    output << "                    }\n";
    output << "                } catch(e) {}\n";
    output << "                this.println('[Runtime Warning] Ollama backend not reachable. Falling back to simulated completion.');\n";
    output << "                return \"- Zenith compiles UI declarations straight to native bindings.\\n\" +\n";
    output << "                       \"- LLM prompts are statically validated at compile-time.\\n\" +\n";
    output << "                       \"- Zero runtime latency wrapper on top of pure C++ loops.\";\n";
    output << "            }\n";
    output << "        };\n\n";

    output << "        const UI = {\n";
    output << "            print: function(msg) { zenith.print(Array.isArray(msg) ? msg.join('') : msg); },\n";
    output << "            println: function(msg) { zenith.println(Array.isArray(msg) ? msg.join('') : msg); },\n";
    output << "            Column: function(children, attrs = {}) {\n";
    output << "                const el = document.createElement('div');\n";
    output << "                el.className = 'zenith-column';\n";
    output << "                for (const child of children) {\n";
    output << "                    if (child) el.appendChild(child);\n";
    output << "                }\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Row: function(children, attrs = {}) {\n";
    output << "                const el = document.createElement('div');\n";
    output << "                el.className = 'zenith-row';\n";
    output << "                for (const child of children) {\n";
    output << "                    if (child) el.appendChild(child);\n";
    output << "                }\n";
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
    output << "                if (attrs.fontWeight === 'bold') {\n";
    output << "                    el.style.fontWeight = 'bold';\n";
    output << "                }\n";
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
        } else {
            generateStatement(stmt.get());
        }
    }
    
    // Add global trigger handles and execute main
    output << "        let counterGlobalInstance = null;\n";
    output << "        function triggerIncrement() {\n";
    output << "            if (counterGlobalInstance) {\n";
    output << "                counterGlobalInstance.increment();\n";
    output << "            }\n";
    output << "        }\n\n";
    
    output << "        // Hook CounterApp initialization to set the global trigger\n";
    output << "        const originalCounterApp = CounterApp;\n";
    output << "        CounterApp = class extends originalCounterApp {\n";
    output << "            constructor(...args) {\n";
    output << "                super(...args);\n";
    output << "                counterGlobalInstance = this;\n";
    output << "            }\n";
    output << "        };\n\n";
    
    output << "        window.onload = () => {\n";
    output << "            main();\n";
    output << "        };\n";
    output << "    </script>\n";
    output << "</body>\n</html>\n";
    
    return output.str();
}
