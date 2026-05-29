#include "../../include/frontend/semantic.h"
#include <functional>

// Forward declaration
static void populateTypeNode(TypeNode* type_node, const std::string& type_str);

// Type inference helper implementations
std::string SemanticAnalyzer::inferAndValidateVarDecl(VarDeclNode* var_decl) {
    std::string expected_type;
    
    // Check if type is explicitly declared or needs inference
    if (var_decl->type->is_inferred || var_decl->type->type_name == "Auto" || 
        var_decl->type->type_name == "let" || var_decl->type->type_name.empty()) {
        
        // Type inference required
        if (!var_decl->initializer) {
            error("Type Inference Error: Variable '" + var_decl->var_name + 
                  "' declared with 'let' must have an initializer.", var_decl);
            expected_type = "Void";
        } else {
            // Infer type from initializer expression
            std::string init_type = type_inferencer.inferType(var_decl->initializer.get());
            
            if (init_type.empty()) {
                // Fall back to type checking if inference fails
                init_type = typeCheckExpression(var_decl->initializer.get());
            }
            
            if (init_type.empty()) {
                error("Type Inference Error: Cannot infer type for variable '" + 
                      var_decl->var_name + "'", var_decl);
                expected_type = "Void";
            } else {
                // Update the type node with inferred type
                populateTypeNode(var_decl->type.get(), init_type);
                var_decl->type->is_inferred = true;
                expected_type = init_type;
            }
        }
    } else {
        // Explicit type - validate against initializer if present
        expected_type = var_decl->type->type_name;
        
        // Add generic parameters to type string
        if (!var_decl->type->generics.empty()) {
            expected_type += "<";
            for (size_t i = 0; i < var_decl->type->generics.size(); ++i) {
                expected_type += var_decl->type->generics[i]->type_name;
                if (i < var_decl->type->generics.size() - 1) expected_type += ",";
            }
            expected_type += ">";
        }
        
        if (var_decl->initializer) {
            std::string init_type = typeCheckExpression(var_decl->initializer.get(), expected_type);
            
            // Skip type mismatch check for RC/GC smart pointer types —
            // Ref<T> and Weak<T> constructors return opaque types; the declared type is authoritative.
            bool is_mem_type = (expected_type.rfind("Ref<", 0) == 0 ||
                                expected_type.rfind("Weak<", 0) == 0);
            // Skip mismatch if init type is unknown/empty (constructor result from FunctionCallNode)
            bool init_unknown = init_type.empty();
            // Also allow gcStats() returning String
            bool is_gc_stats_call = (init_type == "UI" || init_type == "");

            if (!is_mem_type && !init_unknown && !is_gc_stats_call &&
                !isAssignable(init_type, expected_type) &&
                !(init_type == "List<Void>" && expected_type.rfind("List<", 0) == 0) &&
                !(init_type == "Map<Void,Void>" && expected_type.rfind("Map<", 0) == 0)) {
                error("Type Mismatch: Cannot assign type '" + init_type +
                      "' to variable '" + var_decl->var_name + "' of type '" +
                      expected_type + "'", var_decl);
            }
        }
    }
    
    return expected_type;
}

std::string SemanticAnalyzer::inferFunctionReturnType(FunctionNode* func) {
    // If return type is already specified, use it
    if (!func->return_type->is_inferred && 
        func->return_type->type_name != "Auto" && 
        !func->return_type->type_name.empty()) {
        return func->return_type->type_name;
    }
    
    return inferBlockReturnType(func->body);
}

