#include "../../include/backend/wasm_codegen.h"
#include <iostream>

void WASMCodeGenerator::indent() {
    for (int i = 0; i < indent_level * 2; ++i) {
        output << " ";
    }
}

int WASMCodeGenerator::countUINodes(ASTNode* node) {
    if (!node) return 0;
    int count = 0;
    if (dynamic_cast<UIComponentNode*>(node)) {
        count = 1;
    }
    // Deep traversal could be implemented here, but for MVP Zenith AST,
    // we do a naive recursive counting.
    if (auto* ui = dynamic_cast<UIComponentNode*>(node)) {
        for (const auto& child : ui->children) count += countUINodes(child.get());
    } else if (auto* var = dynamic_cast<VarDeclNode*>(node)) {
        count += countUINodes(var->initializer.get());
    } else if (auto* ret = dynamic_cast<ReturnStmtNode*>(node)) {
        count += countUINodes(ret->expression.get());
    } else if (auto* call = dynamic_cast<MethodCallNode*>(node)) {
        for (const auto& arg : call->arguments) count += countUINodes(arg.get());
    }
    return count;
}

std::string WASMCodeGenerator::mapTypeToWASM(TypeNode* type) {
    if (!type) return "i32"; // default/pointer
    if (type->type_name == "Int") return "i32";
    if (type->type_name == "Float") return "f64";
    if (type->type_name == "Bool") return "i32";
    return "i32"; // Objects/Strings are mapped to memory pointers (i32)
}

