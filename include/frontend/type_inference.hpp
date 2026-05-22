#ifndef TYPE_INFERENCE_HPP
#define TYPE_INFERENCE_HPP

#include "../ast/ast.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>

/**
 * Type Inference System for Zenith Compiler
 * 
 * This module provides bidirectional type inference to reduce boilerplate.
 * It supports:
 * - Local variable type inference (let x = 5 → Int)
 * - Function return type inference
 * - Lambda parameter type inference
 * - Generic type inference
 * - Option/Result type inference
 */

class TypeInferencer {
private:
    // Type constraints for unification
    struct TypeConstraint {
        std::string type_var;
        std::string concrete_type;
        ASTNode* source_node;
    };
    
    std::vector<TypeConstraint> constraints;
    std::unordered_map<std::string, std::string> type_substitutions;
    
    // Known primitive types
    static const std::unordered_map<std::string, bool>& getPrimitiveTypes() {
        static const std::unordered_map<std::string, bool> primitives = {
            {"Int", true}, {"Float", true}, {"Bool", true}, {"String", true},
            {"Void", true}, {"Char", true},
            {"i8", true}, {"i16", true}, {"i32", true}, {"i64", true},
            {"u8", true}, {"u16", true}, {"u32", true}, {"u64", true},
            {"f32", true}, {"f64", true}
        };
        return primitives;
    }
    
    // Check if a type is a primitive
    bool isPrimitiveType(const std::string& type) const {
        return getPrimitiveTypes().count(type) > 0;
    }
    
    // Check if a type is a generic container (List<T>, Map<K,V>, Option<T>, Result<T,E>)
    bool isGenericType(const std::string& type) const {
        return type.find('<') != std::string::npos;
    }
    
    // Extract base type name from generic (List<Int> → List)
    std::string getBaseTypeName(const std::string& type) const {
        size_t pos = type.find('<');
        return pos != std::string::npos ? type.substr(0, pos) : type;
    }
    
    // Extract generic parameters (List<Int> → ["Int"])
    std::vector<std::string> getGenericParams(const std::string& type) const {
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
                    if (!current.empty()) params.push_back(current);
                    break;
                }
                depth--;
            }
            else if (c == ',' && depth == 0) {
                if (!current.empty()) params.push_back(current);
                current.clear();
            }
            else {
                current += c;
            }
        }
        
        return params;
    }
    
    // Unify two types, returning true if successful
    bool unify(const std::string& t1, const std::string& t2, ASTNode* node) {
        if (t1 == t2) return true;
        
        // If t1 is a type variable, substitute it
        if (type_substitutions.count(t1)) {
            return unify(type_substitutions[t1], t2, node);
        }
        
        // If t2 is a type variable, substitute it
        if (type_substitutions.count(t2)) {
            return unify(t1, type_substitutions[t2], node);
        }
        
        // Both are concrete types - check compatibility
        if (isPrimitiveType(t1) && isPrimitiveType(t2)) {
            // Allow Int ↔ i32, Float ↔ f64, etc.
            return normalizeType(t1) == normalizeType(t2);
        }
        
        // Handle generic types
        if (isGenericType(t1) && isGenericType(t2)) {
            std::string base1 = getBaseTypeName(t1);
            std::string base2 = getBaseTypeName(t2);
            
            if (base1 != base2) return false;
            
            auto params1 = getGenericParams(t1);
            auto params2 = getGenericParams(t2);
            
            if (params1.size() != params2.size()) return false;
            
            for (size_t i = 0; i < params1.size(); i++) {
                if (!unify(params1[i], params2[i], node)) {
                    return false;
                }
            }
            return true;
        }
        
        return false;
    }
    
    // Normalize type names (Int ↔ i32, Float ↔ f64, etc.)
    std::string normalizeType(const std::string& type) const {
        if (type == "Int" || type == "i32") return "Int";
        if (type == "Float" || type == "f64" || type == "Double") return "Float";
        if (type == "Bool" || type == "boolean") return "Bool";
        if (type == "String" || type == "str") return "String";
        return type;
    }
    
