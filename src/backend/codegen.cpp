#include "../../include/backend/codegen.h"
#include <iostream>
#include <regex>

static std::string cppIdentifier(const std::string& identifier) {
    static const std::unordered_set<std::string> reserved = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
        "bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class",
        "compl", "concept", "const", "constexpr", "const_cast", "continue", "co_await",
        "co_return", "co_yield", "decltype", "default", "delete", "do", "double",
        "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false",
        "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable",
        "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
        "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
        "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
        "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
        "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };
    return reserved.count(identifier) ? identifier + "_zen" : identifier;
}

static std::string mapBuiltinInterfaceBase(const std::string& interface_name) {
    if (interface_name == "Scene") {
        return "zenith::game::Scene";
    }
    return interface_name;
}

static bool classImplementsInterface(const ClassDeclNode* node, const std::string& interface_name) {
    for (const auto& implemented : node->implemented_interfaces) {
        if (implemented == interface_name) {
            return true;
        }
    }
    return false;
}

static bool isMathConstructorName(const std::string& name) {
    return name == "Vec2" || name == "Vec3" || name == "Vec4" || name == "Mat4";
}

static bool isCompileTimeAssetConstructorName(const std::string& name) {
    return name == "texture" || name == "audio" || name == "mesh" || name == "shader" || name == "material";
}

static std::string compileTimeAssetConstructorType(const std::string& name) {
    if (name == "texture") return "TextureHandleView";
    if (name == "audio") return "AudioHandleView";
    if (name == "mesh") return "MeshHandleView";
    if (name == "shader") return "ShaderHandleView";
    if (name == "material") return "MaterialHandleView";
    return "";
}

static bool isNumericTypeName(const std::string& type_name) {
    return type_name == "Int" || type_name == "Float";
}

static bool isVectorTypeName(const std::string& type_name) {
    return type_name == "Vec2" || type_name == "Vec3" || type_name == "Vec4";
}

static bool isMatrixTypeName(const std::string& type_name) {
    return type_name == "Mat4";
}

static bool isMathValueTypeName(const std::string& type_name) {
    return isVectorTypeName(type_name) || isMatrixTypeName(type_name);
}

static std::string inferMathBinaryResult(const std::string& left_type, const std::string& op, const std::string& right_type) {
    if (op == "+" || op == "-") {
        if (left_type == right_type && isMathValueTypeName(left_type)) {
            return left_type;
        }
        return "";
    }

    if (op == "*") {
        if (isVectorTypeName(left_type) && isNumericTypeName(right_type)) return left_type;
        if (isNumericTypeName(left_type) && isVectorTypeName(right_type)) return right_type;
        if (left_type == "Mat4" && right_type == "Mat4") return "Mat4";
        if (left_type == "Mat4" && right_type == "Vec4") return "Vec4";
        if (left_type == "Mat4" && isNumericTypeName(right_type)) return "Mat4";
        return "";
    }

    if (op == "/") {
        if (isVectorTypeName(left_type) && isNumericTypeName(right_type)) return left_type;
        if (left_type == "Mat4" && isNumericTypeName(right_type)) return "Mat4";
        return "";
    }

    return "";
}