void WASMCodeGenerator::generateExpression(ExprNode* expr) {
    if (!expr) return;
    
    if (auto* str = dynamic_cast<StringLiteralNode*>(expr)) {
        indent();
        if (string_literals.find(str->value) == string_literals.end()) {
            string_literals[str->value] = next_string_offset;
            next_string_offset += str->value.length() + 1;
        }
        output << "i32.const " << string_literals[str->value] << "\n";
    }
    else if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
        indent();
        if (num->is_float) {
            output << "f64.const " << num->value << "\n";
        } else {
            output << "i32.const " << num->value << "\n";
        }
    }
    else if (auto* b = dynamic_cast<BoolLiteralNode*>(expr)) {
        indent();
        output << "i32.const " << (b->value ? "1" : "0") << "\n";
    }
    else if (auto* list_lit = dynamic_cast<ListLiteralNode*>(expr)) {
        for (const auto& elem : list_lit->elements) {
            generateExpression(elem.get());
            indent(); output << "drop\n";
        }
        indent(); output << "i32.const 0\n";
    }
    else if (auto* map_lit = dynamic_cast<MapLiteralNode*>(expr)) {
        for (const auto& entry : map_lit->entries) {
            generateExpression(entry.first.get());
            indent(); output << "drop\n";
            generateExpression(entry.second.get());
            indent(); output << "drop\n";
        }
        indent(); output << "i32.const 0\n";
    }
    else if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        indent();
        if (!current_class_name.empty() && class_field_offsets[current_class_name].find(id->name) != class_field_offsets[current_class_name].end()) {
            output << "local.get $this\n";
            indent();
            if (class_field_types[current_class_name][id->name] == "Float") {
                output << "f64.load offset=" << class_field_offsets[current_class_name][id->name] << "\n";
            } else {
                output << "i32.load offset=" << class_field_offsets[current_class_name][id->name] << "\n";
            }
        } else {
            output << "local.get $" << id->name << "\n";
        }
    }
    else if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        if (binary->op == "=") {
            // Assignment expression
            if (auto* id = dynamic_cast<IdentifierNode*>(binary->left.get())) {
                if (!current_class_name.empty() && class_field_offsets[current_class_name].find(id->name) != class_field_offsets[current_class_name].end()) {
                    indent(); output << "local.get $this\n";
                    generateExpression(binary->right.get());
                    indent();
                    if (class_field_types[current_class_name][id->name] == "Float") {
                        output << "f64.store offset=" << class_field_offsets[current_class_name][id->name] << "\n";
                    } else {
                        output << "i32.store offset=" << class_field_offsets[current_class_name][id->name] << "\n";
                    }
                } else {
                    generateExpression(binary->right.get());
                    indent(); output << "local.set $" << id->name << "\n";
                }
            }
            return;
        }
        
        // Handle String Concatenation safely by calling JS host env formatting functions
        bool is_string_left = false;
        if (dynamic_cast<StringLiteralNode*>(binary->left.get())) is_string_left = true;
        if (auto* id = dynamic_cast<IdentifierNode*>(binary->left.get())) {
            is_string_left = (local_types[id->name] == "String");
        }
        
        if (binary->op == "+" && is_string_left) {
            generateExpression(binary->left.get());
            generateExpression(binary->right.get());
            indent();
            if (isFloatExpression(binary->right.get())) {
                output << "call $concat_float\n";
            } else {
                output << "call $concat_int\n";
            }
            return;
        }
        
        generateExpression(binary->left.get());
        generateExpression(binary->right.get());
        
        bool is_float_op = isFloatExpression(binary->left.get()) || isFloatExpression(binary->right.get());
        indent();
        if (binary->op == "+") output << (is_float_op ? "f64.add\n" : "i32.add\n");
        else if (binary->op == "-") output << (is_float_op ? "f64.sub\n" : "i32.sub\n");
        else if (binary->op == "*") output << (is_float_op ? "f64.mul\n" : "i32.mul\n");
        else if (binary->op == "/") output << (is_float_op ? "f64.div\n" : "i32.div_s\n");
        else if (binary->op == "==") output << (is_float_op ? "f64.eq\n" : "i32.eq\n");
        else if (binary->op == "<") output << (is_float_op ? "f64.lt\n" : "i32.lt_s\n");
        else if (binary->op == ">") output << (is_float_op ? "f64.gt\n" : "i32.gt_s\n");
    }
    else if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
        int my_id = ui_node_counter++;
        std::string local_name = "$tmp_ui_" + std::to_string(my_id);
        
        bool is_builtin = (ui->component_type == "Text" || ui->component_type == "Button" ||
                           ui->component_type == "Column" || ui->component_type == "Row" ||
                           ui->component_type == "Image" || ui->component_type == "Video" ||
                           ui->component_type == "Scrolling" || ui->component_type == "Card" ||
                           ui->component_type == "Container" || ui->component_type == "Checkbox" ||
                           ui->component_type == "Slider" || ui->component_type == "Toggle" ||
                           ui->component_type == "Dropdown");
                           
        if (is_builtin) {
            if (ui->component_type == "Text") {
                if (!ui->children.empty()) {
                    generateExpression(ui->children[0].get());
                } else {
                    indent(); output << "i32.const 0\n";
                }
                indent(); output << "call $create_text\n";
            } else if (ui->component_type == "Checkbox") {
                if (!ui->children.empty()) {
                    generateExpression(ui->children[0].get());
                } else {
                    indent(); output << "i32.const 0\n";
                }
                if (!current_class_name.empty()) {
                    indent(); output << "local.get $this\n";
                } else {
                    indent(); output << "i32.const 0\n";
                }
                std::string method_name = "";
                for (const auto& arg : ui->named_args) {
                    if (arg.first == "onChange") {
                        if (auto* id = dynamic_cast<IdentifierNode*>(arg.second.get())) {
                            method_name = id->name;
                        }
                    }
                }
                std::string export_name = current_class_name.empty() ? method_name : (current_class_name + "_" + method_name);
                if (string_literals.find(export_name) == string_literals.end()) {
                    string_literals[export_name] = next_string_offset;
                    next_string_offset += export_name.length() + 1;
                }
                indent(); output << "i32.const " << string_literals[export_name] << "\n";
                indent(); output << "call $create_checkbox\n";
            } else if (ui->component_type == "Slider") {
                if (!current_class_name.empty()) {
                    indent(); output << "local.get $this\n";
                } else {
                    indent(); output << "i32.const 0\n";
                }
                std::string method_name = "";
                for (const auto& arg : ui->named_args) {
                    if (arg.first == "onChange") {
                        if (auto* id = dynamic_cast<IdentifierNode*>(arg.second.get())) {
                            method_name = id->name;
                        }
                    }
                }
                std::string export_name = current_class_name.empty() ? method_name : (current_class_name + "_" + method_name);
                if (string_literals.find(export_name) == string_literals.end()) {
                    string_literals[export_name] = next_string_offset;
                    next_string_offset += export_name.length() + 1;
                }
                indent(); output << "i32.const " << string_literals[export_name] << "\n";
                indent(); output << "call $create_slider\n";
            } else if (ui->component_type == "Toggle") {
                if (!ui->children.empty()) {
                    generateExpression(ui->children[0].get());
                } else {
                    indent(); output << "i32.const 0\n";
                }
                if (!current_class_name.empty()) {
                    indent(); output << "local.get $this\n";
                } else {
                    indent(); output << "i32.const 0\n";
                }
                std::string method_name = "";
                for (const auto& arg : ui->named_args) {
                    if (arg.first == "onChange") {
                        if (auto* id = dynamic_cast<IdentifierNode*>(arg.second.get())) {
                            method_name = id->name;
                        }
                    }
                }
                std::string export_name = current_class_name.empty() ? method_name : (current_class_name + "_" + method_name);
                if (string_literals.find(export_name) == string_literals.end()) {
                    string_literals[export_name] = next_string_offset;
                    next_string_offset += export_name.length() + 1;
                }
                indent(); output << "i32.const " << string_literals[export_name] << "\n";
                indent(); output << "call $create_toggle\n";
            } else if (ui->component_type == "Dropdown") {
                if (!ui->children.empty()) {
                    generateExpression(ui->children[0].get());
                } else {
                    indent(); output << "i32.const 0\n";
                }
                if (!current_class_name.empty()) {
                    indent(); output << "local.get $this\n";
                } else {
                    indent(); output << "i32.const 0\n";
                }
                std::string method_name = "";
                for (const auto& arg : ui->named_args) {
                    if (arg.first == "onChange") {
                        if (auto* id = dynamic_cast<IdentifierNode*>(arg.second.get())) {
                            method_name = id->name;
                        }
                    }
                }
                std::string export_name = current_class_name.empty() ? method_name : (current_class_name + "_" + method_name);
                if (string_literals.find(export_name) == string_literals.end()) {
                    string_literals[export_name] = next_string_offset;
                    next_string_offset += export_name.length() + 1;
                }
                indent(); output << "i32.const " << string_literals[export_name] << "\n";
                indent(); output << "call $create_dropdown\n";
            } else if (ui->component_type == "Button") {
                if (!ui->children.empty()) {
                    generateExpression(ui->children[0].get());
                } else {
                    indent(); output << "i32.const 0\n";
                }
                if (!current_class_name.empty()) {
                    indent(); output << "local.get $this\n";
                } else {
                    indent(); output << "i32.const 0\n";
                }
                std::string method_name = "";
                for (const auto& arg : ui->named_args) {
                    if (arg.first == "onClick") {
                        if (auto* id = dynamic_cast<IdentifierNode*>(arg.second.get())) {
                            method_name = id->name;
                        }
                    }
                }
                std::string export_name = current_class_name.empty() ? method_name : (current_class_name + "_" + method_name);
                if (string_literals.find(export_name) == string_literals.end()) {
                    string_literals[export_name] = next_string_offset;
                    next_string_offset += export_name.length() + 1;
                }
                indent(); output << "i32.const " << string_literals[export_name] << "\n";
                indent(); output << "call $create_button\n";
            } else if (ui->component_type == "Image" || ui->component_type == "Video") {
                if (!ui->children.empty()) {
                    generateExpression(ui->children[0].get());
                } else {
                    indent(); output << "i32.const 0\n";
                }
                indent(); output << "call $create_" << (ui->component_type == "Image" ? "image" : "video") << "\n";
            } else if (ui->component_type == "Column" || ui->component_type == "Row" ||
                       ui->component_type == "Scrolling" || ui->component_type == "Card" ||
                       ui->component_type == "Container") {
                std::string creator_suffix = ui->component_type;
                if (creator_suffix == "Column") creator_suffix = "column";
                else if (creator_suffix == "Row") creator_suffix = "row";
                else if (creator_suffix == "Scrolling") creator_suffix = "scrolling";
                else if (creator_suffix == "Card") creator_suffix = "card";
                else if (creator_suffix == "Container") creator_suffix = "container";
                
                indent(); output << "call $create_" << creator_suffix << "\n";
            }
            
            // Store the created component node index
            indent(); output << "local.set " << local_name << "\n";
            
            // Append children if layout/container component
            if (ui->component_type == "Column" || ui->component_type == "Row" ||
                ui->component_type == "Scrolling" || ui->component_type == "Card" ||
                ui->component_type == "Container") {
                for (const auto& child : ui->children) {
                    indent(); output << "local.get " << local_name << "\n";
                    generateExpression(child.get());
                    indent(); output << "call $append_child\n";
                }
            }
            
            // Apply attributes/styling dynamically
            for (const auto& arg : ui->named_args) {
                if (arg.first == "onClick" && ui->component_type == "Button") {
                    continue; // onClick is handled inside create_button
                }
                if (arg.first == "onChange" && (ui->component_type == "Checkbox" || ui->component_type == "Slider" || ui->component_type == "Toggle" || ui->component_type == "Dropdown")) {
                    continue; // onChange is handled during creation
                }
                std::string key = arg.first;
                if (string_literals.find(key) == string_literals.end()) {
                    string_literals[key] = next_string_offset;
                    next_string_offset += key.length() + 1;
                }
                int key_offset = string_literals[key];
                
                indent(); output << "local.get " << local_name << "\n";
                indent(); output << "i32.const " << key_offset << "\n";
                generateExpression(arg.second.get());
                indent();
                if (isStringExpression(arg.second.get())) {
                    output << "call $set_attribute\n";
                } else {
                    output << "call $set_attribute_int\n";
                }
            }
            
            // Leave the element index on the stack
            indent(); output << "local.get " << local_name << "\n";
        } else {
            // Custom UI Component (Function Call)
            for (auto& arg : ui->named_args) generateExpression(arg.second.get());
            for (auto& child : ui->children) generateExpression(child.get());
            indent(); output << "call $" << ui->component_type << "\n";
        }
    }
    else if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        if (call->method_name == "println" || call->method_name == "print") {
            for (auto& arg : call->arguments) generateExpression(arg.get());
            indent(); output << "call $" << call->method_name << "\n";
        } else if (call->method_name == "render") {
            // Dynamic render dispatch: pass instance pointer (if any) and DOM Node index
            auto* inner_call = dynamic_cast<MethodCallNode*>(call->object.get());
            if (inner_call && inner_call->object) {
                generateExpression(inner_call->object.get());
            } else if (auto* inner_id = dynamic_cast<IdentifierNode*>(call->object.get())) {
                indent(); output << "local.get $" << inner_id->name << "\n";
            } else {
                indent(); output << "i32.const 0\n";
            }
            generateExpression(call->object.get());
            indent(); output << "call $render\n";
        } else {
            if (call->object) {
                // Method call on object instance
                generateExpression(call->object.get()); // Pushes instance pointer ($this)
                for (auto& arg : call->arguments) generateExpression(arg.get());
                
                std::string obj_type = "";
                if (auto* id = dynamic_cast<IdentifierNode*>(call->object.get())) {
                    obj_type = local_types[id->name];
                    std::cout << "[Codegen Debug] id=" << id->name << " obj_type=" << obj_type << " method=" << call->method_name << "\n";
                }
                
                // Intercept built-in collection methods
                if (obj_type == "List" && call->method_name == "push") {
                    indent(); output << "call $push\n";
                    indent(); output << "drop\n";
                } else if (interface_implementations.find(obj_type) != interface_implementations.end()) {
                    std::string resolved_type = obj_type;
                    for (const auto& impl : interface_implementations[obj_type]) {
                        if (class_methods[impl].count(call->method_name)) {
                            resolved_type = impl;
                            break;
                        }
                    }
                    indent(); output << "call $" << resolved_type << "_" << call->method_name << "\n";
                } else if (!obj_type.empty()) {
                    indent(); output << "call $" << obj_type << "_" << call->method_name << "\n";
                } else {
                    indent(); output << "call $" << call->method_name << "\n";
                }
            } else {
                for (auto& arg : call->arguments) generateExpression(arg.get());
                indent(); output << "call $" << call->method_name << "\n";
            }
        }
    }
}