std::string SemanticAnalyzer::inferBlockReturnType(const std::vector<std::unique_ptr<ASTNode>>& block) {
    std::vector<std::string> return_types;
    
    std::function<void(ASTNode*)> find_returns = [&](ASTNode* node) {
        if (!node) return;
        if (auto* ret = dynamic_cast<ReturnStmtNode*>(node)) {
            std::string type = "Void";
            if (ret->expression) {
                type = typeCheckExpression(ret->expression.get());
            }
            return_types.push_back(type);
        } else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(node)) {
            for (const auto& s : if_stmt->then_branch) find_returns(s.get());
            for (const auto& s : if_stmt->else_branch) find_returns(s.get());
        } else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(node)) {
            for (const auto& s : while_stmt->body) find_returns(s.get());
        } else if (auto* set_state = dynamic_cast<SetStateStmtNode*>(node)) {
            for (const auto& s : set_state->body) find_returns(s.get());
        }
    };
    
    for (const auto& stmt : block) {
        find_returns(stmt.get());
    }
    
    if (return_types.empty()) return "Void";
    
    std::string unified = return_types[0];
    for (size_t i = 1; i < return_types.size(); ++i) {
        if (unified != return_types[i]) {
            if (type_inferencer.unifyTypes(unified, return_types[i], nullptr)) {
                // Compatible - we can keep the current unified type
            } else {
                error("Type Inference Error: Mismatched return types in function/lambda body: '" + unified + "' and '" + return_types[i] + "'");
            }
        }
    }
    return unified;
}

static std::vector<std::string> getGenericParams(const std::string& type) {
    std::vector<std::string> params;
    size_t start = type.find('<');
    if (start == std::string::npos) return params;
    
    start++;
    int depth = 0;
    std::string current;
    
    for (size_t i = start; i < type.length(); i++) {
        char c = type[i];
        if (c == '<') depth++;
        else if (c == '>') {
            if (depth == 0) {
                if (!current.empty()) {
                    current.erase(0, current.find_first_not_of(" \t\r\n"));
                    current.erase(current.find_last_not_of(" \t\r\n") + 1);
                    params.push_back(current);
                }
                break;
            }
            depth--;
        }
        else if (c == ',' && depth == 0) {
            if (!current.empty()) {
                current.erase(0, current.find_first_not_of(" \t\r\n"));
                current.erase(current.find_last_not_of(" \t\r\n") + 1);
                params.push_back(current);
            }
            current.clear();
        }
        else {
            current += c;
        }
    }
    
    return params;
}

static bool parseFunctionType(const std::string& type_str, std::vector<std::string>& out_params, std::string& out_ret) {
    if (type_str.rfind("Function<", 0) != 0 || type_str.back() != '>') return false;
    auto params = getGenericParams(type_str);
    if (params.empty()) return false;
    out_ret = params.back();
    for (size_t i = 0; i < params.size() - 1; ++i) {
        out_params.push_back(params[i]);
    }
    return true;
}

static void populateTypeNode(TypeNode* type_node, const std::string& type_str) {
    size_t angle_pos = type_str.find('<');
    if (angle_pos == std::string::npos) {
        type_node->type_name = type_str;
        type_node->generics.clear();
        return;
    }
    
    type_node->type_name = type_str.substr(0, angle_pos);
    type_node->generics.clear();
    
    std::string inner = type_str.substr(angle_pos + 1, type_str.length() - angle_pos - 2);
    int bracket_depth = 0;
    std::string current_part = "";
    for (char c : inner) {
        if (c == '<') {
            bracket_depth++;
            current_part += c;
        } else if (c == '>') {
            bracket_depth--;
            current_part += c;
        } else if (c == ',' && bracket_depth == 0) {
            auto sub_node = std::make_unique<TypeNode>("");
            populateTypeNode(sub_node.get(), current_part);
            type_node->generics.push_back(std::move(sub_node));
            current_part = "";
        } else {
            if (c != ' ' || !current_part.empty()) {
                current_part += c;
            }
        }
    }
    if (!current_part.empty()) {
        auto sub_node = std::make_unique<TypeNode>("");
        populateTypeNode(sub_node.get(), current_part);
        type_node->generics.push_back(std::move(sub_node));
    }
}

