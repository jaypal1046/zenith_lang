#include "../../include/frontend/formatter.h"
#include <iostream>

void Formatter::indent() {
    for (int i = 0; i < indent_level * 4; ++i) {
        output << " ";
    }
}

void Formatter::formatType(TypeNode* type) {
    if (!type) {
        output << "Void";
        return;
    }
    output << type->type_name;
    if (!type->generics.empty()) {
        output << "<";
        for (size_t i = 0; i < type->generics.size(); ++i) {
            formatType(type->generics[i].get());
            if (i < type->generics.size() - 1) output << ", ";
        }
        output << ">";
    }
}

void Formatter::formatExpression(ExprNode* expr) {
    if (!expr) return;
    
    if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        output << id->name;
    }
    else if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
        output << num->value;
    }
    else if (auto* str = dynamic_cast<StringLiteralNode*>(expr)) {
        output << "\"" << str->value << "\"";
    }
    else if (auto* b = dynamic_cast<BoolLiteralNode*>(expr)) {
        output << (b->value ? "true" : "false");
    }
    else if (auto* null_lit = dynamic_cast<NullLiteralNode*>(expr)) {
        output << "null";
    }
    else if (auto* opt = dynamic_cast<OptionExprNode*>(expr)) {
        if (opt->kind == OptionExprNode::OptionKind::Some) {
            output << "Some(";
            formatExpression(opt->value.get());
            output << ")";
        } else {
            output << "None";
        }
    }
    else if (auto* res = dynamic_cast<ResultExprNode*>(expr)) {
        if (res->kind == ResultExprNode::ResultKind::Ok) {
            output << "Ok(";
            formatExpression(res->value.get());
            output << ")";
        } else {
            output << "Err(";
            formatExpression(res->value.get());
            output << ")";
        }
    }
    else if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        formatExpression(binary->left.get());
        output << " " << binary->op << " ";
        formatExpression(binary->right.get());
    }
    else if (auto* list_lit = dynamic_cast<ListLiteralNode*>(expr)) {
        output << "[";
        for (size_t i = 0; i < list_lit->elements.size(); ++i) {
            formatExpression(list_lit->elements[i].get());
            if (i < list_lit->elements.size() - 1) output << ", ";
        }
        output << "]";
    }
    else if (auto* map_lit = dynamic_cast<MapLiteralNode*>(expr)) {
        output << "{";
        for (size_t i = 0; i < map_lit->entries.size(); ++i) {
            formatExpression(map_lit->entries[i].first.get());
            output << ": ";
            formatExpression(map_lit->entries[i].second.get());
            if (i < map_lit->entries.size() - 1) output << ", ";
        }
        output << "}";
    }
    else if (auto* prop = dynamic_cast<PropertyAccessNode*>(expr)) {
        formatExpression(prop->object.get());
        output << "." << prop->property_name;
    }
    else if (auto* call = dynamic_cast<FunctionCallNode*>(expr)) {
        output << call->function_name << "(";
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            formatExpression(call->arguments[i].get());
            if (i < call->arguments.size() - 1) output << ", ";
        }
        output << ")";
    }
    else if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        formatExpression(call->object.get());
        output << "." << call->method_name << "(";
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            formatExpression(call->arguments[i].get());
            if (i < call->arguments.size() - 1) output << ", ";
        }
        output << ")";
    }
    else if (auto* await_expr = dynamic_cast<AwaitExprNode*>(expr)) {
        output << "await ";
        formatExpression(await_expr->expression.get());
    }
    else if (auto* try_expr = dynamic_cast<TryExprNode*>(expr)) {
        output << "try ";
        formatExpression(try_expr->expression.get());
    }
    else if (auto* match_expr = dynamic_cast<MatchExprNode*>(expr)) {
        output << "match ";
        formatExpression(match_expr->subject.get());
        output << " {\n";
        indent_level++;
        for (size_t i = 0; i < match_expr->arms.size(); ++i) {
            indent();
            formatPattern(match_expr->arms[i].pattern.get());
            output << " => ";
            formatExpression(match_expr->arms[i].body.get());
            output << ",\n";
        }
        indent_level--;
        indent();
        output << "}";
    }
    else if (auto* lambda = dynamic_cast<LambdaNode*>(expr)) {
        output << "(";
        for (size_t i = 0; i < lambda->parameters.size(); ++i) {
            if (lambda->parameters[i]->type && !lambda->parameters[i]->type->is_inferred) {
                formatType(lambda->parameters[i]->type.get());
                output << " ";
            }
            output << lambda->parameters[i]->var_name;
            if (i < lambda->parameters.size() - 1) output << ", ";
        }
        output << ") => {\n";
        indent_level++;
        for (const auto& s : lambda->body) {
            formatStatement(s.get());
        }
        indent_level--;
        indent();
        output << "}";
    }
    else if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
        output << ui->component_type << "(";
        bool first = true;
        for (size_t i = 0; i < ui->children.size(); ++i) {
            if (!first) output << ", ";
            first = false;
            formatExpression(ui->children[i].get());
        }
        for (size_t i = 0; i < ui->named_args.size(); ++i) {
            if (!first) output << ", ";
            first = false;
            output << ui->named_args[i].first << ": ";
            formatExpression(ui->named_args[i].second.get());
        }
        output << ")";
    }
}