void WASMCodeGenerator::generateStatement(ASTNode* stmt) {
    if (!stmt) return;
    
    if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt)) {
        if (var_decl->type) {
            local_types[var_decl->var_name] = var_decl->type->type_name;
        }
        // Evaluate initializer, then pop into local
        if (var_decl->initializer) {
            generateExpression(var_decl->initializer.get());
            indent();
            output << "local.set $" << var_decl->var_name << "\n";
        }
    } 
    else if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(stmt)) {
        generateExpression(return_stmt->expression.get());
        indent();
        output << "return\n";
    }
    else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(stmt)) {
        generateExpression(if_stmt->condition.get());
        indent();
        output << "(if\n";
        indent_level++;
        
        indent();
        output << "(then\n";
        indent_level++;
        for (const auto& s : if_stmt->then_branch) {
            generateStatement(s.get());
        }
        indent_level--;
        indent();
        output << ")\n";
        
        if (!if_stmt->else_branch.empty()) {
            indent();
            output << "(else\n";
            indent_level++;
            for (const auto& s : if_stmt->else_branch) {
                generateStatement(s.get());
            }
            indent_level--;
            indent();
            output << ")\n";
        }
        
        indent_level--;
        indent();
        output << ")\n";
    }
    else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(stmt)) {
        indent();
        output << "(block $exit_loop\n";
        indent_level++;
        indent();
        output << "(loop $start_loop\n";
        indent_level++;
        
        generateExpression(while_stmt->condition.get());
        indent();
        output << "i32.eqz\n";
        indent();
        output << "br_if $exit_loop\n";
        
        for (const auto& s : while_stmt->body) {
            generateStatement(s.get());
        }
        
        indent();
        output << "br $start_loop\n";
        
        indent_level--;
        indent();
        output << ")\n";
        indent_level--;
        indent();
        output << ")\n";
    }
    else if (auto* set_state = dynamic_cast<SetStateStmtNode*>(stmt)) {
        for (const auto& s : set_state->body) {
            generateStatement(s.get());
        }
        if (!current_class_name.empty()) {
            // Re-render class: local.get $this, (call build), call $render
            indent(); output << "local.get $this\n"; // instance pointer
            indent(); output << "local.get $this\n"; // pass as first arg to build
            indent(); output << "call $" << current_class_name << "_build\n";
            indent(); output << "call $render\n";
        }
    }
    else if (auto* expr = dynamic_cast<ExprNode*>(stmt)) {
        generateExpression(expr);
    }
}

void WASMCodeGenerator::generateFunction(FunctionNode* node) {
    auto* agentic = dynamic_cast<AgenticFunctionNode*>(node);
    if (agentic) {
        indent();
        std::string full_fn_name = current_class_name.empty() ? node->function_name : (current_class_name + "_" + node->function_name);
        output << "(func $" << full_fn_name << " (export \"" << full_fn_name << "\")\n";
        indent_level++;
        if (!current_class_name.empty()) {
            indent(); output << "(param $this i32)\n";
        }
        for (const auto& param : node->parameters) {
            indent(); output << "(param $" << param->var_name << " " << mapTypeToWASM(param->type.get()) << ")\n";
        }
        indent(); output << "(result i32)\n";
        if (!node->parameters.empty()) {
            indent(); output << "local.get $" << node->parameters[0]->var_name << "\n";
        } else {
            indent(); output << "i32.const 0\n";
        }
        indent(); output << "call $summarizeDocument\n";
        indent(); output << "return\n";
        indent_level--;
        indent(); output << ")\n";
        return;
    }

    indent();
    std::string full_fn_name = current_class_name.empty() ? node->function_name : (current_class_name + "_" + node->function_name);
    output << "(func $" << full_fn_name << " (export \"" << full_fn_name << "\")\n";
    indent_level++;
    
    // Implicit $this parameter for class methods
    if (!current_class_name.empty()) {
        indent();
        output << "(param $this i32)\n";
    }
    
    // Parameters
    for (const auto& param : node->parameters) {
        indent();
        output << "(param $" << param->var_name << " " << mapTypeToWASM(param->type.get()) << ")\n";
    }
    
    // Return type
    if (node->return_type && node->return_type->type_name != "Void") {
        indent();
        output << "(result " << mapTypeToWASM(node->return_type.get()) << ")\n";
    }
    
    // We must declare all locals at the top in WASM
    int total_ui_nodes = 0;
    for (const auto& stmt : node->body) {
        if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt.get())) {
            indent();
            output << "(local $" << var_decl->var_name << " " << mapTypeToWASM(var_decl->type.get()) << ")\n";
            if (var_decl->type) {
                local_types[var_decl->var_name] = var_decl->type->type_name;
            }
        }
        total_ui_nodes += countUINodes(stmt.get());
    }
    
    // Declare temporary locals for UI building
    for (int i = 0; i < total_ui_nodes; i++) {
        indent(); output << "(local $tmp_ui_" << i << " i32)\n";
    }
    ui_node_counter = 0;
    
    // Function body
    for (const auto& stmt : node->body) {
        generateStatement(stmt.get());
    }
    
    indent_level--;
    indent();
    output << ")\n";
}

