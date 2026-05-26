#include "../../include/backend/js_codegen.h"
#include <iostream>
#include <regex>
#include <unordered_map>

static std::string preRenderUIComponent(ASTNode* node, const std::unordered_map<std::string, std::string>& fields) {
    if (!node) return "";
    
    if (auto* id = dynamic_cast<IdentifierNode*>(node)) {
        if (fields.count(id->name)) {
            return fields.at(id->name);
        }
        return "";
    }
    
    if (auto* str = dynamic_cast<StringLiteralNode*>(node)) {
        return str->value;
    }
    
    if (auto* num = dynamic_cast<NumberLiteralNode*>(node)) {
        return num->value;
    }
    
    if (auto* b = dynamic_cast<BoolLiteralNode*>(node)) {
        return b->value ? "true" : "false";
    }
    
    if (auto* binary = dynamic_cast<BinaryExprNode*>(node)) {
        return preRenderUIComponent(binary->left.get(), fields) + preRenderUIComponent(binary->right.get(), fields);
    }
    
    if (auto* ui = dynamic_cast<UIComponentNode*>(node)) {
        std::string tag = "div";
        std::string css_class = "";
        std::string attrs = "";
        std::string text_content = "";
        
        if (ui->component_type == "Column") {
            css_class = "zenith-column";
        } else if (ui->component_type == "Row") {
            css_class = "zenith-row";
        } else if (ui->component_type == "Text") {
            tag = "span";
            css_class = "zenith-text";
            if (!ui->children.empty()) {
                text_content = preRenderUIComponent(ui->children[0].get(), fields);
            }
        } else if (ui->component_type == "Button") {
            tag = "button";
            css_class = "interactive-btn";
            if (!ui->children.empty()) {
                text_content = preRenderUIComponent(ui->children[0].get(), fields);
            }
        } else if (ui->component_type == "Card") {
            css_class = "zenith-card";
            attrs += " style=\"display: flex; flex-direction: column; background: rgba(30, 41, 59, 0.6); backdrop-filter: blur(12px); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 16px; padding: 20px; margin: 10px 0; box-shadow: 0 20px 40px rgba(0, 0, 0, 0.4);\"";
        } else if (ui->component_type == "Container") {
            attrs += " style=\"display: flex; flex-direction: column; border: 1px solid rgba(255, 255, 255, 0.05); border-radius: 12px; padding: 16px; background: rgba(255, 255, 255, 0.02);\"";
        } else if (ui->component_type == "Scrolling") {
            attrs += " style=\"display: flex; flex-direction: column; overflow-y: auto; max-height: 300px; border: 1px solid rgba(255, 255, 255, 0.1); padding: 12px; border-radius: 12px; background: rgba(0, 0, 0, 0.3);\"";
        } else if (ui->component_type == "TextField") {
            tag = "input";
            std::string placeholder = "";
            if (!ui->children.empty()) {
                placeholder = preRenderUIComponent(ui->children[0].get(), fields);
            }
            std::string val_val = "";
            for (const auto& arg : ui->named_args) {
                if (arg.first == "value") {
                    val_val = preRenderUIComponent(arg.second.get(), fields);
                }
            }
            std::string val_attr = val_val.empty() ? "" : " value=\"" + val_val + "\"";
            attrs += " type=\"text\" class=\"zenith-input\" placeholder=\"" + placeholder + "\"" + val_attr;
        } else if (ui->component_type == "Checkbox") {
            std::string label = "";
            if (!ui->children.empty()) {
                label = preRenderUIComponent(ui->children[0].get(), fields);
            }
            std::string is_checked_val = "false";
            for (const auto& arg : ui->named_args) {
                if (arg.first == "checked") {
                    is_checked_val = preRenderUIComponent(arg.second.get(), fields);
                }
            }
            std::string checked_attr = (is_checked_val == "true") ? " checked" : "";
            return "<label style=\"display: flex; align-items: center; gap: 8px; cursor: pointer; margin: 5px; color: rgb(226, 232, 240);\"><input type=\"checkbox\" style=\"width: 18px; height: 18px; accent-color: rgb(0, 242, 254); cursor: pointer;\"" + checked_attr + " /><span>" + label + "</span></label>";
        } else if (ui->component_type == "Slider") {
            tag = "input";
            std::string min_val = "0";
            std::string max_val = "100";
            std::string cur_val = "50";
            for (const auto& arg : ui->named_args) {
                std::string key = arg.first;
                std::string val = preRenderUIComponent(arg.second.get(), fields);
                if (key == "min") min_val = val;
                else if (key == "max") max_val = val;
                else if (key == "value") cur_val = val;
            }
            attrs += " type=\"range\" class=\"zenith-input\" min=\"" + min_val + "\" max=\"" + max_val + "\" value=\"" + cur_val + "\" style=\"accent-color: rgb(0, 242, 254); cursor: pointer;\"";
        } else if (ui->component_type == "Toggle") {
            std::string label = "";
            if (!ui->children.empty()) {
                label = preRenderUIComponent(ui->children[0].get(), fields);
            }
            std::string is_on_val = "false";
            for (const auto& arg : ui->named_args) {
                if (arg.first == "isOn") {
                    is_on_val = preRenderUIComponent(arg.second.get(), fields);
                }
            }
            bool is_on = (is_on_val == "true");
            std::string bg_color = is_on ? "rgb(16, 185, 129)" : "rgb(71, 85, 105)";
            std::string transform = is_on ? "transform: translateX(20px);" : "";
            return "<label style=\"display: flex; align-items: center; gap: 10px; cursor: pointer; margin: 5px;\"><div style=\"position: relative; width: 44px; height: 24px; background-color: " + bg_color + "; border-radius: 12px; transition: background-color 0.2s;\"><div style=\"position: absolute; top: 2px; left: 2px; width: 20px; height: 20px; border-radius: 50%; background-color: rgb(255, 255, 255); transition: transform 0.2s; " + transform + "\"></div></div><span style=\"color: rgb(226, 232, 240);\">" + label + "</span></label>";
        } else if (ui->component_type == "Dropdown") {
            tag = "select";
            css_class = "zenith-input";
            attrs += " style=\"background: rgba(15, 23, 42, 0.8); color: rgb(248, 250, 252); cursor: pointer;\"";
            std::string options_str = "";
            if (!ui->children.empty()) {
                options_str = preRenderUIComponent(ui->children[0].get(), fields);
            }
            std::string current_val = "";
            for (const auto& arg : ui->named_args) {
                if (arg.first == "value") {
                    current_val = preRenderUIComponent(arg.second.get(), fields);
                }
            }
            std::string opts_html = "";
            size_t pos = 0;
            while (pos < options_str.length()) {
                size_t comma = options_str.find(',', pos);
                std::string opt = options_str.substr(pos, comma - pos);
                opt.erase(0, opt.find_first_not_of(" \t\r\n"));
                opt.erase(opt.find_last_not_of(" \t\r\n") + 1);
                if (!opt.empty()) {
                    std::string selected = (opt == current_val) ? " selected" : "";
                    opts_html += "<option value=\"" + opt + "\"" + selected + ">" + opt + "</option>";
                }
                if (comma == std::string::npos) break;
                pos = comma + 1;
            }
            text_content = opts_html;
        } else if (ui->component_type == "Image") {
            tag = "img";
            std::string url = "";
            if (!ui->children.empty()) {
                url = preRenderUIComponent(ui->children[0].get(), fields);
            }
            attrs += " src=\"" + url + "\" style=\"max-width: 100%; border-radius: 12px; box-shadow: 0 8px 30px rgba(0, 0, 0, 0.5); border: 1px solid rgba(255, 255, 255, 0.1);\"";
        }
        
        // Render inline styling named arguments if present
        std::string inline_style = "";
        for (const auto& arg : ui->named_args) {
            std::string key = arg.first;
            std::string val_str = preRenderUIComponent(arg.second.get(), fields);
            if (val_str.empty()) continue;
            
            bool is_digit = (val_str.back() >= '0' && val_str.back() <= '9');
            if (key == "color") inline_style += "color: " + val_str + "; ";
            else if (key == "backgroundColor") inline_style += "background-color: " + val_str + "; ";
            else if (key == "padding") inline_style += "padding: " + val_str + (is_digit ? "px" : "") + "; ";
            else if (key == "margin") inline_style += "margin: " + val_str + (is_digit ? "px" : "") + "; ";
            else if (key == "width") inline_style += "width: " + val_str + (is_digit ? "px" : "") + "; ";
            else if (key == "height") inline_style += "height: " + val_str + (is_digit ? "px" : "") + "; ";
            else if (key == "fontWeight") inline_style += "font-weight: " + val_str + "; ";
            else if (key == "gap") inline_style += "gap: " + val_str + (is_digit ? "px" : "") + "; ";
        }
        if (!inline_style.empty()) {
            attrs += " style=\"" + inline_style + "\"";
        }
        
        std::string class_attr = css_class.empty() ? "" : " class=\"" + css_class + "\"";
        
        std::string children_html = "";
        if (ui->component_type == "Column" || ui->component_type == "Row" || ui->component_type == "Card" || ui->component_type == "Container" || ui->component_type == "Scrolling") {
            for (const auto& child : ui->children) {
                children_html += preRenderUIComponent(child.get(), fields);
            }
        }
        
        if (tag == "img" || tag == "input") {
            return "<" + tag + class_attr + attrs + " />";
        }
        
        return "<" + tag + class_attr + attrs + ">" + text_content + children_html + "</" + tag + ">";
    }
    
    return "";
}


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
    if (auto* fcall = dynamic_cast<FunctionCallNode*>(node)) {
        if (async_fns.count(fcall->function_name)) return true;
        for (const auto& arg : fcall->arguments) {
            if (containsAsyncCall(arg.get(), async_fns)) return true;
        }
    }
    if (dynamic_cast<AwaitExprNode*>(node)) return true;
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
    
    if (auto* lambda = dynamic_cast<LambdaNode*>(expr)) {
        std::string res = "(";
        for (size_t i = 0; i < lambda->parameters.size(); ++i) {
            res += lambda->parameters[i]->var_name;
            if (i < lambda->parameters.size() - 1) res += ", ";
        }
        res += ") => {\n";
        
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
    if (auto* fcall = dynamic_cast<FunctionCallNode*>(expr)) {
        bool is_async_call = async_functions.count(fcall->function_name) > 0;
        std::string res = (is_async_call ? "await " : "") + fcall->function_name + "(";
        for (size_t i = 0; i < fcall->arguments.size(); ++i) {
            res += generateExpression(fcall->arguments[i].get());
            if (i < fcall->arguments.size() - 1) res += ", ";
        }
        res += ")";
        return res;
    }
    if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
        std::string res;
        bool is_class = class_names.count(ui->component_type) > 0;
        bool is_fn = function_names.count(ui->component_type) > 0;
        bool is_method = is_inside_class_method && current_class_methods.count(ui->component_type);
        bool is_variable_call = !ui->component_type.empty() && std::islower(static_cast<unsigned char>(ui->component_type[0]));
        bool is_custom = is_class || is_fn || is_method || is_variable_call;
        
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
    if (auto* await_expr = dynamic_cast<AwaitExprNode*>(expr)) {
        std::string inner = generateExpression(await_expr->expression.get());
        if (inner.rfind("await ", 0) == 0) {
            return inner;
        }
        return "await " + inner;
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
    if (node->is_foreign) {
        return;
    }
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
    
    std::vector<std::string> c_foreign_funcs;
    std::vector<std::string> js_foreign_funcs;
    
    // Register all globals and collect foreign functions
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            class_names.insert(class_decl->class_name);
            for (const auto& method : class_decl->methods) {
                if (method->is_async || dynamic_cast<AgenticFunctionNode*>(method.get())) {
                    agentic_functions.insert(method->function_name);
                }
            }
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            interface_names.insert(interface_decl->interface_name);
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            function_names.insert(fn_decl->function_name);
            if (fn_decl->is_async || dynamic_cast<AgenticFunctionNode*>(fn_decl)) {
                agentic_functions.insert(fn_decl->function_name);
            }
            if (fn_decl->is_foreign) {
                if (fn_decl->foreign_abi == "C") {
                    c_foreign_funcs.push_back(fn_decl->function_name);
                } else if (fn_decl->foreign_abi == "js") {
                    js_foreign_funcs.push_back(fn_decl->function_name);
                }
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
    output << "    </style>\n";

    // ----------------------------------------------------------------
    // CDN/npm library injection — collect import cdn/npm from AST
    // ----------------------------------------------------------------
    for (const auto& stmt : program->statements) {
        if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            if (imp->isActiveFor("web")) {
                if (imp->kind == ImportNode::ImportKind::Cdn || imp->kind == ImportNode::ImportKind::Npm) {
                    std::string url = imp->cdn_url;
                    if (url.length() < 5 || url.substr(url.length() - 5) != ".wasm") {
                        // Emit <script src="..."> in <head> before any app code runs
                        output << "    <!-- Zenith Library: " << imp->module_name << " -->\n";
                        output << "    <script src=\"" << imp->cdn_url << "\" crossorigin=\"anonymous\"></script>\n";
                    }
                }
            }
        }
    }

    // WASM string/module loading runtime helper injections
    output << "    <script>\n";
    output << "        // Helper to copy JS string to WASM memory\n";
    output << "        function writeWasmString(instance, str) {\n";
    output << "            const encoder = new TextEncoder();\n";
    output << "            const bytes = encoder.encode(str + \"\\0\");\n";
    output << "            const allocFn = instance.exports.alloc || instance.exports.malloc;\n";
    output << "            if (!allocFn) {\n";
    output << "                console.error(\"WASM module does not export alloc/malloc!\");\n";
    output << "                return 0;\n";
    output << "            }\n";
    output << "            const ptr = allocFn(bytes.length);\n";
    output << "            const mem = new Uint8Array(instance.exports.memory.buffer);\n";
    output << "            mem.set(bytes, ptr);\n";
    output << "            return ptr;\n";
    output << "        }\n\n";
    output << "        // Helper to read JS string from WASM memory\n";
    output << "        function readWasmString(instance, ptr) {\n";
    output << "            if (ptr === 0) return \"\";\n";
    output << "            const mem = new Uint8Array(instance.exports.memory.buffer);\n";
    output << "            let end = ptr;\n";
    output << "            while (mem[end] !== 0) end++;\n";
    output << "            const bytes = mem.slice(ptr, end);\n";
    output << "            const decoder = new TextDecoder();\n";
    output << "            const str = decoder.decode(bytes);\n";
    output << "            const freeFn = instance.exports.dealloc || instance.exports.free;\n";
    output << "            if (freeFn) freeFn(ptr, bytes.length + 1);\n";
    output << "            return str;\n";
    output << "        }\n\n";
    output << "        // Generic wrapper for WASM functions taking/returning strings\n";
    output << "        function callWasmStringToString(instance, fnName, ...args) {\n";
    output << "            const fn = instance.exports[fnName];\n";
    output << "            if (!fn) {\n";
    output << "                console.error('WASM function not found: ' + fnName);\n";
    output << "                return \"\";\n";
    output << "            }\n";
    output << "            const ptrs = args.map(arg => typeof arg === 'string' ? writeWasmString(instance, arg) : arg);\n";
    output << "            const retPtr = fn(...ptrs);\n";
    output << "            const result = readWasmString(instance, retPtr);\n";
    output << "            const freeFn = instance.exports.dealloc || instance.exports.free;\n";
    output << "            if (freeFn) {\n";
    output << "                ptrs.forEach((ptr, i) => {\n";
    output << "                    if (typeof args[i] === 'string') {\n";
    output << "                        freeFn(ptr, new TextEncoder().encode(args[i]).length + 1);\n";
    output << "                    }\n";
    output << "                });\n";
    output << "            }\n";
    output << "            return result;\n";
    output << "        }\n\n";
    output << "        const wasmPromises = [];\n\n";
    output << "        async function loadRawWasm(url, exportsToGlobal) {\n";
    output << "            try {\n";
    output << "                const response = await fetch(url);\n";
    output << "                const bytes = await response.arrayBuffer();\n";
    output << "                const results = await WebAssembly.instantiate(bytes, { env: {\n";
    output << "                    abort: () => { console.error('abort called'); }\n";
    output << "                }});\n";
    output << "                const instance = results.instance;\n";
    output << "                for (const fnName of exportsToGlobal) {\n";
    output << "                    window[fnName] = function(...args) {\n";
    output << "                        return callWasmStringToString(instance, fnName, ...args);\n";
    output << "                    };\n";
    output << "                }\n";
    output << "                zenith.println('[WASM] Raw WASM module loaded from ' + url);\n";
    output << "            } catch (err) {\n";
    output << "                zenith.println('[WASM Error] Failed to load ' + url + ': ' + err);\n";
    output << "            }\n";
    output << "        }\n\n";
    output << "        async function loadDartWasm(wasmUrl, mjsUrl, exportsToGlobal) {\n";
    output << "            try {\n";
    output << "                const module = await import(mjsUrl);\n";
    output << "                const dartInstance = await module.instantiate(fetch(wasmUrl));\n";
    output << "                for (const fnName of exportsToGlobal) {\n";
    output << "                    if (dartInstance[fnName]) {\n";
    output << "                        window[fnName] = dartInstance[fnName];\n";
    output << "                    } else if (dartInstance.exports && dartInstance.exports[fnName]) {\n";
    output << "                        window[fnName] = dartInstance.exports[fnName];\n";
    output << "                    } else {\n";
    output << "                        window[fnName] = dartInstance[fnName];\n";
    output << "                    }\n";
    output << "                }\n";
    output << "                zenith.println('[WASM] Dart WASM module loaded from ' + wasmUrl);\n";
    output << "            } catch (err) {\n";
    output << "                zenith.println('[WASM Warning] Failed to load Dart WASM via JS module, trying raw loader: ' + err);\n";
    output << "                await loadRawWasm(wasmUrl, exportsToGlobal);\n";
    output << "            }\n";
    output << "        }\n\n";
    
    for (const auto& stmt : program->statements) {
        if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            if (imp->isActiveFor("web") && imp->kind == ImportNode::ImportKind::Cdn) {
                std::string url = imp->cdn_url;
                if (url.length() >= 5 && url.substr(url.length() - 5) == ".wasm") {
                    if (url.find("key_derive") != std::string::npos) {
                        std::string mjs_url = url.substr(0, url.length() - 5) + ".mjs";
                        output << "        wasmPromises.push(loadDartWasm('" << url << "', '" << mjs_url << "', [";
                        for (size_t i = 0; i < js_foreign_funcs.size(); ++i) {
                            output << "'" << js_foreign_funcs[i] << "'";
                            if (i < js_foreign_funcs.size() - 1) output << ", ";
                        }
                        output << "]));\n";
                    } else {
                        output << "        wasmPromises.push(loadRawWasm('" << url << "', [";
                        for (size_t i = 0; i < c_foreign_funcs.size(); ++i) {
                            output << "'" << c_foreign_funcs[i] << "'";
                            if (i < c_foreign_funcs.size() - 1) output << ", ";
                        }
                        output << "]));\n";
                    }
                }
            }
        }
    }
    output << "    </script>\n";

    output << "</head>\n<body>\n";

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
    std::string pre_rendered_html = "";
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            FunctionNode* build_method = nullptr;
            for (const auto& method : class_decl->methods) {
                if (method->function_name == "build") {
                    build_method = method.get();
                    break;
                }
            }
            if (build_method) {
                std::unordered_map<std::string, std::string> fields;
                for (const auto& field : class_decl->fields) {
                    if (field->initializer) {
                        if (auto* str = dynamic_cast<StringLiteralNode*>(field->initializer.get())) {
                            fields[field->var_name] = str->value;
                        } else if (auto* num = dynamic_cast<NumberLiteralNode*>(field->initializer.get())) {
                            fields[field->var_name] = num->value;
                        } else if (auto* b = dynamic_cast<BoolLiteralNode*>(field->initializer.get())) {
                            fields[field->var_name] = b->value ? "true" : "false";
                        }
                    }
                }
                ExprNode* return_expr = nullptr;
                for (const auto& body_stmt : build_method->body) {
                    if (auto* ret_stmt = dynamic_cast<ReturnStmtNode*>(body_stmt.get())) {
                        return_expr = ret_stmt->expression.get();
                        break;
                    }
                }
                if (return_expr) {
                    pre_rendered_html = preRenderUIComponent(return_expr, fields);
                }
                break;
            }
        }
    }

    output << "            <div id=\"zenith-ui-root\">" << pre_rendered_html << "</div>\n";
    output << "        </div>\n";
    output << "        <div class=\"panel terminal-panel\">\n";
    output << "            <div class=\"panel-title\" style=\"color: #10b981;\">Output Console</div>\n";
    output << "            <div class=\"terminal-body\" id=\"zenith-console\"></div>\n";
    output << "        </div>\n";
    output << "    </div>\n\n";
    
    output << "    <script>\n";
    output << "        // Platform Detection Constants\n";
    output << "        const isAndroid = false;\n";
    output << "        const isIos = false;\n";
    output << "        const isMac = false;\n";
    output << "        const isLinux = false;\n";
    output << "        const isWeb = true;\n";
    output << "        const isWindows = false;\n\n";
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
    output << "            Checkbox: function(label, attrs = {}) {\n";
    output << "                const wrapper = document.createElement('label');\n";
    output << "                wrapper.style.display = 'flex';\n";
    output << "                wrapper.style.alignItems = 'center';\n";
    output << "                wrapper.style.gap = '8px';\n";
    output << "                wrapper.style.cursor = 'pointer';\n";
    output << "                wrapper.style.margin = '5px';\n";
    output << "                wrapper.style.color = '#e2e8f0';\n";
    output << "                const el = document.createElement('input');\n";
    output << "                el.type = 'checkbox';\n";
    output << "                el.style.width = '18px';\n";
    output << "                el.style.height = '18px';\n";
    output << "                el.style.accentColor = '#00f2fe';\n";
    output << "                el.style.cursor = 'pointer';\n";
    output << "                if (attrs.checked) el.checked = !!attrs.checked;\n";
    output << "                wrapper.appendChild(el);\n";
    output << "                const textSpan = document.createElement('span');\n";
    output << "                textSpan.textContent = Array.isArray(label) ? label.join('') : label;\n";
    output << "                wrapper.appendChild(textSpan);\n";
    output << "                UI.applyStyles(wrapper, attrs);\n";
    output << "                if (attrs.onChange && typeof attrs.onChange === 'function') {\n";
    output << "                    el.onchange = function() { attrs.onChange(el.checked); };\n";
    output << "                }\n";
    output << "                wrapper.render = function() {\n";
    output << "                    UI.render(wrapper);\n";
    output << "                    return wrapper;\n";
    output << "                };\n";
    output << "                return wrapper;\n";
    output << "            },\n";
    output << "            Slider: function(attrs = {}) {\n";
    output << "                const el = document.createElement('input');\n";
    output << "                el.type = 'range';\n";
    output << "                el.className = 'zenith-input';\n";
    output << "                el.style.accentColor = '#00f2fe';\n";
    output << "                el.style.cursor = 'pointer';\n";
    output << "                if (attrs.min !== undefined) el.min = attrs.min;\n";
    output << "                if (attrs.max !== undefined) el.max = attrs.max;\n";
    output << "                if (attrs.value !== undefined) el.value = attrs.value;\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                if (attrs.onChange && typeof attrs.onChange === 'function') {\n";
    output << "                    el.oninput = function() { attrs.onChange(parseInt(el.value, 10)); };\n";
    output << "                }\n";
    output << "                el.render = function() {\n";
    output << "                    UI.render(el);\n";
    output << "                    return el;\n";
    output << "                };\n";
    output << "                return el;\n";
    output << "            },\n";
    output << "            Toggle: function(label, attrs = {}) {\n";
    output << "                const wrapper = document.createElement('label');\n";
    output << "                wrapper.style.display = 'flex';\n";
    output << "                wrapper.style.alignItems = 'center';\n";
    output << "                wrapper.style.gap = '10px';\n";
    output << "                wrapper.style.cursor = 'pointer';\n";
    output << "                wrapper.style.margin = '5px';\n";
    output << "                const switchDiv = document.createElement('div');\n";
    output << "                switchDiv.style.position = 'relative';\n";
    output << "                switchDiv.style.width = '44px';\n";
    output << "                switchDiv.style.height = '24px';\n";
    output << "                switchDiv.style.backgroundColor = '#475569';\n";
    output << "                switchDiv.style.borderRadius = '12px';\n";
    output << "                switchDiv.style.transition = 'background-color 0.2s';\n";
    output << "                const knob = document.createElement('div');\n";
    output << "                knob.style.position = 'absolute';\n";
    output << "                knob.style.top = '2px';\n";
    output << "                knob.style.left = '2px';\n";
    output << "                knob.style.width = '20px';\n";
    output << "                knob.style.height = '20px';\n";
    output << "                knob.style.borderRadius = '50%';\n";
    output << "                knob.style.backgroundColor = '#ffffff';\n";
    output << "                knob.style.transition = 'transform 0.2s';\n";
    output << "                switchDiv.appendChild(knob);\n";
    output << "                let isOn = !!attrs.isOn;\n";
    output << "                const updateToggleUI = (state) => {\n";
    output << "                    isOn = state;\n";
    output << "                    if (isOn) {\n";
    output << "                        switchDiv.style.backgroundColor = '#10b981';\n";
    output << "                        knob.style.transform = 'translateX(20px)';\n";
    output << "                    } else {\n";
    output << "                        switchDiv.style.backgroundColor = '#475569';\n";
    output << "                        knob.style.transform = 'translateX(0)';\n";
    output << "                    }\n";
    output << "                };\n";
    output << "                updateToggleUI(isOn);\n";
    output << "                wrapper.onclick = (e) => {\n";
    output << "                    e.preventDefault();\n";
    output << "                    updateToggleUI(!isOn);\n";
    output << "                    if (attrs.onChange && typeof attrs.onChange === 'function') {\n";
    output << "                        attrs.onChange(isOn);\n";
    output << "                    }\n";
    output << "                };\n";
    output << "                wrapper.appendChild(switchDiv);\n";
    output << "                if (label) {\n";
    output << "                    const textSpan = document.createElement('span');\n";
    output << "                    textSpan.textContent = Array.isArray(label) ? label.join('') : label;\n";
    output << "                    textSpan.style.color = '#e2e8f0';\n";
    output << "                    wrapper.appendChild(textSpan);\n";
    output << "                }\n";
    output << "                UI.applyStyles(wrapper, attrs);\n";
    output << "                wrapper.render = function() {\n";
    output << "                    UI.render(wrapper);\n";
    output << "                    return wrapper;\n";
    output << "                };\n";
    output << "                return wrapper;\n";
    output << "            },\n";
    output << "            Dropdown: function(options, attrs = {}) {\n";
    output << "                const el = document.createElement('select');\n";
    output << "                el.className = 'zenith-input';\n";
    output << "                el.style.background = 'rgba(15, 23, 42, 0.8)';\n";
    output << "                el.style.color = '#f8fafc';\n";
    output << "                el.style.cursor = 'pointer';\n";
    output << "                const populateOptions = (str) => {\n";
    output << "                    el.innerHTML = '';\n";
    output << "                    const opts = str.split(',');\n";
    output << "                    for (const opt of opts) {\n";
    output << "                        const trimmed = opt.trim();\n";
    output << "                        if (!trimmed) continue;\n";
    output << "                        const optionEl = document.createElement('option');\n";
    output << "                        optionEl.value = trimmed;\n";
    output << "                        optionEl.textContent = trimmed;\n";
    output << "                        el.appendChild(optionEl);\n";
    output << "                    }\n";
    output << "                };\n";
    output << "                const optsStr = Array.isArray(options) ? options.join(',') : (options || attrs.options || \"\");\n";
    output << "                populateOptions(optsStr);\n";
    output << "                if (attrs.value) el.value = attrs.value;\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                if (attrs.onChange && typeof attrs.onChange === 'function') {\n";
    output << "                    el.onchange = function() { attrs.onChange(el.value); };\n";
    output << "                }\n";
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
    
    output << "        window.onload = async () => {\n";
    output << "            if (typeof wasmPromises !== 'undefined' && wasmPromises.length > 0) {\n";
    output << "                zenith.println('[WASM] Waiting for ' + wasmPromises.length + ' WASM module(s)...');\n";
    output << "                await Promise.all(wasmPromises);\n";
    output << "                zenith.println('[WASM] All modules loaded. Ready.');\n";
    output << "            }\n";
    output << "            // Define fallback mocks if WASM failed to load\n";
    output << "            if (typeof encrypt === 'undefined') {\n";
    output << "                window.encrypt = function(plaintext, key) {\n";
    output << "                    let result = '';\n";
    output << "                    for (let i = 0; i < plaintext.length; i++) {\n";
    output << "                        result += String.fromCharCode(plaintext.charCodeAt(i) ^ key.charCodeAt(i % key.length));\n";
    output << "                    }\n";
    output << "                    return btoa(result);\n";
    output << "                };\n";
    output << "            }\n";
    output << "            if (typeof decrypt === 'undefined') {\n";
    output << "                window.decrypt = function(ciphertext, key) {\n";
    output << "                    try {\n";
    output << "                        let plaintext = atob(ciphertext);\n";
    output << "                        let result = '';\n";
    output << "                        for (let i = 0; i < plaintext.length; i++) {\n";
    output << "                            result += String.fromCharCode(plaintext.charCodeAt(i) ^ key.charCodeAt(i % key.length));\n";
    output << "                        }\n";
    output << "                        return result;\n";
    output << "                    } catch (e) {\n";
    output << "                        return 'Decryption Error: Invalid Key/Data';\n";
    output << "                    }\n";
    output << "                };\n";
    output << "            }\n";
    output << "            if (typeof deriveKey === 'undefined') {\n";
    output << "                window.deriveKey = function(password, salt) {\n";
    output << "                    return btoa(password + ':' + salt);\n";
    output << "                };\n";
    output << "            }\n";
    output << "            if (typeof println === 'undefined') window.println = (msg) => zenith.println(msg);\n";
    output << "            if (typeof print === 'undefined') window.print = (msg) => zenith.print(msg);\n";
    output << "            main();\n";
    output << "        };\n";
    output << "    </script>\n";
    output << "</body>\n</html>\n";
    
    return output.str();
}