void Formatter::formatStatement(ASTNode* node) {
    if (!node) return;
    
    if (auto* import_node = dynamic_cast<ImportNode*>(node)) {
        indent();
        std::string mod = import_node->module_name;
        if (mod.front() == '"' || mod.front() == '\'') {
            output << "import " << mod << ";\n";
        } else if (mod.find('/') != std::string::npos || mod.find('\\') != std::string::npos || (mod.length() > 4 && mod.substr(mod.length() - 4) == ".zen")) {
            output << "import \"" << mod << "\";\n";
        } else {
            output << "import " << mod << ";\n";
        }
    }
    else if (auto* var_decl = dynamic_cast<VarDeclNode*>(node)) {
        indent();
        if (var_decl->type && var_decl->type->is_inferred) {
            output << "let " << var_decl->var_name;
            if (var_decl->initializer) {
                output << " = ";
                formatExpression(var_decl->initializer.get());
            }
            output << ";\n";
        } else {
            if (var_decl->type) {
                formatType(var_decl->type.get());
                output << " ";
            }
            output << var_decl->var_name;
            if (var_decl->initializer) {
                output << " = ";
                formatExpression(var_decl->initializer.get());
            }
            output << ";\n";
        }
    }
    else if (auto* if_stmt = dynamic_cast<IfStmtNode*>(node)) {
        indent();
        output << "if (";
        formatExpression(if_stmt->condition.get());
        output << ") {\n";
        indent_level++;
        for (const auto& s : if_stmt->then_branch) {
            formatStatement(s.get());
        }
        indent_level--;
        indent();
        output << "}";
        if (!if_stmt->else_branch.empty()) {
            output << " else {\n";
            indent_level++;
            for (const auto& s : if_stmt->else_branch) {
                formatStatement(s.get());
            }
            indent_level--;
            indent();
            output << "}\n";
        } else {
            output << "\n";
        }
    }
    else if (auto* while_stmt = dynamic_cast<WhileStmtNode*>(node)) {
        indent();
        output << "while (";
        formatExpression(while_stmt->condition.get());
        output << ") {\n";
        indent_level++;
        for (const auto& s : while_stmt->body) {
            formatStatement(s.get());
        }
        indent_level--;
        indent();
        output << "}\n";
    }
    else if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(node)) {
        indent();
        output << "return";
        if (return_stmt->expression) {
            output << " ";
            formatExpression(return_stmt->expression.get());
        }
        output << ";\n";
    }
    else if (auto* set_state = dynamic_cast<SetStateStmtNode*>(node)) {
        indent();
        output << "setState {\n";
        indent_level++;
        for (const auto& s : set_state->body) {
            formatStatement(s.get());
        }
        indent_level--;
        indent();
        output << "}\n";
    }
    else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(node)) {
        indent();
        if (class_decl->is_reactive) output << "reactive ";
        output << "class " << class_decl->class_name;
        
        if (!class_decl->primary_constructor_args.empty()) {
            output << "(";
            for (size_t i = 0; i < class_decl->primary_constructor_args.size(); ++i) {
                auto* arg = class_decl->primary_constructor_args[i].get();
                if (arg->type && !arg->type->is_inferred) {
                    formatType(arg->type.get());
                    output << " ";
                }
                output << arg->var_name;
                if (arg->initializer) {
                    output << " = ";
                    formatExpression(arg->initializer.get());
                }
                if (i < class_decl->primary_constructor_args.size() - 1) output << ", ";
            }
            output << ")";
        } else {
            output << "()";
        }
        
        if (!class_decl->implemented_interfaces.empty()) {
            output << " implements ";
            for (size_t i = 0; i < class_decl->implemented_interfaces.size(); ++i) {
                output << class_decl->implemented_interfaces[i];
                if (i < class_decl->implemented_interfaces.size() - 1) output << ", ";
            }
        }
        
        output << " {\n";
        indent_level++;
        
        for (const auto& f : class_decl->fields) {
            formatStatement(f.get());
        }
        
        for (const auto& m : class_decl->methods) {
            output << "\n";
            formatStatement(m.get());
        }
        
        indent_level--;
        indent();
        output << "}\n";
    }
    else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(node)) {
        indent();
        output << "interface " << interface_decl->interface_name << " {\n";
        indent_level++;
        for (const auto& m : interface_decl->methods) {
            indent();
            if (m->is_async) output << "async ";
            formatType(m->return_type.get());
            output << " " << m->function_name << "(";
            for (size_t i = 0; i < m->parameters.size(); ++i) {
                auto* p = m->parameters[i].get();
                if (p->type && !p->type->is_inferred) {
                    formatType(p->type.get());
                    output << " ";
                }
                output << p->var_name;
                if (i < m->parameters.size() - 1) output << ", ";
            }
            output << ");\n";
        }
        indent_level--;
        indent();
        output << "}\n";
    }
    else if (auto* agentic_fn = dynamic_cast<AgenticFunctionNode*>(node)) {
        indent();
        if (agentic_fn->is_async) output << "async ";
        formatType(agentic_fn->return_type.get());
        output << " " << agentic_fn->function_name << "(";
        for (size_t i = 0; i < agentic_fn->parameters.size(); ++i) {
            auto* p = agentic_fn->parameters[i].get();
            if (p->type && !p->type->is_inferred) {
                formatType(p->type.get());
                output << " ";
            }
            output << p->var_name;
            if (i < agentic_fn->parameters.size() - 1) output << ", ";
        }
        output << ") {\n";
        indent_level++;
        indent();
        output << "prompt: \"" << agentic_fn->prompt_template << "\"\n";
        indent_level--;
        indent();
        output << "}\n";
    }
    else if (auto* fn = dynamic_cast<FunctionNode*>(node)) {
        indent();
        if (fn->is_async) output << "async ";
        formatType(fn->return_type.get());
        output << " " << fn->function_name << "(";
        for (size_t i = 0; i < fn->parameters.size(); ++i) {
            auto* p = fn->parameters[i].get();
            if (p->type && !p->type->is_inferred) {
                formatType(p->type.get());
                output << " ";
            }
            output << p->var_name;
            if (p->initializer) {
                output << " = ";
                formatExpression(p->initializer.get());
            }
            if (i < fn->parameters.size() - 1) output << ", ";
        }
        output << ") {\n";
        indent_level++;
        for (const auto& s : fn->body) {
            formatStatement(s.get());
        }
        indent_level--;
        indent();
        output << "}\n";
    }
    else if (auto* expr = dynamic_cast<ExprNode*>(node)) {
        indent();
        formatExpression(expr);
        output << ";\n";
    }
}