void SemanticAnalyzer::analyzeFunction(FunctionNode* node) {
    if (node->is_foreign) {
        return;
    }
    SymbolTable* function_scope = new SymbolTable(current_scope);
    SymbolTable* previous_scope = current_scope;
    current_scope = function_scope;
    
    // Register generic type parameters as valid placeholder types in function scope
    for (const auto& generic_param : node->generic_params) {
        current_scope->define(generic_param, "TypeVar:" + generic_param);
    }
    
    // Support return type inference
    if (node->return_type->is_inferred || node->return_type->type_name == "Auto") {
        current_fn_return_type = inferFunctionReturnType(node);
        populateTypeNode(node->return_type.get(), current_fn_return_type);
    } else {
        current_fn_return_type = node->return_type->type_name;
    }
    
    // Register parameters with type inference support
    for (const auto& param : node->parameters) {
        std::string param_type;
        
        // Handle type inference for parameters with default values
        if (param->type->is_inferred && param->initializer) {
            std::string init_type = type_inferencer.inferType(param->initializer.get());
            if (init_type.empty()) {
                init_type = typeCheckExpression(param->initializer.get());
            }
            if (!init_type.empty()) {
                populateTypeNode(param->type.get(), init_type);
                param->type->is_inferred = true;
                param_type = init_type;
            } else {
                param_type = param->type->type_name;
            }
        } else {
            param_type = param->type->type_name;
        }
        
        current_scope->define(param->var_name, param_type);
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
    
    // Register generic type parameters as valid placeholder types in function scope
    for (const auto& generic_param : node->generic_params) {
        current_scope->define(generic_param, "TypeVar:" + generic_param);
    }
    
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

void SemanticAnalyzer::analyzeOrchestration(AgentOrchestrationNode* node) {
    if (node->strategy != "sequential" && node->strategy != "parallel") {
        error("Orchestration Error: Strategy must be either 'sequential' or 'parallel', got '" + node->strategy + "'", node);
    }
    if (node->agents.empty()) {
        error("Orchestration Error: Orchestration '" + node->orchestration_name + "' must have at least one agent.", node);
    }
    for (const auto& agent_name : node->agents) {
        if (!functions.count(agent_name)) {
            error("Orchestration Error: Agent function '" + agent_name + "' is not defined.", node);
        } else {
            auto* fn = functions[agent_name];
            if (!dynamic_cast<AgenticFunctionNode*>(fn)) {
                error("Orchestration Error: Function '" + agent_name + "' is not an agentic function.", node);
            }
        }
    }
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
        // Use the new type inference helper
        std::string expected_type = inferAndValidateVarDecl(var_decl);
        current_scope->define(var_decl->var_name, expected_type);
    }
    else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(stmt)) {
        std::string cond_type = typeCheckExpression(if_stmt->condition.get(), "Bool");
        if (cond_type != "Bool") {
            error("Type Mismatch: If statement condition must be of type 'Bool', got '" + cond_type + "'", if_stmt);
        }
        analyzeBlock(if_stmt->then_branch);
        if (!if_stmt->else_branch.empty()) {
            analyzeBlock(if_stmt->else_branch);
        }
    }
    else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(stmt)) {
        std::string cond_type = typeCheckExpression(while_stmt->condition.get(), "Bool");
        if (cond_type != "Bool") {
            error("Type Mismatch: While loop condition must be of type 'Bool', got '" + cond_type + "'", while_stmt);
        }
        analyzeBlock(while_stmt->body);
    }
    else if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(stmt)) {
        std::string ret_type = "Void";
        if (return_stmt->expression) {
            ret_type = typeCheckExpression(return_stmt->expression.get(), current_fn_return_type);
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

std::string SemanticAnalyzer::typeCheckExpression(ExprNode* expr, const std::string& expected_type) {
    if (auto* await_expr = dynamic_cast<AwaitExprNode*>(expr)) {
        std::string awaited_expected = expected_type.empty() ? "" : "Future<" + expected_type + ">";
        std::string inner_type = typeCheckExpression(await_expr->expression.get(), awaited_expected);
        if (inner_type.rfind("Future<", 0) == 0 && !inner_type.empty() && inner_type.back() == '>') {
            return inner_type.substr(7, inner_type.length() - 8);
        }
        return inner_type;
    }
    if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
        return num->is_float ? "Float" : "Int";
    }
    if (auto* str = dynamic_cast<StringLiteralNode*>(expr)) {
        return "String";
    }
    if (auto* b = dynamic_cast<BoolLiteralNode*>(expr)) {
        return "Bool";
    }
    if (auto* opt = dynamic_cast<OptionExprNode*>(expr)) {
        if (opt->kind == OptionExprNode::OptionKind::None) {
            if (expected_type.rfind("Option<", 0) == 0 && expected_type.back() == '>') {
                return expected_type;
            }
            return "Option<Void>";
        } else {
            std::string inner_expected = "";
            if (expected_type.rfind("Option<", 0) == 0 && expected_type.back() == '>') {
                inner_expected = expected_type.substr(7, expected_type.length() - 8);
            }
            std::string inner_type = typeCheckExpression(opt->value.get(), inner_expected);
            return "Option<" + inner_type + ">";
        }
    }
    if (auto* res = dynamic_cast<ResultExprNode*>(expr)) {
        std::string inner_expected = "";
        if (expected_type.rfind("Result<", 0) == 0 && expected_type.back() == '>') {
            auto params = getGenericParams(expected_type);
            if (res->kind == ResultExprNode::ResultKind::Ok && params.size() > 0) {
                inner_expected = params[0];
            } else if (res->kind == ResultExprNode::ResultKind::Err && params.size() > 1) {
                inner_expected = params[1];
            }
        }
        std::string inner_type = typeCheckExpression(res->value.get(), inner_expected);
        if (res->kind == ResultExprNode::ResultKind::Ok) {
            return "Result<" + inner_type + ", Error>";
        } else {
            return "Result<Void, " + inner_type + ">";
        }
    }
    if (dynamic_cast<NullLiteralNode*>(expr)) {
        if (expected_type.rfind("Option<", 0) == 0 && expected_type.back() == '>') {
            return expected_type;
        }
        return "Null";
    }
    if (auto* try_expr = dynamic_cast<TryExprNode*>(expr)) {
        std::string result_type = typeCheckExpression(try_expr->expression.get());
        if (result_type.rfind("Result<", 0) == 0 && result_type.back() == '>') {
            auto params = getGenericParams(result_type);
            if (params.size() > 0) return params[0];
        }
        return result_type;
    }
    if (auto* match = dynamic_cast<MatchExprNode*>(expr)) {
        std::string subject_type = typeCheckExpression(match->subject.get());
        std::string unified_type = "";
        for (const auto& arm : match->arms) {
            std::string arm_type = typeCheckExpression(arm.second.get(), expected_type);
            if (unified_type.empty()) {
                unified_type = arm_type;
            } else if (unified_type != arm_type) {
                if (!type_inferencer.unifyTypes(unified_type, arm_type, match)) {
                    error("Type Error: Mismatched types in match expression arms: '" + unified_type + "' and '" + arm_type + "'", match);
                }
            }
        }
        return unified_type;
    }
    if (auto* lambda = dynamic_cast<LambdaNode*>(expr)) {
        std::vector<std::string> expected_params;
        std::string expected_ret = "Void";
        bool has_context = false;
        
        if (!expected_type.empty()) {
            has_context = parseFunctionType(expected_type, expected_params, expected_ret);
        }
        
        SymbolTable* lambda_scope = new SymbolTable(current_scope);
        SymbolTable* previous_scope = current_scope;
        current_scope = lambda_scope;
        
        for (size_t i = 0; i < lambda->parameters.size(); ++i) {
            auto* param = lambda->parameters[i].get();
            std::string param_type = "Void";
            
            if (param->type && !param->type->is_inferred && param->type->type_name != "Auto" && !param->type->type_name.empty()) {
                param_type = param->type->type_name;
            } else if (has_context && i < expected_params.size()) {
                param_type = expected_params[i];
                if (param->type) {
                    populateTypeNode(param->type.get(), param_type);
                    param->type->is_inferred = true;
                }
            } else if (param->initializer) {
                param_type = typeCheckExpression(param->initializer.get());
                if (param->type) {
                    populateTypeNode(param->type.get(), param_type);
                    param->type->is_inferred = true;
                }
            } else {
                error("Type Inference Error: Cannot infer type for lambda parameter '" + param->var_name + "'", lambda);
            }
            current_scope->define(param->var_name, param_type);
        }
        
        std::string old_fn_ret = current_fn_return_type;
        std::string inferred_ret = "Void";
        
        if (has_context) {
            current_fn_return_type = expected_ret;
            inferred_ret = expected_ret;
        } else if (lambda->return_type && !lambda->return_type->is_inferred && lambda->return_type->type_name != "Auto" && !lambda->return_type->type_name.empty()) {
            current_fn_return_type = lambda->return_type->type_name;
            inferred_ret = lambda->return_type->type_name;
        } else {
            current_fn_return_type = "Auto";
        }
        
        analyzeBlock(lambda->body);
        
        if (current_fn_return_type == "Auto") {
            inferred_ret = inferBlockReturnType(lambda->body);
            if (lambda->return_type) {
                populateTypeNode(lambda->return_type.get(), inferred_ret);
                lambda->return_type->is_inferred = true;
            }
        }
        
        current_fn_return_type = old_fn_ret;
        current_scope = previous_scope;
        delete lambda_scope;
        
        std::string sig = "";
        for (size_t i = 0; i < lambda->parameters.size(); ++i) {
            sig += lambda->parameters[i]->type->type_name + ",";
        }
        sig += inferred_ret;
        return "Function<" + sig + ">";
    }
    if (auto* ident = dynamic_cast<IdentifierNode*>(expr)) {
        std::string t = current_scope->getType(ident->name);
        ident->type_hint = t;
        if (t.empty()) {
            if (functions.count(ident->name)) {
                ident->type_hint = "Function";
                return "Function";
            }
            error("Symbol Error: Variable '" + ident->name + "' is not declared in this scope.", ident);
            return "";
        }
        return t;
    }
    if (auto* list_lit = dynamic_cast<ListLiteralNode*>(expr)) {
        if (list_lit->elements.empty()) {
            if (expected_type.rfind("List<", 0) == 0 && expected_type.back() == '>') {
                return expected_type;
            }
            return "List<Void>";
        }
        std::string elem_expected = "";
        if (expected_type.rfind("List<", 0) == 0 && expected_type.back() == '>') {
            elem_expected = expected_type.substr(5, expected_type.length() - 6);
        }
        std::string elem_type = typeCheckExpression(list_lit->elements[0].get(), elem_expected);
        for (size_t i = 1; i < list_lit->elements.size(); ++i) {
            std::string t = typeCheckExpression(list_lit->elements[i].get(), elem_expected);
            if (t != elem_type) {
                if (!type_inferencer.unifyTypes(elem_type, t, list_lit)) {
                    error("Type Mismatch: elements in List have mismatched types '" + elem_type + "' and '" + t + "'", list_lit);
                }
            }
        }
        return "List<" + elem_type + ">";
    }
    if (auto* map_lit = dynamic_cast<MapLiteralNode*>(expr)) {
        if (map_lit->entries.empty()) {
            if (expected_type.rfind("Map<", 0) == 0 && expected_type.back() == '>') {
                return expected_type;
            }
            return "Map<Void,Void>";
        }
        std::string key_expected = "", val_expected = "";
        if (expected_type.rfind("Map<", 0) == 0 && expected_type.back() == '>') {
            std::string inner = expected_type.substr(4, expected_type.length() - 5);
            size_t comma = inner.find(',');
            if (comma != std::string::npos) {
                key_expected = inner.substr(0, comma);
                val_expected = inner.substr(comma + 1);
            }
        }
        std::string key_type = typeCheckExpression(map_lit->entries[0].first.get(), key_expected);
        std::string val_type = typeCheckExpression(map_lit->entries[0].second.get(), val_expected);
        return "Map<" + key_type + "," + val_type + ">";
    }
    if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        if (binary->op == "=") {
            std::string type_l = typeCheckExpression(binary->left.get());
            std::string type_r = typeCheckExpression(binary->right.get(), type_l);
            if (!isAssignable(type_r, type_l)) {
                error("Type Mismatch: Cannot assign '" + type_r + "' to '" + type_l + "'", binary);
            }
            return type_l;
        }
        
        std::string type_l = typeCheckExpression(binary->left.get(), expected_type);
        std::string type_r = typeCheckExpression(binary->right.get(), expected_type);
        
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
        for (const auto& arg : ui->named_args) {
            typeCheckExpression(arg.second.get());
        }

        // If component_type is a variable in scope holding a Function
        if (current_scope->lookup(ui->component_type)) {
            std::string var_type = current_scope->getType(ui->component_type);
            std::vector<std::string> params;
            std::string ret_type;
            if (parseFunctionType(var_type, params, ret_type)) {
                if (ui->children.size() != params.size()) {
                    error("Call Error: Variable function '" + ui->component_type + "' expects " +
                          std::to_string(params.size()) + " arguments, got " +
                          std::to_string(ui->children.size()), ui);
                } else {
                    for (size_t i = 0; i < ui->children.size(); ++i) {
                        std::string arg_type = typeCheckExpression(ui->children[i].get(), params[i]);
                        if (!isAssignable(arg_type, params[i])) {
                            error("Call Argument Mismatch: expected '" + params[i] + "', got '" + arg_type + "'", ui);
                        }
                    }
                }
                return ret_type;
            }
        }

        if (classes.count(ui->component_type)) {
            ClassDeclNode* class_decl = classes[ui->component_type];
            if (ui->children.size() != class_decl->primary_constructor_args.size()) {
                error("Constructor Error: Class '" + ui->component_type + "' expects " + 
                      std::to_string(class_decl->primary_constructor_args.size()) + " arguments, got " + 
                      std::to_string(ui->children.size()), ui);
            } else {
                for (size_t i = 0; i < ui->children.size(); ++i) {
                    std::string param_type = class_decl->primary_constructor_args[i]->type->type_name;
                    std::string arg_type = typeCheckExpression(ui->children[i].get(), param_type);
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
                    std::string param_type = fn->parameters[i]->type->type_name;
                    std::string arg_type = typeCheckExpression(ui->children[i].get(), param_type);
                    if (!isAssignable(arg_type, param_type)) {
                        error("Call Argument Mismatch: expected '" + param_type + "', got '" + arg_type + "'", ui);
                    }
                }
            }
            if (fn->is_async) {
                return "Future<" + fn->return_type->type_name + ">";
            }
            return fn->return_type->type_name;
        }
        if (orchestrations.count(ui->component_type)) {
            AgentOrchestrationNode* orch = orchestrations[ui->component_type];
            if (ui->children.size() != 1) {
                error("Orchestration Call Error: Orchestration '" + ui->component_type + "' expects exactly 1 argument, got " + std::to_string(ui->children.size()), ui);
            } else {
                std::string arg_type = typeCheckExpression(ui->children[0].get(), "String");
                if (!isAssignable(arg_type, "String")) {
                    error("Orchestration Call Argument Mismatch: expected 'String', got '" + arg_type + "'", ui);
                }
            }
            if (orch->strategy == "sequential") {
                return "String";
            } else {
                return "List<String>";
            }
        }
        for (const auto& child : ui->children) {
            typeCheckExpression(child.get());
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
                            std::string param_type = method->parameters[i]->type->type_name;
                            std::string arg_type = typeCheckExpression(call->arguments[i].get(), param_type);
                            if (!isAssignable(arg_type, param_type)) {
                                error("Method Call Argument Mismatch: expected '" + param_type + "', got '" + arg_type + "'", call);
                            }
                        }
                    }
                    if (method->is_async) {
                        return "Future<" + method->return_type->type_name + ">";
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
                            std::string param_type = method->parameters[i]->type->type_name;
                            std::string arg_type = typeCheckExpression(call->arguments[i].get(), param_type);
                            if (!isAssignable(arg_type, param_type)) {
                                error("Method Call Argument Mismatch: expected '" + param_type + "', got '" + arg_type + "'", call);
                            }
                        }
                    }
                    if (method->is_async) {
                        return "Future<" + method->return_type->type_name + ">";
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
                size_t start = obj_type.find('<') + 1;
                size_t end = obj_type.rfind('>');
                std::string generic_type = obj_type.substr(start, end - start);
                std::string arg_type = typeCheckExpression(call->arguments[0].get(), generic_type);
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
    orchestrations.clear();
    builtin_fns.clear();
    has_errors = false;

    // Register platform detection constants
    current_scope->define("isAndroid", "Bool");
    current_scope->define("isIos", "Bool");
    current_scope->define("isMac", "Bool");
    current_scope->define("isLinux", "Bool");
    current_scope->define("isWeb", "Bool");
    current_scope->define("isWindows", "Bool");

    // First pass: Register all global classes, interfaces, and functions
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            classes[class_decl->class_name] = class_decl;
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            interfaces[interface_decl->interface_name] = interface_decl;
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            functions[fn_decl->function_name] = fn_decl;
        } else if (auto* orch_decl = dynamic_cast<AgentOrchestrationNode*>(stmt.get())) {
            orchestrations[orch_decl->orchestration_name] = orch_decl;
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
        if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt.get())) {
            // Handle top-level global variables with type inference
            std::string expected_type = var_decl->type->type_name;
            if (expected_type == "Auto") {
                if (!var_decl->initializer) {
                    error("Type Inference Error: Variable '" + var_decl->var_name + "' declared with 'let' must have an initializer.", var_decl);
                    expected_type = "Void";
                } else {
                    std::string init_type = typeCheckExpression(var_decl->initializer.get());
                    populateTypeNode(var_decl->type.get(), init_type);
                    expected_type = init_type;
                }
            } else {
                if (!var_decl->type->generics.empty()) {
                    expected_type += "<";
                    for (size_t i = 0; i < var_decl->type->generics.size(); ++i) {
                        expected_type += var_decl->type->generics[i]->type_name;
                        if (i < var_decl->type->generics.size() - 1) expected_type += ",";
                    }
                    expected_type += ">";
                }
                if (var_decl->initializer) {
                    std::string init_type = typeCheckExpression(var_decl->initializer.get(), expected_type);
                    if (!isAssignable(init_type, expected_type) && 
                        !(init_type == "List<Void>" && expected_type.rfind("List<", 0) == 0) &&
                        !(init_type == "Map<Void,Void>" && expected_type.rfind("Map<", 0) == 0)) {
                        error("Type Mismatch: Cannot assign type '" + init_type + "' to variable '" + var_decl->var_name + "' of type '" + expected_type + "'", var_decl);
                    }
                }
            }
            current_scope->define(var_decl->var_name, expected_type);
        }
        else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            SymbolTable* class_scope = new SymbolTable(current_scope);
            current_scope = class_scope; // Enter class scope

            // Register generic type parameters as valid placeholder types in class scope
            for (const auto& generic_param : class_decl->generic_params) {
                current_scope->define(generic_param, "TypeVar:" + generic_param);
            }

            // Register primary constructor arguments as class properties
            for (const auto& arg : class_decl->primary_constructor_args) {
                current_scope->define(arg->var_name, arg->type->type_name);
            }

            // Register class methods in scope
            for (const auto& method : class_decl->methods) {
                current_scope->define(method->function_name, "Function");
            }

            // Register and type-check custom class fields
            for (auto& field : class_decl->fields) {
                std::string expected_type = field->type->type_name;
                if (expected_type == "Auto") {
                    if (!field->initializer) {
                        error("Type Inference Error: Field '" + field->var_name + "' declared with 'let' must have an initializer.", field.get());
                        expected_type = "Void";
                    } else {
                        std::string init_type = typeCheckExpression(field->initializer.get());
                        populateTypeNode(field->type.get(), init_type);
                        expected_type = init_type;
                    }
                } else {
                    if (!field->type->generics.empty()) {
                        expected_type += "<";
                        for (size_t i = 0; i < field->type->generics.size(); ++i) {
                            expected_type += field->type->generics[i]->type_name;
                            if (i < field->type->generics.size() - 1) expected_type += ",";
                        }
                        expected_type += ">";
                    }
                    if (field->initializer) {
                        std::string init_type = typeCheckExpression(field->initializer.get(), expected_type);
                        if (!isAssignable(init_type, expected_type)) {
                            error("Type Mismatch: Cannot assign field initializer of type '" + init_type + "' to field '" + field->var_name + "' of type '" + expected_type + "'", field.get());
                        }
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
        } else if (auto* orch_decl = dynamic_cast<AgentOrchestrationNode*>(stmt.get())) {
            analyzeOrchestration(orch_decl);
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
