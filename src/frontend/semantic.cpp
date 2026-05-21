#include "../../include/frontend/semantic.h"

void SemanticAnalyzer::analyzeFunction(FunctionNode* node) {
    SymbolTable* function_scope = new SymbolTable(current_scope);
    SymbolTable* previous_scope = current_scope;
    current_scope = function_scope;
    
    current_fn_return_type = node->return_type->type_name;
    
    // Register parameters
    for (const auto& param : node->parameters) {
        current_scope->define(param->var_name, param->type->type_name);
    }
    
    // Analyze block
    analyzeBlock(node->body);
    
    current_scope = previous_scope;
    delete function_scope;
}

void SemanticAnalyzer::analyzeAgenticFunction(AgenticFunctionNode* node) {
    SymbolTable* function_scope = new SymbolTable(current_scope);
    SymbolTable* previous_scope = current_scope;
    current_scope = function_scope;
    
    current_fn_return_type = node->return_type->type_name;
    
    // Add parameters to scope
    for (const auto& param : node->parameters) {
        current_scope->define(param->var_name, param->type->type_name);
    }

    // AGENTIC PROMPT COMPILE-TIME VALIDATION
    std::regex var_regex("\\{([a-zA-Z_][a-zA-Z0-9_]*)\\}");
    auto words_begin = std::sregex_iterator(node->prompt_template.begin(), node->prompt_template.end(), var_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string var_name = match[1].str();

        if (!current_scope->lookup(var_name)) {
            error("Agentic Prompt Error in '" + node->function_name + "': "
                  "Variable '" + var_name + "' is used in the prompt but is not defined in scope.", node);
        }
    }

    current_scope = previous_scope;
    delete function_scope;
}

void SemanticAnalyzer::analyzeBlock(const std::vector<std::unique_ptr<ASTNode>>& block) {
    SymbolTable* block_scope = new SymbolTable(current_scope);
    SymbolTable* previous_scope = current_scope;
    current_scope = block_scope;
    
    for (const auto& stmt : block) {
        analyzeStatement(stmt.get());
    }
    
    current_scope = previous_scope;
    delete block_scope;
}

void SemanticAnalyzer::analyzeStatement(ASTNode* stmt) {
    if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt)) {
        std::string expected_type = var_decl->type->type_name;
        if (!var_decl->type->generics.empty()) {
            expected_type += "<";
            for (size_t i = 0; i < var_decl->type->generics.size(); ++i) {
                expected_type += var_decl->type->generics[i]->type_name;
                if (i < var_decl->type->generics.size() - 1) expected_type += ",";
            }
            expected_type += ">";
        }
        
        if (var_decl->initializer) {
            std::string init_type = typeCheckExpression(var_decl->initializer.get());
            if (!isAssignable(init_type, expected_type) && 
                !(init_type == "List<Void>" && expected_type.rfind("List<", 0) == 0) &&
                !(init_type == "Map<Void,Void>" && expected_type.rfind("Map<", 0) == 0)) {
                error("Type Mismatch: Cannot assign type '" + init_type + "' to variable '" + var_decl->var_name + "' of type '" + expected_type + "'", var_decl);
            }
        }
        current_scope->define(var_decl->var_name, expected_type);
    }
    else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(stmt)) {
        std::string cond_type = typeCheckExpression(if_stmt->condition.get());
        if (cond_type != "Bool") {
            error("Type Mismatch: If statement condition must be of type 'Bool', got '" + cond_type + "'", if_stmt);
        }
        analyzeBlock(if_stmt->then_branch);
        if (!if_stmt->else_branch.empty()) {
            analyzeBlock(if_stmt->else_branch);
        }
    }
    else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(stmt)) {
        std::string cond_type = typeCheckExpression(while_stmt->condition.get());
        if (cond_type != "Bool") {
            error("Type Mismatch: While loop condition must be of type 'Bool', got '" + cond_type + "'", while_stmt);
        }
        analyzeBlock(while_stmt->body);
    }
    else if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(stmt)) {
        std::string ret_type = "Void";
        if (return_stmt->expression) {
            ret_type = typeCheckExpression(return_stmt->expression.get());
        }
        if (!isAssignable(ret_type, current_fn_return_type)) {
            error("Type Mismatch: Function return type expected '" + current_fn_return_type + "', but returned '" + ret_type + "'", return_stmt);
        }
    }
    else if (auto* set_state = dynamic_cast<SetStateStmtNode*>(stmt)) {
        for (const auto& s : set_state->body) {
            analyzeStatement(s.get());
        }
    }
    else if (auto* expr = dynamic_cast<ExprNode*>(stmt)) {
        typeCheckExpression(expr);
    }
}