void Formatter::formatPattern(PatternNode* pattern) {
    if (!pattern) return;
    if (auto* wild = dynamic_cast<WildcardPatternNode*>(pattern)) {
        output << "_";
    } else if (auto* lit = dynamic_cast<LiteralPatternNode*>(pattern)) {
        formatExpression(lit->literal.get());
    } else if (auto* id = dynamic_cast<IdentifierPatternNode*>(pattern)) {
        output << id->name;
    } else if (auto* en = dynamic_cast<EnumPatternNode*>(pattern)) {
        output << en->enum_name << "." << en->variant_name;
        if (!en->sub_patterns.empty()) {
            output << "(";
            for (size_t i = 0; i < en->sub_patterns.size(); ++i) {
                formatPattern(en->sub_patterns[i].get());
                if (i < en->sub_patterns.size() - 1) output << ", ";
            }
            output << ")";
        }
    } else if (auto* st = dynamic_cast<StructPatternNode*>(pattern)) {
        output << st->struct_name << " { ";
        for (size_t i = 0; i < st->fields.size(); ++i) {
            output << st->fields[i].first;
            if (st->fields[i].second) {
                output << ": ";
                formatPattern(st->fields[i].second.get());
            }
            if (i < st->fields.size() - 1) output << ", ";
        }
        output << " }";
    }
}

std::string Formatter::format(ProgramNode* program) {
    if (!program) return "";
    output.str("");
    output.clear();
    indent_level = 0;
    
    bool is_first = true;
    for (const auto& s : program->statements) {
        if (!is_first) {
            // Add spacing between functions and classes
            if (dynamic_cast<ClassDeclNode*>(s.get()) || 
                dynamic_cast<FunctionNode*>(s.get()) ||
                dynamic_cast<InterfaceDeclNode*>(s.get())) {
                output << "\n";
            }
        }
        is_first = false;
        formatStatement(s.get());
    }
    return output.str();
}