public:
    TypeInferencer() = default;
    
    /**
     * Infer type from an expression node
     * Returns the inferred type name, or empty string if inference fails
     */
    std::string inferType(ExprNode* expr) {
        if (!expr) return "";
        
        // Literal types - direct inference
        if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
            return num->is_float ? "Float" : "Int";
        }
        
        if (auto* str = dynamic_cast<StringLiteralNode*>(expr)) {
            return "String";
        }
        
        if (auto* b = dynamic_cast<BoolLiteralNode*>(expr)) {
            return "Bool";
        }
        
        if (auto* null_lit = dynamic_cast<NullLiteralNode*>(expr)) {
            return "Null";
        }
        
        // Option types
        if (auto* opt = dynamic_cast<OptionExprNode*>(expr)) {
            if (opt->kind == OptionExprNode::OptionKind::None) {
                return "Option<Void>";  // Will be refined by context
            } else if (opt->value) {
                std::string val_type = inferType(opt->value.get());
                return "Option<" + val_type + ">";
            }
            return "Option<Void>";
        }
        
        // Result types
        if (auto* res = dynamic_cast<ResultExprNode*>(expr)) {
            std::string val_type = res->value ? inferType(res->value.get()) : "Void";
            if (res->kind == ResultExprNode::ResultKind::Ok) {
                return "Result<" + val_type + ", Error>";
            } else {
                return "Result<Void, " + val_type + ">";
            }
        }
        
        // List literals
        if (auto* list = dynamic_cast<ListLiteralNode*>(expr)) {
            if (list->elements.empty()) {
                return "List<Void>";  // Will be refined by context
            }
            std::string elem_type = inferType(list->elements[0].get());
            
            // Verify all elements have compatible types
            for (size_t i = 1; i < list->elements.size(); i++) {
                std::string other_type = inferType(list->elements[i].get());
                if (elem_type != other_type) {
                    // Could add error reporting here
                }
            }
            
            return "List<" + elem_type + ">";
        }
        
        // Map literals
        if (auto* map = dynamic_cast<MapLiteralNode*>(expr)) {
            if (map->entries.empty()) {
                return "Map<Void, Void>";
            }
            std::string key_type = inferType(map->entries[0].first.get());
            std::string val_type = inferType(map->entries[0].second.get());
            return "Map<" + key_type + ", " + val_type + ">";
        }
        
        // Binary expressions
        if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
            std::string left_type = inferType(binary->left.get());
            std::string right_type = inferType(binary->right.get());
            
            // String concatenation
            if (binary->op == "+" && (left_type == "String" || right_type == "String")) {
                return "String";
            }
            
            // Arithmetic operations
            if (binary->op == "+" || binary->op == "-" || binary->op == "*" || binary->op == "/") {
                if (left_type == "Float" || right_type == "Float") return "Float";
                if (left_type == "Int" || right_type == "Int") return "Int";
            }
            
            // Comparison operations
            if (binary->op == "==" || binary->op == "!=" || 
                binary->op == "<" || binary->op == ">" || 
                binary->op == "<=" || binary->op == ">=") {
                return "Bool";
            }
            
            // Logical operations
            if (binary->op == "&&" || binary->op == "||") {
                return "Bool";
            }
            
            return left_type;  // Default to left type
        }
        
        // Lambda expressions - infer from context or annotations
        if (auto* lambda = dynamic_cast<LambdaNode*>(expr)) {
            std::string ret_type = lambda->return_type ? lambda->return_type->type_name : "Void";
            
            // Build function type signature
            std::string sig = "(";
            for (size_t i = 0; i < lambda->parameters.size(); i++) {
                if (i > 0) sig += ", ";
                
                // Infer parameter type from initializer if not specified
                if (lambda->parameters[i]->type->is_inferred && lambda->parameters[i]->initializer) {
                    std::string param_type = inferType(lambda->parameters[i]->initializer.get());
                    sig += param_type;
                } else {
                    sig += lambda->parameters[i]->type->type_name;
                }
            }
            sig += ") -> " + ret_type;
            
            return "Function<" + sig + ">";
        }
        
        // Await expressions - unwrap async type
        if (auto* await = dynamic_cast<AwaitExprNode*>(expr)) {
            std::string async_type = inferType(await->expression.get());
            // Remove Async/Task wrapper if present
            if (async_type.find("Async<") == 0 || async_type.find("Task<") == 0) {
                auto params = getGenericParams(async_type);
                if (!params.empty()) return params[0];
            }
            return async_type;
        }
        
        // Try expressions - extract Ok type from Result
        if (auto* try_expr = dynamic_cast<TryExprNode*>(expr)) {
            std::string result_type = inferType(try_expr->expression.get());
            if (result_type.find("Result<") == 0) {
                auto params = getGenericParams(result_type);
                if (!params.empty()) return params[0];
            }
            return result_type;
        }
        
        // Match expressions - unify all arm types
        if (auto* match = dynamic_cast<MatchExprNode*>(expr)) {
            std::string subject_type = inferType(match->subject.get());
            std::string result_type;
            
            for (const auto& arm : match->arms) {
                std::string arm_type = inferType(arm.second.get());
                if (result_type.empty()) {
                    result_type = arm_type;
                } else if (result_type != arm_type) {
                    // Type mismatch in match arms - could report error
                }
            }
            
            return result_type;
        }
        
        // Property access - type depends on object
        if (auto* prop = dynamic_cast<PropertyAccessNode*>(expr)) {
            // This requires symbol table lookup - handled by semantic analyzer
            return "";
        }
        
        // Function/Method calls - require symbol table
        if (dynamic_cast<FunctionCallNode*>(expr) || 
            dynamic_cast<MethodCallNode*>(expr)) {
            return "";
        }
        
        // Identifiers - require symbol table lookup
        if (dynamic_cast<IdentifierNode*>(expr)) {
            return "";
        }
        
        // UI Components
        if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
            return "UI<" + ui->component_type + ">";
        }
        
        return "";
    }
    
    /**
     * Check if a type can be inferred from context
     */
    bool canInferFromContext(ExprNode* expr) {
        if (!expr) return false;
        
        // Direct literals can always be inferred
        if (dynamic_cast<NumberLiteralNode*>(expr) ||
            dynamic_cast<StringLiteralNode*>(expr) ||
            dynamic_cast<BoolLiteralNode*>(expr)) {
            return true;
        }
        
        // Collection literals
        if (dynamic_cast<ListLiteralNode*>(expr) ||
            dynamic_cast<MapLiteralNode*>(expr)) {
            return true;
        }
        
        // Expressions with known return types
        if (dynamic_cast<BinaryExprNode*>(expr) ||
            dynamic_cast<OptionExprNode*>(expr) ||
            dynamic_cast<ResultExprNode*>(expr)) {
            return true;
        }
        
        return false;
    }
    
    /**
     * Create a TypeNode with inferred type from expression
     */
    std::unique_ptr<TypeNode> createInferredType(ExprNode* expr) {
        std::string inferred = inferType(expr);
        auto type_node = std::make_unique<TypeNode>(inferred.empty() ? "Void" : inferred);
        type_node->is_inferred = true;
        
        // Parse generic parameters if present
        if (isGenericType(inferred)) {
            auto params = getGenericParams(inferred);
            for (const auto& param : params) {
                type_node->generics.push_back(std::make_unique<TypeNode>(param));
            }
        }
        
        return type_node;
    }
};

#endif // TYPE_INFERENCE_HPP