static std::string inferGeneratedExpressionType(const ExprNode* expr) {
    if (!expr) return "";
    if (auto* id = dynamic_cast<const IdentifierNode*>(expr)) return id->type_hint;
    if (auto* str = dynamic_cast<const StringLiteralNode*>(expr)) return "String";
    if (auto* num = dynamic_cast<const NumberLiteralNode*>(expr)) return num->is_float ? "Float" : "Int";
    if (dynamic_cast<const BoolLiteralNode*>(expr)) return "Bool";
    if (auto* list = dynamic_cast<const ListLiteralNode*>(expr)) return list->elements.empty() ? "List<Void>" : "List";
    if (auto* map = dynamic_cast<const MapLiteralNode*>(expr)) return map->entries.empty() ? "Map<Void,Void>" : "Map";
    if (auto* ui = dynamic_cast<const UIComponentNode*>(expr)) {
        if (isMathConstructorName(ui->component_type)) return ui->component_type;
        if (isCompileTimeAssetConstructorName(ui->component_type)) return compileTimeAssetConstructorType(ui->component_type);
        return "";
    }
    if (auto* prop = dynamic_cast<const PropertyAccessNode*>(expr)) {
        const std::string object_type = inferGeneratedExpressionType(prop->object.get());
        if (isVectorTypeName(object_type)) return "Float";
        if (object_type == "Mat4") return "Float";
        if (object_type.rfind("List<", 0) == 0 || object_type == "List") {
            if (prop->property_name == "size" || prop->property_name == "length") return "Int";
        }
        if (object_type.rfind("Map<", 0) == 0 || object_type == "Map") {
            if (prop->property_name == "size" || prop->property_name == "length") return "Int";
        }
        return "";
    }
    if (auto* call = dynamic_cast<const MethodCallNode*>(expr)) {
        const std::string object_type = inferGeneratedExpressionType(call->object.get());
        if (call->method_name == "length" && call->arguments.empty()) {
            if (isVectorTypeName(object_type)) return "Float";
            return "Int";
        }
        if (call->method_name == "lengthSquared" && call->arguments.empty() && isVectorTypeName(object_type)) return "Float";
        if (call->method_name == "normalized" && call->arguments.empty() && isVectorTypeName(object_type)) return object_type;
        return "";
    }
    if (auto* binary = dynamic_cast<const BinaryExprNode*>(expr)) {
        const std::string left_type = inferGeneratedExpressionType(binary->left.get());
        const std::string right_type = inferGeneratedExpressionType(binary->right.get());
        if (binary->op == "=" || binary->op == "+=" || binary->op == "-=" || binary->op == "*=" || binary->op == "/=") {
            return left_type;
        }
        if (binary->op == "+" && (binary->is_string_concat || left_type == "String" || right_type == "String")) {
            return "String";
        }
        const std::string math_result = inferMathBinaryResult(left_type, binary->op, right_type);
        if (!math_result.empty()) return math_result;
        if (isNumericTypeName(left_type) && isNumericTypeName(right_type)) {
            return left_type == "Float" || right_type == "Float" ? "Float" : "Int";
        }
        if (binary->op == "==" || binary->op == "!=" || binary->op == "<" || binary->op == ">" ||
            binary->op == "<=" || binary->op == ">=") {
            return "Bool";
        }
        return "";
    }
    return "";
}

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
    else if (base == "Canvas") base = "zenith::Canvas";
    else if (base == "EntityId") base = "zenith::game::EntityId";
    else if (base == "Scene") base = "zenith::game::Scene";
    else if (base == "Transform2DView") base = "zenith::game::Transform2DView";
    else if (base == "Body2DView") base = "zenith::game::Body2DView";
    else if (base == "BoxCollider2DView") base = "zenith::game::BoxCollider2DView";
    else if (base == "CircleCollider2DView") base = "zenith::game::CircleCollider2DView";
    else if (base == "CapsuleCollider2DView") base = "zenith::game::CapsuleCollider2DView";
    else if (base == "Camera2DView") base = "zenith::game::Camera2DView";
    else if (base == "AudioListener2DView") base = "zenith::game::AudioListener2DView";
    else if (base == "TextureHandleView") base = "zenith::game::TextureHandleView";
    else if (base == "AudioHandleView") base = "zenith::game::AudioHandleView";
    else if (base == "MeshHandleView") base = "zenith::game::MeshHandleView";
    else if (base == "ShaderHandleView") base = "zenith::game::ShaderHandleView";
    else if (base == "MaterialHandleView") base = "zenith::game::MaterialHandleView";
    else if (base == "Vec2") base = "zenith::physics::Vec2";
    else if (base == "Vec3") base = "zenith::physics::Vec3";
    else if (base == "Vec4") base = "zenith::physics::Vec4";
    else if (base == "Mat4") base = "zenith::physics::Mat4";
    else if (base == "Sprite2DView") base = "zenith::game::Sprite2DView";
    else if (base == "Tilemap2DView") base = "zenith::game::Tilemap2DView";
    else if (base == "Character2DView") base = "zenith::game::Character2DView";
    else if (base == "AudioSource2DView") base = "zenith::game::AudioSource2DView";
    else if (base == "RaycastHit2DResult") base = "zenith::game::RaycastHit2DResult";
    else if (base == "Transform3DView") base = "zenith::game::Transform3DView";
    else if (base == "Body3DView") base = "zenith::game::Body3DView";
    else if (base == "BoxCollider3DView") base = "zenith::game::BoxCollider3DView";
    else if (base == "SphereCollider3DView") base = "zenith::game::SphereCollider3DView";
    else if (base == "Camera3DView") base = "zenith::game::Camera3DView";
    else if (base == "AudioListener3DView") base = "zenith::game::AudioListener3DView";
    else if (base == "PointLight3DView") base = "zenith::game::PointLight3DView";
    else if (base == "DirectionalLight3DView") base = "zenith::game::DirectionalLight3DView";
    else if (base == "Mesh3DView") base = "zenith::game::Mesh3DView";
    else if (base == "MaterialPropertyView") base = "zenith::game::MaterialPropertyView";
    else if (base == "MaterialPropertyOptionView") base = "zenith::game::MaterialPropertyOptionView";
    else if (base == "Character3DView") base = "zenith::game::Character3DView";
    else if (base == "AudioSource3DView") base = "zenith::game::AudioSource3DView";
    else if (base == "RaycastHit3DResult") base = "zenith::game::RaycastHit3DResult";
    else if (base == "UI") base = "zenith::UIElement";
    else if (base == "List") base = "std::vector";
    else if (base == "Map") base = "std::unordered_map";
    else if (base == "Future") base = "zenith::stdlib::Future";
    else if (base == "Promise") base = "zenith::stdlib::Promise";
    else if (base == "Ref") base = "zenith::mem::Ref";
    else if (base == "Weak") base = "zenith::mem::Weak";
    else if (base == "Temp") base = "zenith::mem::Temp";
    else if (base == "Pool") base = "zenith::mem::Pool";
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