void WASMCodeGenerator::generateClass(ClassDeclNode* node) {
    current_class_name = node->class_name;
    
    // Map constructor args and custom fields to memory offsets
    int offset = 0;
    for (const auto& arg : node->primary_constructor_args) {
        class_field_offsets[node->class_name][arg->var_name] = offset;
        if (arg->type) {
            class_field_types[node->class_name][arg->var_name] = arg->type->type_name;
            offset += (arg->type->type_name == "Float") ? 8 : 4;
        } else {
            offset += 4;
        }
    }
    for (const auto& field : node->fields) {
        class_field_offsets[node->class_name][field->var_name] = offset;
        if (field->type) {
            class_field_types[node->class_name][field->var_name] = field->type->type_name;
            offset += (field->type->type_name == "Float") ? 8 : 4;
        } else {
            offset += 4;
        }
    }
    class_sizes[node->class_name] = offset;
    
    // Generate constructor function
    indent(); output << "(func $" << node->class_name << " (export \"" << node->class_name << "\")\n";
    indent_level++;
    for (const auto& arg : node->primary_constructor_args) {
        indent(); output << "(param $" << arg->var_name << " " << mapTypeToWASM(arg->type.get()) << ")\n";
    }
    indent(); output << "(result i32)\n";
    indent(); output << "(local $this i32)\n";
    
    // Allocate memory from global free_mem pointer
    indent(); output << "global.get $free_mem\n";
    indent(); output << "local.set $this\n";
    
    // Store constructor arguments
    for (const auto& arg : node->primary_constructor_args) {
        indent(); output << "local.get $this\n";
        indent(); output << "local.get $" << arg->var_name << "\n";
        if (arg->type && arg->type->type_name == "Float") {
            indent(); output << "f64.store offset=" << class_field_offsets[node->class_name][arg->var_name] << "\n";
        } else {
            indent(); output << "i32.store offset=" << class_field_offsets[node->class_name][arg->var_name] << "\n";
        }
    }
    
    // Store custom field initializers
    for (const auto& field : node->fields) {
        if (field->initializer) {
            indent(); output << "local.get $this\n";
            generateExpression(field->initializer.get());
            if (field->type && field->type->type_name == "Float") {
                indent(); output << "f64.store offset=" << class_field_offsets[node->class_name][field->var_name] << "\n";
            } else {
                indent(); output << "i32.store offset=" << class_field_offsets[node->class_name][field->var_name] << "\n";
            }
        }
    }
    
    // Advance free_mem pointer
    indent(); output << "global.get $free_mem\n";
    indent(); output << "i32.const " << (class_sizes[node->class_name] > 0 ? class_sizes[node->class_name] : 4) << "\n";
    indent(); output << "i32.add\n";
    indent(); output << "global.set $free_mem\n";
    
    indent(); output << "local.get $this\n";
    indent(); output << "return\n";
    indent_level--;
    indent(); output << ")\n\n";
    
    // Compile class methods
    for (const auto& method : node->methods) {
        generateFunction(method.get());
    }
    
    current_class_name = "";
}

std::string WASMCodeGenerator::generate(ProgramNode* program) {
    output.str("");
    output.clear();
    string_literals.clear();
    next_string_offset = 1024;
    ui_node_counter = 0;
    current_class_name = "";
    class_field_offsets.clear();
    class_field_types.clear();
    class_sizes.clear();
    local_types.clear();
    interface_implementations.clear();
    class_methods.clear();
    
    // Pre-pass: map interfaces and methods
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            for (const auto& intf : class_decl->implemented_interfaces) {
                interface_implementations[intf].push_back(class_decl->class_name);
            }
            for (const auto& method : class_decl->methods) {
                class_methods[class_decl->class_name].insert(method->function_name);
            }
        }
    }
    
    output << "(module\n";
    indent_level++;
    
    // Core WASM imports (JS Environment bindings)
    indent(); output << "(import \"env\" \"print\" (func $print (param i32)))\n";
    indent(); output << "(import \"env\" \"println\" (func $println (param i32)))\n";
    indent(); output << "(import \"env\" \"create_column\" (func $create_column (result i32)))\n";
    indent(); output << "(import \"env\" \"create_row\" (func $create_row (result i32)))\n";
    indent(); output << "(import \"env\" \"create_text\" (func $create_text (param i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"append_child\" (func $append_child (param i32 i32)))\n";
    indent(); output << "(import \"env\" \"render\" (func $render (param i32 i32)))\n";
    indent(); output << "(import \"env\" \"push\" (func $push (param i32)))\n";
    indent(); output << "(import \"env\" \"summarizeDocument\" (func $summarizeDocument (param i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"concat_int\" (func $concat_int (param i32 i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"concat_float\" (func $concat_float (param i32 f64) (result i32)))\n";
    indent(); output << "(import \"env\" \"create_button\" (func $create_button (param i32 i32 i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"httpGet\" (func $httpGet (param i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"httpPost\" (func $httpPost (param i32 i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"create_image\" (func $create_image (param i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"create_video\" (func $create_video (param i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"create_scrolling\" (func $create_scrolling (result i32)))\n";
    indent(); output << "(import \"env\" \"create_card\" (func $create_card (result i32)))\n";
    indent(); output << "(import \"env\" \"create_container\" (func $create_container (result i32)))\n";
    indent(); output << "(import \"env\" \"create_checkbox\" (func $create_checkbox (param i32 i32 i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"create_slider\" (func $create_slider (param i32 i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"create_toggle\" (func $create_toggle (param i32 i32 i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"create_dropdown\" (func $create_dropdown (param i32 i32 i32) (result i32)))\n";
    indent(); output << "(import \"env\" \"set_attribute\" (func $set_attribute (param i32 i32 i32)))\n";
    indent(); output << "(import \"env\" \"set_attribute_int\" (func $set_attribute_int (param i32 i32 i32)))\n";
    
    // Linear Memory Declaration (1 page = 64KB)
    indent();
    output << "(memory (export \"memory\") 1)\n";
    // Global allocator pointer starts at 8192 (leaving 1024-8192 for static strings)
    indent();
    output << "(global $free_mem (mut i32) (i32.const 8192))\n\n";

    // Visit AST
    for (const auto& stmt : program->statements) {
        if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            generateFunction(fn_decl);
        } else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            generateClass(class_decl);
        }
    }
    
    // Emit String Literals into Data Section
    for (const auto& pair : string_literals) {
        indent();
        output << "(data (i32.const " << pair.second << ") \"" << pair.first << "\\00\")\n";
    }
    
    indent_level--;
    output << ")\n";
    return output.str();
}