std::string SemanticAnalyzer::typeCheckExpression(ExprNode* expr) {
    if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
        return num->is_float ? "Float" : "Int";
    }
    if (auto* str = dynamic_cast<StringLiteralNode*>(expr)) {
        return "String";
    }
    if (auto* b = dynamic_cast<BoolLiteralNode*>(expr)) {
        return "Bool";
    }
    if (auto* ident = dynamic_cast<IdentifierNode*>(expr)) {
        std::string t = current_scope->getType(ident->name);
        if (t.empty()) {
            if (functions.count(ident->name)) {
                return "Function";
            }
            error("Symbol Error: Variable '" + ident->name + "' is not declared in this scope.", ident);
            return "";
        }
        return t;
    }
    if (auto* list_lit = dynamic_cast<ListLiteralNode*>(expr)) {
        if (list_lit->elements.empty()) {
            return "List<Void>";
        }
        std::string elem_type = typeCheckExpression(list_lit->elements[0].get());
        return "List<" + elem_type + ">";
    }
    if (auto* map_lit = dynamic_cast<MapLiteralNode*>(expr)) {
        if (map_lit->entries.empty()) {
            return "Map<Void,Void>";
        }
        std::string key_type = typeCheckExpression(map_lit->entries[0].first.get());
        std::string val_type = typeCheckExpression(map_lit->entries[0].second.get());
        return "Map<" + key_type + "," + val_type + ">";
    }
    if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        std::string type_l = typeCheckExpression(binary->left.get());
        std::string type_r = typeCheckExpression(binary->right.get());
        
        if (binary->op == "=") {
            if (!isAssignable(type_r, type_l)) {
                error("Type Mismatch: Cannot assign '" + type_r + "' to '" + type_l + "'", binary);
            }
            return type_l;
        }
        if (binary->op == "==" || binary->op == "!=" || binary->op == "<" || binary->op == ">" || binary->op == "<=" || binary->op == ">=") {
            if (type_l != type_r) {
                error("Type Mismatch: Comparison operands must match, got '" + type_l + "' and '" + type_r + "'", binary);
            }
            return "Bool";
        }
        if (binary->op == "+" || binary->op == "-" || binary->op == "*" || binary->op == "/") {
            if (binary->op == "+" && (type_l == "String" || type_r == "String")) {
                binary->is_string_concat = true;
                return "String";
            }
            if ((type_l == "Int" || type_l == "Float") && (type_r == "Int" || type_r == "Float")) {
                if (type_l == "Float" || type_r == "Float") return "Float";
                return "Int";
            }
            error("Type Error: Arithmetic operators require numeric types, got '" + type_l + "' and '" + type_r + "'", binary);
            return "Int";
        }
        return "";
    }
    if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
        std::vector<std::string> child_types;
        for (const auto& child : ui->children) {
            child_types.push_back(typeCheckExpression(child.get()));
        }
        for (const auto& arg : ui->named_args) {
            typeCheckExpression(arg.second.get());
        }

        if (classes.count(ui->component_type)) {
            ClassDeclNode* class_decl = classes[ui->component_type];
            if (ui->children.size() != class_decl->primary_constructor_args.size()) {
                error("Constructor Error: Class '" + ui->component_type + "' expects " + 
                      std::to_string(class_decl->primary_constructor_args.size()) + " arguments, got " + 
                      std::to_string(ui->children.size()), ui);
            } else {
                for (size_t i = 0; i < ui->children.size(); ++i) {
                    std::string arg_type = child_types[i];
                    std::string param_type = class_decl->primary_constructor_args[i]->type->type_name;
                    if (!isAssignable(arg_type, param_type)) {
                        error("Constructor Argument Mismatch: expected '" + param_type + "', got '" + arg_type + "'", ui);
                    }
                }
            }
            return ui->component_type;
        }
        if (functions.count(ui->component_type)) {
            FunctionNode* fn = functions[ui->component_type];
            if (ui->children.size() != fn->parameters.size()) {
                error("Call Error: Function '" + ui->component_type + "' expects " + 
                      std::to_string(fn->parameters.size()) + " arguments, got " + 
                      std::to_string(ui->children.size()), ui);
            } else {
                for (size_t i = 0; i < ui->children.size(); ++i) {
                    std::string arg_type = child_types[i];
                    std::string param_type = fn->parameters[i]->type->type_name;
                    if (!isAssignable(arg_type, param_type)) {
                        error("Call Argument Mismatch: expected '" + param_type + "', got '" + arg_type + "'", ui);
                    }
                }
            }
            return fn->return_type->type_name;
        }
        return "UI";
    }
    if (auto* prop = dynamic_cast<PropertyAccessNode*>(expr)) {
        std::string obj_type = typeCheckExpression(prop->object.get());
        if (classes.count(obj_type)) {
            ClassDeclNode* class_decl = classes[obj_type];
            for (const auto& arg : class_decl->primary_constructor_args) {
                if (arg->var_name == prop->property_name) {
                    return arg->type->type_name;
                }
            }
            for (const auto& field : class_decl->fields) {
                if (field->var_name == prop->property_name) {
                    return field->type->type_name;
                }
            }
            error("Property Error: Class '" + obj_type + "' has no property named '" + prop->property_name + "'", prop);
            return "";
        }
        if (obj_type.rfind("List<", 0) == 0 || obj_type.rfind("Map<", 0) == 0) {
            if (prop->property_name == "size") {
                return "Int";
            }
        }
        error("Property Access Error: Object of type '" + obj_type + "' has no property '" + prop->property_name + "'", prop);
        return "";
    }
    if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        std::string obj_type = typeCheckExpression(call->object.get());
        if (classes.count(obj_type)) {
            ClassDeclNode* class_decl = classes[obj_type];
            for (const auto& method : class_decl->methods) {
                if (method->function_name == call->method_name) {
                    if (call->arguments.size() != method->parameters.size()) {
                        error("Method Call Error: Method '" + call->method_name + "' expects " + 
                              std::to_string(method->parameters.size()) + " arguments, got " + 
                              std::to_string(call->arguments.size()), call);
                    } else {
                        for (size_t i = 0; i < call->arguments.size(); ++i) {
                            std::string arg_type = typeCheckExpression(call->arguments[i].get());
                            std::string param_type = method->parameters[i]->type->type_name;
                            if (!isAssignable(arg_type, param_type)) {
                                error("Method Call Argument Mismatch: expected '" + param_type + "', got '" + arg_type + "'", call);
                            }
                        }
                    }
                    return method->return_type->type_name;
                }
            }
            error("Method Error: Class '" + obj_type + "' has no method named '" + call->method_name + "'", call);
            return "";
        }
        if (interfaces.count(obj_type)) {
            InterfaceDeclNode* iface_decl = interfaces[obj_type];
            for (const auto& method : iface_decl->methods) {
                if (method->function_name == call->method_name) {
                    if (call->arguments.size() != method->parameters.size()) {
                        error("Method Call Error: Method '" + call->method_name + "' expects " + 
                              std::to_string(method->parameters.size()) + " arguments, got " + 
                              std::to_string(call->arguments.size()), call);
                    } else {
                        for (size_t i = 0; i < call->arguments.size(); ++i) {
                            std::string arg_type = typeCheckExpression(call->arguments[i].get());
                            std::string param_type = method->parameters[i]->type->type_name;
                            if (!isAssignable(arg_type, param_type)) {
                                error("Method Call Argument Mismatch: expected '" + param_type + "', got '" + arg_type + "'", call);
                            }
                        }
                    }
                    return method->return_type->type_name;
                }
            }
            error("Method Error: Interface '" + obj_type + "' has no method named '" + call->method_name + "'", call);
            return "";
        }
        if (obj_type.rfind("List<", 0) == 0) {
            if (call->method_name == "push" || call->method_name == "push_back") {
                if (call->arguments.size() != 1) {
                    error("Method Call Error: push expects 1 argument, got " + std::to_string(call->arguments.size()), call);
                    return "Void";
                }
                std::string arg_type = typeCheckExpression(call->arguments[0].get());
                size_t start = obj_type.find('<') + 1;
                size_t end = obj_type.rfind('>');
                std::string generic_type = obj_type.substr(start, end - start);
                if (arg_type != generic_type) {
                    error("Type Mismatch: Cannot push item of type '" + arg_type + "' into list of type '" + obj_type + "'", call);
                }
                return "Void";
            }
        }
        if (obj_type == "UI") {
            if (call->method_name == "render") {
                if (call->arguments.size() != 0) {
                    error("Method Call Error: render expects 0 arguments", call);
                }
                return "Void";
            }
        }
        error("Method Call Error: Object of type '" + obj_type + "' has no method '" + call->method_name + "'", call);
        return "";
    }
    return "";
}