std::string CodeGenerator::mapParameterType(TypeNode* type) {
    if (type && type->type_name == "Canvas") {
        return "zenith::Canvas&";
    }
    return mapType(type);
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
    else if (base == "TextureHandleView") base = "zenith::game::TextureHandleView";
    else if (base == "AudioHandleView") base = "zenith::game::AudioHandleView";
    else if (base == "MeshHandleView") base = "zenith::game::MeshHandleView";
    else if (base == "ShaderHandleView") base = "zenith::game::ShaderHandleView";
    else if (base == "MaterialHandleView") base = "zenith::game::MaterialHandleView";
    else if (base == "Vec2") base = "zenith::physics::Vec2";
    else if (base == "Vec3") base = "zenith::physics::Vec3";
    else if (base == "Vec4") base = "zenith::physics::Vec4";
    else if (base == "Mat4") base = "zenith::physics::Mat4";
    else if (base == "RaycastHit2DResult") base = "zenith::game::RaycastHit2DResult";
    else if (base == "RaycastHit3DResult") base = "zenith::game::RaycastHit3DResult";
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
    if (auto* unary = dynamic_cast<UnaryExprNode*>(expr)) {
        return unary->op + generateExpression(unary->expression.get());
    }
    if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        return cppIdentifier(id->name);
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
        if (prop->property_name == "size" || prop->property_name == "length") {
            return generateExpression(prop->object.get()) + ".size()";
        }
        return generateExpression(prop->object.get()) + "." + cppIdentifier(prop->property_name);
    }
    if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        const std::string object_type = inferGeneratedExpressionType(call->object.get());
        if (call->method_name == "run" && call->arguments.empty()) {
            return "zenith::runGameLoop(" + generateExpression(call->object.get()) + ")";
        }
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
        if (m_name == "length" && (object_type.rfind("List<", 0) == 0 || object_type.rfind("Map<", 0) == 0 || object_type == "List" || object_type == "Map")) {
            m_name = "size";
        }
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
        if (fname == "String") {
            if (fcall->arguments.empty()) return "std::string()";
            return "zenith::toString(" + generateExpression(fcall->arguments[0].get()) + ")";
        }
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
        std::string res = cppIdentifier(fname) + "(";
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
        if (isMathConstructorName(ui->component_type)) {
            std::string res = "zenith::physics::" + ui->component_type + "(";
            for (size_t i = 0; i < ui->children.size(); ++i) {
                res += generateExpression(ui->children[i].get());
                if (i < ui->children.size() - 1) res += ", ";
            }
            res += ")";
            return res;
        }
        if (isCompileTimeAssetConstructorName(ui->component_type)) {
            if (ui->component_type == "texture") {
                return "zenith::game::TextureHandleView{" + generateExpression(ui->children[0].get()) + ", 0, 0, false, 0, 0, 0, 0}";
            }
            if (ui->component_type == "audio") {
                return "zenith::game::AudioHandleView{" + generateExpression(ui->children[0].get()) + ", 0, 0, false, 0.0f, false}";
            }
            if (ui->component_type == "mesh") {
                return "zenith::game::MeshHandleView{" + generateExpression(ui->children[0].get()) + ", 0, 0, false, 0, 0, 0, 0}";
            }
            if (ui->component_type == "shader") {
                return "zenith::game::ShaderHandleView{" + generateExpression(ui->children[0].get()) + ", 0, 0, false, 0}";
            }
            if (ui->component_type == "material") {
                std::string shader_path = ui->children.size() > 1 ? generateExpression(ui->children[1].get()) : "\"\"";
                return "zenith::game::MaterialHandleView{" + generateExpression(ui->children[0].get()) + ", 0, 0, false, " + shader_path + ", 0}";
            }
        }

        std::string res;
        bool is_class = class_names.count(ui->component_type) > 0;
        bool is_fn = function_names.count(ui->component_type) > 0;
        bool is_variable_call = !ui->component_type.empty() && std::islower(static_cast<unsigned char>(ui->component_type[0]));
        bool is_custom = is_class || is_fn || is_variable_call;
        bool is_no_first_param = (!is_custom && ui->component_type == "Slider");
        
        if (is_custom) {
            res = ui->component_type + "(";
        } else {
            res = "zenith::UI::" + ui->component_type + "(";
        }
        
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
        
        if (!is_custom) {
            res += (is_no_first_param ? "{" : ", {");
            bool first_arg = true;
            for (size_t i = 0; i < ui->named_args.size(); ++i) {
                std::string key = ui->named_args[i].first;
                if (key == "validator") continue;
                std::string val = generateExpression(ui->named_args[i].second.get());
                if (key.rfind("on", 0) == 0) {
                    if (val.front() != '"') {
                        val = "\"" + val + "\"";
                    }
                }
                if (!first_arg) res += ", ";
                res += "{\"" + key + "\", zenith::toString(" + val + ")}";
                first_arg = false;
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
        } else if (current_function_is_exported_with_string_return) {
            output << "_zenith_ret = " << generateExpression(return_stmt->expression.get()) << ";\n";
        } else {
            output << "return " << generateExpression(return_stmt->expression.get()) << ";\n";
        }
    } else if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt)) {
        indent();
        std::string type_name = var_decl->type->type_name;
        std::string var_name = cppIdentifier(var_decl->var_name);
        // Handle type inference - use 'auto' for inferred types
        bool use_auto = var_decl->type->is_inferred && var_decl->initializer != nullptr &&
                        var_decl->type->type_name != "List" && var_decl->type->type_name != "Map";
        
        if (interface_names.count(type_name)) {
            if (var_decl->initializer) {
                if (auto* id = dynamic_cast<IdentifierNode*>(var_decl->initializer.get())) {
                    output << type_name << "& " << var_name << " = " << cppIdentifier(id->name) << ";\n";
                } else {
                    output << "auto temp_" << var_name << " = " << generateExpression(var_decl->initializer.get()) << ";\n";
                    indent();
                    output << type_name << "& " << var_name << " = temp_" << var_name << ";\n";
                }
            } else {
                output << type_name << "* " << var_name << " = nullptr;\n";
            }
        } else {
            if (use_auto) {
                // Use C++ auto keyword for type inference
                output << "auto " << var_name;
                if (var_decl->initializer) {
                    output << " = " << generateExpression(var_decl->initializer.get());
                }
                output << ";\n";
            } else if (var_decl->type && (var_decl->type->type_name == "Ref" || var_decl->type->type_name == "Weak")
                       && !var_decl->type->generics.empty()) {
                // RC/GC smart pointer variable declaration
                std::string inner_type = mapType(var_decl->type->generics[0].get());
                std::string cpp_type   = mapType(var_decl->type.get());
                output << cpp_type << " " << var_name;
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
                output << mapType(var_decl->type.get()) << " " << var_name;
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
    } else if (auto* for_stmt = dynamic_cast<ForStmtNode*>(stmt)) {
        indent();
        output << "for (";
        if (for_stmt->initializer) {
            if (auto* var_decl = dynamic_cast<VarDeclNode*>(for_stmt->initializer.get())) {
                output << mapType(var_decl->type.get()) << " " << cppIdentifier(var_decl->var_name);
                if (var_decl->initializer) {
                    output << " = " << generateExpression(var_decl->initializer.get());
                }
            } else if (auto* expr = dynamic_cast<ExprNode*>(for_stmt->initializer.get())) {
                output << generateExpression(expr);
            }
        }
        output << "; ";
        if (for_stmt->condition) {
            output << generateExpression(for_stmt->condition.get());
        }
        output << "; ";
        if (for_stmt->update) {
            output << generateExpression(for_stmt->update.get());
        }
        output << ") {\n";
        indent_level++;
        for (const auto& s : for_stmt->body) generateStatement(s.get());
        indent_level--;
        indent(); output << "}\n";
    } else if (dynamic_cast<BreakStmtNode*>(stmt)) {
        indent();
        output << "break;\n";
    } else if (dynamic_cast<ContinueStmtNode*>(stmt)) {
        indent();
        output << "continue;\n";
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
    } else if (auto* local_function = dynamic_cast<FunctionNode*>(stmt)) {
        indent();
        output << "auto " << cppIdentifier(local_function->function_name) << " = [&](";
        for (size_t i = 0; i < local_function->parameters.size(); ++i) {
            auto& param = local_function->parameters[i];
            output << mapParameterType(param->type.get()) << " " << cppIdentifier(param->var_name);
            if (param->initializer) {
                output << " = " << generateExpression(param->initializer.get());
            }
            if (i < local_function->parameters.size() - 1) output << ", ";
        }
        output << ") {\n";
        indent_level++;
        for (const auto& local_stmt : local_function->body) {
            generateStatement(local_stmt.get());
        }
        indent_level--;
        indent();
        output << "};\n";
    } else if (auto* expr = dynamic_cast<ExprNode*>(stmt)) {
        indent();
        output << generateExpression(expr) << ";\n";
    }
}

void CodeGenerator::generateAgenticFunction(AgenticFunctionNode* node) {
    indent();
    output << mapType(node->return_type.get()) << " " << node->function_name << "(";
    
    for (size_t i = 0; i < node->parameters.size(); ++i) {
        output << mapParameterType(node->parameters[i]->type.get()) << " " << node->parameters[i]->var_name;
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

    indent();
    if (is_inside_class_method) {
        output << "zenith::LLMClient client(this->url.empty() ? \"http://localhost:11434\" : this->url);\n";
    } else {
        output << "zenith::LLMClient client(\"http://localhost:11434\");\n";
    }

    if (node->is_streaming) {
        indent();
        output << "return client.promptStream(prompt, [](const std::string& chunk) {\n";
        indent_level++;
        indent();
        output << "std::cout << chunk << std::flush;\n";
        indent_level--;
        indent();
        output << "}, " << (image_path_var.empty() ? "\"\"" : image_path_var) << ");\n";
    } else {
        indent();
        output << "return client.prompt(prompt, " << (image_path_var.empty() ? "\"\"" : image_path_var) << ");\n";
    }

    indent_level--;
    indent(); output << "}\n\n";
}

void CodeGenerator::generateFunction(FunctionNode* node) {
    if (node->is_foreign) {
        if (node->foreign_abi == "python") {
            indent();
            output << mapType(node->return_type.get()) << " " << node->function_name << "(";
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << mapParameterType(node->parameters[i]->type.get()) << " " << node->parameters[i]->var_name;
                if (i < node->parameters.size() - 1) output << ", ";
            }
            output << ") {\n";
            indent_level++;
            indent(); output << "// --- Python FFI Bridge: Call function ---\n";
            std::string ret_type = mapType(node->return_type.get());
            indent();
            if (ret_type != "void") {
                output << "return ";
            }
            output << "zenith::ffi::PythonFFIBridge::call<" << ret_type << ">(\"" << node->function_name << "\"";
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << ", " << node->parameters[i]->var_name;
            }
            output << ");\n";
            indent_level--;
            indent(); output << "}\n\n";
            return;
        } else if (node->foreign_abi == "C") {
            std::string lib_path = node->attributes.count("library") ? node->attributes.at("library") : "";
            
            if (lib_path == "builtin") {
                output << "extern \"C\" " << mapTypeForCFFI(node->return_type.get(), true) 
                       << " _zenith_builtin_" << node->function_name << "(";
                for (size_t i = 0; i < node->parameters.size(); ++i) {
                    output << mapTypeForCFFI(node->parameters[i]->type.get(), false);
                    if (i < node->parameters.size() - 1) output << ", ";
                }
                output << ");\n\n";
            } else if (lib_path.empty()) {
                output << "extern \"C\" " << mapTypeForCFFI(node->return_type.get(), true) 
                       << " " << node->function_name << "(";
                for (size_t i = 0; i < node->parameters.size(); ++i) {
                    output << mapTypeForCFFI(node->parameters[i]->type.get(), false);
                    if (i < node->parameters.size() - 1) output << ", ";
                }
                output << ");\n\n";
            }
            
            indent();
            output << mapType(node->return_type.get()) << " " << node->function_name << "(";
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << mapParameterType(node->parameters[i]->type.get()) << " " << node->parameters[i]->var_name;
                if (i < node->parameters.size() - 1) output << ", ";
            }
            output << ") {\n";
            indent_level++;
            
            if (lib_path == "builtin") {
                indent();
                if (mapType(node->return_type.get()) != "void") {
                    output << "return ";
                }
                output << "::_zenith_builtin_" << node->function_name << "(";
                for (size_t i = 0; i < node->parameters.size(); ++i) {
                    if (mapType(node->parameters[i]->type.get()) == "std::string") {
                        output << node->parameters[i]->var_name << ".c_str()";
                    } else {
                        output << node->parameters[i]->var_name;
                    }
                    if (i < node->parameters.size() - 1) output << ", ";
                }
                output << ");\n";
            } else if (lib_path.empty()) {
                indent();
                if (mapType(node->return_type.get()) != "void") {
                    output << "return ";
                }
                output << "::" << node->function_name << "(";
                for (size_t i = 0; i < node->parameters.size(); ++i) {
                    if (mapType(node->parameters[i]->type.get()) == "std::string") {
                        output << node->parameters[i]->var_name << ".c_str()";
                    } else {
                        output << node->parameters[i]->var_name;
                    }
                    if (i < node->parameters.size() - 1) output << ", ";
                }
                output << ");\n";
            } else {
                std::string ret_c = mapTypeForCFFI(node->return_type.get(), true);
                indent();
                output << "typedef " << ret_c << " (*FuncType)(";
                for (size_t i = 0; i < node->parameters.size(); ++i) {
                    output << mapTypeForCFFI(node->parameters[i]->type.get(), false);
                    if (i < node->parameters.size() - 1) output << ", ";
                }
                output << ");\n";
                
                indent();
                output << "static FuncType func = (FuncType)zenith::ffi::LibraryManager::getInstance().loadLibrary(\"" << lib_path << "\").getFunction<void*>(\"" << node->function_name << "\");\n";
                
                indent();
                output << "if (!func) {\n";
                indent_level++;
                indent();
                output << "std::cerr << \"[FFI Error] Failed to load function '" << node->function_name << "' from '" << lib_path << "'\" << std::endl;\n";
                if (mapType(node->return_type.get()) == "std::string") {
                    indent(); output << "return \"\";\n";
                } else if (mapType(node->return_type.get()) == "bool") {
                    indent(); output << "return false;\n";
                } else if (mapType(node->return_type.get()) != "void") {
                    indent(); output << "return 0;\n";
                } else {
                    indent(); output << "return;\n";
                }
                indent_level--;
                indent(); output << "}\n";
                
                indent();
                if (mapType(node->return_type.get()) != "void") {
                    if (mapType(node->return_type.get()) == "std::string") {
                        output << "return std::string(func(";
                    } else {
                        output << "return func(";
                    }
                } else {
                    output << "func(";
                }
                
                for (size_t i = 0; i < node->parameters.size(); ++i) {
                    if (mapType(node->parameters[i]->type.get()) == "std::string") {
                        output << node->parameters[i]->var_name << ".c_str()";
                    } else {
                        output << node->parameters[i]->var_name;
                    }
                    if (i < node->parameters.size() - 1) output << ", ";
                }
                
                if (mapType(node->return_type.get()) == "std::string") {
                    output << "));\n";
                } else {
                    output << ");\n";
                }
            }
            indent_level--;
            indent(); output << "}\n\n";
            return;
        } else { // "js"
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
            // Use CFFI-compatible types for exported functions (C ABI)
            if (node->is_exported) {
                output << mapTypeForCFFI(node->return_type.get(), true) << " " << node->function_name << "(";
            } else {
                output << mapType(node->return_type.get()) << " " << node->function_name << "(";
            }
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
            // Use CFFI-compatible types for exported functions (C ABI)
            if (node->is_exported) {
                output << mapTypeForCFFI(param->type.get(), false) << " " << param->var_name;
            } else {
                output << mapParameterType(param->type.get()) << " " << param->var_name;
            }
            if (param->initializer) {
                output << " = " << generateExpression(param->initializer.get());
            }
        }
        
        if (i < node->parameters.size() - 1) output << ", ";
    }
    output << ") {\n";
    
    indent_level++;

    // For main(): inject GC lifecycle management at top and Python FFI initialization if needed
    bool is_main = (node->function_name == "main");
    if (is_main) {
        // Initialize Python FFI bridge if Python foreign functions are used
        if (has_python_ffi) {
            indent(); output << "// --- Python FFI Bridge: Initialize runtime ---\n";
            indent(); output << "zenith::ffi::PythonFFIBridge::initialize();\n\n";
        }
        indent(); output << "// --- Zenith RC+GC Memory Manager: Start background cycle collector ---\n";
        indent(); output << "zenith::mem::GcHeap::instance().start_background_gc(5000);\n";
        indent(); output << "// --- Zenith Frame/Arena Allocator: Ready for scoped Temp<T> allocations ---\n\n";
    }
    
    // Set flag for exported functions with String return type (for ABI conversion)
    bool was_exported_string = false;
    if (node->is_exported && mapType(node->return_type.get()) == "std::string" && !node->is_async) {
        current_function_is_exported_with_string_return = true;
        was_exported_string = true;
        indent(); output << "std::string _zenith_ret;\n";
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
    
    // Reset flag after function generation
    if (was_exported_string) {
        current_function_is_exported_with_string_return = false;
    }
    
    // ABI conversion wrapper for exported functions with String return type
    // Convert std::string to char* for C ABI compatibility
    bool needs_string_conversion = node->is_exported && 
                                   mapType(node->return_type.get()) == "std::string" &&
                                   !node->is_async;
    
    if (needs_string_conversion) {
        indent(); output << "    // ABI conversion: std::string -> char*\n";
        indent(); output << "    static thread_local std::string _ret_storage;\n";
        indent(); output << "    _ret_storage = _zenith_ret;\n";
        indent(); output << "    return const_cast<char*>(_ret_storage.c_str());\n";
    } else if (is_main) {
        // Finalize Python FFI bridge if Python foreign functions are used
        if (has_python_ffi) {
            indent(); output << "\n";
            indent(); output << "// --- Python FFI Bridge: Finalize runtime ---\n";
            indent(); output << "zenith::ffi::PythonFFIBridge::finalize();\n";
        }
        // For main(): inject GC epilogue — stop background thread, do final sweep, print stats
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
    bool is_scene_class = classImplementsInterface(node, "Scene");
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
            output << "public " << mapBuiltinInterfaceBase(node->implemented_interfaces[i]);
            first = false;
        }
    }
    output << " {\n";
    output << "private:\n";
    
    indent_level++;
    for (const auto& arg : node->primary_constructor_args) {
        indent(); output << mapType(arg->type.get()) << " " << cppIdentifier(arg->var_name) << ";\n";
    }
    indent_level--;

    output << "public:\n";
    indent_level++;
    
    // Custom fields
    for (const auto& field : node->fields) {
        indent(); output << mapType(field->type.get()) << " " << cppIdentifier(field->var_name);
        if (field->initializer) {
            output << " = " << generateExpression(field->initializer.get());
        }
        output << ";\n";
    }
    output << "\n";
    
    indent(); output << node->class_name << "(";
    for (size_t i = 0; i < node->primary_constructor_args.size(); ++i) {
        output << mapType(node->primary_constructor_args[i]->type.get()) << " " << cppIdentifier(node->primary_constructor_args[i]->var_name);
        if (i < node->primary_constructor_args.size() - 1) output << ", ";
    }
    output << ") ";
    
    if (!node->primary_constructor_args.empty()) {
        output << ": ";
        for (size_t i = 0; i < node->primary_constructor_args.size(); ++i) {
            output << cppIdentifier(node->primary_constructor_args[i]->var_name) << "(" << cppIdentifier(node->primary_constructor_args[i]->var_name) << ")";
            if (i < node->primary_constructor_args.size() - 1) output << ", ";
        }
    }
    output << " {}\n";

    if (node->primary_constructor_args.empty() && !node->fields.empty()) {
        indent(); output << node->class_name << "(";
        for (size_t i = 0; i < node->fields.size(); ++i) {
            output << mapType(node->fields[i]->type.get()) << " " << cppIdentifier(node->fields[i]->var_name);
            if (i < node->fields.size() - 1) output << ", ";
        }
        output << ") : ";
        for (size_t i = 0; i < node->fields.size(); ++i) {
            output << cppIdentifier(node->fields[i]->var_name) << "(" << cppIdentifier(node->fields[i]->var_name) << ")";
            if (i < node->fields.size() - 1) output << ", ";
        }
        output << " {}\n";
    }

    if (is_scene_class) {
        output << "\n";
        indent(); output << "zenith::game::EntityId createEntity(std::string name) {\n";
        indent_level++;
        indent(); output << "return world.createEntity(name);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void setEntityName(zenith::game::EntityId entity, std::string name) {\n";
        indent_level++;
        indent(); output << "if (!world.isAlive(entity)) {\n";
        indent_level++;
        indent(); output << "return;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (zenith::game::NameComponent* component = world.getName(entity)) {\n";
        indent_level++;
        indent(); output << "component->value = name;\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "world.addName(entity, name);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "std::string entityName(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "if (const zenith::game::NameComponent* component = world.getName(entity)) {\n";
        indent_level++;
        indent(); output << "return component->value;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return \"\";\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void setEntityTag(zenith::game::EntityId entity, std::string tag) {\n";
        indent_level++;
        indent(); output << "if (!world.isAlive(entity)) {\n";
        indent_level++;
        indent(); output << "return;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (zenith::game::TagComponent* component = world.getTag(entity)) {\n";
        indent_level++;
        indent(); output << "component->value = tag;\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "world.addTag(entity, tag);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "std::string entityTag(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "if (const zenith::game::TagComponent* component = world.getTag(entity)) {\n";
        indent_level++;
        indent(); output << "return component->value;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return \"\";\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::EntityId findEntityByName(std::string name) {\n";
        indent_level++;
        indent(); output << "std::optional<zenith::game::EntityId> entity = world.findByName(name);\n";
        indent(); output << "return entity.has_value() ? entity.value() : zenith::game::EntityId::invalid();\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::EntityId findEntityByTag(std::string tag) {\n";
        indent_level++;
        indent(); output << "std::optional<zenith::game::EntityId> entity = world.findByTag(tag);\n";
        indent(); output << "return entity.has_value() ? entity.value() : zenith::game::EntityId::invalid();\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool setParent(zenith::game::EntityId child, zenith::game::EntityId parent) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::setParent(child, parent);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool clearParent(zenith::game::EntityId child) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::clearParent(child);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::EntityId parentOf(zenith::game::EntityId child) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::parentOf(child);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "int childCount(zenith::game::EntityId parent) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::childCount(parent);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::EntityId childAt(zenith::game::EntityId parent, int index) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::childAt(parent, index);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::EntityId spawnSprite(std::string name, float x, float y, float w, float h, std::string color) {\n";
        indent_level++;
        indent(); output << "zenith::game::EntityId entity = world.createEntity(name);\n";
        indent(); output << "zenith::game::Transform2D& transform = world.addTransform2D(entity);\n";
        indent(); output << "transform.position = zenith::physics::Vec2(x, y);\n";
        indent(); output << "zenith::game::SpriteRenderer2D& sprite = world.addSpriteRenderer2D(entity);\n";
        indent(); output << "sprite.size = zenith::physics::Vec2(w, h);\n";
        indent(); output << "sprite.tintColor = color;\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::EntityId spawnTexturedSprite(std::string name, std::string texturePath, float x, float y, float w, float h, std::string color) {\n";
        indent_level++;
        indent(); output << "zenith::game::EntityId entity = spawnSprite(name, x, y, w, h, color);\n";
        indent(); output << "if (zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {\n";
        indent_level++;
        indent(); output << "sprite->textureHandle = zenith::resource::ResourceManager::getInstance().loadTextureHandle(texturePath);\n";
        indent(); output << "sprite->texture = zenith::resource::ResourceManager::getInstance().loadTexture(sprite->textureHandle);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::EntityId spawnCamera2D(std::string name, float x, float y, float zoom, bool primary) {\n";
        indent_level++;
        indent(); output << "zenith::game::EntityId entity = world.createEntity(name);\n";
        indent(); output << "zenith::game::Transform2D& transform = world.addTransform2D(entity);\n";
        indent(); output << "transform.position = zenith::physics::Vec2(x, y);\n";
        indent(); output << "if (primary) {\n";
        indent_level++;
        indent(); output << "std::optional<zenith::game::EntityId> current = world.primaryCamera2D();\n";
        indent(); output << "if (current.has_value()) {\n";
        indent_level++;
        indent(); output << "if (zenith::game::Camera2DComponent* existing = world.getCamera2D(current.value())) {\n";
        indent_level++;
        indent(); output << "existing->primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "zenith::game::Camera2DComponent& camera = world.addCamera2D(entity);\n";
        indent(); output << "camera.zoom = zoom;\n";
        indent(); output << "camera.primary = primary;\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool destroyEntity(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "return world.destroyEntity(entity);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool isEntityAlive(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "return world.isAlive(entity);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "int entityCount() {\n";
        indent_level++;
        indent(); output << "return static_cast<int>(world.entityCount());\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void setEntityPosition2D(zenith::game::EntityId entity, float x, float y) {\n";
        indent_level++;
        indent(); output << "if (!world.isAlive(entity)) {\n";
        indent_level++;
        indent(); output << "return;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "zenith::game::Transform2D* transform = world.getTransform2D(entity);\n";
        indent(); output << "if (transform == nullptr) {\n";
        indent_level++;
        indent(); output << "transform = &world.addTransform2D(entity);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "transform->position = zenith::physics::Vec2(x, y);\n";
        indent(); output << "if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {\n";
        indent_level++;
        indent(); output << "body->position = transform->position;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void moveEntity2D(zenith::game::EntityId entity, float dx, float dy) {\n";
        indent_level++;
        indent(); output << "setEntityPosition2D(entity, entityPositionX(entity) + dx, entityPositionY(entity) + dy);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "float entityPositionX(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {\n";
        indent_level++;
        indent(); output << "return transform->position.x;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {\n";
        indent_level++;
        indent(); output << "return body->position.x;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return 0.0f;\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "float entityPositionY(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {\n";
        indent_level++;
        indent(); output << "return transform->position.y;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {\n";
        indent_level++;
        indent(); output << "return body->position.y;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return 0.0f;\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void setSpriteColor(zenith::game::EntityId entity, std::string color) {\n";
        indent_level++;
        indent(); output << "if (zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {\n";
        indent_level++;
        indent(); output << "sprite->tintColor = color;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void setSpriteTexture(zenith::game::EntityId entity, std::string texturePath) {\n";
        indent_level++;
        indent(); output << "if (!world.isAlive(entity)) {\n";
        indent_level++;
        indent(); output << "return;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity);\n";
        indent(); output << "if (sprite == nullptr) {\n";
        indent_level++;
        indent(); output << "sprite = &world.addSpriteRenderer2D(entity);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "sprite->textureHandle = zenith::resource::ResourceManager::getInstance().loadTextureHandle(texturePath);\n";
        indent(); output << "sprite->texture = zenith::resource::ResourceManager::getInstance().loadTexture(sprite->textureHandle);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "std::string spriteTexturePath(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "if (const zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {\n";
        indent_level++;
        indent(); output << "if (!sprite->textureHandle.path.empty()) {\n";
        indent_level++;
        indent(); output << "return sprite->textureHandle.path;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (sprite->texture) {\n";
        indent_level++;
        indent(); output << "return sprite->texture->path;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return \"\";\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void attachBody2D(zenith::game::EntityId entity, float mass, float gravityScale, float friction, float restitution) {\n";
        indent_level++;
        indent(); output << "if (!world.isAlive(entity)) {\n";
        indent_level++;
        indent(); output << "return;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity);\n";
        indent(); output << "if (body == nullptr) {\n";
        indent_level++;
        indent(); output << "body = &world.addRigidBody2D(entity);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "body->mass = (mass <= 0.0f) ? 1.0f : mass;\n";
        indent(); output << "body->gravityScale = gravityScale;\n";
        indent(); output << "body->friction = friction;\n";
        indent(); output << "body->restitution = restitution;\n";
        indent(); output << "if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {\n";
        indent_level++;
        indent(); output << "body->position = transform->position;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::BoxCollider2DView boxCollider2D(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::boxCollider2D(entity);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::CircleCollider2DView circleCollider2D(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::circleCollider2D(entity);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::CapsuleCollider2DView capsuleCollider2D(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::capsuleCollider2D(entity);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void attachBoxCollider2D(zenith::game::EntityId entity, float width, float height, bool isTrigger) {\n";
        indent_level++;
        indent(); output << "zenith::game::Scene::attachBoxCollider2D(entity, width, height, isTrigger);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void attachCircleCollider2D(zenith::game::EntityId entity, float radius, bool isTrigger) {\n";
        indent_level++;
        indent(); output << "zenith::game::Scene::attachCircleCollider2D(entity, radius, isTrigger);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void attachCapsuleCollider2D(zenith::game::EntityId entity, float height, float radius, bool isTrigger) {\n";
        indent_level++;
        indent(); output << "zenith::game::Scene::attachCapsuleCollider2D(entity, height, radius, isTrigger);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void setBodyVelocity2D(zenith::game::EntityId entity, float vx, float vy) {\n";
        indent_level++;
        indent(); output << "if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {\n";
        indent_level++;
        indent(); output << "body->velocity = zenith::physics::Vec2(vx, vy);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void applyBodyImpulse2D(zenith::game::EntityId entity, float ix, float iy) {\n";
        indent_level++;
        indent(); output << "if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {\n";
        indent_level++;
        indent(); output << "body->applyImpulse(zenith::physics::Vec2(ix, iy));\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "float bodyVelocityX(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {\n";
        indent_level++;
        indent(); output << "return body->velocity.x;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return 0.0f;\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "float bodyVelocityY(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {\n";
        indent_level++;
        indent(); output << "return body->velocity.y;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return 0.0f;\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool overlaps2D(zenith::game::EntityId first, zenith::game::EntityId second) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::overlaps2D(first, second);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool containsPoint2D(zenith::game::EntityId entity, float x, float y) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::containsPoint2D(entity, x, y);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::RaycastHit2DResult raycast2D(float originX, float originY, float directionX, float directionY, float maxDistance) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::raycast2D(originX, originY, directionX, directionY, maxDistance);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::BoxCollider3DView boxCollider3D(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::boxCollider3D(entity);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::SphereCollider3DView sphereCollider3D(zenith::game::EntityId entity) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::sphereCollider3D(entity);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void attachBoxCollider3D(zenith::game::EntityId entity, float width, float height, float depth, bool isTrigger) {\n";
        indent_level++;
        indent(); output << "zenith::game::Scene::attachBoxCollider3D(entity, width, height, depth, isTrigger);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "void attachSphereCollider3D(zenith::game::EntityId entity, float radius, bool isTrigger) {\n";
        indent_level++;
        indent(); output << "zenith::game::Scene::attachSphereCollider3D(entity, radius, isTrigger);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool overlaps3D(zenith::game::EntityId first, zenith::game::EntityId second) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::overlaps3D(first, second);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool containsPoint3D(zenith::game::EntityId entity, float x, float y, float z) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::containsPoint3D(entity, x, y, z);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "zenith::game::RaycastHit3DResult raycast3D(float originX, float originY, float originZ, float directionX, float directionY, float directionZ, float maxDistance) {\n";
        indent_level++;
        indent(); output << "return zenith::game::Scene::raycast3D(originX, originY, originZ, directionX, directionY, directionZ, maxDistance);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool followPrimaryCamera2D(zenith::game::EntityId target, float offsetX, float offsetY, float smoothing) {\n";
        indent_level++;
        indent(); output << "return zenith::followPrimaryCamera2D(world, target, zenith::physics::Vec2(offsetX, offsetY), smoothing);\n";
        indent_level--;
        indent(); output << "}\n\n";

        indent(); output << "bool followPrimaryCamera3D(zenith::game::EntityId target, float offsetX, float offsetY, float offsetZ, float smoothing) {\n";
        indent_level++;
        indent(); output << "return zenith::followPrimaryCamera3D(world, target, zenith::physics::Vec3(offsetX, offsetY, offsetZ), smoothing);\n";
        indent_level--;
        indent(); output << "}\n";
    }

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

    bool has_trigger_entity_callback = false;
    for (const auto& method : node->methods) {
        if (method->function_name == "triggerEntityCallback") {
            has_trigger_entity_callback = true;
            break;
        }
    }

    if (is_scene_class && !has_trigger_entity_callback) {
        indent(); output << "void triggerEntityCallback(std::string name, zenith::game::EntityId entity) override {\n";
        indent_level++;
        for (const auto& method : node->methods) {
            if (method->function_name == "build" || method->function_name == "triggerCallback") {
                continue;
            }
            if (method->parameters.size() == 1) {
                std::string param_type = mapType(method->parameters[0]->type.get());
                if (param_type == "zenith::game::EntityId") {
                    indent();
                    output << "if (name == \"" << method->function_name << "\") { this->" << method->function_name << "(entity); return; }\n";
                }
            }
        }
        indent_level--;
        indent(); output << "}\n\n";
    }
    
    indent_level--;
    indent(); output << "};\n\n";
}

void CodeGenerator::generateInterface(InterfaceDeclNode* node) {
    if (node->interface_name == "Canvas" || node->interface_name == "Scene") {
        return;
    }
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
            output << mapParameterType(method->parameters[i]->type.get()) << " " << method->parameters[i]->var_name;
            if (i < method->parameters.size() - 1) output << ", ";
        }
        output << ") = 0;\n";
    }
    indent_level--;
    indent(); output << "};\n\n";
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
        indent(); output << "std::vector<std::future<std::string>> futures;\n";
        for (const auto& agent : node->agents) {
            indent(); output << "futures.push_back(std::async(std::launch::async, [input]() {\n";
            indent_level++;
            indent(); output << "return " << agent << "(input);\n";
            indent_level--;
            indent(); output << "}));\n";
        }
        indent(); output << "std::vector<std::string> results;\n";
        indent(); output << "for (auto& f : futures) {\n";
        indent_level++;
        indent(); output << "results.push_back(f.get());\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return results;\n";
    }
    indent_level--;
    indent(); output << "}\n\n";
}

std::string CodeGenerator::generate(ProgramNode* program) {
    std::cout << "4. Running Code Generator (Transpiling to C++17)...\n";
    
    // Clear and populate class, interface, and function names
    class_names.clear();
    interface_names.clear();
    function_names.clear();
    has_python_ffi = false;
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            class_names.insert(class_decl->class_name);
        } else if (auto* interface_decl = dynamic_cast<InterfaceDeclNode*>(stmt.get())) {
            interface_names.insert(interface_decl->interface_name);
        } else if (auto* fn_decl = dynamic_cast<FunctionNode*>(stmt.get())) {
            function_names.insert(fn_decl->function_name);
            // Check for Python FFI foreign functions
            if (fn_decl->is_foreign && fn_decl->foreign_abi == "python") {
                has_python_ffi = true;
            }
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
    if (has_python_ffi) {
        output << "#define ZENITH_ENABLE_PYTHON_FFI\n";
    }
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

    output << "template <typename T>\n";
    output << "inline void print(const T& msg) { std::cout << msg; }\n";
    output << "template <typename T>\n";
    output << "inline void println(const T& msg) { std::cout << msg << std::endl; }\n";

    if (has_std_io) {
        output << "inline std::string httpGet(std::string url) { return zenith::httpGet(url); }\n";
        output << "inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }\n";
        output << "inline std::string gcStats() { return zenith::mem::gcStatsString(); }\n\n";
        // Register gcStats as a known function so it's not treated as a UI component
        function_names.insert("gcStats");
    }

    for (const auto& stmt : program->statements) {
        if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt.get())) {
            generateStatement(var_decl);
        } else if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
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