std::string WASMCodeGenerator::generateHTMLWrapper() {
    std::stringstream html;
    html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
    html << "    <meta charset=\"UTF-8\">\n    <title>Zenith WASM Target</title>\n";
    html << "    <style>\n";
    html << "        body {\n";
    html << "            background: #090d16;\n";
    html << "            color: #f8fafc;\n";
    html << "            font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, sans-serif;\n";
    html << "            margin: 0;\n";
    html << "            padding: 24px;\n";
    html << "            display: flex;\n";
    html << "            flex-direction: column;\n";
    html << "            align-items: center;\n";
    html << "            min-height: 100vh;\n";
    html << "        }\n";
    html << "        .header {\n";
    html << "            margin-bottom: 24px;\n";
    html << "            text-align: center;\n";
    html << "        }\n";
    html << "        .header h1 {\n";
    html << "            color: #38bdf8;\n";
    html << "            font-size: 2rem;\n";
    html << "            margin: 0 0 8px 0;\n";
    html << "            letter-spacing: 1px;\n";
    html << "            text-transform: uppercase;\n";
    html << "        }\n";
    html << "        .header p {\n";
    html << "            color: #94a3b8;\n";
    html << "            margin: 0;\n";
    html << "            font-size: 0.95rem;\n";
    html << "        }\n";
    html << "        .container {\n";
    html << "            display: flex;\n";
    html << "            gap: 24px;\n";
    html << "            width: 100%;\n";
    html << "            max-width: 1200px;\n";
    html << "            height: 600px;\n";
    html << "        }\n";
    html << "        .card {\n";
    html << "            flex: 1;\n";
    html << "            background: rgba(15, 23, 42, 0.6);\n";
    html << "            backdrop-filter: blur(12px);\n";
    html << "            border: 1px solid rgba(56, 189, 248, 0.2);\n";
    html << "            border-radius: 16px;\n";
    html << "            padding: 24px;\n";
    html << "            display: flex;\n";
    html << "            flex-direction: column;\n";
    html << "            box-shadow: 0 10px 30px rgba(0,0,0,0.5);\n";
    html << "        }\n";
    html << "        .card h3 {\n";
    html << "            color: #38bdf8;\n";
    html << "            margin-top: 0;\n";
    html << "            margin-bottom: 16px;\n";
    html << "            border-bottom: 1px solid rgba(56, 189, 248, 0.1);\n";
    html << "            padding-bottom: 8px;\n";
    html << "            font-size: 1.1rem;\n";
    html << "            text-transform: uppercase;\n";
    html << "            letter-spacing: 0.5px;\n";
    html << "        }\n";
    html << "        #app-root {\n";
    html << "            flex: 1;\n";
    html << "            display: flex;\n";
    html << "            flex-direction: column;\n";
    html << "            justify-content: center;\n";
    html << "            align-items: center;\n";
    html << "            background: #020617;\n";
    html << "            border-radius: 12px;\n";
    html << "            border: 1px solid rgba(255, 255, 255, 0.05);\n";
    html << "            padding: 20px;\n";
    html << "            color: #e2e8f0;\n";
    html << "        }\n";
    html << "        #console {\n";
    html << "            flex: 1;\n";
    html << "            background: #020617;\n";
    html << "            border: 1px solid rgba(255, 255, 255, 0.05);\n";
    html << "            border-radius: 16px;\n";
    html << "            padding: 20px;\n";
    html << "            font-family: monospace;\n";
    html << "            font-size: 0.85rem;\n";
    html << "            color: #a7f3d0;\n";
    html << "            overflow-y: auto;\n";
    html << "            box-shadow: 0 10px 30px rgba(0,0,0,0.5);\n";
    html << "        }\n";
    html << "        .log-item {\n";
    html << "            margin-bottom: 6px;\n";
    html << "            line-height: 1.4;\n";
    html << "        }\n";
    html << "        .log-system { color: #64748b; }\n";
    html << "        .log-wasm { color: #f472b6; }\n";
    html << "        .log-event { color: #fbbf24; }\n";
    html << "    </style>\n";
    html << "    <!-- WABT.js to compile .wat to .wasm natively in browser -->\n";
    html << "    <script src=\"https://cdn.jsdelivr.net/gh/AssemblyScript/wabt.js/index.js\"></script>\n";
    html << "</head>\n<body>\n";
    html << "    <div class=\"header\">\n";
    html << "        <h1>Zenith WebAssembly Engine</h1>\n";
    html << "        <p>Statically compiled Zenith application running inside WebAssembly sandbox</p>\n";
    html << "    </div>\n";
    html << "    <div class=\"container\">\n";
    html << "        <div class=\"card\">\n";
    html << "            <h3>Live Application</h3>\n";
    html << "            <div id=\"app-root\"></div>\n";
    html << "        </div>\n";
    html << "        <div id=\"console\"></div>\n";
    html << "    </div>\n";
    html << "    <script>\n";
    html << "        function logToConsole(msg, type = 'system') {\n";
    html << "            const consoleEl = document.getElementById('console');\n";
    html << "            if (consoleEl) {\n";
    html << "                const item = document.createElement('div');\n";
    html << "                item.className = 'log-item log-' + type;\n";
    html << "                item.innerHTML = msg;\n";
    html << "                consoleEl.appendChild(item);\n";
    html << "                consoleEl.scrollTop = consoleEl.scrollHeight;\n";
    html << "            }\n";
    html << "        }\n";
    html << "        let wasmMemory;\n";
    html << "        let wasmInstance;\n";
    html << "        function readString(ptr) {\n";
    html << "            if (!wasmMemory) return 'null';\n";
    html << "            const bytes = new Uint8Array(wasmMemory.buffer);\n";
    html << "            let str = '';\n";
    html << "            while (bytes[ptr] !== 0) {\n";
    html << "                str += String.fromCharCode(bytes[ptr]);\n";
    html << "                ptr++;\n";
    html << "            }\n";
    html << "            return str;\n";
    html << "        }\n";
    html << "        function writeStringToMemory(str, ptr) {\n";
    html << "            if (!wasmMemory) return 0;\n";
    html << "            const bytes = new Uint8Array(wasmMemory.buffer);\n";
    html << "            for (let i = 0; i < str.length; i++) {\n";
    html << "                bytes[ptr + i] = str.charCodeAt(i);\n";
    html << "            }\n";
    html << "            bytes[ptr + str.length] = 0;\n";
    html << "            return ptr;\n";
    html << "        }\n";
    html << "        \n";
    html << "        const DOMNodes = [null]; // Index 0 is null\n";
    html << "        const InstanceDOM = {}; // Map of class instance pointer -> DOM element\n";
    html << "        const importObject = {\n";
    html << "            env: {\n";
    html << "                print: (ptr) => logToConsole(`[WASM print]: ${readString(ptr)}`, 'wasm'),\n";
    html << "                println: (ptr) => logToConsole(`[WASM println]: ${readString(ptr)}`, 'wasm'),\n";
    html << "                create_column: () => {\n";
    html << "                    const el = document.createElement('div');\n";
    html << "                    el.style.display = 'flex'; el.style.flexDirection = 'column';\n";
    html << "                    el.style.border = '1px solid rgba(56, 189, 248, 0.3)'; el.style.padding = '15px'; el.style.margin = '5px';\n";
    html << "                    el.style.borderRadius = '10px';\n";
    html << "                    el.style.background = 'rgba(56, 189, 248, 0.05)';\n";
    html << "                    DOMNodes.push(el); return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_row: () => {\n";
    html << "                    const el = document.createElement('div');\n";
    html << "                    el.style.display = 'flex'; el.style.flexDirection = 'row'; el.style.gap = '10px';\n";
    html << "                    DOMNodes.push(el); return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_text: (ptr) => {\n";
    html << "                    const el = document.createElement('span');\n";
    html << "                    el.textContent = readString(ptr);\n";
    html << "                    DOMNodes.push(el); return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                append_child: (parentIdx, childIdx) => {\n";
    html << "                    if (DOMNodes[parentIdx] && DOMNodes[childIdx]) {\n";
    html << "                        DOMNodes[parentIdx].appendChild(DOMNodes[childIdx]);\n";
    html << "                    }\n";
    html << "                },\n";
    html << "                render: (instancePtr, nodeIdx) => {\n";
    html << "                    if (DOMNodes[nodeIdx]) {\n";
    html << "                        if (instancePtr && InstanceDOM[instancePtr]) {\n";
    html << "                            // Replace old node with new node\n";
    html << "                            const oldNode = InstanceDOM[instancePtr];\n";
    html << "                            if (oldNode.parentNode) {\n";
    html << "                                oldNode.parentNode.replaceChild(DOMNodes[nodeIdx], oldNode);\n";
    html << "                            }\n";
    html << "                            InstanceDOM[instancePtr] = DOMNodes[nodeIdx];\n";
    html << "                            logToConsole('<i>[Runtime] setState: Re-rendering UI Layout...</i>', 'system');\n";
    html << "                        } else {\n";
    html << "                            const appRoot = document.getElementById('app-root');\n";
    html << "                            appRoot.innerHTML = '';\n";
    html << "                            appRoot.appendChild(DOMNodes[nodeIdx]);\n";
    html << "                            if (instancePtr) {\n";
    html << "                                InstanceDOM[instancePtr] = DOMNodes[nodeIdx];\n";
    html << "                            }\n";
    html << "                            logToConsole('<i>-- UI Component Successfully Rendered to DOM --</i>', 'system');\n";
    html << "                        }\n";
    html << "                    }\n";
    html << "                },\n";
    html << "                push: (val) => {},\n";
    html << "                summarizeDocument: (ptr) => { logToConsole(`[Agentic] mock summarizeDocument: ${readString(ptr)}`, 'wasm'); return ptr; },\n";
    html << "                concat_int: (strPtr, val) => {\n";
    html << "                    const prefix = readString(strPtr);\n";
    html << "                    const result = prefix + val;\n";
    html << "                    const bytes = new Uint8Array(wasmMemory.buffer);\n";
    html << "                    let ptr = 4096;\n";
    html << "                    for (let i = 0; i < result.length; i++) {\n";
    html << "                        bytes[ptr + i] = result.charCodeAt(i);\n";
    html << "                    }\n";
    html << "                    bytes[ptr + result.length] = 0;\n";
    html << "                    return ptr;\n";
    html << "                },\n";
    html << "                concat_float: (strPtr, val) => {\n";
    html << "                    const prefix = readString(strPtr);\n";
    html << "                    let strVal = val.toString();\n";
    html << "                    if (strVal.includes('.')) {\n";
    html << "                        const parts = strVal.split('.');\n";
    html << "                        if (parts[1].length > 6) strVal = val.toFixed(6);\n";
    html << "                    }\n";
    html << "                    const result = prefix + strVal;\n";
    html << "                    const bytes = new Uint8Array(wasmMemory.buffer);\n";
    html << "                    let ptr = 6144;\n";
    html << "                    for (let i = 0; i < result.length; i++) {\n";
    html << "                        bytes[ptr + i] = result.charCodeAt(i);\n";
    html << "                    }\n";
    html << "                    bytes[ptr + result.length] = 0;\n";
    html << "                    return ptr;\n";
    html << "                },\n";
    html << "                create_button: (textPtr, instancePtr, methodPtr) => {\n";
    html << "                    const label = readString(textPtr);\n";
    html << "                    const methodName = readString(methodPtr);\n";
    html << "                    const el = document.createElement('button');\n";
    html << "                    el.textContent = label;\n";
    html << "                    el.style.padding = '8px 16px';\n";
    html << "                    el.style.margin = '5px';\n";
    html << "                    el.style.border = 'none';\n";
    html << "                    el.style.borderRadius = '6px';\n";
    html << "                    el.style.background = '#38bdf8';\n";
    html << "                    el.style.color = '#0f172a';\n";
    html << "                    el.style.fontWeight = 'bold';\n";
    html << "                    el.style.cursor = 'pointer';\n";
    html << "                    el.style.transition = 'all 0.2s';\n";
    html << "                    el.onmouseover = () => { el.style.background = '#0ea5e9'; };\n";
    html << "                    el.onmouseout = () => { el.style.background = '#38bdf8'; };\n";
    html << "                    el.onclick = () => {\n";
    html << "                        if (wasmInstance && wasmInstance.exports[methodName]) {\n";
    html << "                            logToConsole(`[Event] Button Clicked: calling ${methodName}(instancePtr: ${instancePtr})`, 'event');\n";
    html << "                            wasmInstance.exports[methodName](instancePtr);\n";
    html << "                        } else {\n";
    html << "                            logToConsole(`[Event Error] Exported method not found: ${methodName}`, 'system');\n";
    html << "                        }\n";
    html << "                    };\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_checkbox: (textPtr, instancePtr, methodPtr) => {\n";
    html << "                    const labelText = readString(textPtr);\n";
    html << "                    const methodName = readString(methodPtr);\n";
    html << "                    const label = document.createElement('label');\n";
    html << "                    label.style.display = 'flex';\n";
    html << "                    label.style.alignItems = 'center';\n";
    html << "                    label.style.gap = '8px';\n";
    html << "                    label.style.cursor = 'pointer';\n";
    html << "                    label.style.color = '#e2e8f0';\n";
    html << "                    label.style.margin = '5px';\n";
    html << "                    const input = document.createElement('input');\n";
    html << "                    input.type = 'checkbox';\n";
    html << "                    input.style.cursor = 'pointer';\n";
    html << "                    input.style.width = '18px';\n";
    html << "                    input.style.height = '18px';\n";
    html << "                    input.style.accentColor = '#38bdf8';\n";
    html << "                    input.onchange = () => {\n";
    html << "                        if (wasmInstance && wasmInstance.exports[methodName]) {\n";
    html << "                            logToConsole(`[Event] Checkbox Changed: calling ${methodName}(instancePtr: ${instancePtr}, checked: ${input.checked})`, 'event');\n";
    html << "                            wasmInstance.exports[methodName](instancePtr, input.checked ? 1 : 0);\n";
    html << "                        }\n";
    html << "                    };\n";
    html << "                    label.appendChild(input);\n";
    html << "                    const textSpan = document.createElement('span');\n";
    html << "                    textSpan.textContent = labelText;\n";
    html << "                    label.appendChild(textSpan);\n";
    html << "                    DOMNodes.push(label);\n";
    html << "                    label.inputElement = input;\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_slider: (instancePtr, methodPtr) => {\n";
    html << "                    const methodName = readString(methodPtr);\n";
    html << "                    const el = document.createElement('input');\n";
    html << "                    el.type = 'range';\n";
    html << "                    el.style.margin = '5px';\n";
    html << "                    el.style.accentColor = '#38bdf8';\n";
    html << "                    el.style.background = 'rgba(255,255,255,0.1)';\n";
    html << "                    el.oninput = () => {\n";
    html << "                        if (wasmInstance && wasmInstance.exports[methodName]) {\n";
    html << "                            logToConsole(`[Event] Slider Changed: calling ${methodName}(instancePtr: ${instancePtr}, value: ${el.value})`, 'event');\n";
    html << "                            wasmInstance.exports[methodName](instancePtr, parseInt(el.value, 10));\n";
    html << "                        }\n";
    html << "                    };\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    el.inputElement = el;\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_toggle: (textPtr, instancePtr, methodPtr) => {\n";
    html << "                    const labelText = readString(textPtr);\n";
    html << "                    const methodName = readString(methodPtr);\n";
    html << "                    const label = document.createElement('label');\n";
    html << "                    label.style.display = 'flex';\n";
    html << "                    label.style.alignItems = 'center';\n";
    html << "                    label.style.gap = '10px';\n";
    html << "                    label.style.cursor = 'pointer';\n";
    html << "                    label.style.margin = '5px';\n";
    html << "                    const switchDiv = document.createElement('div');\n";
    html << "                    switchDiv.style.position = 'relative';\n";
    html << "                    switchDiv.style.width = '44px';\n";
    html << "                    switchDiv.style.height = '24px';\n";
    html << "                    switchDiv.style.backgroundColor = '#475569';\n";
    html << "                    switchDiv.style.borderRadius = '12px';\n";
    html << "                    switchDiv.style.transition = 'background-color 0.2s';\n";
    html << "                    const knob = document.createElement('div');\n";
    html << "                    knob.style.position = 'absolute';\n";
    html << "                    knob.style.top = '2px';\n";
    html << "                    knob.style.left = '2px';\n";
    html << "                    knob.style.width = '20px';\n";
    html << "                    knob.style.height = '20px';\n";
    html << "                    knob.style.borderRadius = '50%';\n";
    html << "                    knob.style.backgroundColor = '#ffffff';\n";
    html << "                    knob.style.transition = 'transform 0.2s';\n";
    html << "                    switchDiv.appendChild(knob);\n";
    html << "                    let isOn = false;\n";
    html << "                    const updateToggleUI = (state) => {\n";
    html << "                        isOn = state;\n";
    html << "                        if (isOn) {\n";
    html << "                            switchDiv.style.backgroundColor = '#10b981';\n";
    html << "                            knob.style.transform = 'translateX(20px)';\n";
    html << "                        } else {\n";
    html << "                            switchDiv.style.backgroundColor = '#475569';\n";
    html << "                            knob.style.transform = 'translateX(0)';\n";
    html << "                        }\n";
    html << "                    };\n";
    html << "                    label.onclick = (e) => {\n";
    html << "                        e.preventDefault();\n";
    html << "                        updateToggleUI(!isOn);\n";
    html << "                        if (wasmInstance && wasmInstance.exports[methodName]) {\n";
    html << "                            logToConsole(`[Event] Toggle Changed: calling ${methodName}(instancePtr: ${instancePtr}, isOn: ${isOn})`, 'event');\n";
    html << "                            wasmInstance.exports[methodName](instancePtr, isOn ? 1 : 0);\n";
    html << "                        }\n";
    html << "                    };\n";
    html << "                    label.appendChild(switchDiv);\n";
    html << "                    if (labelText) {\n";
    html << "                        const textSpan = document.createElement('span');\n";
    html << "                        textSpan.textContent = labelText;\n";
    html << "                        textSpan.style.color = '#e2e8f0';\n";
    html << "                        label.appendChild(textSpan);\n";
    html << "                    }\n";
    html << "                    label.setToggleState = updateToggleUI;\n";
    html << "                    label.inputElement = label;\n";
    html << "                    DOMNodes.push(label);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_dropdown: (optionsPtr, instancePtr, methodPtr) => {\n";
    html << "                    const optionsStr = readString(optionsPtr);\n";
    html << "                    const methodName = readString(methodPtr);\n";
    html << "                    const el = document.createElement('select');\n";
    html << "                    el.className = 'zenith-input';\n";
    html << "                    el.style.background = 'rgba(15, 23, 42, 0.8)';\n";
    html << "                    el.style.color = '#f8fafc';\n";
    html << "                    el.style.border = '1px solid rgba(0, 242, 254, 0.3)';\n";
    html << "                    el.style.borderRadius = '8px';\n";
    html << "                    el.style.padding = '8px 12px';\n";
    html << "                    el.style.cursor = 'pointer';\n";
    html << "                    const populateOptions = (str) => {\n";
    html << "                        el.innerHTML = '';\n";
    html << "                        const opts = str.split(',');\n";
    html << "                        for (const opt of opts) {\n";
    html << "                            const trimmed = opt.trim();\n";
    html << "                            if (!trimmed) continue;\n";
    html << "                            const optionEl = document.createElement('option');\n";
    html << "                            optionEl.value = trimmed;\n";
    html << "                            optionEl.textContent = trimmed;\n";
    html << "                            el.appendChild(optionEl);\n";
    html << "                        }\n";
    html << "                    };\n";
    html << "                    if (optionsStr) {\n";
    html << "                        populateOptions(optionsStr);\n";
    html << "                    }\n";
    html << "                    el.onchange = () => {\n";
    html << "                        if (wasmInstance && wasmInstance.exports[methodName]) {\n";
    html << "                            const strPtr = writeStringToMemory(el.value, 36864);\n";
    html << "                            logToConsole(`[Event] Dropdown Changed: calling ${methodName}(instancePtr: ${instancePtr}, value: \"${el.value}\")`, 'event');\n";
    html << "                            wasmInstance.exports[methodName](instancePtr, strPtr);\n";
    html << "                        }\n";
    html << "                    };\n";
    html << "                    el.inputElement = el;\n";
    html << "                    el.populateOptions = populateOptions;\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                httpGet: (urlPtr) => {\n";
    html << "                    const url = readString(urlPtr);\n";
    html << "                    logToConsole(`[Network] httpGet: Fetching ${url}...`, 'system');\n";
    html << "                    try {\n";
    html << "                        const xhr = new XMLHttpRequest();\n";
    html << "                        xhr.open('GET', url, false);\n";
    html << "                        xhr.send(null);\n";
    html << "                        if (xhr.status >= 200 && xhr.status < 300) {\n";
    html << "                            return writeStringToMemory(xhr.responseText, 32768);\n";
    html << "                        } else {\n";
    html << "                            return writeStringToMemory(`{\\\"error\\\": \\\"HTTP ${xhr.status}\\\"}`, 32768);\n";
    html << "                        }\n";
    html << "                    } catch (e) {\n";
    html << "                        logToConsole(`[Network Error/CORS] Using mock response for ${url}`, 'system');\n";
    html << "                        if (url.indexOf(\\\"users\\\") !== -1) {\n";
    html << "                            return writeStringToMemory(`{\\\"users\\\": [\\\"Sam\\\", \\\"Jay\\\", \\\"Alex\\\"], \\\"status\\\": \\\"active\\\"}`, 32768);\n";
    html << "                        }\n";
    html << "                        return writeStringToMemory(`{\\\"message\\\": \\\"Hello from Zenith WASM mock endpoint!\\\", \\\"status\\\": \\\"success\\\"}`, 32768);\n";
    html << "                    }\n";
    html << "                },\n";
    html << "                httpPost: (urlPtr, bodyPtr) => {\n";
    html << "                    const url = readString(urlPtr);\n";
    html << "                    const body = readString(bodyPtr);\n";
    html << "                    logToConsole(`[Network] httpPost: Posting to ${url}...`, 'system');\n";
    html << "                    try {\n";
    html << "                        const xhr = new XMLHttpRequest();\n";
    html << "                        xhr.open('POST', url, false);\n";
    html << "                        xhr.setRequestHeader('Content-Type', 'application/json');\n";
    html << "                        xhr.send(body);\n";
    html << "                        if (xhr.status >= 200 && xhr.status < 300) {\n";
    html << "                            return writeStringToMemory(xhr.responseText, 32768);\n";
    html << "                        } else {\n";
    html << "                            return writeStringToMemory(`{\\\"error\\\": \\\"HTTP ${xhr.status}\\\"}`, 32768);\n";
    html << "                        }\n";
    html << "                    } catch (e) {\n";
    html << "                        logToConsole(`[Network Error/CORS] Using mock response for POST ${url}`, 'system');\n";
    html << "                        return writeStringToMemory(`{\\\"status\\\": \\\"posted\\\", \\\"received\\\": ${body}}`, 32768);\n";
    html << "                    }\n";
    html << "                },\n";
    html << "                create_image: (urlPtr) => {\n";
    html << "                    const el = document.createElement('img');\n";
    html << "                    el.src = readString(urlPtr);\n";
    html << "                    el.style.maxWidth = '100%';\n";
    html << "                    el.style.borderRadius = '8px';\n";
    html << "                    el.style.boxShadow = '0 4px 6px -1px rgba(0,0,0,0.1)';\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_video: (urlPtr) => {\n";
    html << "                    const el = document.createElement('video');\n";
    html << "                    el.src = readString(urlPtr);\n";
    html << "                    el.controls = true;\n";
    html << "                    el.style.maxWidth = '100%';\n";
    html << "                    el.style.borderRadius = '8px';\n";
    html << "                    el.style.boxShadow = '0 4px 6px -1px rgba(0,0,0,0.1)';\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_scrolling: () => {\n";
    html << "                    const el = document.createElement('div');\n";
    html << "                    el.style.display = 'flex';\n";
    html << "                    el.style.flexDirection = 'column';\n";
    html << "                    el.style.overflowY = 'auto';\n";
    html << "                    el.style.maxHeight = '200px';\n";
    html << "                    el.style.border = '1px solid rgba(255, 255, 255, 0.1)';\n";
    html << "                    el.style.padding = '8px';\n";
    html << "                    el.style.borderRadius = '8px';\n";
    html << "                    el.style.background = 'rgba(0, 0, 0, 0.2)';\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_card: () => {\n";
    html << "                    const el = document.createElement('div');\n";
    html << "                    el.style.display = 'flex';\n";
    html << "                    el.style.flexDirection = 'column';\n";
    html << "                    el.style.background = 'rgba(30, 41, 59, 0.7)';\n";
    html << "                    el.style.border = '1px solid rgba(56, 189, 248, 0.3)';\n";
    html << "                    el.style.borderRadius = '12px';\n";
    html << "                    el.style.padding = '16px';\n";
    html << "                    el.style.margin = '8px 0';\n";
    html << "                    el.style.boxShadow = '0 10px 15px -3px rgba(0,0,0,0.3)';\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                create_container: () => {\n";
    html << "                    const el = document.createElement('div');\n";
    html << "                    el.style.display = 'flex';\n";
    html << "                    el.style.flexDirection = 'column';\n";
    html << "                    el.style.border = '1px solid rgba(255, 255, 255, 0.05)';\n";
    html << "                    el.style.borderRadius = '8px';\n";
    html << "                    DOMNodes.push(el);\n";
    html << "                    return DOMNodes.length - 1;\n";
    html << "                },\n";
    html << "                set_attribute: (nodeIdx, keyPtr, valPtr) => {\n";
    html << "                    let el = DOMNodes[nodeIdx];\n";
    html << "                    if (!el) return;\n";
    html << "                    const key = readString(keyPtr);\n";
    html << "                    const val = readString(valPtr);\n";
    html << "                    if (el.inputElement && (key === 'checked' || key === 'disabled' || key === 'value' || key === 'isOn' || key === 'options')) {\n";
    html << "                        el = el.inputElement;\n";
    html << "                    }\n";
    html << "                    if (key === 'color') {\n";
    html << "                        el.style.color = val;\n";
    html << "                    } else if (key === 'backgroundColor') {\n";
    html << "                        el.style.backgroundColor = val;\n";
    html << "                    } else if (key === 'fontWeight') {\n";
    html << "                        el.style.fontWeight = val;\n";
    html << "                    } else if (key === 'width') {\n";
    html << "                        el.style.width = val;\n";
    html << "                    } else if (key === 'height') {\n";
    html << "                        el.style.height = val;\n";
    html << "                    } else if (key === 'flexDirection') {\n";
    html << "                        el.style.flexDirection = val;\n";
    html << "                    } else if (key === 'justifyContent') {\n";
    html << "                        el.style.justifyContent = val;\n";
    html << "                    } else if (key === 'alignItems') {\n";
    html << "                        el.style.alignItems = val;\n";
    html << "                    } else if (key === 'src' || key === 'url') {\n";
    html << "                        el.src = val;\n";
    html << "                    } else if (key === 'checked') {\n";
    html << "                        el.checked = (val === 'true' || val === '1');\n";
    html << "                    } else if (key === 'isOn') {\n";
    html << "                        if (el.setToggleState) el.setToggleState(val === 'true' || val === '1');\n";
    html << "                    } else if (key === 'value') {\n";
    html << "                        el.value = val;\n";
    html << "                    } else if (key === 'options') {\n";
    html << "                        if (el.populateOptions) el.populateOptions(val);\n";
    html << "                    } else {\n";
    html << "                        el.setAttribute(key, val);\n";
    html << "                    }\n";
    html << "                },\n";
    html << "                set_attribute_int: (nodeIdx, keyPtr, val) => {\n";
    html << "                    let el = DOMNodes[nodeIdx];\n";
    html << "                    if (!el) return;\n";
    html << "                    const key = readString(keyPtr);\n";
    html << "                    if (el.inputElement && (key === 'checked' || key === 'disabled' || key === 'value' || key === 'isOn' || key === 'min' || key === 'max')) {\n";
    html << "                        el = el.inputElement;\n";
    html << "                    }\n";
    html << "                    if (key === 'padding') {\n";
    html << "                        el.style.padding = val + 'px';\n";
    html << "                    } else if (key === 'margin') {\n";
    html << "                        el.style.margin = val + 'px';\n";
    html << "                    } else if (key === 'width') {\n";
    html << "                        el.style.width = val + 'px';\n";
    html << "                    } else if (key === 'height') {\n";
    html << "                        el.style.height = val + 'px';\n";
    html << "                    } else if (key === 'flexGrow') {\n";
    html << "                        el.style.flexGrow = val;\n";
    html << "                    } else if (key === 'gap') {\n";
    html << "                        el.style.gap = val + 'px';\n";
    html << "                    } else if (key === 'checked') {\n";
    html << "                        el.checked = (val !== 0);\n";
    html << "                    } else if (key === 'isOn') {\n";
    html << "                        if (el.setToggleState) el.setToggleState(val !== 0);\n";
    html << "                    } else if (key === 'value') {\n";
    html << "                        el.value = val;\n";
    html << "                    } else if (key === 'min') {\n";
    html << "                        el.min = val;\n";
    html << "                    } else if (key === 'max') {\n";
    html << "                        el.max = val;\n";
    html << "                    } else {\n";
    html << "                        el.setAttribute(key, val.toString());\n";
    html << "                    }\n";
    html << "                }\n";
    html << "            }\n";
    html << "        };\n\n";
    html << "        async function loadWasm() {\n";
    html << "            try {\n";
    html << "                // Fetch the generated .wat file dynamically based on page name\n";
    html << "                const watName = window.location.pathname.split('/').pop().replace('_wasm.html', '.wat') || 'main.wat';\n";
    html << "                const response = await fetch(watName + '?t=' + Date.now());\n";
    html << "                const watCode = await response.text();\n";
    html << "                \n";
    html << "                // Init WABT and parse the .wat to .wasm\n";
    html << "                const wabt = await WabtModule();\n";
    html << "                const wasmModule = wabt.parseWat(watName, watCode);\n";
    html << "                const { buffer } = wasmModule.toBinary({});\n";
    html << "                \n";
    html << "                // Instantiate the WebAssembly module\n";
    html << "                const { instance } = await WebAssembly.instantiate(buffer, importObject);\n";
    html << "                wasmInstance = instance;\n";
    html << "                wasmMemory = instance.exports.memory;\n";
    html << "                logToConsole('-- WASM Module Booted --', 'system');\n";
    html << "                \n";
    html << "                // Call the main function if it exists\n";
    html << "                if (instance.exports.main) {\n";
    html << "                    const result = instance.exports.main();\n";
    html << "                    logToConsole(`Main returned: ${result}`, 'system');\n";
    html << "                }\n";
    html << "            } catch (err) {\n";
    html << "                logToConsole(`<span style=\"color:#f87171\">WASM Error: ${err.message}</span>`, 'system');\n";
    html << "            }\n";
    html << "        }\n";
    html << "        loadWasm();\n";
    html << "    </script>\n";
    html << "</body>\n</html>\n";
    return html.str();
}

bool WASMCodeGenerator::isFloatExpression(ExprNode* expr) {
    if (!expr) return false;
    if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
        return num->is_float;
    }
    if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        if (!current_class_name.empty() && class_field_types[current_class_name].count(id->name)) {
            return class_field_types[current_class_name][id->name] == "Float";
        }
        if (local_types.count(id->name)) {
            return local_types[id->name] == "Float";
        }
    }
    if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        return isFloatExpression(binary->left.get()) || isFloatExpression(binary->right.get());
    }
    if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        if (call->method_name == "getArea") return true;
    }
    return false;
}

bool WASMCodeGenerator::isStringExpression(ExprNode* expr) {
    if (!expr) return false;
    if (dynamic_cast<StringLiteralNode*>(expr)) return true;
    if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        if (local_types.count(id->name)) {
            return local_types[id->name] == "String";
        }
    }
    if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        return binary->is_string_concat || isStringExpression(binary->left.get()) || isStringExpression(binary->right.get());
    }
    return false;
}