bool SemanticAnalyzer::analyze(ProgramNode* program) {
    std::cout << "3. Running Semantic Analysis...\n";

    classes.clear();
    interfaces.clear();
    functions.clear();
    builtin_fns.clear();
    has_errors = false;

    // First pass: Register all global classes, interfaces, and functions
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            classes[class_decl->class_name] = class_decl;
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            interfaces[interface_decl->interface_name] = interface_decl;
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            functions[fn_decl->function_name] = fn_decl;
        } else if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            if (imp->module_name == "std.io") {
                auto print_fn = std::make_unique<FunctionNode>(std::make_unique<TypeNode>("Void"), "print");
                print_fn->parameters.push_back(std::make_unique<VarDeclNode>(std::make_unique<TypeNode>("String"), "msg"));
                
                auto println_fn = std::make_unique<FunctionNode>(std::make_unique<TypeNode>("Void"), "println");
                println_fn->parameters.push_back(std::make_unique<VarDeclNode>(std::make_unique<TypeNode>("String"), "msg"));

                auto httpGet_fn = std::make_unique<FunctionNode>(std::make_unique<TypeNode>("String"), "httpGet");
                httpGet_fn->parameters.push_back(std::make_unique<VarDeclNode>(std::make_unique<TypeNode>("String"), "url"));

                auto httpPost_fn = std::make_unique<FunctionNode>(std::make_unique<TypeNode>("String"), "httpPost");
                httpPost_fn->parameters.push_back(std::make_unique<VarDeclNode>(std::make_unique<TypeNode>("String"), "url"));
                httpPost_fn->parameters.push_back(std::make_unique<VarDeclNode>(std::make_unique<TypeNode>("String"), "json_body"));

                builtin_fns.push_back(std::move(print_fn));
                builtin_fns.push_back(std::move(println_fn));
                builtin_fns.push_back(std::move(httpGet_fn));
                builtin_fns.push_back(std::move(httpPost_fn));

                functions["print"] = builtin_fns[builtin_fns.size() - 4].get();
                functions["println"] = builtin_fns[builtin_fns.size() - 3].get();
                functions["httpGet"] = builtin_fns[builtin_fns.size() - 2].get();
                functions["httpPost"] = builtin_fns[builtin_fns.size() - 1].get();
            }
        }
    }

    // Second pass: Analyze all entities in scope
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            SymbolTable* class_scope = new SymbolTable(current_scope);
            current_scope = class_scope; // Enter class scope

            // Register primary constructor arguments as class properties
            for (const auto& arg : class_decl->primary_constructor_args) {
                current_scope->define(arg->var_name, arg->type->type_name);
            }

            // Register class methods in scope
            for (const auto& method : class_decl->methods) {
                current_scope->define(method->function_name, "Function");
            }

            // Register and type-check custom class fields
            for (const auto& field : class_decl->fields) {
                std::string expected_type = field->type->type_name;
                if (field->initializer) {
                    std::string init_type = typeCheckExpression(field->initializer.get());
                    if (!isAssignable(init_type, expected_type)) {
                        error("Type Mismatch: Cannot assign field initializer of type '" + init_type + "' to field '" + field->var_name + "' of type '" + expected_type + "'", field.get());
                    }
                }
                current_scope->define(field->var_name, expected_type);
            }

            // Validate interface implementation
            for (const auto& iface_name : class_decl->implemented_interfaces) {
                if (!interfaces.count(iface_name)) {
                    error("Class '" + class_decl->class_name + "' implements unknown interface '" + iface_name + "'", class_decl);
                    continue;
                }
                InterfaceDeclNode* iface = interfaces[iface_name];
                for (const auto& i_method : iface->methods) {
                    bool method_found = false;
                    for (const auto& c_method : class_decl->methods) {
                        if (c_method->function_name == i_method->function_name) {
                            method_found = true;
                            if (c_method->return_type->type_name != i_method->return_type->type_name) {
                                error("Interface Implementation Error: Return type mismatch for method '" + i_method->function_name + "'. Expected '" + i_method->return_type->type_name + "', got '" + c_method->return_type->type_name + "'", c_method.get());
                            }
                            if (c_method->parameters.size() != i_method->parameters.size()) {
                                error("Interface Implementation Error: Parameter count mismatch for method '" + i_method->function_name + "'. Expected " + std::to_string(i_method->parameters.size()) + ", got " + std::to_string(c_method->parameters.size()), c_method.get());
                            } else {
                                for (size_t p = 0; p < i_method->parameters.size(); ++p) {
                                    if (c_method->parameters[p]->type->type_name != i_method->parameters[p]->type->type_name) {
                                        error("Interface Implementation Error: Parameter type mismatch for parameter '" + c_method->parameters[p]->var_name + "' in method '" + i_method->function_name + "'", c_method.get());
                                    }
                                }
                            }
                            break;
                        }
                    }
                    if (!method_found) {
                        error("Interface Implementation Error: Class '" + class_decl->class_name + "' does not implement method '" + i_method->function_name + "' declared in interface '" + iface_name + "'", class_decl);
                    }
                }
            }

            // Analyze methods
            for (const auto& method : class_decl->methods) {
                if (auto* agentic = dynamic_cast<AgenticFunctionNode*>(method.get())) {
                    analyzeAgenticFunction(agentic);
                } else {
                    analyzeFunction(method.get());
                }
            }

            current_scope = current_scope->getParent(); // Exit class scope
            delete class_scope;
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            if (auto* agentic = dynamic_cast<AgenticFunctionNode*>(fn_decl)) {
                analyzeAgenticFunction(agentic);
            } else {
                analyzeFunction(fn_decl);
            }
        }
    }

    if (has_errors) {
        std::cerr << "   [FAILED] Semantic Analysis found errors. Compilation aborted.\n";
        return false;
    }

    std::cout << "   [OK] Semantic Analysis Passed. Code is statically sound.\n";
    return true;
}

bool SemanticAnalyzer::isAssignable(const std::string& source, const std::string& target) {
    if (source == target) return true;
    if (interfaces.count(target) && classes.count(source)) {
        ClassDeclNode* cl = classes[source];
        for (const auto& iface : cl->implemented_interfaces) {
            if (iface == target) return true;
        }
    }
    return false;
}
