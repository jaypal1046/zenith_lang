#include "../../include/backend/js_codegen.h"
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <unordered_map>

struct PreRenderValue {
    enum class Kind { Scalar, Html, HtmlList };

    Kind kind = Kind::Scalar;
    std::string text;
    std::vector<std::string> html_items;

    static PreRenderValue scalar(std::string value) {
        PreRenderValue result;
        result.kind = Kind::Scalar;
        result.text = std::move(value);
        return result;
    }

    static PreRenderValue html(std::string value) {
        PreRenderValue result;
        result.kind = Kind::Html;
        result.text = std::move(value);
        return result;
    }

    static PreRenderValue list(std::vector<std::string> items = {}) {
        PreRenderValue result;
        result.kind = Kind::HtmlList;
        result.html_items = std::move(items);
        return result;
    }
};

struct PreRenderContext {
    std::unordered_map<std::string, PreRenderValue> values;
};

static bool classImplementsScene(const ClassDeclNode* node) {
    for (const auto& implemented : node->implemented_interfaces) {
        if (implemented == "Scene") {
            return true;
        }
    }
    return false;
}

static void addBuiltinSceneMethodNames(std::unordered_set<std::string>& methods) {
    static const char* names[] = {
        "onLoad",
        "onFrame",
        "onFixedUpdate",
        "onPostPhysics",
        "onDraw",
        "load",
        "updateFrame",
        "simulateFixedStep",
        "render",
        "setPaused",
        "isLoaded",
        "interpolationAlpha",
        "totalFrames",
        "totalFixedSteps",
        "framesWithDroppedSteps",
        "lastSubstepCount",
        "accumulatedTime",
        "frameDelta",
        "inspectEntity",
        "inspectedEntity",
        "inspectMaterial",
        "inspectedMaterialPath",
        "clearInspectorTarget",
        "selectNextInspectorEntity",
        "selectPreviousInspectorEntity",
        "registerPrefabCallback",
        "hasPrefab",
        "instantiatePrefab",
        "instantiateArchetype",
        "registerSceneStreamCallback",
        "hasSceneStream",
        "loadSceneStream",
        "unloadSceneStream",
        "isSceneStreamLoaded",
        "sceneStreamEntityCount",
        "init",
        "update",
        "draw",
        "createEntity",
        "setEntityName",
        "entityName",
        "setEntityTag",
        "entityTag",
        "findEntityByName",
        "findEntityByTag",
        "setParent",
        "clearParent",
        "parentOf",
        "childCount",
        "childAt",
        "loadTexture",
        "loadAudio",
        "loadMesh",
        "loadShader",
        "loadMaterial",
        "importAsset",
        "createAssetBundle",
        "addAssetToBundle",
        "assetBundleAssetCount",
        "assetBundleAsset",
        "setAssetMemoryBudget",
        "assetMemoryBudget",
        "assetMemoryUsage",
        "setAssetHotReload",
        "assetHotReloadEnabled",
        "pollAssetChanges",
        "reloadAsset",
        "reloadDirtyAssets",
        "markAssetDirty",
        "assetGroup",
        "importedAssetPath",
        "assetDatabaseJson",
        "bakeAssetMetadata",
        "spawnSprite",
        "spawnTexturedSprite",
        "spawnTexturedSpriteHandle",
        "spawnCamera2D",
        "destroyEntity",
        "destroyEntityHierarchy",
        "isEntityAlive",
        "entityCount",
        "setEntityLayer",
        "entityLayer",
        "setEntityMask",
        "entityMask",
        "setEntityLayerMask",
        "canEntitiesInteract",
        "setEntityPosition2D",
        "moveEntity2D",
        "entityPositionX",
        "entityPositionY",
        "transform2D",
        "body2D",
        "boxCollider2D",
        "circleCollider2D",
        "capsuleCollider2D",
        "camera2D",
        "audioListener2D",
        "sprite2D",
        "tilemap2D",
        "character2D",
        "spawnTilemap2D",
        "resizeTilemap2D",
        "setTilemapCell",
        "tilemapCell",
        "fillTilemap",
        "clearTilemap",
        "setTilemapPaletteColor",
        "tilemapPaletteColor",
        "spawnCharacter2D",
        "spawnCharacter2DHandle",
        "audioSource2D",
        "spawnAudioSource2D",
        "spawnAudioSource2DHandle",
        "spawnAudioListener2D",
        "spawnMesh",
        "spawnMeshHandle",
        "createMaterial",
        "materialExists",
        "setMaterialShaderPath",
        "materialShaderPath",
        "cloneMaterial",
        "copyMaterialProperties",
        "removeMaterialProperty",
        "clearMaterialProperties",
        "defineMaterialText",
        "defineMaterialNumber",
        "defineMaterialToggle",
        "defineMaterialRadio",
        "defineMaterialImage",
        "defineMaterialButton",
        "defineMaterialColor",
        "setMaterialTextProperty",
        "materialTextProperty",
        "setMaterialNumberProperty",
        "materialNumberProperty",
        "setMaterialToggleProperty",
        "materialToggleProperty",
        "setMaterialRadioProperty",
        "materialRadioProperty",
        "setMaterialImageProperty",
        "materialImageProperty",
        "setMaterialButtonProperty",
        "materialButtonProperty",
        "triggerMaterialButton",
        "materialButtonTriggerCount",
        "setMaterialPropertyCallback",
        "materialPropertyCallback",
        "notifyMaterialProperty",
        "setMaterialColorProperty",
        "materialColorProperty",
        "materialHasProperty",
        "materialPropertyCount",
        "materialProperty",
        "materialPropertyAt",
        "materialPropertyOptionCount",
        "materialPropertyOption",
        "addMaterialPropertyOption",
        "removeMaterialPropertyOption",
        "clearMaterialPropertyOptions",
        "materialPropertyNameAt",
        "materialPropertyKind",
        "materialPropertyLabel",
        "materialPropertyOptions",
        "setMeshMaterial",
        "meshMaterialPath",
        "meshMaterialHandle",
        "audioSource3D",
        "spawnAudioSource3D",
        "spawnAudioSource3DHandle",
        "playAudio",
        "stopAudio",
        "spawnCamera3D",
        "setEntityPosition3D",
        "moveEntity3D",
        "entityPositionZ",
        "transform3D",
        "body3D",
        "boxCollider3D",
        "sphereCollider3D",
        "camera3D",
        "audioListener3D",
        "pointLight3D",
        "directionalLight3D",
        "mesh3D",
        "character3D",
        "spawnPointLight3D",
        "spawnDirectionalLight3D",
        "spawnCharacter3D",
        "spawnCharacter3DHandle",
        "spawnAudioListener3D",
        "setSpriteColor",
        "setSpriteTexture",
        "spriteTexturePath",
        "attachBody2D",
        "attachBoxCollider2D",
        "attachCircleCollider2D",
        "attachCapsuleCollider2D",
        "setBodyVelocity2D",
        "applyBodyImpulse2D",
        "bodyVelocityX",
        "bodyVelocityY",
        "overlaps2D",
        "containsPoint2D",
        "raycast2D",
        "raycast2DMask",
        "attachBoxCollider3D",
        "attachSphereCollider3D",
        "setCharacterMove3D",
        "jumpCharacter3D",
        "overlaps3D",
        "containsPoint3D",
        "raycast3D",
        "raycast3DMask",
        "followPrimaryCamera2D",
        "followPrimaryCamera3D"
    };

    for (const char* name : names) {
        methods.insert(name);
    }
}

static std::string escapeHtml(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char c : value) {
        switch (c) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

static std::string escapeAttribute(const std::string& value) {
    return escapeHtml(value);
}

static std::string trimCopy(std::string value) {
    const char* whitespace = " \t\r\n";
    size_t start = value.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    size_t end = value.find_last_not_of(whitespace);
    return value.substr(start, end - start + 1);
}

static bool parseNumber(const std::string& value, double& out) {
    char* end = nullptr;
    out = std::strtod(value.c_str(), &end);
    return end != value.c_str() && *end == '\0';
}

static std::string formatNumber(double value) {
    double rounded = std::round(value);
    if (std::fabs(value - rounded) < 0.0000001) {
        return std::to_string(static_cast<long long>(rounded));
    }

    std::ostringstream ss;
    ss << value;
    return ss.str();
}

static std::string valueToText(const PreRenderValue& value) {
    if (value.kind == PreRenderValue::Kind::HtmlList) {
        std::string joined;
        for (const auto& item : value.html_items) joined += item;
        return joined;
    }
    return value.text;
}

static std::string valueToHtml(const PreRenderValue& value) {
    if (value.kind == PreRenderValue::Kind::HtmlList) {
        std::string joined;
        for (const auto& item : value.html_items) joined += item;
        return joined;
    }
    if (value.kind == PreRenderValue::Kind::Html) {
        return value.text;
    }
    return escapeHtml(value.text);
}

static bool valueToBool(const PreRenderValue& value) {
    std::string text = valueToText(value);
    if (text == "true") return true;
    if (text == "false" || text.empty()) return false;

    double number = 0.0;
    if (parseNumber(text, number)) return number != 0.0;
    return true;
}

static bool isNumericText(const std::string& value) {
    double ignored = 0.0;
    return parseNumber(value, ignored);
}

static PreRenderValue preRenderExpression(ExprNode* expr, PreRenderContext& context);

static std::string getNamedArgText(UIComponentNode* ui, const std::string& name, PreRenderContext& context, const std::string& fallback = "") {
    for (const auto& arg : ui->named_args) {
        if (arg.first == name) {
            return valueToText(preRenderExpression(arg.second.get(), context));
        }
    }
    return fallback;
}

static std::string styleValueWithUnit(const std::string& value) {
    if (value.empty()) return value;
    char last = value.back();
    if (last >= '0' && last <= '9') return value + "px";
    return value;
}

static std::string preRenderInlineStyles(UIComponentNode* ui, PreRenderContext& context) {
    std::string style;
    for (const auto& arg : ui->named_args) {
        std::string key = arg.first;
        std::string value = valueToText(preRenderExpression(arg.second.get(), context));
        if (value.empty()) continue;

        if (key == "color") style += "color: " + value + "; ";
        else if (key == "backgroundColor") style += "background-color: " + value + "; ";
        else if (key == "padding") style += "padding: " + styleValueWithUnit(value) + "; ";
        else if (key == "margin") style += "margin: " + styleValueWithUnit(value) + "; ";
        else if (key == "width") style += "width: " + styleValueWithUnit(value) + "; ";
        else if (key == "height") style += "height: " + styleValueWithUnit(value) + "; ";
        else if (key == "fontWeight") style += "font-weight: " + value + "; ";
        else if (key == "gap") style += "gap: " + styleValueWithUnit(value) + "; ";
        else if (key == "flexGrow") style += "flex-grow: " + value + "; ";
        else if (key == "justifyContent") style += "justify-content: " + value + "; ";
        else if (key == "alignItems") style += "align-items: " + value + "; ";
        else if (key == "flexDirection") style += "flex-direction: " + value + "; ";
        else if (key == "fontSize") style += "font-size: " + styleValueWithUnit(value) + "; ";
        else if (key == "borderRadius") style += "border-radius: " + styleValueWithUnit(value) + "; ";
        else if (key == "border") style += "border: " + value + "; ";
        else if (key == "opacity") style += "opacity: " + value + "; ";
        else if (key == "cursor") style += "cursor: " + value + "; ";
        else if (key == "display") style += "display: " + value + "; ";
        else if (key == "boxShadow") style += "box-shadow: " + value + "; ";
        else if (key == "alignSelf") style += "align-self: " + value + "; ";
        else if (key == "fontFamily") style += "font-family: " + value + "; ";
        else if (key == "transition") style += "transition: " + value + "; ";
    }
    return style;
}

static std::string renderStyleAttribute(const std::string& base_style, const std::string& inline_style) {
    std::string style = base_style + inline_style;
    return style.empty() ? "" : " style=\"" + escapeAttribute(style) + "\"";
}

static std::string preRenderUIComponent(UIComponentNode* ui, PreRenderContext& context) {
    if (!ui) return "";

    std::string tag = "div";
    std::string css_class;
    std::string attrs;
    std::string base_style;
    std::string text_content;
    std::string children_html;

    if (ui->component_type == "Column") {
        css_class = "zenith-column";
    } else if (ui->component_type == "Row") {
        css_class = "zenith-row";
    } else if (ui->component_type == "Text") {
        tag = "span";
        css_class = "zenith-text";
        if (!ui->children.empty()) {
            text_content = escapeHtml(valueToText(preRenderExpression(ui->children[0].get(), context)));
        }
    } else if (ui->component_type == "Button") {
        tag = "button";
        css_class = "interactive-btn";
        if (!ui->children.empty()) {
            text_content = escapeHtml(valueToText(preRenderExpression(ui->children[0].get(), context)));
        }
    } else if (ui->component_type == "Card") {
        css_class = "zenith-card";
        base_style = "display: flex; flex-direction: column; background: rgba(30, 41, 59, 0.6); backdrop-filter: blur(12px); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 16px; padding: 20px; margin: 10px 0; box-shadow: 0 20px 40px rgba(0, 0, 0, 0.4); ";
    } else if (ui->component_type == "Container") {
        base_style = "display: flex; flex-direction: column; border: 1px solid rgba(255, 255, 255, 0.05); border-radius: 12px; padding: 16px; background: rgba(255, 255, 255, 0.02); ";
    } else if (ui->component_type == "Scrolling") {
        base_style = "display: flex; flex-direction: column; overflow-y: auto; max-height: 300px; border: 1px solid rgba(255, 255, 255, 0.1); padding: 12px; border-radius: 12px; background: rgba(0, 0, 0, 0.3); ";
    } else if (ui->component_type == "TextField") {
        tag = "input";
        css_class = "zenith-input";
        std::string placeholder = ui->children.empty() ? "" : valueToText(preRenderExpression(ui->children[0].get(), context));
        std::string value = getNamedArgText(ui, "value", context);
        attrs += " type=\"text\" placeholder=\"" + escapeAttribute(placeholder) + "\"";
        if (!value.empty()) attrs += " value=\"" + escapeAttribute(value) + "\"";
    } else if (ui->component_type == "Checkbox") {
        std::string label = ui->children.empty() ? "" : valueToText(preRenderExpression(ui->children[0].get(), context));
        bool is_checked = valueToBool(PreRenderValue::scalar(getNamedArgText(ui, "checked", context, "false")));
        std::string checked_attr = is_checked ? " checked" : "";
        return "<label style=\"display: flex; align-items: center; gap: 8px; cursor: pointer; margin: 5px; color: rgb(226, 232, 240);\"><input type=\"checkbox\" style=\"width: 18px; height: 18px; accent-color: rgb(0, 242, 254); cursor: pointer;\"" + checked_attr + " /><span>" + escapeHtml(label) + "</span></label>";
    } else if (ui->component_type == "Slider") {
        tag = "input";
        css_class = "zenith-input";
        attrs += " type=\"range\" min=\"" + escapeAttribute(getNamedArgText(ui, "min", context, "0")) + "\"";
        attrs += " max=\"" + escapeAttribute(getNamedArgText(ui, "max", context, "100")) + "\"";
        attrs += " value=\"" + escapeAttribute(getNamedArgText(ui, "value", context, "50")) + "\"";
        base_style = "accent-color: rgb(0, 242, 254); cursor: pointer; ";
    } else if (ui->component_type == "Toggle") {
        std::string label = ui->children.empty() ? "" : valueToText(preRenderExpression(ui->children[0].get(), context));
        bool is_on = valueToBool(PreRenderValue::scalar(getNamedArgText(ui, "isOn", context, "false")));
        std::string bg_color = is_on ? "rgb(16, 185, 129)" : "rgb(71, 85, 105)";
        std::string transform = is_on ? "transform: translateX(20px);" : "";
        return "<label style=\"display: flex; align-items: center; gap: 10px; cursor: pointer; margin: 5px;\"><div style=\"position: relative; width: 44px; height: 24px; background-color: " + bg_color + "; border-radius: 12px; transition: background-color 0.2s;\"><div style=\"position: absolute; top: 2px; left: 2px; width: 20px; height: 20px; border-radius: 50%; background-color: rgb(255, 255, 255); transition: transform 0.2s; " + transform + "\"></div></div><span style=\"color: rgb(226, 232, 240);\">" + escapeHtml(label) + "</span></label>";
    } else if (ui->component_type == "Dropdown") {
        tag = "select";
        css_class = "zenith-input";
        base_style = "background: rgba(15, 23, 42, 0.8); color: rgb(248, 250, 252); cursor: pointer; ";
        std::string options = ui->children.empty() ? "" : valueToText(preRenderExpression(ui->children[0].get(), context));
        std::string current = getNamedArgText(ui, "value", context);
        size_t pos = 0;
        while (pos < options.length()) {
            size_t comma = options.find(',', pos);
            std::string option = trimCopy(options.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos));
            if (!option.empty()) {
                std::string selected = (option == current) ? " selected" : "";
                text_content += "<option value=\"" + escapeAttribute(option) + "\"" + selected + ">" + escapeHtml(option) + "</option>";
            }
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
    } else if (ui->component_type == "Image" || ui->component_type == "Video") {
        tag = ui->component_type == "Image" ? "img" : "video";
        std::string url = ui->children.empty() ? "" : valueToText(preRenderExpression(ui->children[0].get(), context));
        attrs += " src=\"" + escapeAttribute(url) + "\"";
        if (ui->component_type == "Video") attrs += " controls";
        base_style = "max-width: 100%; border-radius: 12px; box-shadow: 0 8px 30px rgba(0, 0, 0, 0.5); border: 1px solid rgba(255, 255, 255, 0.1); ";
    }

    if (ui->component_type == "Column" || ui->component_type == "Row" ||
        ui->component_type == "Card" || ui->component_type == "Container" ||
        ui->component_type == "Scrolling") {
        for (const auto& child : ui->children) {
            children_html += valueToHtml(preRenderExpression(child.get(), context));
        }
    }

    std::string class_attr = css_class.empty() ? "" : " class=\"" + escapeAttribute(css_class) + "\"";
    std::string style_attr = renderStyleAttribute(base_style, preRenderInlineStyles(ui, context));
    
    if (tag == "img" || tag == "input") {
        return "<" + tag + class_attr + attrs + style_attr + " />";
    }
    
    return "<" + tag + class_attr + attrs + style_attr + ">" + text_content + children_html + "</" + tag + ">";
}

static PreRenderValue preRenderExpression(ExprNode* expr, PreRenderContext& context) {
    if (!expr) return PreRenderValue::scalar("");

    if (auto* id = dynamic_cast<IdentifierNode*>(expr)) {
        auto found = context.values.find(id->name);
        return found == context.values.end() ? PreRenderValue::scalar("") : found->second;
    }
    if (auto* str = dynamic_cast<StringLiteralNode*>(expr)) {
        return PreRenderValue::scalar(str->value);
    }
    if (auto* num = dynamic_cast<NumberLiteralNode*>(expr)) {
        return PreRenderValue::scalar(num->value);
    }
    if (auto* b = dynamic_cast<BoolLiteralNode*>(expr)) {
        return PreRenderValue::scalar(b->value ? "true" : "false");
    }
    if (auto* list = dynamic_cast<ListLiteralNode*>(expr)) {
        std::vector<std::string> items;
        for (const auto& element : list->elements) {
            items.push_back(valueToHtml(preRenderExpression(element.get(), context)));
        }
        return PreRenderValue::list(items);
    }
    if (auto* unary = dynamic_cast<UnaryExprNode*>(expr)) {
        PreRenderValue inner = preRenderExpression(unary->expression.get(), context);
        if (unary->op == "!") return PreRenderValue::scalar(valueToBool(inner) ? "false" : "true");
        if (unary->op == "-") {
            double number = 0.0;
            if (parseNumber(valueToText(inner), number)) return PreRenderValue::scalar(formatNumber(-number));
        }
        return inner;
    }
    if (auto* binary = dynamic_cast<BinaryExprNode*>(expr)) {
        if (binary->op == "=") {
            PreRenderValue right = preRenderExpression(binary->right.get(), context);
            if (auto* left_id = dynamic_cast<IdentifierNode*>(binary->left.get())) {
                context.values[left_id->name] = right;
            }
            return right;
        }

        PreRenderValue left = preRenderExpression(binary->left.get(), context);
        PreRenderValue right = preRenderExpression(binary->right.get(), context);
        std::string left_text = valueToText(left);
        std::string right_text = valueToText(right);

        if (binary->op == "==" || binary->op == "!=") {
            bool equal = left_text == right_text;
            return PreRenderValue::scalar((binary->op == "==" ? equal : !equal) ? "true" : "false");
        }
        if (binary->op == "&&") return PreRenderValue::scalar((valueToBool(left) && valueToBool(right)) ? "true" : "false");
        if (binary->op == "||") return PreRenderValue::scalar((valueToBool(left) || valueToBool(right)) ? "true" : "false");

        double left_num = 0.0;
        double right_num = 0.0;
        bool left_is_num = parseNumber(left_text, left_num);
        bool right_is_num = parseNumber(right_text, right_num);

        if (binary->op == "+") {
            if (binary->is_string_concat || !left_is_num || !right_is_num) {
                return PreRenderValue::scalar(left_text + right_text);
            }
            return PreRenderValue::scalar(formatNumber(left_num + right_num));
        }
        if (left_is_num && right_is_num) {
            if (binary->op == "-") return PreRenderValue::scalar(formatNumber(left_num - right_num));
            if (binary->op == "*") return PreRenderValue::scalar(formatNumber(left_num * right_num));
            if (binary->op == "/") return PreRenderValue::scalar(right_num == 0.0 ? "0" : formatNumber(left_num / right_num));
            if (binary->op == "<") return PreRenderValue::scalar(left_num < right_num ? "true" : "false");
            if (binary->op == ">") return PreRenderValue::scalar(left_num > right_num ? "true" : "false");
            if (binary->op == "<=") return PreRenderValue::scalar(left_num <= right_num ? "true" : "false");
            if (binary->op == ">=") return PreRenderValue::scalar(left_num >= right_num ? "true" : "false");
        }
        return PreRenderValue::scalar("");
    }
    if (auto* call = dynamic_cast<MethodCallNode*>(expr)) {
        if (call->method_name == "push" && call->arguments.size() == 1) {
            if (auto* object_id = dynamic_cast<IdentifierNode*>(call->object.get())) {
                PreRenderValue& target = context.values[object_id->name];
                if (target.kind != PreRenderValue::Kind::HtmlList) {
                    target = PreRenderValue::list();
                }
                target.html_items.push_back(valueToHtml(preRenderExpression(call->arguments[0].get(), context)));
            }
        }
        return PreRenderValue::scalar("");
    }
    if (auto* ui = dynamic_cast<UIComponentNode*>(expr)) {
        return PreRenderValue::html(preRenderUIComponent(ui, context));
    }

    return PreRenderValue::scalar("");
}

static bool preRenderStatement(ASTNode* stmt, PreRenderContext& context, std::string& returned_html);

static bool preRenderStatements(const std::vector<std::unique_ptr<ASTNode>>& statements, PreRenderContext& context, std::string& returned_html) {
    for (const auto& statement : statements) {
        if (preRenderStatement(statement.get(), context, returned_html)) return true;
    }
    return false;
}

static bool preRenderStatement(ASTNode* stmt, PreRenderContext& context, std::string& returned_html) {
    if (!stmt) return false;

    if (auto* var_decl = dynamic_cast<VarDeclNode*>(stmt)) {
        context.values[var_decl->var_name] = var_decl->initializer
            ? preRenderExpression(var_decl->initializer.get(), context)
            : PreRenderValue::scalar("");
        return false;
    }
    if (auto* if_stmt = dynamic_cast<IfStmtNode*>(stmt)) {
        if (valueToBool(preRenderExpression(if_stmt->condition.get(), context))) {
            return preRenderStatements(if_stmt->then_branch, context, returned_html);
        }
        return preRenderStatements(if_stmt->else_branch, context, returned_html);
    }
    if (auto* return_stmt = dynamic_cast<ReturnStmtNode*>(stmt)) {
        returned_html = valueToHtml(preRenderExpression(return_stmt->expression.get(), context));
        return true;
    }
    if (auto* expr = dynamic_cast<ExprNode*>(stmt)) {
        preRenderExpression(expr, context);
    }
    return false;
}

static std::string stripHtmlTags(const std::string& html) {
    std::string text;
    bool in_tag = false;
    for (char c : html) {
        if (c == '<') {
            in_tag = true;
            text += ' ';
        } else if (c == '>') {
            in_tag = false;
        } else if (!in_tag) {
            text += c;
        }
    }
    return text;
}

static std::string decodeBasicHtmlEntities(std::string value) {
    const std::vector<std::pair<std::string, std::string>> entities = {
        {"&amp;", "&"},
        {"&lt;", "<"},
        {"&gt;", ">"},
        {"&quot;", "\""},
        {"&#39;", "'"}
    };

    for (const auto& entity : entities) {
        size_t pos = 0;
        while ((pos = value.find(entity.first, pos)) != std::string::npos) {
            value.replace(pos, entity.first.size(), entity.second);
            pos += entity.second.size();
        }
    }
    return value;
}

static std::string collapseWhitespace(const std::string& value) {
    std::string result;
    bool last_space = true;
    for (char c : value) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!last_space) result += ' ';
            last_space = true;
        } else {
            result += c;
            last_space = false;
        }
    }
    return trimCopy(result);
}

static std::string computePreRenderedApp(ProgramNode* program, std::string& page_title, std::string& page_description) {
    for (const auto& stmt : program->statements) {
        auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get());
        if (!class_decl) continue;

        FunctionNode* build_method = nullptr;
        for (const auto& method : class_decl->methods) {
            if (method->function_name == "build") {
                build_method = method.get();
                break;
            }
        }
        if (!build_method) continue;

        PreRenderContext context;
        for (const auto& field : class_decl->fields) {
            context.values[field->var_name] = field->initializer
                ? preRenderExpression(field->initializer.get(), context)
                : PreRenderValue::scalar("");
        }

        auto title = context.values.find("title");
        if (title != context.values.end() && !valueToText(title->second).empty()) {
            page_title = valueToText(title->second);
        } else {
            page_title = class_decl->class_name;
        }

        std::string html;
        preRenderStatements(build_method->body, context, html);
        std::string description = collapseWhitespace(decodeBasicHtmlEntities(stripHtmlTags(html)));
        if (!description.empty()) {
            if (description.size() > 160) description = description.substr(0, 157) + "...";
            page_description = description;
        }
        return html;
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
    if (auto* for_stmt = dynamic_cast<ForStmtNode*>(node)) {
        if (containsAsyncCall(for_stmt->initializer.get(), async_fns)) return true;
        if (containsAsyncCall(for_stmt->condition.get(), async_fns)) return true;
        if (containsAsyncCall(for_stmt->update.get(), async_fns)) return true;
        for (const auto& s : for_stmt->body) {
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
    if (auto* unary = dynamic_cast<UnaryExprNode*>(node)) {
        return containsAsyncCall(unary->expression.get(), async_fns);
    }
    return false;
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
        if (isVectorTypeName(object_type) || isMatrixTypeName(object_type)) return "Float";
        if (object_type.rfind("List<", 0) == 0 || object_type == "List") {
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
        const std::string left_expr = generateExpression(binary->left.get());
        const std::string right_expr = generateExpression(binary->right.get());
        const std::string left_type = inferGeneratedExpressionType(binary->left.get());
        const std::string right_type = inferGeneratedExpressionType(binary->right.get());

        if ((binary->op == "==" || binary->op == "!=") && left_type == right_type && isMathValueTypeName(left_type)) {
            const std::string equal_expr = "zenith.mathEqual(" + left_expr + ", " + right_expr + ")";
            return binary->op == "==" ? equal_expr : "!(" + equal_expr + ")";
        }
        if ((binary->op == "+=" || binary->op == "-=" || binary->op == "*=" || binary->op == "/=") && isMathValueTypeName(left_type)) {
            std::string helper = "mathAdd";
            if (binary->op == "-=") helper = "mathSub";
            else if (binary->op == "*=") helper = "mathMul";
            else if (binary->op == "/=") helper = "mathDiv";
            return left_expr + " = zenith." + helper + "(" + left_expr + ", " + right_expr + ")";
        }
        if (binary->op == "+" && (binary->is_string_concat || left_type == "String" || right_type == "String")) {
            return left_expr + " + " + right_expr;
        }
        const std::string math_result = inferMathBinaryResult(left_type, binary->op, right_type);
        if (!math_result.empty()) {
            std::string helper = "mathAdd";
            if (binary->op == "-") helper = "mathSub";
            else if (binary->op == "*") helper = "mathMul";
            else if (binary->op == "/") helper = "mathDiv";
            return "zenith." + helper + "(" + left_expr + ", " + right_expr + ")";
        }
        return left_expr + " " + binary->op + " " + right_expr;
    }
    if (auto* unary = dynamic_cast<UnaryExprNode*>(expr)) {
        return unary->op + generateExpression(unary->expression.get());
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
        const std::string object_type = inferGeneratedExpressionType(call->object.get());
        if (call->method_name == "run" && call->arguments.empty()) {
            return "zenith.runGameLoop(" + generateExpression(call->object.get()) + ")";
        }
        if (call->method_name == "length" && call->arguments.empty() &&
            (object_type == "String" || object_type == "List" || object_type.rfind("List<", 0) == 0)) {
            return generateExpression(call->object.get()) + ".length";
        }
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
        if (isMathConstructorName(ui->component_type)) {
            return "zenith.make" + ui->component_type + "(" +
                   [&]() {
                       std::string args;
                       for (size_t i = 0; i < ui->children.size(); ++i) {
                           args += generateExpression(ui->children[i].get());
                           if (i < ui->children.size() - 1) args += ", ";
                       }
                       return args;
                   }() + ")";
        }
        if (isCompileTimeAssetConstructorName(ui->component_type)) {
            if (ui->component_type == "texture") {
                return "{ __zenithType: \"TextureHandleView\", path: " + generateExpression(ui->children[0].get()) + ", id: 0, refCount: 0, loaded: false, width: 0, height: 0, channels: 0, gpuId: 0 }";
            }
            if (ui->component_type == "audio") {
                return "{ __zenithType: \"AudioHandleView\", path: " + generateExpression(ui->children[0].get()) + ", id: 0, refCount: 0, loaded: false, duration: 0, spatial: false }";
            }
            if (ui->component_type == "mesh") {
                return "{ __zenithType: \"MeshHandleView\", path: " + generateExpression(ui->children[0].get()) + ", id: 0, refCount: 0, loaded: false, vertexCount: 0, triangleCount: 0, vbo: 0, ebo: 0 }";
            }
            if (ui->component_type == "shader") {
                return "{ __zenithType: \"ShaderHandleView\", path: " + generateExpression(ui->children[0].get()) + ", id: 0, refCount: 0, loaded: false, programId: 0 }";
            }
            if (ui->component_type == "material") {
                std::string shader_path = ui->children.size() > 1 ? generateExpression(ui->children[1].get()) : "\"\"";
                return "{ __zenithType: \"MaterialHandleView\", path: " + generateExpression(ui->children[0].get()) + ", id: 0, refCount: 0, loaded: false, shaderPath: " + shader_path + ", propertyCount: 0 }";
            }
        }

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
    } else if (auto* for_stmt = dynamic_cast<ForStmtNode*>(stmt)) {
        indent();
        output << "for (";
        if (for_stmt->initializer) {
            if (auto* var_decl = dynamic_cast<VarDeclNode*>(for_stmt->initializer.get())) {
                output << "let " << var_decl->var_name;
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
    bool is_scene_class = classImplementsScene(node);
    current_class_fields.clear();
    for (const auto& param : node->primary_constructor_args) {
        current_class_fields.insert(param->var_name);
    }
    for (const auto& member : node->fields) {
        current_class_fields.insert(member->var_name);
    }
    if (is_scene_class) {
        current_class_fields.insert("name");
        current_class_fields.insert("clearColor");
        current_class_fields.insert("fixedDeltaTime");
        current_class_fields.insert("maxFrameDelta");
        current_class_fields.insert("maxFixedStepsPerFrame");
        current_class_fields.insert("autoSimulatePhysics");
        current_class_fields.insert("autoRenderWorld2D");
        current_class_fields.insert("drawEntityNames");
        current_class_fields.insert("debugDrawGrid2D");
        current_class_fields.insert("debugDrawColliders2D");
        current_class_fields.insert("debugDrawTransforms2D");
        current_class_fields.insert("debugDrawCameraBounds2D");
        current_class_fields.insert("debugDrawRuntimeStats");
        current_class_fields.insert("debugGridCellWidth");
        current_class_fields.insert("debugGridCellHeight");
        current_class_fields.insert("debugOverlayColor");
        current_class_fields.insert("debugOverlayEnabled");
        current_class_fields.insert("minimalInspectorEnabled");
        current_class_fields.insert("paused");
    }
    current_class_methods.clear();
    for (const auto& method : node->methods) {
        current_class_methods.insert(method->function_name);
    }
    if (is_scene_class) {
        addBuiltinSceneMethodNames(current_class_methods);
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
    if (is_scene_class) {
        indent(); output << "this.name = \"" << node->class_name << "\";\n";
        indent(); output << "this.clearColor = \"black\";\n";
        indent(); output << "this.fixedDeltaTime = 1.0 / 60.0;\n";
        indent(); output << "this.maxFrameDelta = 0.25;\n";
        indent(); output << "this.maxFixedStepsPerFrame = 8;\n";
        indent(); output << "this.autoSimulatePhysics = true;\n";
        indent(); output << "this.autoRenderWorld2D = false;\n";
        indent(); output << "this.drawEntityNames = true;\n";
        indent(); output << "this.debugDrawGrid2D = false;\n";
        indent(); output << "this.debugDrawColliders2D = false;\n";
        indent(); output << "this.debugDrawTransforms2D = false;\n";
        indent(); output << "this.debugDrawCameraBounds2D = false;\n";
        indent(); output << "this.debugDrawRuntimeStats = false;\n";
        indent(); output << "this.debugGridCellWidth = 1;\n";
        indent(); output << "this.debugGridCellHeight = 1;\n";
        indent(); output << "this.debugOverlayColor = 'cyan';\n";
        indent(); output << "this.debugOverlayEnabled = true;\n";
        indent(); output << "this.minimalInspectorEnabled = false;\n";
        indent(); output << "this.paused = false;\n";
        indent(); output << "this._zenithLoaded = false;\n";
        indent(); output << "this._zenithAccumulator = 0.0;\n";
        indent(); output << "this._zenithLastFrameDelta = 0.0;\n";
        indent(); output << "this._zenithFrameCount = 0;\n";
        indent(); output << "this._zenithFixedStepCount = 0;\n";
        indent(); output << "this._zenithDroppedStepFrames = 0;\n";
        indent(); output << "this._zenithLastFixedSteps = 0;\n";
        indent(); output << "this._zenithNextEntityId = 1;\n";
        indent(); output << "this._zenithNextAssetHandleId = 1;\n";
        indent(); output << "this._zenithEntities = [];\n";
        indent(); output << "this._zenithMaterials = new Map();\n";
        indent(); output << "this._zenithTextureHandles = new Map();\n";
        indent(); output << "this._zenithAudioHandles = new Map();\n";
        indent(); output << "this._zenithMeshHandles = new Map();\n";
        indent(); output << "this._zenithShaderHandles = new Map();\n";
        indent(); output << "this._zenithMaterialHandles = new Map();\n";
        indent(); output << "this._zenithAssetMetadata = new Map();\n";
        indent(); output << "this._zenithAssetBundles = new Map();\n";
        indent(); output << "this._zenithAssetBudgets = new Map();\n";
        indent(); output << "this._zenithAssetHotReload = true;\n";
        indent(); output << "this._zenithBakedAssetOutputs = new Map();\n";
        indent(); output << "this._zenithPrefabs = new Map();\n";
        indent(); output << "this._zenithSceneStreams = new Map();\n";
        indent(); output << "this._zenithLoadedSceneStreams = new Map();\n";
        indent(); output << "this._zenithPrimaryCamera = null;\n";
        indent(); output << "this._zenithPrimaryAudioListener = null;\n";
        indent(); output << "this._zenithPrimaryCamera3D = null;\n";
        indent(); output << "this._zenithPrimaryAudioListener3D = null;\n";
        indent(); output << "this._zenithInspectorEntity = null;\n";
        indent(); output << "this._zenithInspectorMaterialPath = '';\n";
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

    if (current_class_methods.count("triggerCallback") == 0) {
        indent();
        output << "triggerCallback(name, val = '') {\n";
        indent_level++;
        for (const auto& method : node->methods) {
            if (method->function_name == "build" || method->function_name == "triggerCallback") {
                continue;
            }
            if (method->parameters.empty()) {
                indent();
                output << "if (name === \"" << method->function_name << "\") { this." << method->function_name << "(); return; }\n";
            } else if (method->parameters.size() == 1) {
                std::string param_type = method->parameters[0]->type ? method->parameters[0]->type->type_name : "";
                indent();
                if (param_type == "String") {
                    output << "if (name === \"" << method->function_name << "\") { this." << method->function_name << "(val === undefined || val === null ? '' : String(val)); return; }\n";
                } else if (param_type == "Bool") {
                    output << "if (name === \"" << method->function_name << "\") { this." << method->function_name << "(val === true || val === 'true'); return; }\n";
                } else if (param_type == "Int") {
                    output << "if (name === \"" << method->function_name << "\") { const parsed = Number.parseInt(val, 10); if (!Number.isNaN(parsed)) this." << method->function_name << "(parsed); return; }\n";
                } else if (param_type == "Float") {
                    output << "if (name === \"" << method->function_name << "\") { const parsed = Number(val); if (!Number.isNaN(parsed)) this." << method->function_name << "(parsed); return; }\n";
                }
            }
        }
        indent_level--;
        indent(); output << "}\n";
    }

    if (is_scene_class && current_class_methods.count("triggerEntityCallback") == 0) {
        indent();
        output << "triggerEntityCallback(name, entity = null) {\n";
        indent_level++;
        for (const auto& method : node->methods) {
            if (method->function_name == "build" || method->function_name == "triggerCallback") {
                continue;
            }
            if (method->parameters.size() == 1) {
                std::string param_type = method->parameters[0]->type ? method->parameters[0]->type->type_name : "";
                if (param_type == "EntityId") {
                    indent();
                    output << "if (name === \"" << method->function_name << "\") { this." << method->function_name << "(entity); return; }\n";
                }
            }
        }
        indent_level--;
        indent(); output << "}\n";
    }

    if (is_scene_class) {
        indent();
        output << "createEntity(name) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'entity',\n";
        indent(); output << "x: 0,\n";
        indent(); output << "y: 0,\n";
        indent(); output << "z: 0,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "visible: true,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureTransform2D(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity) return null;\n";
        indent(); output << "if (entity.x === undefined) entity.x = 0;\n";
        indent(); output << "if (entity.y === undefined) entity.y = 0;\n";
        indent(); output << "if (entity.rotation === undefined) entity.rotation = 0;\n";
        indent(); output << "if (entity.scaleX === undefined) entity.scaleX = 1;\n";
        indent(); output << "if (entity.scaleY === undefined) entity.scaleY = 1;\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureTransform3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "if (target.z === undefined) target.z = 0;\n";
        indent(); output << "if (target.rotationX === undefined) target.rotationX = 0;\n";
        indent(); output << "if (target.rotationY === undefined) target.rotationY = 0;\n";
        indent(); output << "if (target.rotationZ === undefined) target.rotationZ = 0;\n";
        indent(); output << "if (target.scaleZ === undefined) target.scaleZ = 1;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureBody2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasBody = true;\n";
        indent(); output << "if (target.vx === undefined) target.vx = 0;\n";
        indent(); output << "if (target.vy === undefined) target.vy = 0;\n";
        indent(); output << "if (target.mass === undefined || target.mass <= 0) target.mass = 1;\n";
        indent(); output << "if (target.gravityScale === undefined) target.gravityScale = 1;\n";
        indent(); output << "if (target.friction === undefined) target.friction = 0.2;\n";
        indent(); output << "if (target.restitution === undefined) target.restitution = 0;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureBoxCollider2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasBoxCollider2D = true;\n";
        indent(); output << "if (target.boxColliderOffsetX === undefined) target.boxColliderOffsetX = 0;\n";
        indent(); output << "if (target.boxColliderOffsetY === undefined) target.boxColliderOffsetY = 0;\n";
        indent(); output << "if (target.boxColliderWidth === undefined || target.boxColliderWidth <= 0) target.boxColliderWidth = 1;\n";
        indent(); output << "if (target.boxColliderHeight === undefined || target.boxColliderHeight <= 0) target.boxColliderHeight = 1;\n";
        indent(); output << "if (target.boxColliderIsTrigger === undefined) target.boxColliderIsTrigger = false;\n";
        indent(); output << "if (target.boxColliderEnabled === undefined) target.boxColliderEnabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureCircleCollider2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasCircleCollider2D = true;\n";
        indent(); output << "if (target.circleColliderOffsetX === undefined) target.circleColliderOffsetX = 0;\n";
        indent(); output << "if (target.circleColliderOffsetY === undefined) target.circleColliderOffsetY = 0;\n";
        indent(); output << "if (target.circleColliderRadius === undefined || target.circleColliderRadius <= 0) target.circleColliderRadius = 0.5;\n";
        indent(); output << "if (target.circleColliderIsTrigger === undefined) target.circleColliderIsTrigger = false;\n";
        indent(); output << "if (target.circleColliderEnabled === undefined) target.circleColliderEnabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureCapsuleCollider2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasCapsuleCollider2D = true;\n";
        indent(); output << "if (target.capsuleColliderOffsetX === undefined) target.capsuleColliderOffsetX = 0;\n";
        indent(); output << "if (target.capsuleColliderOffsetY === undefined) target.capsuleColliderOffsetY = 0;\n";
        indent(); output << "if (target.capsuleColliderRadius === undefined || target.capsuleColliderRadius <= 0) target.capsuleColliderRadius = 0.5;\n";
        indent(); output << "if (target.capsuleColliderHeight === undefined || target.capsuleColliderHeight < target.capsuleColliderRadius * 2) target.capsuleColliderHeight = Math.max(2, target.capsuleColliderRadius * 2);\n";
        indent(); output << "if (target.capsuleColliderIsTrigger === undefined) target.capsuleColliderIsTrigger = false;\n";
        indent(); output << "if (target.capsuleColliderEnabled === undefined) target.capsuleColliderEnabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithBoxColliderBounds2D(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity || entity.hasBoxCollider2D !== true || entity.boxColliderEnabled === false) return null;\n";
        indent(); output << "const width = entity.boxColliderWidth !== undefined && entity.boxColliderWidth > 0 ? entity.boxColliderWidth : 1;\n";
        indent(); output << "const height = entity.boxColliderHeight !== undefined && entity.boxColliderHeight > 0 ? entity.boxColliderHeight : 1;\n";
        indent(); output << "const centerX = (entity.x !== undefined ? entity.x : 0) + (entity.boxColliderOffsetX !== undefined ? entity.boxColliderOffsetX : 0);\n";
        indent(); output << "const centerY = (entity.y !== undefined ? entity.y : 0) + (entity.boxColliderOffsetY !== undefined ? entity.boxColliderOffsetY : 0);\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "minX: centerX - (width * 0.5),\n";
        indent(); output << "maxX: centerX + (width * 0.5),\n";
        indent(); output << "minY: centerY - (height * 0.5),\n";
        indent(); output << "maxY: centerY + (height * 0.5)\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCircleColliderState2D(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity || entity.hasCircleCollider2D !== true || entity.circleColliderEnabled === false) return null;\n";
        indent(); output << "const radius = entity.circleColliderRadius !== undefined && entity.circleColliderRadius > 0 ? entity.circleColliderRadius : 0.5;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "x: (entity.x !== undefined ? entity.x : 0) + (entity.circleColliderOffsetX !== undefined ? entity.circleColliderOffsetX : 0),\n";
        indent(); output << "y: (entity.y !== undefined ? entity.y : 0) + (entity.circleColliderOffsetY !== undefined ? entity.circleColliderOffsetY : 0),\n";
        indent(); output << "radius\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleColliderState2D(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity || entity.hasCapsuleCollider2D !== true || entity.capsuleColliderEnabled === false) return null;\n";
        indent(); output << "const radius = entity.capsuleColliderRadius !== undefined && entity.capsuleColliderRadius > 0 ? entity.capsuleColliderRadius : 0.5;\n";
        indent(); output << "const height = entity.capsuleColliderHeight !== undefined && entity.capsuleColliderHeight >= radius * 2 ? entity.capsuleColliderHeight : (radius * 2);\n";
        indent(); output << "const halfSegment = Math.max(0, (height * 0.5) - radius);\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "x: (entity.x !== undefined ? entity.x : 0) + (entity.capsuleColliderOffsetX !== undefined ? entity.capsuleColliderOffsetX : 0),\n";
        indent(); output << "y: (entity.y !== undefined ? entity.y : 0) + (entity.capsuleColliderOffsetY !== undefined ? entity.capsuleColliderOffsetY : 0),\n";
        indent(); output << "radius,\n";
        indent(); output << "height,\n";
        indent(); output << "halfSegment\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithBoxIntersectsCircle2D(bounds, circle) {\n";
        indent_level++;
        indent(); output << "if (!bounds || !circle) return false;\n";
        indent(); output << "const closestX = Math.max(bounds.minX, Math.min(circle.x, bounds.maxX));\n";
        indent(); output << "const closestY = Math.max(bounds.minY, Math.min(circle.y, bounds.maxY));\n";
        indent(); output << "const dx = circle.x - closestX;\n";
        indent(); output << "const dy = circle.y - closestY;\n";
        indent(); output << "return (dx * dx) + (dy * dy) <= (circle.radius * circle.radius);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleMiddleBoxBounds2D(capsule) {\n";
        indent_level++;
        indent(); output << "if (!capsule || capsule.halfSegment <= 0.000001) return null;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "minX: capsule.x - capsule.radius,\n";
        indent(); output << "maxX: capsule.x + capsule.radius,\n";
        indent(); output << "minY: capsule.y - capsule.halfSegment,\n";
        indent(); output << "maxY: capsule.y + capsule.halfSegment\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleTopCircle2D(capsule) {\n";
        indent_level++;
        indent(); output << "if (!capsule) return null;\n";
        indent(); output << "return { x: capsule.x, y: capsule.y - capsule.halfSegment, radius: capsule.radius };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleBottomCircle2D(capsule) {\n";
        indent_level++;
        indent(); output << "if (!capsule) return null;\n";
        indent(); output << "return { x: capsule.x, y: capsule.y + capsule.halfSegment, radius: capsule.radius };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCirclesOverlap2D(first, second) {\n";
        indent_level++;
        indent(); output << "if (!first || !second) return false;\n";
        indent(); output << "const dx = first.x - second.x;\n";
        indent(); output << "const dy = first.y - second.y;\n";
        indent(); output << "const radius = first.radius + second.radius;\n";
        indent(); output << "return (dx * dx) + (dy * dy) <= (radius * radius);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleIntersectsBox2D(capsule, box) {\n";
        indent_level++;
        indent(); output << "if (!capsule || !box) return false;\n";
        indent(); output << "const middle = this._zenithCapsuleMiddleBoxBounds2D(capsule);\n";
        indent(); output << "if (middle && middle.minX <= box.maxX && middle.maxX >= box.minX && middle.minY <= box.maxY && middle.maxY >= box.minY) return true;\n";
        indent(); output << "if (this._zenithBoxIntersectsCircle2D(box, this._zenithCapsuleTopCircle2D(capsule))) return true;\n";
        indent(); output << "if (this._zenithBoxIntersectsCircle2D(box, this._zenithCapsuleBottomCircle2D(capsule))) return true;\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleIntersectsCircle2D(capsule, circle) {\n";
        indent_level++;
        indent(); output << "if (!capsule || !circle) return false;\n";
        indent(); output << "const middle = this._zenithCapsuleMiddleBoxBounds2D(capsule);\n";
        indent(); output << "if (middle && this._zenithBoxIntersectsCircle2D(middle, circle)) return true;\n";
        indent(); output << "if (this._zenithCirclesOverlap2D(this._zenithCapsuleTopCircle2D(capsule), circle)) return true;\n";
        indent(); output << "if (this._zenithCirclesOverlap2D(this._zenithCapsuleBottomCircle2D(capsule), circle)) return true;\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleIntersectsCapsule2D(first, second) {\n";
        indent_level++;
        indent(); output << "if (!first || !second) return false;\n";
        indent(); output << "const firstMiddle = this._zenithCapsuleMiddleBoxBounds2D(first);\n";
        indent(); output << "const secondMiddle = this._zenithCapsuleMiddleBoxBounds2D(second);\n";
        indent(); output << "const firstTop = this._zenithCapsuleTopCircle2D(first);\n";
        indent(); output << "const firstBottom = this._zenithCapsuleBottomCircle2D(first);\n";
        indent(); output << "const secondTop = this._zenithCapsuleTopCircle2D(second);\n";
        indent(); output << "const secondBottom = this._zenithCapsuleBottomCircle2D(second);\n";
        indent(); output << "if (firstMiddle && secondMiddle && firstMiddle.minX <= secondMiddle.maxX && firstMiddle.maxX >= secondMiddle.minX && firstMiddle.minY <= secondMiddle.maxY && firstMiddle.maxY >= secondMiddle.minY) return true;\n";
        indent(); output << "if (firstMiddle && (this._zenithBoxIntersectsCircle2D(firstMiddle, secondTop) || this._zenithBoxIntersectsCircle2D(firstMiddle, secondBottom))) return true;\n";
        indent(); output << "if (secondMiddle && (this._zenithBoxIntersectsCircle2D(secondMiddle, firstTop) || this._zenithBoxIntersectsCircle2D(secondMiddle, firstBottom))) return true;\n";
        indent(); output << "return this._zenithCirclesOverlap2D(firstTop, secondTop) || this._zenithCirclesOverlap2D(firstTop, secondBottom) || this._zenithCirclesOverlap2D(firstBottom, secondTop) || this._zenithCirclesOverlap2D(firstBottom, secondBottom);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCapsuleContainsPoint2D(capsule, x, y) {\n";
        indent_level++;
        indent(); output << "if (!capsule) return false;\n";
        indent(); output << "const middle = this._zenithCapsuleMiddleBoxBounds2D(capsule);\n";
        indent(); output << "if (middle && x >= middle.minX && x <= middle.maxX && y >= middle.minY && y <= middle.maxY) return true;\n";
        indent(); output << "const top = this._zenithCapsuleTopCircle2D(capsule);\n";
        indent(); output << "const topDx = x - top.x;\n";
        indent(); output << "const topDy = y - top.y;\n";
        indent(); output << "if ((topDx * topDx) + (topDy * topDy) <= (top.radius * top.radius)) return true;\n";
        indent(); output << "const bottom = this._zenithCapsuleBottomCircle2D(capsule);\n";
        indent(); output << "const bottomDx = x - bottom.x;\n";
        indent(); output << "const bottomDy = y - bottom.y;\n";
        indent(); output << "return (bottomDx * bottomDx) + (bottomDy * bottomDy) <= (bottom.radius * bottom.radius);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithRaycastCapsule2D(originX, originY, dirX, dirY, capsule, maxDistance) {\n";
        indent_level++;
        indent(); output << "if (!capsule || maxDistance < 0) return null;\n";
        indent(); output << "let bestHit = null;\n";
        indent(); output << "let bestDistance = maxDistance;\n";
        indent(); output << "const consider = (hit) => {\n";
        indent_level++;
        indent(); output << "if (!hit || hit.distance > bestDistance) return;\n";
        indent(); output << "bestDistance = hit.distance;\n";
        indent(); output << "bestHit = hit;\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "const middle = this._zenithCapsuleMiddleBoxBounds2D(capsule);\n";
        indent(); output << "if (middle) consider(this._zenithRaycastBox2D(originX, originY, dirX, dirY, middle, bestDistance));\n";
        indent(); output << "consider(this._zenithRaycastCircle2D(originX, originY, dirX, dirY, this._zenithCapsuleTopCircle2D(capsule), bestDistance));\n";
        indent(); output << "consider(this._zenithRaycastCircle2D(originX, originY, dirX, dirY, this._zenithCapsuleBottomCircle2D(capsule), bestDistance));\n";
        indent(); output << "return bestHit;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureBody3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasBody = true;\n";
        indent(); output << "target.hasBody3D = true;\n";
        indent(); output << "if (target.vx === undefined) target.vx = 0;\n";
        indent(); output << "if (target.vy === undefined) target.vy = 0;\n";
        indent(); output << "if (target.vz === undefined) target.vz = 0;\n";
        indent(); output << "if (target.mass === undefined || target.mass <= 0) target.mass = 1;\n";
        indent(); output << "if (target.gravityScale === undefined) target.gravityScale = 1;\n";
        indent(); output << "if (target.friction === undefined) target.friction = 0.2;\n";
        indent(); output << "if (target.restitution === undefined) target.restitution = 0;\n";
        indent(); output << "if (target.useGravity === undefined) target.useGravity = true;\n";
        indent(); output << "if (target.isGrounded === undefined) target.isGrounded = false;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureBoxCollider3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasBoxCollider3D = true;\n";
        indent(); output << "if (target.boxCollider3DOffsetX === undefined) target.boxCollider3DOffsetX = 0;\n";
        indent(); output << "if (target.boxCollider3DOffsetY === undefined) target.boxCollider3DOffsetY = 0;\n";
        indent(); output << "if (target.boxCollider3DOffsetZ === undefined) target.boxCollider3DOffsetZ = 0;\n";
        indent(); output << "if (target.boxCollider3DWidth === undefined || target.boxCollider3DWidth <= 0) target.boxCollider3DWidth = 1;\n";
        indent(); output << "if (target.boxCollider3DHeight === undefined || target.boxCollider3DHeight <= 0) target.boxCollider3DHeight = 1;\n";
        indent(); output << "if (target.boxCollider3DDepth === undefined || target.boxCollider3DDepth <= 0) target.boxCollider3DDepth = 1;\n";
        indent(); output << "if (target.boxCollider3DIsTrigger === undefined) target.boxCollider3DIsTrigger = false;\n";
        indent(); output << "if (target.boxCollider3DEnabled === undefined) target.boxCollider3DEnabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureSphereCollider3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasSphereCollider3D = true;\n";
        indent(); output << "if (target.sphereCollider3DOffsetX === undefined) target.sphereCollider3DOffsetX = 0;\n";
        indent(); output << "if (target.sphereCollider3DOffsetY === undefined) target.sphereCollider3DOffsetY = 0;\n";
        indent(); output << "if (target.sphereCollider3DOffsetZ === undefined) target.sphereCollider3DOffsetZ = 0;\n";
        indent(); output << "if (target.sphereCollider3DRadius === undefined || target.sphereCollider3DRadius <= 0) target.sphereCollider3DRadius = 0.5;\n";
        indent(); output << "if (target.sphereCollider3DIsTrigger === undefined) target.sphereCollider3DIsTrigger = false;\n";
        indent(); output << "if (target.sphereCollider3DEnabled === undefined) target.sphereCollider3DEnabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithBoxColliderBounds3D(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity || entity.hasBoxCollider3D !== true || entity.boxCollider3DEnabled === false) return null;\n";
        indent(); output << "const width = entity.boxCollider3DWidth !== undefined && entity.boxCollider3DWidth > 0 ? entity.boxCollider3DWidth : 1;\n";
        indent(); output << "const height = entity.boxCollider3DHeight !== undefined && entity.boxCollider3DHeight > 0 ? entity.boxCollider3DHeight : 1;\n";
        indent(); output << "const depth = entity.boxCollider3DDepth !== undefined && entity.boxCollider3DDepth > 0 ? entity.boxCollider3DDepth : 1;\n";
        indent(); output << "const centerX = (entity.x !== undefined ? entity.x : 0) + (entity.boxCollider3DOffsetX !== undefined ? entity.boxCollider3DOffsetX : 0);\n";
        indent(); output << "const centerY = (entity.y !== undefined ? entity.y : 0) + (entity.boxCollider3DOffsetY !== undefined ? entity.boxCollider3DOffsetY : 0);\n";
        indent(); output << "const centerZ = (entity.z !== undefined ? entity.z : 0) + (entity.boxCollider3DOffsetZ !== undefined ? entity.boxCollider3DOffsetZ : 0);\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "minX: centerX - (width * 0.5),\n";
        indent(); output << "maxX: centerX + (width * 0.5),\n";
        indent(); output << "minY: centerY - (height * 0.5),\n";
        indent(); output << "maxY: centerY + (height * 0.5),\n";
        indent(); output << "minZ: centerZ - (depth * 0.5),\n";
        indent(); output << "maxZ: centerZ + (depth * 0.5)\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithSphereColliderState3D(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity || entity.hasSphereCollider3D !== true || entity.sphereCollider3DEnabled === false) return null;\n";
        indent(); output << "const radius = entity.sphereCollider3DRadius !== undefined && entity.sphereCollider3DRadius > 0 ? entity.sphereCollider3DRadius : 0.5;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "x: (entity.x !== undefined ? entity.x : 0) + (entity.sphereCollider3DOffsetX !== undefined ? entity.sphereCollider3DOffsetX : 0),\n";
        indent(); output << "y: (entity.y !== undefined ? entity.y : 0) + (entity.sphereCollider3DOffsetY !== undefined ? entity.sphereCollider3DOffsetY : 0),\n";
        indent(); output << "z: (entity.z !== undefined ? entity.z : 0) + (entity.sphereCollider3DOffsetZ !== undefined ? entity.sphereCollider3DOffsetZ : 0),\n";
        indent(); output << "radius\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithBoxIntersectsSphere3D(bounds, sphere) {\n";
        indent_level++;
        indent(); output << "if (!bounds || !sphere) return false;\n";
        indent(); output << "const closestX = Math.max(bounds.minX, Math.min(sphere.x, bounds.maxX));\n";
        indent(); output << "const closestY = Math.max(bounds.minY, Math.min(sphere.y, bounds.maxY));\n";
        indent(); output << "const closestZ = Math.max(bounds.minZ, Math.min(sphere.z, bounds.maxZ));\n";
        indent(); output << "const dx = sphere.x - closestX;\n";
        indent(); output << "const dy = sphere.y - closestY;\n";
        indent(); output << "const dz = sphere.z - closestZ;\n";
        indent(); output << "return (dx * dx) + (dy * dy) + (dz * dz) <= (sphere.radius * sphere.radius);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithRaycastSphere3D(originX, originY, originZ, dirX, dirY, dirZ, sphere, maxDistance) {\n";
        indent_level++;
        indent(); output << "if (!sphere || maxDistance < 0) return null;\n";
        indent(); output << "const a = (dirX * dirX) + (dirY * dirY) + (dirZ * dirZ);\n";
        indent(); output << "if (a <= 0.0000001) return null;\n";
        indent(); output << "const ocX = originX - sphere.x;\n";
        indent(); output << "const ocY = originY - sphere.y;\n";
        indent(); output << "const ocZ = originZ - sphere.z;\n";
        indent(); output << "const b = 2 * ((ocX * dirX) + (ocY * dirY) + (ocZ * dirZ));\n";
        indent(); output << "const c = (ocX * ocX) + (ocY * ocY) + (ocZ * ocZ) - (sphere.radius * sphere.radius);\n";
        indent(); output << "const discriminant = (b * b) - (4 * a * c);\n";
        indent(); output << "if (discriminant < 0) return null;\n";
        indent(); output << "const sqrtDisc = Math.sqrt(discriminant);\n";
        indent(); output << "let distance = (-b - sqrtDisc) / (2 * a);\n";
        indent(); output << "if (distance < 0) distance = (-b + sqrtDisc) / (2 * a);\n";
        indent(); output << "if (distance < 0 || distance > maxDistance) return null;\n";
        indent(); output << "const pointX = originX + (dirX * distance);\n";
        indent(); output << "const pointY = originY + (dirY * distance);\n";
        indent(); output << "const pointZ = originZ + (dirZ * distance);\n";
        indent(); output << "let normalX = pointX - sphere.x;\n";
        indent(); output << "let normalY = pointY - sphere.y;\n";
        indent(); output << "let normalZ = pointZ - sphere.z;\n";
        indent(); output << "const normalLength = Math.sqrt((normalX * normalX) + (normalY * normalY) + (normalZ * normalZ));\n";
        indent(); output << "if (normalLength > 0.0000001) {\n";
        indent_level++;
        indent(); output << "normalX /= normalLength;\n";
        indent(); output << "normalY /= normalLength;\n";
        indent(); output << "normalZ /= normalLength;\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "normalX = -dirX;\n";
        indent(); output << "normalY = -dirY;\n";
        indent(); output << "normalZ = -dirZ;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return { distance, pointX, pointY, pointZ, normalX, normalY, normalZ };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithRaycastBox3D(originX, originY, originZ, dirX, dirY, dirZ, bounds, maxDistance) {\n";
        indent_level++;
        indent(); output << "if (!bounds || maxDistance < 0) return null;\n";
        indent(); output << "const dirEpsilon = 0.0000001;\n";
        indent(); output << "let tMin = 0;\n";
        indent(); output << "let tMax = maxDistance;\n";
        indent(); output << "let normalX = 0;\n";
        indent(); output << "let normalY = 0;\n";
        indent(); output << "let normalZ = 0;\n";
        indent(); output << "const axisTest = (origin, dir, min, max, nx, ny, nz) => {\n";
        indent_level++;
        indent(); output << "if (Math.abs(dir) <= dirEpsilon) {\n";
        indent_level++;
        indent(); output << "return origin >= min && origin <= max;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "let near = (min - origin) / dir;\n";
        indent(); output << "let far = (max - origin) / dir;\n";
        indent(); output << "let hitNx = nx;\n";
        indent(); output << "let hitNy = ny;\n";
        indent(); output << "let hitNz = nz;\n";
        indent(); output << "if (near > far) {\n";
        indent_level++;
        indent(); output << "const swap = near;\n";
        indent(); output << "near = far;\n";
        indent(); output << "far = swap;\n";
        indent(); output << "hitNx = -nx;\n";
        indent(); output << "hitNy = -ny;\n";
        indent(); output << "hitNz = -nz;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (near > tMin) {\n";
        indent_level++;
        indent(); output << "tMin = near;\n";
        indent(); output << "normalX = hitNx;\n";
        indent(); output << "normalY = hitNy;\n";
        indent(); output << "normalZ = hitNz;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (far < tMax) tMax = far;\n";
        indent(); output << "return tMin <= tMax;\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "if (!axisTest(originX, dirX, bounds.minX, bounds.maxX, -1, 0, 0)) return null;\n";
        indent(); output << "if (!axisTest(originY, dirY, bounds.minY, bounds.maxY, 0, -1, 0)) return null;\n";
        indent(); output << "if (!axisTest(originZ, dirZ, bounds.minZ, bounds.maxZ, 0, 0, -1)) return null;\n";
        indent(); output << "if (tMax < 0) return null;\n";
        indent(); output << "const distance = tMin >= 0 ? tMin : 0;\n";
        indent(); output << "if (distance > maxDistance) return null;\n";
        indent(); output << "const pointX = originX + (dirX * distance);\n";
        indent(); output << "const pointY = originY + (dirY * distance);\n";
        indent(); output << "const pointZ = originZ + (dirZ * distance);\n";
        indent(); output << "if (distance === 0 && normalX === 0 && normalY === 0 && normalZ === 0) {\n";
        indent_level++;
        indent(); output << "normalX = -dirX;\n";
        indent(); output << "normalY = -dirY;\n";
        indent(); output << "normalZ = -dirZ;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return { distance, pointX, pointY, pointZ, normalX, normalY, normalZ };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureCamera2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "if (target.zoom === undefined) target.zoom = 1;\n";
        indent(); output << "if (target.primary === undefined) target.primary = false;\n";
        indent(); output << "if (target.viewportX === undefined) target.viewportX = 0;\n";
        indent(); output << "if (target.viewportY === undefined) target.viewportY = 0;\n";
        indent(); output << "if (target.viewportWidth === undefined) target.viewportWidth = 1;\n";
        indent(); output << "if (target.viewportHeight === undefined) target.viewportHeight = 1;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureAudioListener2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasAudioListener2D = true;\n";
        indent(); output << "if (target.gain === undefined) target.gain = 1;\n";
        indent(); output << "if (target.primary === undefined) target.primary = false;\n";
        indent(); output << "if (target.enabled === undefined) target.enabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureCamera3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "if (target.fov === undefined) target.fov = 60;\n";
        indent(); output << "if (target.nearClip === undefined) target.nearClip = 0.1;\n";
        indent(); output << "if (target.farClip === undefined) target.farClip = 1000;\n";
        indent(); output << "if (target.primary === undefined) target.primary = false;\n";
        indent(); output << "if (target.viewportX === undefined) target.viewportX = 0;\n";
        indent(); output << "if (target.viewportY === undefined) target.viewportY = 0;\n";
        indent(); output << "if (target.viewportWidth === undefined) target.viewportWidth = 1;\n";
        indent(); output << "if (target.viewportHeight === undefined) target.viewportHeight = 1;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureAudioListener3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasAudioListener3D = true;\n";
        indent(); output << "if (target.gain === undefined) target.gain = 1;\n";
        indent(); output << "if (target.primary === undefined) target.primary = false;\n";
        indent(); output << "if (target.enabled === undefined) target.enabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureSprite2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "if (target.w === undefined) target.w = 1;\n";
        indent(); output << "if (target.h === undefined) target.h = 1;\n";
        indent(); output << "if (target.anchorX === undefined) target.anchorX = 0.5;\n";
        indent(); output << "if (target.anchorY === undefined) target.anchorY = 0.5;\n";
        indent(); output << "if (target.color === undefined) target.color = 'white';\n";
        indent(); output << "if (target.texturePath === undefined) target.texturePath = '';\n";
        indent(); output << "if (target.sortOrder === undefined) target.sortOrder = 0;\n";
        indent(); output << "if (target.visible === undefined) target.visible = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureTilemap2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "if (target.columns === undefined || target.columns < 1) target.columns = 1;\n";
        indent(); output << "if (target.rows === undefined || target.rows < 1) target.rows = 1;\n";
        indent(); output << "if (target.tileWidth === undefined || target.tileWidth <= 0) target.tileWidth = 1;\n";
        indent(); output << "if (target.tileHeight === undefined || target.tileHeight <= 0) target.tileHeight = 1;\n";
        indent(); output << "if (target.anchorX === undefined) target.anchorX = 0;\n";
        indent(); output << "if (target.anchorY === undefined) target.anchorY = 0;\n";
        indent(); output << "if (target.sortOrder === undefined) target.sortOrder = 0;\n";
        indent(); output << "if (target.visible === undefined) target.visible = true;\n";
        indent(); output << "const expectedCellCount = target.columns * target.rows;\n";
        indent(); output << "if (!Array.isArray(target.cells)) {\n";
        indent_level++;
        indent(); output << "target.cells = new Array(expectedCellCount).fill(0);\n";
        indent_level--;
        indent(); output << "} else if (target.cells.length !== expectedCellCount) {\n";
        indent_level++;
        indent(); output << "const nextCells = new Array(expectedCellCount).fill(0);\n";
        indent(); output << "const copyCount = Math.min(expectedCellCount, target.cells.length);\n";
        indent(); output << "for (let i = 0; i < copyCount; i += 1) nextCells[i] = Number(target.cells[i]) || 0;\n";
        indent(); output << "target.cells = nextCells;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (!Array.isArray(target.palette) || target.palette.length === 0) target.palette = ['', 'white'];\n";
        indent(); output << "if (target.palette[0] === undefined) target.palette[0] = '';\n";
        indent(); output << "if (target.palette.length < 2) target.palette.push('white');\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureCharacter2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureSprite2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "this._zenithEnsureBody2D(target);\n";
        indent(); output << "if (target.moveSpeed === undefined) target.moveSpeed = 10;\n";
        indent(); output << "if (target.jumpForce === undefined) target.jumpForce = 12;\n";
        indent(); output << "if (target.isGrounded === undefined) target.isGrounded = false;\n";
        indent(); output << "if (target.facingRight === undefined) target.facingRight = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureAudioSource2D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform2D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasAudio2D = true;\n";
        indent(); output << "if (target.clipPath === undefined) target.clipPath = '';\n";
        indent(); output << "if (target.volume === undefined) target.volume = 1;\n";
        indent(); output << "if (target.pitch === undefined) target.pitch = 1;\n";
        indent(); output << "if (target.loop === undefined) target.loop = false;\n";
        indent(); output << "if (target.playOnAwake === undefined) target.playOnAwake = false;\n";
        indent(); output << "if (target.isPlaying === undefined) target.isPlaying = false;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureMesh3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "if (target.meshPath === undefined) target.meshPath = '';\n";
        indent(); output << "if (target.shaderPath === undefined) target.shaderPath = '';\n";
        indent(); output << "if (target.materialPath === undefined) target.materialPath = '';\n";
        indent(); output << "if (target.visible === undefined) target.visible = true;\n";
        indent(); output << "if (target.castShadows === undefined) target.castShadows = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureCharacter3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureMesh3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "this._zenithEnsureBody3D(target);\n";
        indent(); output << "if (target.moveSpeed === undefined) target.moveSpeed = 6;\n";
        indent(); output << "if (target.turnSpeed === undefined) target.turnSpeed = 4;\n";
        indent(); output << "if (target.jumpSpeed === undefined) target.jumpSpeed = 7.5;\n";
        indent(); output << "if (target.groundAcceleration === undefined) target.groundAcceleration = 36;\n";
        indent(); output << "if (target.airAcceleration === undefined) target.airAcceleration = 14;\n";
        indent(); output << "if (target.groundFriction === undefined) target.groundFriction = 20;\n";
        indent(); output << "if (target.airControl === undefined) target.airControl = 0.35;\n";
        indent(); output << "if (target.groundSnapDistance === undefined) target.groundSnapDistance = 0.2;\n";
        indent(); output << "if (target.maxSlopeAngle === undefined) target.maxSlopeAngle = 55;\n";
        indent(); output << "if (target.moveInputX === undefined) target.moveInputX = 0;\n";
        indent(); output << "if (target.moveInputY === undefined) target.moveInputY = 0;\n";
        indent(); output << "if (target.moveInputZ === undefined) target.moveInputZ = 0;\n";
        indent(); output << "if (target.isGrounded === undefined) target.isGrounded = false;\n";
        indent(); output << "if (target.useGravity === undefined) target.useGravity = true;\n";
        indent(); output << "if (target.jumpQueued === undefined) target.jumpQueued = false;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureAudioSource3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasAudio3D = true;\n";
        indent(); output << "if (target.clipPath === undefined) target.clipPath = '';\n";
        indent(); output << "if (target.volume === undefined) target.volume = 1;\n";
        indent(); output << "if (target.pitch === undefined) target.pitch = 1;\n";
        indent(); output << "if (target.loop === undefined) target.loop = false;\n";
        indent(); output << "if (target.playOnAwake === undefined) target.playOnAwake = false;\n";
        indent(); output << "if (target.isPlaying === undefined) target.isPlaying = false;\n";
        indent(); output << "if (target.minDistance === undefined) target.minDistance = 1;\n";
        indent(); output << "if (target.maxDistance === undefined) target.maxDistance = 20;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsurePointLight3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasPointLight3D = true;\n";
        indent(); output << "if (target.intensity === undefined) target.intensity = 1;\n";
        indent(); output << "if (target.range === undefined) target.range = 10;\n";
        indent(); output << "if (target.color === undefined) target.color = 'white';\n";
        indent(); output << "if (target.enabled === undefined) target.enabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureDirectionalLight3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTransform3D(entity);\n";
        indent(); output << "if (!target) return null;\n";
        indent(); output << "target.hasDirectionalLight3D = true;\n";
        indent(); output << "if (target.directionX === undefined) target.directionX = 0;\n";
        indent(); output << "if (target.directionY === undefined) target.directionY = -1;\n";
        indent(); output << "if (target.directionZ === undefined) target.directionZ = 0;\n";
        indent(); output << "if (target.intensity === undefined) target.intensity = 1;\n";
        indent(); output << "if (target.color === undefined) target.color = 'white';\n";
        indent(); output << "if (target.castShadows === undefined) target.castShadows = true;\n";
        indent(); output << "if (target.enabled === undefined) target.enabled = true;\n";
        indent(); output << "return target;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithNormalizeAssetPath(path) {\n";
        indent_level++;
        indent(); output << "const raw = path === undefined || path === null ? '' : String(path).replace(/\\\\/g, '/');\n";
        indent(); output << "if (!raw) return '';\n";
        indent(); output << "const parts = raw.split('/');\n";
        indent(); output << "const stack = [];\n";
        indent(); output << "for (const part of parts) {\n";
        indent_level++;
        indent(); output << "if (!part || part === '.') continue;\n";
        indent(); output << "if (part === '..') {\n";
        indent_level++;
        indent(); output << "if (stack.length > 0 && stack[stack.length - 1] !== '..') stack.pop();\n";
        indent(); output << "else stack.push(part);\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "stack.push(part);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return stack.join('/');\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithGuessAssetType(path) {\n";
        indent_level++;
        indent(); output << "const normalized = this._zenithNormalizeAssetPath(path).toLowerCase();\n";
        indent(); output << "const dot = normalized.lastIndexOf('.');\n";
        indent(); output << "const ext = dot >= 0 ? normalized.slice(dot) : '';\n";
        indent(); output << "if (['.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tga', '.dds', '.webp'].includes(ext)) return 'Texture';\n";
        indent(); output << "if (['.wav', '.mp3', '.ogg', '.flac'].includes(ext)) return 'Audio';\n";
        indent(); output << "if (['.obj', '.fbx', '.gltf', '.glb', '.dae', '.mesh'].includes(ext)) return 'Mesh';\n";
        indent(); output << "if (['.shader', '.vert', '.frag', '.glsl', '.hlsl'].includes(ext)) return 'Shader';\n";
        indent(); output << "if (['.mat', '.material'].includes(ext)) return 'Material';\n";
        indent(); output << "if (['.ttf', '.otf', '.fnt'].includes(ext)) return 'Font';\n";
        indent(); output << "if (['.particle', '.particles', '.vfx'].includes(ext)) return 'Particles';\n";
        indent(); output << "return 'Unknown';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithAssetFolder(assetType) {\n";
        indent_level++;
        indent(); output << "switch (assetType) {\n";
        indent_level++;
        indent(); output << "case 'Texture': return 'textures';\n";
        indent(); output << "case 'Audio': return 'audio';\n";
        indent(); output << "case 'Mesh': return 'meshes';\n";
        indent(); output << "case 'Shader': return 'shaders';\n";
        indent(); output << "case 'Material': return 'materials';\n";
        indent(); output << "case 'Font': return 'fonts';\n";
        indent(); output << "case 'Particles': return 'particles';\n";
        indent(); output << "default: return 'misc';\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithDefaultAssetBytes(assetType) {\n";
        indent_level++;
        indent(); output << "switch (assetType) {\n";
        indent_level++;
        indent(); output << "case 'Texture': return 4096;\n";
        indent(); output << "case 'Audio': return 2048;\n";
        indent(); output << "case 'Mesh': return 6144;\n";
        indent(); output << "case 'Shader': return 1024;\n";
        indent(); output << "case 'Material': return 1024;\n";
        indent(); output << "case 'Font': return 2048;\n";
        indent(); output << "case 'Particles': return 1024;\n";
        indent(); output << "default: return 1024;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEstimateAssetBytes(assetType, sourceBytes = 0) {\n";
        indent_level++;
        indent(); output << "const seed = sourceBytes > 0 ? sourceBytes : this._zenithDefaultAssetBytes(assetType);\n";
        indent(); output << "switch (assetType) {\n";
        indent_level++;
        indent(); output << "case 'Texture': return Math.max(4096, seed * 4);\n";
        indent(); output << "case 'Audio': return Math.max(2048, seed * 2);\n";
        indent(); output << "case 'Mesh': return Math.max(6144, seed * 3);\n";
        indent(); output << "case 'Shader': return Math.max(1024, seed);\n";
        indent(); output << "case 'Material': return Math.max(1024, seed);\n";
        indent(); output << "case 'Font': return Math.max(2048, seed * 2);\n";
        indent(); output << "case 'Particles': return Math.max(1024, seed * 2);\n";
        indent(); output << "default: return Math.max(1024, seed);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithFindAssetMetadata(path) {\n";
        indent_level++;
        indent(); output << "const normalized = this._zenithNormalizeAssetPath(path);\n";
        indent(); output << "if (!normalized) return null;\n";
        indent(); output << "if (this._zenithAssetMetadata.has(normalized)) return this._zenithAssetMetadata.get(normalized);\n";
        indent(); output << "for (const metadata of this._zenithAssetMetadata.values()) {\n";
        indent_level++;
        indent(); output << "if (metadata && metadata.sourcePath === normalized) return metadata;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureAssetMetadata(path, requestedType = 'Unknown') {\n";
        indent_level++;
        indent(); output << "const normalized = this._zenithNormalizeAssetPath(path);\n";
        indent(); output << "if (!normalized) return null;\n";
        indent(); output << "let metadata = this._zenithFindAssetMetadata(normalized);\n";
        indent(); output << "if (!metadata) {\n";
        indent_level++;
        indent(); output << "const type = requestedType && requestedType !== 'Unknown' ? requestedType : this._zenithGuessAssetType(normalized);\n";
        indent(); output << "metadata = {\n";
        indent_level++;
        indent(); output << "sourcePath: normalized,\n";
        indent(); output << "importedPath: normalized,\n";
        indent(); output << "type,\n";
        indent(); output << "group: '',\n";
        indent(); output << "sourceBytes: 0,\n";
        indent(); output << "estimatedMemoryBytes: this._zenithEstimateAssetBytes(type, 0),\n";
        indent(); output << "sourceTimestamp: 0,\n";
        indent(); output << "importedTimestamp: 0,\n";
        indent(); output << "version: 1,\n";
        indent(); output << "hotReloadable: true,\n";
        indent(); output << "dirty: false\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithAssetMetadata.set(normalized, metadata);\n";
        indent_level--;
        indent(); output << "} else if ((!metadata.type || metadata.type === 'Unknown') && requestedType && requestedType !== 'Unknown') {\n";
        indent_level++;
        indent(); output << "metadata.type = requestedType;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (!metadata.estimatedMemoryBytes || metadata.estimatedMemoryBytes <= 0) {\n";
        indent_level++;
        indent(); output << "metadata.estimatedMemoryBytes = this._zenithEstimateAssetBytes(metadata.type || requestedType || 'Unknown', metadata.sourceBytes || 0);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return metadata;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithAssetHandleStore(assetType) {\n";
        indent_level++;
        indent(); output << "switch (assetType) {\n";
        indent_level++;
        indent(); output << "case 'Texture': return this._zenithTextureHandles;\n";
        indent(); output << "case 'Audio': return this._zenithAudioHandles;\n";
        indent(); output << "case 'Mesh': return this._zenithMeshHandles;\n";
        indent(); output << "case 'Shader': return this._zenithShaderHandles;\n";
        indent(); output << "case 'Material': return this._zenithMaterialHandles;\n";
        indent(); output << "default: return null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithAssetMemoryUsageByType(assetType) {\n";
        indent_level++;
        indent(); output << "const store = this._zenithAssetHandleStore(assetType);\n";
        indent(); output << "if (!store) return 0;\n";
        indent(); output << "let total = 0;\n";
        indent(); output << "for (const handle of store.values()) {\n";
        indent_level++;
        indent(); output << "if (handle && handle.loaded) total += handle.memoryBytes || 0;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return total;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithAssetBudgetValue(assetType) {\n";
        indent_level++;
        indent(); output << "return this._zenithAssetBudgets.has(assetType) ? Number(this._zenithAssetBudgets.get(assetType)) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCanLoadAsset(assetType, handle, requestedBytes) {\n";
        indent_level++;
        indent(); output << "const budget = this._zenithAssetBudgetValue(assetType);\n";
        indent(); output << "if (budget <= 0) return true;\n";
        indent(); output << "const existingBytes = handle && handle.loaded ? Number(handle.memoryBytes || 0) : 0;\n";
        indent(); output << "return this._zenithAssetMemoryUsageByType(assetType) - existingBytes + requestedBytes <= budget;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureHandleRecord(store, path, assetType = 'Unknown') {\n";
        indent_level++;
        indent(); output << "const assetPath = this._zenithNormalizeAssetPath(path);\n";
        indent(); output << "if (!assetPath) return null;\n";
        indent(); output << "let handle = store.get(assetPath);\n";
        indent(); output << "if (!handle) {\n";
        indent_level++;
        indent(); output << "handle = { path: assetPath, id: this._zenithNextAssetHandleId++, refCount: 0, loaded: false, memoryBytes: 0, version: 1, type: assetType || 'Unknown' };\n";
        indent(); output << "store.set(assetPath, handle);\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "handle.path = assetPath;\n";
        indent(); output << "handle.type = assetType || handle.type || 'Unknown';\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return handle;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithTextureHandle(texturePath) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithEnsureAssetMetadata(texturePath, 'Texture');\n";
        indent(); output << "if (!metadata) return { path: '', id: 0, refCount: 0, loaded: false, width: 0, height: 0, channels: 0, gpuId: 0 };\n";
        indent(); output << "const handle = this._zenithEnsureHandleRecord(this._zenithTextureHandles, metadata.importedPath || texturePath, 'Texture');\n";
        indent(); output << "const memoryBytes = metadata.estimatedMemoryBytes || this._zenithEstimateAssetBytes('Texture', metadata.sourceBytes || 0);\n";
        indent(); output << "if (!this._zenithCanLoadAsset('Texture', handle, memoryBytes)) {\n";
        indent_level++;
        indent(); output << "handle.loaded = false;\n";
        indent(); output << "handle.refCount = 0;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: 0, loaded: false, width: 0, height: 0, channels: 0, gpuId: 0 };\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "handle.loaded = true;\n";
        indent(); output << "handle.refCount = 1;\n";
        indent(); output << "handle.memoryBytes = memoryBytes;\n";
        indent(); output << "handle.version = metadata.version || 1;\n";
        indent(); output << "metadata.type = 'Texture';\n";
        indent(); output << "metadata.estimatedMemoryBytes = memoryBytes;\n";
        indent(); output << "metadata.dirty = false;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: handle.refCount, loaded: handle.loaded, width: 0, height: 0, channels: 0, gpuId: 1 };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithAudioHandle(clipPath, spatial = false) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithEnsureAssetMetadata(clipPath, 'Audio');\n";
        indent(); output << "if (!metadata) return { path: '', id: 0, refCount: 0, loaded: false, duration: 0, spatial: false };\n";
        indent(); output << "const handle = this._zenithEnsureHandleRecord(this._zenithAudioHandles, metadata.importedPath || clipPath, 'Audio');\n";
        indent(); output << "const memoryBytes = metadata.estimatedMemoryBytes || this._zenithEstimateAssetBytes('Audio', metadata.sourceBytes || 0);\n";
        indent(); output << "if (!this._zenithCanLoadAsset('Audio', handle, memoryBytes)) {\n";
        indent_level++;
        indent(); output << "handle.loaded = false;\n";
        indent(); output << "handle.refCount = 0;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: 0, loaded: false, duration: 0, spatial: false };\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "handle.loaded = true;\n";
        indent(); output << "handle.refCount = 1;\n";
        indent(); output << "handle.memoryBytes = memoryBytes;\n";
        indent(); output << "handle.version = metadata.version || 1;\n";
        indent(); output << "metadata.type = 'Audio';\n";
        indent(); output << "metadata.estimatedMemoryBytes = memoryBytes;\n";
        indent(); output << "metadata.dirty = false;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: handle.refCount, loaded: handle.loaded, duration: 0, spatial: spatial === true };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithMeshHandle(meshPath) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithEnsureAssetMetadata(meshPath, 'Mesh');\n";
        indent(); output << "if (!metadata) return { path: '', id: 0, refCount: 0, loaded: false, vertexCount: 0, triangleCount: 0, vbo: 0, ebo: 0 };\n";
        indent(); output << "const handle = this._zenithEnsureHandleRecord(this._zenithMeshHandles, metadata.importedPath || meshPath, 'Mesh');\n";
        indent(); output << "const memoryBytes = metadata.estimatedMemoryBytes || this._zenithEstimateAssetBytes('Mesh', metadata.sourceBytes || 0);\n";
        indent(); output << "if (!this._zenithCanLoadAsset('Mesh', handle, memoryBytes)) {\n";
        indent_level++;
        indent(); output << "handle.loaded = false;\n";
        indent(); output << "handle.refCount = 0;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: 0, loaded: false, vertexCount: 0, triangleCount: 0, vbo: 0, ebo: 0 };\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "handle.loaded = true;\n";
        indent(); output << "handle.refCount = 1;\n";
        indent(); output << "handle.memoryBytes = memoryBytes;\n";
        indent(); output << "handle.version = metadata.version || 1;\n";
        indent(); output << "metadata.type = 'Mesh';\n";
        indent(); output << "metadata.estimatedMemoryBytes = memoryBytes;\n";
        indent(); output << "metadata.dirty = false;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: handle.refCount, loaded: handle.loaded, vertexCount: 0, triangleCount: 0, vbo: 0, ebo: 0 };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithShaderHandle(shaderPath) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithEnsureAssetMetadata(shaderPath, 'Shader');\n";
        indent(); output << "if (!metadata) return { path: '', id: 0, refCount: 0, loaded: false, programId: 0 };\n";
        indent(); output << "const handle = this._zenithEnsureHandleRecord(this._zenithShaderHandles, metadata.importedPath || shaderPath, 'Shader');\n";
        indent(); output << "const memoryBytes = metadata.estimatedMemoryBytes || this._zenithEstimateAssetBytes('Shader', metadata.sourceBytes || 0);\n";
        indent(); output << "if (!this._zenithCanLoadAsset('Shader', handle, memoryBytes)) {\n";
        indent_level++;
        indent(); output << "handle.loaded = false;\n";
        indent(); output << "handle.refCount = 0;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: 0, loaded: false, programId: 0 };\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "handle.loaded = true;\n";
        indent(); output << "handle.refCount = 1;\n";
        indent(); output << "handle.memoryBytes = memoryBytes;\n";
        indent(); output << "handle.version = metadata.version || 1;\n";
        indent(); output << "metadata.type = 'Shader';\n";
        indent(); output << "metadata.estimatedMemoryBytes = memoryBytes;\n";
        indent(); output << "metadata.dirty = false;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: handle.refCount, loaded: handle.loaded, programId: 0 };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithMaterialHandle(materialPath, shaderPath = '') {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithEnsureAssetMetadata(materialPath, 'Material');\n";
        indent(); output << "if (!metadata) return { path: '', id: 0, refCount: 0, loaded: false, shaderPath: '', propertyCount: 0 };\n";
        indent(); output << "const path = metadata.importedPath || this._zenithNormalizeAssetPath(materialPath);\n";
        indent(); output << "const material = this._zenithEnsureMaterial(path, shaderPath);\n";
        indent(); output << "const handle = this._zenithEnsureHandleRecord(this._zenithMaterialHandles, path, 'Material');\n";
        indent(); output << "const memoryBytes = metadata.estimatedMemoryBytes || this._zenithEstimateAssetBytes('Material', metadata.sourceBytes || 0);\n";
        indent(); output << "if (!this._zenithCanLoadAsset('Material', handle, memoryBytes)) {\n";
        indent_level++;
        indent(); output << "handle.loaded = false;\n";
        indent(); output << "handle.refCount = 0;\n";
        indent(); output << "return { path: handle.path, id: handle.id, refCount: 0, loaded: false, shaderPath: material && material.shaderPath ? material.shaderPath : '', propertyCount: material && material.properties ? material.properties.size : 0 };\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "handle.loaded = true;\n";
        indent(); output << "handle.refCount = 1;\n";
        indent(); output << "handle.memoryBytes = memoryBytes;\n";
        indent(); output << "handle.version = metadata.version || 1;\n";
        indent(); output << "metadata.type = 'Material';\n";
        indent(); output << "metadata.estimatedMemoryBytes = memoryBytes;\n";
        indent(); output << "metadata.dirty = false;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "path,\n";
        indent(); output << "id: handle ? handle.id : 0,\n";
        indent(); output << "refCount: handle ? handle.refCount : 0,\n";
        indent(); output << "loaded: !!(handle && handle.loaded),\n";
        indent(); output << "shaderPath: material && material.shaderPath ? material.shaderPath : '',\n";
        indent(); output << "propertyCount: material && material.properties ? material.properties.size : 0\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureMaterial(materialPath, shaderPath = '') {\n";
        indent_level++;
        indent(); output << "if (!materialPath) return null;\n";
        indent(); output << "let material = this._zenithMaterials.get(materialPath);\n";
        indent(); output << "if (!material) {\n";
        indent_level++;
        indent(); output << "material = {\n";
        indent_level++;
        indent(); output << "path: materialPath,\n";
        indent(); output << "shaderPath: shaderPath || '',\n";
        indent(); output << "properties: new Map()\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithMaterials.set(materialPath, material);\n";
        indent_level--;
        indent(); output << "} else if (shaderPath) {\n";
        indent_level++;
        indent(); output << "material.shaderPath = shaderPath;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return material;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithParseMaterialOptions(optionsCsv) {\n";
        indent_level++;
        indent(); output << "if (!optionsCsv) return [];\n";
        indent(); output << "return String(optionsCsv)\n";
        indent_level++;
        indent(); output << ".split(',')\n";
        indent(); output << ".map((item) => item.trim())\n";
        indent(); output << ".filter((item) => item.length > 0)\n";
        indent(); output << ".map((item) => ({ label: item, value: item }));\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithNormalizeMaterialPropertyKind(kindName) {\n";
        indent_level++;
        indent(); output << "const value = kindName === undefined || kindName === null ? '' : String(kindName).trim();\n";
        indent(); output << "const validKinds = ['Text', 'Number', 'Toggle', 'Radio', 'Image', 'Button', 'Color'];\n";
        indent(); output << "return validKinds.includes(value) ? value : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEnsureMaterialProperty(materialPath, propertyName, kind, label) {\n";
        indent_level++;
        indent(); output << "const material = this._zenithEnsureMaterial(materialPath);\n";
        indent(); output << "if (!material || !propertyName) return null;\n";
        indent(); output << "let property = material.properties.get(propertyName);\n";
        indent(); output << "if (!property) {\n";
        indent_level++;
        indent(); output << "property = {\n";
        indent_level++;
        indent(); output << "name: propertyName,\n";
        indent(); output << "label: label || '',\n";
        indent(); output << "kind,\n";
        indent(); output << "stringValue: '',\n";
        indent(); output << "numberValue: 0,\n";
        indent(); output << "boolValue: false,\n";
        indent(); output << "callbackName: '',\n";
        indent(); output << "triggerCount: 0,\n";
        indent(); output << "options: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "material.properties.set(propertyName, property);\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "property.kind = kind;\n";
        indent(); output << "property.label = label || '';\n";
        indent(); output << "if (property.callbackName === undefined || property.callbackName === null) property.callbackName = '';\n";
        indent(); output << "if (property.triggerCount === undefined || property.triggerCount === null) property.triggerCount = 0;\n";
        indent(); output << "property.options = [];\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return property;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithMaterialProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "if (!materialPath || !propertyName) return null;\n";
        indent(); output << "const material = this._zenithMaterials.get(materialPath);\n";
        indent(); output << "if (!material) return null;\n";
        indent(); output << "return material.properties.get(propertyName) || null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithMaterialPropertyCallbackValue(property) {\n";
        indent_level++;
        indent(); output << "if (!property) return '';\n";
        indent(); output << "if (property.kind === 'Number') return String(property.numberValue !== undefined ? property.numberValue : 0);\n";
        indent(); output << "if (property.kind === 'Toggle') return property.boolValue === true ? 'true' : 'false';\n";
        indent(); output << "return property.stringValue === undefined || property.stringValue === null ? '' : String(property.stringValue);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithMaterialPropertyOption(materialPath, propertyName, index) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property || !Array.isArray(property.options) || index < 0 || index >= property.options.length) return null;\n";
        indent(); output << "return property.options[index] || null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCloneMaterialPropertyState(property, fallbackName = '') {\n";
        indent_level++;
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "name: property && property.name !== undefined ? property.name : fallbackName,\n";
        indent(); output << "label: property && property.label !== undefined ? property.label : '',\n";
        indent(); output << "kind: property && property.kind !== undefined ? property.kind : 'Text',\n";
        indent(); output << "stringValue: property && property.stringValue !== undefined ? property.stringValue : '',\n";
        indent(); output << "numberValue: property && property.numberValue !== undefined ? property.numberValue : 0,\n";
        indent(); output << "boolValue: property ? property.boolValue === true : false,\n";
        indent(); output << "callbackName: property && property.callbackName !== undefined ? property.callbackName : '',\n";
        indent(); output << "triggerCount: property && property.triggerCount !== undefined ? Math.max(0, Math.trunc(Number(property.triggerCount) || 0)) : 0,\n";
        indent(); output << "options: Array.isArray(property && property.options) ? property.options.map((option) => ({ label: option && option.label !== undefined ? option.label : (option && option.value !== undefined ? option.value : ''), value: option && option.value !== undefined ? option.value : (option && option.label !== undefined ? option.label : '') })) : []\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithInitEntityLayerMask(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity) return entity;\n";
        indent(); output << "const rawLayer = Math.trunc(Number(entity.layer) || 0);\n";
        indent(); output << "const rawMask = Math.trunc(Number(entity.mask));\n";
        indent(); output << "entity.layer = rawLayer === 0 ? 1 : rawLayer;\n";
        indent(); output << "entity.mask = Number.isNaN(rawMask) ? 0xFFFFFFFF : rawMask;\n";
        indent(); output << "if (!Array.isArray(entity.children)) entity.children = [];\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEntityLayer(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity) return 1;\n";
        indent(); output << "this._zenithInitEntityLayerMask(entity);\n";
        indent(); output << "return entity.layer;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEntityMask(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity) return 0xFFFFFFFF;\n";
        indent(); output << "this._zenithInitEntityLayerMask(entity);\n";
        indent(); output << "return entity.mask;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithLayerMaskMatches(maskBits, layerBits) {\n";
        indent_level++;
        indent(); output << "const normalizedLayer = (() => { const bits = Math.trunc(Number(layerBits) || 0); return bits === 0 ? 1 : bits; })();\n";
        indent(); output << "const normalizedMask = Math.trunc(Number(maskBits));\n";
        indent(); output << "return ((Number.isNaN(normalizedMask) ? 0xFFFFFFFF : normalizedMask) & normalizedLayer) !== 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCanEntitiesInteract(first, second) {\n";
        indent_level++;
        indent(); output << "if (!first || !second) return false;\n";
        indent(); output << "return this._zenithLayerMaskMatches(this._zenithEntityMask(first), this._zenithEntityLayer(second)) && this._zenithLayerMaskMatches(this._zenithEntityMask(second), this._zenithEntityLayer(first));\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCaptureNewEntities(beforeIds) {\n";
        indent_level++;
        indent(); output << "const created = [];\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (entity && !beforeIds.has(entity.id)) created.push(entity);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return created;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithHierarchyDepth(entity) {\n";
        indent_level++;
        indent(); output << "let depth = 0;\n";
        indent(); output << "let cursor = entity && entity.parent ? entity.parent : null;\n";
        indent(); output << "while (cursor) {\n";
        indent_level++;
        indent(); output << "depth += 1;\n";
        indent(); output << "cursor = cursor.parent || null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return depth;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCloneEntity(source, entityName = '') {\n";
        indent_level++;
        indent(); output << "if (!source || this._zenithEntities.indexOf(source) < 0) return null;\n";
        indent(); output << "const clone = { ...source, id: this._zenithNextEntityId++, name: entityName || source.name || '', parent: null, children: [] };\n";
        indent(); output << "if (clone.kind === 'camera2d' || clone.kind === 'listener2d' || clone.kind === 'camera3d' || clone.kind === 'listener3d') clone.primary = false;\n";
        indent(); output << "this._zenithInitEntityLayerMask(clone);\n";
        indent(); output << "this._zenithEntities.push(clone);\n";
        indent(); output << "return clone;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithCloneEntityHierarchy(source, entityName = '') {\n";
        indent_level++;
        indent(); output << "const root = this._zenithCloneEntity(source, entityName);\n";
        indent(); output << "if (!root) return null;\n";
        indent(); output << "const children = source && Array.isArray(source.children) ? source.children.slice() : [];\n";
        indent(); output << "for (const child of children) {\n";
        indent_level++;
        indent(); output << "const cloneChild = this._zenithCloneEntityHierarchy(child, '');\n";
        indent(); output << "if (cloneChild) this.setParent(cloneChild, root);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return root;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "registerPrefabCallback(prefabName, callbackName) {\n";
        indent_level++;
        indent(); output << "this._zenithPrefabs.set(prefabName, { callbackName: callbackName === undefined || callbackName === null ? '' : String(callbackName) });\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "hasPrefab(prefabName) {\n";
        indent_level++;
        indent(); output << "return this._zenithPrefabs.has(prefabName);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "instantiatePrefab(prefabName, entityName = '') {\n";
        indent_level++;
        indent(); output << "const prefab = this._zenithPrefabs.get(prefabName);\n";
        indent(); output << "if (!prefab) return null;\n";
        indent(); output << "const entity = this.createEntity(entityName || prefabName);\n";
        indent(); output << "this.triggerEntityCallback(prefab.callbackName || '', entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "instantiateArchetype(source, entityName = '') {\n";
        indent_level++;
        indent(); output << "return this._zenithCloneEntityHierarchy(source, entityName || '');\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "registerSceneStreamCallback(streamName, callbackName) {\n";
        indent_level++;
        indent(); output << "this._zenithSceneStreams.set(streamName, { callbackName: callbackName === undefined || callbackName === null ? '' : String(callbackName) });\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "hasSceneStream(streamName) {\n";
        indent_level++;
        indent(); output << "return this._zenithSceneStreams.has(streamName);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "loadSceneStream(streamName, instanceName = '') {\n";
        indent_level++;
        indent(); output << "const stream = this._zenithSceneStreams.get(streamName);\n";
        indent(); output << "if (!stream) return false;\n";
        indent(); output << "const key = instanceName ? String(instanceName) : String(streamName);\n";
        indent(); output << "if (this._zenithLoadedSceneStreams.has(key)) return false;\n";
        indent(); output << "const beforeIds = new Set(this._zenithEntities.map((entity) => entity.id));\n";
        indent(); output << "this.triggerCallback(stream.callbackName || '', key);\n";
        indent(); output << "this._zenithLoadedSceneStreams.set(key, { source: String(streamName), entities: this._zenithCaptureNewEntities(beforeIds) });\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "unloadSceneStream(instanceName) {\n";
        indent_level++;
        indent(); output << "const stream = this._zenithLoadedSceneStreams.get(instanceName);\n";
        indent(); output << "if (!stream || !Array.isArray(stream.entities)) return false;\n";
        indent(); output << "const entities = stream.entities.slice().sort((left, right) => this._zenithHierarchyDepth(right) - this._zenithHierarchyDepth(left));\n";
        indent(); output << "for (const entity of entities) {\n";
        indent_level++;
        indent(); output << "if (this.isEntityAlive(entity)) this.destroyEntity(entity);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithLoadedSceneStreams.delete(instanceName);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "isSceneStreamLoaded(instanceName) {\n";
        indent_level++;
        indent(); output << "return this._zenithLoadedSceneStreams.has(instanceName);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "sceneStreamEntityCount(instanceName) {\n";
        indent_level++;
        indent(); output << "const stream = this._zenithLoadedSceneStreams.get(instanceName);\n";
        indent(); output << "if (!stream || !Array.isArray(stream.entities)) return 0;\n";
        indent(); output << "let count = 0;\n";
        indent(); output << "for (const entity of stream.entities) {\n";
        indent_level++;
        indent(); output << "if (this.isEntityAlive(entity)) count += 1;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return count;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setEntityLayer(entity, layerBits) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "this._zenithInitEntityLayerMask(entity);\n";
        indent(); output << "const bits = Math.trunc(Number(layerBits) || 0);\n";
        indent(); output << "entity.layer = bits === 0 ? 1 : bits;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityLayer(entity) {\n";
        indent_level++;
        indent(); output << "return this._zenithEntityLayer(entity);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setEntityMask(entity, maskBits) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "this._zenithInitEntityLayerMask(entity);\n";
        indent(); output << "const bits = Math.trunc(Number(maskBits));\n";
        indent(); output << "entity.mask = Number.isNaN(bits) ? 0xFFFFFFFF : bits;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityMask(entity) {\n";
        indent_level++;
        indent(); output << "return this._zenithEntityMask(entity);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setEntityLayerMask(entity, layerBits, maskBits) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "this.setEntityLayer(entity, layerBits);\n";
        indent(); output << "this.setEntityMask(entity, maskBits);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "canEntitiesInteract(first, second) {\n";
        indent_level++;
        indent(); output << "return this._zenithCanEntitiesInteract(first, second);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setEntityName(entity, name) {\n";
        indent_level++;
        indent(); output << "if (entity) entity.name = name;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityName(entity) {\n";
        indent_level++;
        indent(); output << "return entity && entity.name ? entity.name : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setEntityTag(entity, tag) {\n";
        indent_level++;
        indent(); output << "if (entity) entity.tag = tag;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityTag(entity) {\n";
        indent_level++;
        indent(); output << "return entity && entity.tag ? entity.tag : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "findEntityByName(name) {\n";
        indent_level++;
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (entity.name === name) return entity;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "findEntityByTag(tag) {\n";
        indent_level++;
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (entity.tag === tag) return entity;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setParent(child, parent) {\n";
        indent_level++;
        indent(); output << "if (!child || !parent || child === parent) return false;\n";
        indent(); output << "if (child.parent && child.parent.children) {\n";
        indent_level++;
        indent(); output << "const oldIndex = child.parent.children.indexOf(child);\n";
        indent(); output << "if (oldIndex >= 0) child.parent.children.splice(oldIndex, 1);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "child.parent = parent;\n";
        indent(); output << "parent.children = parent.children || [];\n";
        indent(); output << "if (parent.children.indexOf(child) < 0) parent.children.push(child);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "clearParent(child) {\n";
        indent_level++;
        indent(); output << "if (!child || !child.parent) return false;\n";
        indent(); output << "if (child.parent.children) {\n";
        indent_level++;
        indent(); output << "const index = child.parent.children.indexOf(child);\n";
        indent(); output << "if (index >= 0) child.parent.children.splice(index, 1);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "child.parent = null;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "parentOf(child) {\n";
        indent_level++;
        indent(); output << "return child && child.parent ? child.parent : null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "childCount(parent) {\n";
        indent_level++;
        indent(); output << "return parent && parent.children ? parent.children.length : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "childAt(parent, index) {\n";
        indent_level++;
        indent(); output << "if (!parent || !parent.children || index < 0 || index >= parent.children.length) return null;\n";
        indent(); output << "return parent.children[index];\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "loadTexture(texturePath) {\n";
        indent_level++;
        indent(); output << "return this._zenithTextureHandle(texturePath);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "loadAudio(clipPath, spatial) {\n";
        indent_level++;
        indent(); output << "return this._zenithAudioHandle(clipPath, spatial === true);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "loadMesh(meshPath) {\n";
        indent_level++;
        indent(); output << "return this._zenithMeshHandle(meshPath);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "loadShader(shaderPath) {\n";
        indent_level++;
        indent(); output << "return this._zenithShaderHandle(shaderPath);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "loadMaterial(materialPath, shaderPath) {\n";
        indent_level++;
        indent(); output << "return this._zenithMaterialHandle(materialPath, shaderPath);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "importAsset(sourcePath, importedPath, groupName, bundleName) {\n";
        indent_level++;
        indent(); output << "const source = this._zenithNormalizeAssetPath(sourcePath);\n";
        indent(); output << "if (!source) return '';\n";
        indent(); output << "const type = this._zenithGuessAssetType(source);\n";
        indent(); output << "const imported = this._zenithNormalizeAssetPath(importedPath) || ('assets/imported/' + this._zenithAssetFolder(type) + '/' + source.split('/').pop());\n";
        indent(); output << "let metadata = this._zenithFindAssetMetadata(source);\n";
        indent(); output << "if (metadata && metadata.importedPath && metadata.importedPath !== imported) {\n";
        indent_level++;
        indent(); output << "this._zenithAssetMetadata.delete(metadata.importedPath);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "metadata = metadata || {};\n";
        indent(); output << "metadata.sourcePath = source;\n";
        indent(); output << "metadata.importedPath = imported;\n";
        indent(); output << "metadata.type = type;\n";
        indent(); output << "metadata.group = groupName ? String(groupName) : (metadata.group || '');\n";
        indent(); output << "metadata.sourceBytes = metadata.sourceBytes || 0;\n";
        indent(); output << "metadata.estimatedMemoryBytes = this._zenithEstimateAssetBytes(type, metadata.sourceBytes || 0);\n";
        indent(); output << "metadata.sourceTimestamp = metadata.sourceTimestamp || 0;\n";
        indent(); output << "metadata.importedTimestamp = metadata.importedTimestamp || 0;\n";
        indent(); output << "metadata.version = Math.max(1, Number(metadata.version || 1));\n";
        indent(); output << "metadata.hotReloadable = metadata.hotReloadable !== false;\n";
        indent(); output << "metadata.dirty = false;\n";
        indent(); output << "this._zenithAssetMetadata.set(imported, metadata);\n";
        indent(); output << "if (bundleName) this.addAssetToBundle(bundleName, imported);\n";
        indent(); output << "return imported;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "createAssetBundle(bundleName) {\n";
        indent_level++;
        indent(); output << "const name = bundleName === undefined || bundleName === null ? '' : String(bundleName);\n";
        indent(); output << "if (!name) return false;\n";
        indent(); output << "if (!this._zenithAssetBundles.has(name)) this._zenithAssetBundles.set(name, { name, assets: [] });\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "addAssetToBundle(bundleName, assetPath) {\n";
        indent_level++;
        indent(); output << "const name = bundleName === undefined || bundleName === null ? '' : String(bundleName);\n";
        indent(); output << "const resolvedPath = this.importedAssetPath(assetPath) || this._zenithNormalizeAssetPath(assetPath);\n";
        indent(); output << "if (!name || !resolvedPath) return false;\n";
        indent(); output << "this.createAssetBundle(name);\n";
        indent(); output << "this._zenithEnsureAssetMetadata(resolvedPath, this._zenithGuessAssetType(resolvedPath));\n";
        indent(); output << "const bundle = this._zenithAssetBundles.get(name);\n";
        indent(); output << "if (bundle.assets.indexOf(resolvedPath) < 0) bundle.assets.push(resolvedPath);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "assetBundleAssetCount(bundleName) {\n";
        indent_level++;
        indent(); output << "const bundle = this._zenithAssetBundles.get(String(bundleName || ''));\n";
        indent(); output << "return bundle && bundle.assets ? bundle.assets.length : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "assetBundleAsset(bundleName, index) {\n";
        indent_level++;
        indent(); output << "const bundle = this._zenithAssetBundles.get(String(bundleName || ''));\n";
        indent(); output << "if (!bundle || !bundle.assets) return '';\n";
        indent(); output << "const i = Math.trunc(index);\n";
        indent(); output << "return i >= 0 && i < bundle.assets.length ? bundle.assets[i] : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setAssetMemoryBudget(assetType, bytes) {\n";
        indent_level++;
        indent(); output << "const type = assetType === undefined || assetType === null ? '' : String(assetType);\n";
        indent(); output << "if (!type) return false;\n";
        indent(); output << "this._zenithAssetBudgets.set(type, Math.max(0, Math.trunc(bytes)));\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "assetMemoryBudget(assetType) {\n";
        indent_level++;
        indent(); output << "const type = assetType === undefined || assetType === null ? '' : String(assetType);\n";
        indent(); output << "return this._zenithAssetBudgetValue(type);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "assetMemoryUsage(assetType) {\n";
        indent_level++;
        indent(); output << "const type = assetType === undefined || assetType === null ? '' : String(assetType);\n";
        indent(); output << "return this._zenithAssetMemoryUsageByType(type);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setAssetHotReload(enabled) {\n";
        indent_level++;
        indent(); output << "this._zenithAssetHotReload = enabled === true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "assetHotReloadEnabled() {\n";
        indent_level++;
        indent(); output << "return this._zenithAssetHotReload === true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "pollAssetChanges() {\n";
        indent_level++;
        indent(); output << "let dirty = 0;\n";
        indent(); output << "for (const metadata of this._zenithAssetMetadata.values()) {\n";
        indent_level++;
        indent(); output << "if (metadata && metadata.dirty) dirty += 1;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return dirty;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "reloadAsset(assetPath) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithEnsureAssetMetadata(assetPath, this._zenithGuessAssetType(assetPath));\n";
        indent(); output << "if (!metadata) return false;\n";
        indent(); output << "const type = metadata.type || this._zenithGuessAssetType(metadata.importedPath || assetPath);\n";
        indent(); output << "const handleStore = this._zenithAssetHandleStore(type);\n";
        indent(); output << "const targetPath = metadata.importedPath || this._zenithNormalizeAssetPath(assetPath);\n";
        indent(); output << "const handle = handleStore ? this._zenithEnsureHandleRecord(handleStore, targetPath, type) : null;\n";
        indent(); output << "const memoryBytes = this._zenithEstimateAssetBytes(type, metadata.sourceBytes || 0);\n";
        indent(); output << "if (handle && !this._zenithCanLoadAsset(type, handle, memoryBytes)) return false;\n";
        indent(); output << "metadata.type = type;\n";
        indent(); output << "metadata.version = Math.max(1, Number(metadata.version || 1) + 1);\n";
        indent(); output << "metadata.estimatedMemoryBytes = memoryBytes;\n";
        indent(); output << "metadata.dirty = false;\n";
        indent(); output << "if (handle) {\n";
        indent_level++;
        indent(); output << "handle.loaded = true;\n";
        indent(); output << "handle.refCount = 1;\n";
        indent(); output << "handle.memoryBytes = memoryBytes;\n";
        indent(); output << "handle.version = metadata.version;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "reloadDirtyAssets() {\n";
        indent_level++;
        indent(); output << "if (!this._zenithAssetHotReload) return 0;\n";
        indent(); output << "let reloaded = 0;\n";
        indent(); output << "for (const metadata of this._zenithAssetMetadata.values()) {\n";
        indent_level++;
        indent(); output << "if (metadata && metadata.dirty && this.reloadAsset(metadata.importedPath || metadata.sourcePath)) reloaded += 1;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return reloaded;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "markAssetDirty(assetPath) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithEnsureAssetMetadata(assetPath, this._zenithGuessAssetType(assetPath));\n";
        indent(); output << "if (!metadata) return false;\n";
        indent(); output << "metadata.dirty = true;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "assetGroup(assetPath) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithFindAssetMetadata(assetPath);\n";
        indent(); output << "return metadata && metadata.group ? metadata.group : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "importedAssetPath(assetPath) {\n";
        indent_level++;
        indent(); output << "const metadata = this._zenithFindAssetMetadata(assetPath);\n";
        indent(); output << "return metadata && metadata.importedPath ? metadata.importedPath : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "assetDatabaseJson() {\n";
        indent_level++;
        indent(); output << "const assets = Array.from(this._zenithAssetMetadata.values()).slice().sort((a, b) => String(a.importedPath || '').localeCompare(String(b.importedPath || '')));\n";
        indent(); output << "const bundles = Array.from(this._zenithAssetBundles.values()).map(bundle => ({ name: bundle.name, assets: Array.isArray(bundle.assets) ? bundle.assets.slice() : [] })).sort((a, b) => String(a.name).localeCompare(String(b.name)));\n";
        indent(); output << "const budgets = {};\n";
        indent(); output << "for (const [type, value] of this._zenithAssetBudgets.entries()) budgets[type] = Number(value);\n";
        indent(); output << "return JSON.stringify({ hotReloadEnabled: this._zenithAssetHotReload === true, assets, bundles, budgets });\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "bakeAssetMetadata(outputPath) {\n";
        indent_level++;
        indent(); output << "const target = outputPath === undefined || outputPath === null ? '' : String(outputPath);\n";
        indent(); output << "if (!target) return false;\n";
        indent(); output << "this._zenithBakedAssetOutputs.set(target, this.assetDatabaseJson());\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnSprite(name, x, y, w, h, color) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'sprite2d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z: 0,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "w,\n";
        indent(); output << "h,\n";
        indent(); output << "color,\n";
        indent(); output << "texturePath: '',\n";
        indent(); output << "anchorX: 0.5,\n";
        indent(); output << "anchorY: 0.5,\n";
        indent(); output << "sortOrder: 0,\n";
        indent(); output << "visible: true,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: [],\n";
        indent(); output << "hasBody: false,\n";
        indent(); output << "vx: 0,\n";
        indent(); output << "vy: 0,\n";
        indent(); output << "mass: 1,\n";
        indent(); output << "gravityScale: 1,\n";
        indent(); output << "friction: 0.2,\n";
        indent(); output << "restitution: 0\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnTexturedSprite(name, texturePath, x, y, w, h, color) {\n";
        indent_level++;
        indent(); output << "const entity = this.spawnSprite(name, x, y, w, h, color);\n";
        indent(); output << "entity.texturePath = texturePath;\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnTexturedSpriteHandle(name, texture, x, y, w, h, color) {\n";
        indent_level++;
        indent(); output << "const texturePath = texture && texture.path ? String(texture.path) : '';\n";
        indent(); output << "return this.spawnTexturedSprite(name, texturePath, x, y, w, h, color);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnTilemap2D(name, x, y, columns, rows, tileWidth, tileHeight, defaultColor) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'tilemap2d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z: 0,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "columns: Math.max(1, Math.trunc(columns)),\n";
        indent(); output << "rows: Math.max(1, Math.trunc(rows)),\n";
        indent(); output << "tileWidth: tileWidth > 0 ? tileWidth : 1,\n";
        indent(); output << "tileHeight: tileHeight > 0 ? tileHeight : 1,\n";
        indent(); output << "anchorX: 0,\n";
        indent(); output << "anchorY: 0,\n";
        indent(); output << "sortOrder: 0,\n";
        indent(); output << "visible: true,\n";
        indent(); output << "cells: [],\n";
        indent(); output << "palette: ['', defaultColor || 'white'],\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "entity.cells = new Array(entity.columns * entity.rows).fill(0);\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnCharacter2D(name, texturePath, x, y, w, h, color) {\n";
        indent_level++;
        indent(); output << "const entity = this.spawnTexturedSprite(name, texturePath, x, y, w, h, color);\n";
        indent(); output << "entity.kind = 'character2d';\n";
        indent(); output << "this._zenithEnsureCharacter2D(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnCharacter2DHandle(name, texture, x, y, w, h, color) {\n";
        indent_level++;
        indent(); output << "const texturePath = texture && texture.path ? String(texture.path) : '';\n";
        indent(); output << "return this.spawnCharacter2D(name, texturePath, x, y, w, h, color);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnAudioSource2D(name, clipPath, x, y, playOnAwake) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'audio2d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z: 0,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "const target = this._zenithEnsureAudioSource2D(entity);\n";
        indent(); output << "if (target) {\n";
        indent_level++;
        indent(); output << "target.clipPath = clipPath || '';\n";
        indent(); output << "target.playOnAwake = playOnAwake === true;\n";
        indent(); output << "target.isPlaying = playOnAwake === true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnAudioSource2DHandle(name, clip, x, y, playOnAwake) {\n";
        indent_level++;
        indent(); output << "const clipPath = clip && clip.path ? String(clip.path) : '';\n";
        indent(); output << "return this.spawnAudioSource2D(name, clipPath, x, y, playOnAwake);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnAudioListener2D(name, x, y, primary) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'listener2d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z: 0,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "const target = this._zenithEnsureAudioListener2D(entity);\n";
        indent(); output << "if (target) {\n";
        indent_level++;
        indent(); output << "if (primary) {\n";
        indent_level++;
        indent(); output << "for (const existing of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing !== target && existing.kind === 'listener2d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithPrimaryAudioListener = target;\n";
        indent_level--;
        indent(); output << "} else if (this._zenithPrimaryAudioListener === target) {\n";
        indent_level++;
        indent(); output << "this._zenithPrimaryAudioListener = null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "target.primary = primary === true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnCamera2D(name, x, y, zoom, primary) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'camera2d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z: 0,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "zoom,\n";
        indent(); output << "primary,\n";
        indent(); output << "viewportX: 0,\n";
        indent(); output << "viewportY: 0,\n";
        indent(); output << "viewportWidth: 1,\n";
        indent(); output << "viewportHeight: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "if (primary) {\n";
        indent_level++;
        indent(); output << "for (const existing of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing.kind === 'camera2d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithPrimaryCamera = entity;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "destroyEntity(entity) {\n";
        indent_level++;
        indent(); output << "const index = this._zenithEntities.indexOf(entity);\n";
        indent(); output << "if (index < 0) return false;\n";
        indent(); output << "if (entity.parent && entity.parent.children) {\n";
        indent_level++;
        indent(); output << "const parentIndex = entity.parent.children.indexOf(entity);\n";
        indent(); output << "if (parentIndex >= 0) entity.parent.children.splice(parentIndex, 1);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (entity.children) {\n";
        indent_level++;
        indent(); output << "for (const child of entity.children) {\n";
        indent_level++;
        indent(); output << "child.parent = null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "entity.children = [];\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this._zenithPrimaryCamera === entity) this._zenithPrimaryCamera = null;\n";
        indent(); output << "if (this._zenithPrimaryAudioListener === entity) this._zenithPrimaryAudioListener = null;\n";
        indent(); output << "if (this._zenithPrimaryCamera3D === entity) this._zenithPrimaryCamera3D = null;\n";
        indent(); output << "if (this._zenithPrimaryAudioListener3D === entity) this._zenithPrimaryAudioListener3D = null;\n";
        indent(); output << "this._zenithEntities.splice(index, 1);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "destroyEntityHierarchy(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity) return false;\n";
        indent(); output << "const children = Array.isArray(entity.children) ? entity.children.slice() : [];\n";
        indent(); output << "for (const child of children) {\n";
        indent_level++;
        indent(); output << "this.destroyEntityHierarchy(child);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return this.destroyEntity(entity);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "isEntityAlive(entity) {\n";
        indent_level++;
        indent(); output << "return this._zenithEntities.indexOf(entity) >= 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityCount() {\n";
        indent_level++;
        indent(); output << "return this._zenithEntities.length;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setEntityPosition2D(entity, x, y) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "entity.x = x;\n";
        indent(); output << "entity.y = y;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "moveEntity2D(entity, dx, dy) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "entity.x = (entity.x || 0) + dx;\n";
        indent(); output << "entity.y = (entity.y || 0) + dy;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityPositionX(entity) {\n";
        indent_level++;
        indent(); output << "return entity ? (entity.x || 0) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityPositionY(entity) {\n";
        indent_level++;
        indent(); output << "return entity ? (entity.y || 0) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "resizeTilemap2D(entity, columns, rows, fillTileId) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTilemap2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "const nextColumns = Math.max(1, Math.trunc(columns));\n";
        indent(); output << "const nextRows = Math.max(1, Math.trunc(rows));\n";
        indent(); output << "const fillValue = Math.trunc(fillTileId || 0);\n";
        indent(); output << "const previousColumns = target.columns;\n";
        indent(); output << "const previousRows = target.rows;\n";
        indent(); output << "const previousCells = Array.isArray(target.cells) ? target.cells.slice() : [];\n";
        indent(); output << "target.columns = nextColumns;\n";
        indent(); output << "target.rows = nextRows;\n";
        indent(); output << "target.cells = new Array(nextColumns * nextRows).fill(fillValue);\n";
        indent(); output << "const copyColumns = Math.min(previousColumns, nextColumns);\n";
        indent(); output << "const copyRows = Math.min(previousRows, nextRows);\n";
        indent(); output << "for (let row = 0; row < copyRows; row += 1) {\n";
        indent_level++;
        indent(); output << "for (let column = 0; column < copyColumns; column += 1) {\n";
        indent_level++;
        indent(); output << "target.cells[(row * nextColumns) + column] = Number(previousCells[(row * previousColumns) + column]) || 0;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setTilemapCell(entity, column, row, tileId) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTilemap2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "const tileColumn = Math.trunc(column);\n";
        indent(); output << "const tileRow = Math.trunc(row);\n";
        indent(); output << "if (tileColumn < 0 || tileColumn >= target.columns || tileRow < 0 || tileRow >= target.rows) return;\n";
        indent(); output << "const value = Math.trunc(tileId);\n";
        indent(); output << "target.cells[(tileRow * target.columns) + tileColumn] = value;\n";
        indent(); output << "while (value >= 0 && target.palette.length <= value) target.palette.push('');\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "tilemapCell(entity, column, row) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTilemap2D(entity);\n";
        indent(); output << "if (!target) return 0;\n";
        indent(); output << "const tileColumn = Math.trunc(column);\n";
        indent(); output << "const tileRow = Math.trunc(row);\n";
        indent(); output << "if (tileColumn < 0 || tileColumn >= target.columns || tileRow < 0 || tileRow >= target.rows) return 0;\n";
        indent(); output << "return Number(target.cells[(tileRow * target.columns) + tileColumn]) || 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "fillTilemap(entity, tileId) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTilemap2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "const value = Math.trunc(tileId);\n";
        indent(); output << "target.cells.fill(value);\n";
        indent(); output << "while (value >= 0 && target.palette.length <= value) target.palette.push('');\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "clearTilemap(entity) {\n";
        indent_level++;
        indent(); output << "this.fillTilemap(entity, 0);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setTilemapPaletteColor(entity, tileId, color) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTilemap2D(entity);\n";
        indent(); output << "const index = Math.trunc(tileId);\n";
        indent(); output << "if (!target || index < 0) return;\n";
        indent(); output << "while (target.palette.length <= index) target.palette.push('');\n";
        indent(); output << "target.palette[index] = color || '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "tilemapPaletteColor(entity, tileId) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureTilemap2D(entity);\n";
        indent(); output << "const index = Math.trunc(tileId);\n";
        indent(); output << "if (!target || index < 0 || index >= target.palette.length) return '';\n";
        indent(); output << "return target.palette[index] || '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnMesh(name, meshPath, shaderPath, x, y, z) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'mesh3d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "meshPath,\n";
        indent(); output << "shaderPath,\n";
        indent(); output << "materialPath: '',\n";
        indent(); output << "visible: true,\n";
        indent(); output << "castShadows: true,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnMeshHandle(name, mesh, shader, x, y, z) {\n";
        indent_level++;
        indent(); output << "const meshPath = mesh && mesh.path ? String(mesh.path) : '';\n";
        indent(); output << "const shaderPath = shader && shader.path ? String(shader.path) : '';\n";
        indent(); output << "return this.spawnMesh(name, meshPath, shaderPath, x, y, z);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnCharacter3D(name, meshPath, shaderPath, materialPath, x, y, z) {\n";
        indent_level++;
        indent(); output << "const entity = this.spawnMesh(name, meshPath, shaderPath, x, y, z);\n";
        indent(); output << "entity.kind = 'character3d';\n";
        indent(); output << "entity.materialPath = materialPath || '';\n";
        indent(); output << "if (entity.materialPath) this._zenithEnsureMaterial(entity.materialPath);\n";
        indent(); output << "this._zenithEnsureCharacter3D(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnCharacter3DHandle(name, mesh, shader, material, x, y, z) {\n";
        indent_level++;
        indent(); output << "const meshPath = mesh && mesh.path ? String(mesh.path) : '';\n";
        indent(); output << "const shaderPath = shader && shader.path ? String(shader.path) : '';\n";
        indent(); output << "const materialPath = material && material.path ? String(material.path) : '';\n";
        indent(); output << "return this.spawnCharacter3D(name, meshPath, shaderPath, materialPath, x, y, z);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnAudioSource3D(name, clipPath, x, y, z, playOnAwake) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'audio3d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "const target = this._zenithEnsureAudioSource3D(entity);\n";
        indent(); output << "if (target) {\n";
        indent_level++;
        indent(); output << "target.clipPath = clipPath || '';\n";
        indent(); output << "target.playOnAwake = playOnAwake === true;\n";
        indent(); output << "target.isPlaying = playOnAwake === true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnAudioSource3DHandle(name, clip, x, y, z, playOnAwake) {\n";
        indent_level++;
        indent(); output << "const clipPath = clip && clip.path ? String(clip.path) : '';\n";
        indent(); output << "return this.spawnAudioSource3D(name, clipPath, x, y, z, playOnAwake);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnPointLight3D(name, x, y, z, color, intensity, range) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'pointlight3d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "const target = this._zenithEnsurePointLight3D(entity);\n";
        indent(); output << "if (target) {\n";
        indent_level++;
        indent(); output << "target.color = color || 'white';\n";
        indent(); output << "target.intensity = intensity;\n";
        indent(); output << "target.range = range;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnDirectionalLight3D(name, x, y, z, dirX, dirY, dirZ, color, intensity, castShadows) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'directionallight3d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "const target = this._zenithEnsureDirectionalLight3D(entity);\n";
        indent(); output << "if (target) {\n";
        indent_level++;
        indent(); output << "target.directionX = dirX;\n";
        indent(); output << "target.directionY = dirY;\n";
        indent(); output << "target.directionZ = dirZ;\n";
        indent(); output << "target.color = color || 'white';\n";
        indent(); output << "target.intensity = intensity;\n";
        indent(); output << "target.castShadows = castShadows !== false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnAudioListener3D(name, x, y, z, primary) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'listener3d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "const target = this._zenithEnsureAudioListener3D(entity);\n";
        indent(); output << "if (target) {\n";
        indent_level++;
        indent(); output << "if (primary) {\n";
        indent_level++;
        indent(); output << "for (const existing of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing !== target && existing.kind === 'listener3d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithPrimaryAudioListener3D = target;\n";
        indent_level--;
        indent(); output << "} else if (this._zenithPrimaryAudioListener3D === target) {\n";
        indent_level++;
        indent(); output << "this._zenithPrimaryAudioListener3D = null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "target.primary = primary === true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "createMaterial(materialPath, shaderPath) {\n";
        indent_level++;
        indent(); output << "const material = this._zenithEnsureMaterial(materialPath, shaderPath);\n";
        indent(); output << "return material ? material.path : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialExists(materialPath) {\n";
        indent_level++;
        indent(); output << "return !!materialPath && this._zenithMaterials.has(materialPath);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialShaderPath(materialPath, shaderPath) {\n";
        indent_level++;
        indent(); output << "const material = this._zenithEnsureMaterial(materialPath, shaderPath);\n";
        indent(); output << "if (material) material.shaderPath = shaderPath;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialShaderPath(materialPath) {\n";
        indent_level++;
        indent(); output << "const material = this._zenithEnsureMaterial(materialPath);\n";
        indent(); output << "return material ? (material.shaderPath || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "cloneMaterial(sourceMaterialPath, targetMaterialPath) {\n";
        indent_level++;
        indent(); output << "if (!targetMaterialPath) return '';\n";
        indent(); output << "const source = sourceMaterialPath ? this._zenithMaterials.get(sourceMaterialPath) : null;\n";
        indent(); output << "if (!source) return '';\n";
        indent(); output << "const target = this._zenithEnsureMaterial(targetMaterialPath, source.shaderPath || '');\n";
        indent(); output << "if (!target) return '';\n";
        indent(); output << "target.shaderPath = source.shaderPath || '';\n";
        indent(); output << "target.properties = new Map();\n";
        indent(); output << "for (const [propertyName, property] of source.properties.entries()) {\n";
        indent_level++;
        indent(); output << "target.properties.set(propertyName, this._zenithCloneMaterialPropertyState(property, propertyName));\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return target.path || targetMaterialPath;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "copyMaterialProperties(sourceMaterialPath, targetMaterialPath) {\n";
        indent_level++;
        indent(); output << "if (!targetMaterialPath) return 0;\n";
        indent(); output << "const source = sourceMaterialPath ? this._zenithMaterials.get(sourceMaterialPath) : null;\n";
        indent(); output << "const target = this._zenithEnsureMaterial(targetMaterialPath);\n";
        indent(); output << "if (!source || !target) return 0;\n";
        indent(); output << "target.properties = new Map();\n";
        indent(); output << "for (const [propertyName, property] of source.properties.entries()) {\n";
        indent_level++;
        indent(); output << "target.properties.set(propertyName, this._zenithCloneMaterialPropertyState(property, propertyName));\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return target.properties.size;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "removeMaterialProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "if (!materialPath || !propertyName) return false;\n";
        indent(); output << "const material = this._zenithMaterials.get(materialPath);\n";
        indent(); output << "if (!material) return false;\n";
        indent(); output << "return material.properties.delete(propertyName);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "clearMaterialProperties(materialPath) {\n";
        indent_level++;
        indent(); output << "if (!materialPath) return 0;\n";
        indent(); output << "const material = this._zenithMaterials.get(materialPath);\n";
        indent(); output << "if (!material) return 0;\n";
        indent(); output << "const removed = material.properties.size;\n";
        indent(); output << "material.properties.clear();\n";
        indent(); output << "return removed;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "defineMaterialText(materialPath, propertyName, label, defaultValue) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithEnsureMaterialProperty(materialPath, propertyName, 'Text', label);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = defaultValue;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "defineMaterialNumber(materialPath, propertyName, label, defaultValue) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithEnsureMaterialProperty(materialPath, propertyName, 'Number', label);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.numberValue = defaultValue;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "defineMaterialToggle(materialPath, propertyName, label, defaultValue) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithEnsureMaterialProperty(materialPath, propertyName, 'Toggle', label);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.boolValue = defaultValue === true;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "defineMaterialRadio(materialPath, propertyName, label, optionsCsv, defaultValue) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithEnsureMaterialProperty(materialPath, propertyName, 'Radio', label);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.options = this._zenithParseMaterialOptions(optionsCsv);\n";
        indent(); output << "property.stringValue = defaultValue;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "defineMaterialImage(materialPath, propertyName, label, defaultValue) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithEnsureMaterialProperty(materialPath, propertyName, 'Image', label);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = defaultValue;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "defineMaterialButton(materialPath, propertyName, label, actionValue) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithEnsureMaterialProperty(materialPath, propertyName, 'Button', label);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = actionValue;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "defineMaterialColor(materialPath, propertyName, label, defaultValue) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithEnsureMaterialProperty(materialPath, propertyName, 'Color', label);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = defaultValue;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialTextProperty(materialPath, propertyName, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = value;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialTextProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.stringValue || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialNumberProperty(materialPath, propertyName, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.numberValue = value;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialNumberProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.numberValue || 0) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialToggleProperty(materialPath, propertyName, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.boolValue = value === true;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialToggleProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? property.boolValue === true : false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialRadioProperty(materialPath, propertyName, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = value;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialRadioProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.stringValue || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialImageProperty(materialPath, propertyName, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = value;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialImageProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.stringValue || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialButtonProperty(materialPath, propertyName, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = value;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialButtonProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.stringValue || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "triggerMaterialButton(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property || property.kind !== 'Button') return false;\n";
        indent(); output << "property.triggerCount = Math.max(0, Math.trunc(Number(property.triggerCount) || 0)) + 1;\n";
        indent(); output << "if (property.stringValue && typeof this.triggerCallback === 'function') this.triggerCallback(property.stringValue);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialButtonTriggerCount(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? Math.max(0, Math.trunc(Number(property.triggerCount) || 0)) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialPropertyCallback(materialPath, propertyName, callbackName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.callbackName = callbackName === undefined || callbackName === null ? '' : String(callbackName);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyCallback(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.callbackName || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "notifyMaterialProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property || !property.callbackName) return false;\n";
        indent(); output << "property.triggerCount = Math.max(0, Math.trunc(Number(property.triggerCount) || 0)) + 1;\n";
        indent(); output << "if (typeof this.triggerCallback === 'function') this.triggerCallback(property.callbackName, this._zenithMaterialPropertyCallbackValue(property));\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMaterialColorProperty(materialPath, propertyName, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "property.stringValue = value;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialColorProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.stringValue || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyCount(materialPath) {\n";
        indent_level++;
        indent(); output << "const material = this._zenithEnsureMaterial(materialPath);\n";
        indent(); output << "return material ? material.properties.size : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialHasProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "return this._zenithMaterialProperty(materialPath, propertyName) !== null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyOptionCount(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property && Array.isArray(property.options) ? property.options.length : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "addMaterialPropertyOption(materialPath, propertyName, label, value) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property) return false;\n";
        indent(); output << "const optionValue = value === undefined || value === null || String(value).length === 0 ? (label === undefined || label === null ? '' : String(label)) : String(value);\n";
        indent(); output << "if (!optionValue) return false;\n";
        indent(); output << "const optionLabel = label === undefined || label === null || String(label).length === 0 ? optionValue : String(label);\n";
        indent(); output << "if (!Array.isArray(property.options)) property.options = [];\n";
        indent(); output << "property.options.push({ label: optionLabel, value: optionValue });\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "removeMaterialPropertyOption(materialPath, propertyName, index) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property || !Array.isArray(property.options) || index < 0 || index >= property.options.length) return false;\n";
        indent(); output << "property.options.splice(index, 1);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "clearMaterialPropertyOptions(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property || !Array.isArray(property.options)) return 0;\n";
        indent(); output << "const removed = property.options.length;\n";
        indent(); output << "property.options = [];\n";
        indent(); output << "return removed;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyNameAt(materialPath, index) {\n";
        indent_level++;
        indent(); output << "const material = this._zenithEnsureMaterial(materialPath);\n";
        indent(); output << "if (!material || index < 0 || index >= material.properties.size) return '';\n";
        indent(); output << "return Array.from(material.properties.keys())[index] || '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyKind(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.kind || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyLabel(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "return property ? (property.label || '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyOptions(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const property = this._zenithMaterialProperty(materialPath, propertyName);\n";
        indent(); output << "if (!property || !Array.isArray(property.options) || property.options.length === 0) return '';\n";
        indent(); output << "return property.options.map((option) => option && option.value !== undefined ? option.value : (option && option.label !== undefined ? option.label : '')).join(',');\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialProperty(materialPath, propertyName) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "let currentName = propertyName || '';\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get exists() { return self._zenithMaterialProperty(materialPath, currentName) !== null; },\n";
        indent(); output << "set exists(value) {},\n";
        indent(); output << "get name() { return currentName; },\n";
        indent(); output << "set name(value) {\n";
        indent_level++;
        indent(); output << "const nextName = value === undefined || value === null ? '' : String(value);\n";
        indent(); output << "if (!nextName || nextName === currentName) return;\n";
        indent(); output << "const material = materialPath ? self._zenithMaterials.get(materialPath) : null;\n";
        indent(); output << "if (!material || !material.properties.has(currentName) || material.properties.has(nextName)) return;\n";
        indent(); output << "const property = material.properties.get(currentName);\n";
        indent(); output << "material.properties.delete(currentName);\n";
        indent(); output << "property.name = nextName;\n";
        indent(); output << "material.properties.set(nextName, property);\n";
        indent(); output << "currentName = nextName;\n";
        indent_level--;
        indent(); output << "},\n";
        indent(); output << "get label() { const property = self._zenithMaterialProperty(materialPath, currentName); return property ? (property.label || '') : ''; },\n";
        indent(); output << "set label(value) { const property = self._zenithMaterialProperty(materialPath, currentName); if (property) property.label = value === undefined || value === null ? '' : String(value); },\n";
        indent(); output << "get kind() { const property = self._zenithMaterialProperty(materialPath, currentName); return property ? (property.kind || '') : ''; },\n";
        indent(); output << "set kind(value) { const property = self._zenithMaterialProperty(materialPath, currentName); const normalized = self._zenithNormalizeMaterialPropertyKind(value); if (property && normalized) { property.kind = normalized; if (normalized !== 'Radio') property.options = []; } },\n";
        indent(); output << "get options() { const property = self._zenithMaterialProperty(materialPath, currentName); return (!property || !Array.isArray(property.options) || property.options.length === 0) ? '' : property.options.map((option) => option && option.value !== undefined ? option.value : (option && option.label !== undefined ? option.label : '')).join(','); },\n";
        indent(); output << "set options(value) { const property = self._zenithMaterialProperty(materialPath, currentName); if (property) property.options = self._zenithParseMaterialOptions(value); },\n";
        indent(); output << "get callback() { const property = self._zenithMaterialProperty(materialPath, currentName); return property ? (property.callbackName || '') : ''; },\n";
        indent(); output << "set callback(value) { const property = self._zenithMaterialProperty(materialPath, currentName); if (property) property.callbackName = value === undefined || value === null ? '' : String(value); },\n";
        indent(); output << "get stringValue() { const property = self._zenithMaterialProperty(materialPath, currentName); return property ? (property.stringValue || '') : ''; },\n";
        indent(); output << "set stringValue(value) { const property = self._zenithMaterialProperty(materialPath, currentName); if (property) property.stringValue = value === undefined || value === null ? '' : String(value); },\n";
        indent(); output << "get numberValue() { const property = self._zenithMaterialProperty(materialPath, currentName); return property ? (property.numberValue || 0) : 0; },\n";
        indent(); output << "set numberValue(value) { const property = self._zenithMaterialProperty(materialPath, currentName); if (property) property.numberValue = Number(value) || 0; },\n";
        indent(); output << "get boolValue() { const property = self._zenithMaterialProperty(materialPath, currentName); return property ? property.boolValue === true : false; },\n";
        indent(); output << "set boolValue(value) { const property = self._zenithMaterialProperty(materialPath, currentName); if (property) property.boolValue = value === true; },\n";
        indent(); output << "get triggerCount() { const property = self._zenithMaterialProperty(materialPath, currentName); return property ? Math.max(0, Math.trunc(Number(property.triggerCount) || 0)) : 0; },\n";
        indent(); output << "set triggerCount(value) { const property = self._zenithMaterialProperty(materialPath, currentName); if (property) property.triggerCount = Math.max(0, Math.trunc(Number(value) || 0)); }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyAt(materialPath, index) {\n";
        indent_level++;
        indent(); output << "return this.materialProperty(materialPath, this.materialPropertyNameAt(materialPath, index));\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "materialPropertyOption(materialPath, propertyName, index) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "const optionIndex = index;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get exists() { return self._zenithMaterialPropertyOption(materialPath, propertyName, optionIndex) !== null; },\n";
        indent(); output << "set exists(value) {},\n";
        indent(); output << "get label() { const option = self._zenithMaterialPropertyOption(materialPath, propertyName, optionIndex); return option ? (option.label !== undefined && option.label !== null && option.label !== '' ? option.label : (option.value || '')) : ''; },\n";
        indent(); output << "set label(value) { const option = self._zenithMaterialPropertyOption(materialPath, propertyName, optionIndex); if (option) option.label = value === undefined || value === null ? '' : String(value); },\n";
        indent(); output << "get value() { const option = self._zenithMaterialPropertyOption(materialPath, propertyName, optionIndex); return option ? (option.value || '') : ''; },\n";
        indent(); output << "set value(value) { const option = self._zenithMaterialPropertyOption(materialPath, propertyName, optionIndex); if (option) option.value = value === undefined || value === null ? '' : String(value); }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setMeshMaterial(entity, materialPath) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureMesh3D(entity);\n";
        indent(); output << "if (!target) return false;\n";
        indent(); output << "target.materialPath = materialPath || '';\n";
        indent(); output << "if (materialPath) this._zenithEnsureMaterial(materialPath);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "meshMaterialPath(entity) {\n";
        indent_level++;
        indent(); output << "return entity ? (entity.materialPath !== undefined ? entity.materialPath : '') : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "meshMaterialHandle(entity) {\n";
        indent_level++;
        indent(); output << "const materialPath = entity && entity.materialPath ? entity.materialPath : '';\n";
        indent(); output << "const shaderPath = entity && entity.shaderPath ? entity.shaderPath : '';\n";
        indent(); output << "return this._zenithMaterialHandle(materialPath, shaderPath);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "playAudio(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity) return false;\n";
        indent(); output << "if (entity.hasAudio2D === true) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureAudioSource2D(entity);\n";
        indent(); output << "if (!target) return false;\n";
        indent(); output << "target.isPlaying = true;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (entity.hasAudio3D === true) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureAudioSource3D(entity);\n";
        indent(); output << "if (!target) return false;\n";
        indent(); output << "target.isPlaying = true;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "stopAudio(entity) {\n";
        indent_level++;
        indent(); output << "if (!entity) return false;\n";
        indent(); output << "if (entity.hasAudio2D === true) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureAudioSource2D(entity);\n";
        indent(); output << "if (!target) return false;\n";
        indent(); output << "target.isPlaying = false;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (entity.hasAudio3D === true) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureAudioSource3D(entity);\n";
        indent(); output << "if (!target) return false;\n";
        indent(); output << "target.isPlaying = false;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spawnCamera3D(name, x, y, z, fov, primary) {\n";
        indent_level++;
        indent(); output << "const entity = {\n";
        indent_level++;
        indent(); output << "id: this._zenithNextEntityId++,\n";
        indent(); output << "name,\n";
        indent(); output << "tag: '',\n";
        indent(); output << "kind: 'camera3d',\n";
        indent(); output << "x,\n";
        indent(); output << "y,\n";
        indent(); output << "z,\n";
        indent(); output << "rotation: 0,\n";
        indent(); output << "rotationX: 0,\n";
        indent(); output << "rotationY: 0,\n";
        indent(); output << "rotationZ: 0,\n";
        indent(); output << "scaleX: 1,\n";
        indent(); output << "scaleY: 1,\n";
        indent(); output << "scaleZ: 1,\n";
        indent(); output << "fov,\n";
        indent(); output << "nearClip: 0.1,\n";
        indent(); output << "farClip: 1000,\n";
        indent(); output << "primary,\n";
        indent(); output << "viewportX: 0,\n";
        indent(); output << "viewportY: 0,\n";
        indent(); output << "viewportWidth: 1,\n";
        indent(); output << "viewportHeight: 1,\n";
        indent(); output << "parent: null,\n";
        indent(); output << "children: []\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "if (primary) {\n";
        indent_level++;
        indent(); output << "for (const existing of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing.kind === 'camera3d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithPrimaryCamera3D = entity;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithEntities.push(entity);\n";
        indent(); output << "return entity;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setEntityPosition3D(entity, x, y, z) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "entity.x = x;\n";
        indent(); output << "entity.y = y;\n";
        indent(); output << "entity.z = z;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "moveEntity3D(entity, dx, dy, dz) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "entity.x = (entity.x || 0) + dx;\n";
        indent(); output << "entity.y = (entity.y || 0) + dy;\n";
        indent(); output << "entity.z = (entity.z || 0) + dz;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "entityPositionZ(entity) {\n";
        indent_level++;
        indent(); output << "return entity ? (entity.z || 0) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "transform2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureTransform2D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureTransform2D(entity); if (target) target.y = value; },\n";
        indent(); output << "get rotation() { return entity ? (entity.rotation !== undefined ? entity.rotation : 0) : 0; },\n";
        indent(); output << "set rotation(value) { const target = self._zenithEnsureTransform2D(entity); if (target) target.rotation = value; },\n";
        indent(); output << "get scaleX() { return entity ? (entity.scaleX !== undefined ? entity.scaleX : 1) : 1; },\n";
        indent(); output << "set scaleX(value) { const target = self._zenithEnsureTransform2D(entity); if (target) target.scaleX = value; },\n";
        indent(); output << "get scaleY() { return entity ? (entity.scaleY !== undefined ? entity.scaleY : 1) : 1; },\n";
        indent(); output << "set scaleY(value) { const target = self._zenithEnsureTransform2D(entity); if (target) target.scaleY = value; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "body2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.y = value; },\n";
        indent(); output << "get vx() { return entity ? (entity.vx !== undefined ? entity.vx : 0) : 0; },\n";
        indent(); output << "set vx(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.vx = value; },\n";
        indent(); output << "get vy() { return entity ? (entity.vy !== undefined ? entity.vy : 0) : 0; },\n";
        indent(); output << "set vy(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.vy = value; },\n";
        indent(); output << "get mass() { return entity ? (entity.mass !== undefined ? entity.mass : 1) : 1; },\n";
        indent(); output << "set mass(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.mass = value > 0 ? value : 1; },\n";
        indent(); output << "get gravityScale() { return entity ? (entity.gravityScale !== undefined ? entity.gravityScale : 1) : 1; },\n";
        indent(); output << "set gravityScale(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.gravityScale = value; },\n";
        indent(); output << "get friction() { return entity ? (entity.friction !== undefined ? entity.friction : 0.2) : 0.2; },\n";
        indent(); output << "set friction(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.friction = value; },\n";
        indent(); output << "get restitution() { return entity ? (entity.restitution !== undefined ? entity.restitution : 0) : 0; },\n";
        indent(); output << "set restitution(value) { const target = self._zenithEnsureBody2D(entity); if (target) target.restitution = value; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "boxCollider2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get offsetX() { return entity ? (entity.boxColliderOffsetX !== undefined ? entity.boxColliderOffsetX : 0) : 0; },\n";
        indent(); output << "set offsetX(value) { const target = self._zenithEnsureBoxCollider2D(entity); if (target) target.boxColliderOffsetX = value; },\n";
        indent(); output << "get offsetY() { return entity ? (entity.boxColliderOffsetY !== undefined ? entity.boxColliderOffsetY : 0) : 0; },\n";
        indent(); output << "set offsetY(value) { const target = self._zenithEnsureBoxCollider2D(entity); if (target) target.boxColliderOffsetY = value; },\n";
        indent(); output << "get width() { return entity ? (entity.boxColliderWidth !== undefined ? entity.boxColliderWidth : 1) : 1; },\n";
        indent(); output << "set width(value) { const target = self._zenithEnsureBoxCollider2D(entity); if (target) target.boxColliderWidth = value > 0 ? value : 0.01; },\n";
        indent(); output << "get height() { return entity ? (entity.boxColliderHeight !== undefined ? entity.boxColliderHeight : 1) : 1; },\n";
        indent(); output << "set height(value) { const target = self._zenithEnsureBoxCollider2D(entity); if (target) target.boxColliderHeight = value > 0 ? value : 0.01; },\n";
        indent(); output << "get isTrigger() { return entity ? entity.boxColliderIsTrigger === true : false; },\n";
        indent(); output << "set isTrigger(value) { const target = self._zenithEnsureBoxCollider2D(entity); if (target) target.boxColliderIsTrigger = value === true; },\n";
        indent(); output << "get enabled() { return entity ? entity.boxColliderEnabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureBoxCollider2D(entity); if (target) target.boxColliderEnabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "circleCollider2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get offsetX() { return entity ? (entity.circleColliderOffsetX !== undefined ? entity.circleColliderOffsetX : 0) : 0; },\n";
        indent(); output << "set offsetX(value) { const target = self._zenithEnsureCircleCollider2D(entity); if (target) target.circleColliderOffsetX = value; },\n";
        indent(); output << "get offsetY() { return entity ? (entity.circleColliderOffsetY !== undefined ? entity.circleColliderOffsetY : 0) : 0; },\n";
        indent(); output << "set offsetY(value) { const target = self._zenithEnsureCircleCollider2D(entity); if (target) target.circleColliderOffsetY = value; },\n";
        indent(); output << "get radius() { return entity ? (entity.circleColliderRadius !== undefined ? entity.circleColliderRadius : 0.5) : 0.5; },\n";
        indent(); output << "set radius(value) { const target = self._zenithEnsureCircleCollider2D(entity); if (target) target.circleColliderRadius = value > 0 ? value : 0.01; },\n";
        indent(); output << "get isTrigger() { return entity ? entity.circleColliderIsTrigger === true : false; },\n";
        indent(); output << "set isTrigger(value) { const target = self._zenithEnsureCircleCollider2D(entity); if (target) target.circleColliderIsTrigger = value === true; },\n";
        indent(); output << "get enabled() { return entity ? entity.circleColliderEnabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureCircleCollider2D(entity); if (target) target.circleColliderEnabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "capsuleCollider2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get offsetX() { return entity ? (entity.capsuleColliderOffsetX !== undefined ? entity.capsuleColliderOffsetX : 0) : 0; },\n";
        indent(); output << "set offsetX(value) { const target = self._zenithEnsureCapsuleCollider2D(entity); if (target) target.capsuleColliderOffsetX = value; },\n";
        indent(); output << "get offsetY() { return entity ? (entity.capsuleColliderOffsetY !== undefined ? entity.capsuleColliderOffsetY : 0) : 0; },\n";
        indent(); output << "set offsetY(value) { const target = self._zenithEnsureCapsuleCollider2D(entity); if (target) target.capsuleColliderOffsetY = value; },\n";
        indent(); output << "get height() { return entity ? (entity.capsuleColliderHeight !== undefined ? entity.capsuleColliderHeight : 2) : 2; },\n";
        indent(); output << "set height(value) { const target = self._zenithEnsureCapsuleCollider2D(entity); if (target) target.capsuleColliderHeight = Math.max(value, (target.capsuleColliderRadius !== undefined ? target.capsuleColliderRadius : 0.5) * 2); },\n";
        indent(); output << "get radius() { return entity ? (entity.capsuleColliderRadius !== undefined ? entity.capsuleColliderRadius : 0.5) : 0.5; },\n";
        indent(); output << "set radius(value) { const target = self._zenithEnsureCapsuleCollider2D(entity); if (target) { target.capsuleColliderRadius = value > 0 ? value : 0.01; target.capsuleColliderHeight = Math.max(target.capsuleColliderHeight !== undefined ? target.capsuleColliderHeight : 2, target.capsuleColliderRadius * 2); } },\n";
        indent(); output << "get isTrigger() { return entity ? entity.capsuleColliderIsTrigger === true : false; },\n";
        indent(); output << "set isTrigger(value) { const target = self._zenithEnsureCapsuleCollider2D(entity); if (target) target.capsuleColliderIsTrigger = value === true; },\n";
        indent(); output << "get enabled() { return entity ? entity.capsuleColliderEnabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureCapsuleCollider2D(entity); if (target) target.capsuleColliderEnabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "camera2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureCamera2D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureCamera2D(entity); if (target) target.y = value; },\n";
        indent(); output << "get zoom() { return entity ? (entity.zoom !== undefined ? entity.zoom : 1) : 1; },\n";
        indent(); output << "set zoom(value) { const target = self._zenithEnsureCamera2D(entity); if (target) target.zoom = value; },\n";
        indent(); output << "get primary() { return entity ? entity.primary === true : false; },\n";
        indent(); output << "set primary(value) {\n";
        indent_level++;
        indent(); output << "const target = self._zenithEnsureCamera2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "if (value) {\n";
        indent_level++;
        indent(); output << "for (const existing of self._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing !== target && existing.kind === 'camera2d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "self._zenithPrimaryCamera = target;\n";
        indent_level--;
        indent(); output << "} else if (self._zenithPrimaryCamera === target) {\n";
        indent_level++;
        indent(); output << "self._zenithPrimaryCamera = null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "target.primary = value === true;\n";
        indent_level--;
        indent(); output << "},\n";
        indent(); output << "get viewportX() { return entity ? (entity.viewportX !== undefined ? entity.viewportX : 0) : 0; },\n";
        indent(); output << "set viewportX(value) { const target = self._zenithEnsureCamera2D(entity); if (target) target.viewportX = value; },\n";
        indent(); output << "get viewportY() { return entity ? (entity.viewportY !== undefined ? entity.viewportY : 0) : 0; },\n";
        indent(); output << "set viewportY(value) { const target = self._zenithEnsureCamera2D(entity); if (target) target.viewportY = value; },\n";
        indent(); output << "get viewportWidth() { return entity ? (entity.viewportWidth !== undefined ? entity.viewportWidth : 1) : 1; },\n";
        indent(); output << "set viewportWidth(value) { const target = self._zenithEnsureCamera2D(entity); if (target) target.viewportWidth = value; },\n";
        indent(); output << "get viewportHeight() { return entity ? (entity.viewportHeight !== undefined ? entity.viewportHeight : 1) : 1; },\n";
        indent(); output << "set viewportHeight(value) { const target = self._zenithEnsureCamera2D(entity); if (target) target.viewportHeight = value; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "audioListener2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureAudioListener2D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureAudioListener2D(entity); if (target) target.y = value; },\n";
        indent(); output << "get gain() { return entity ? (entity.gain !== undefined ? entity.gain : 1) : 1; },\n";
        indent(); output << "set gain(value) { const target = self._zenithEnsureAudioListener2D(entity); if (target) target.gain = value; },\n";
        indent(); output << "get primary() { return entity ? entity.primary === true : false; },\n";
        indent(); output << "set primary(value) {\n";
        indent_level++;
        indent(); output << "const target = self._zenithEnsureAudioListener2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "if (value) {\n";
        indent_level++;
        indent(); output << "for (const existing of self._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing !== target && existing.kind === 'listener2d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "self._zenithPrimaryAudioListener = target;\n";
        indent_level--;
        indent(); output << "} else if (self._zenithPrimaryAudioListener === target) {\n";
        indent_level++;
        indent(); output << "self._zenithPrimaryAudioListener = null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "target.primary = value === true;\n";
        indent_level--;
        indent(); output << "},\n";
        indent(); output << "get enabled() { return entity ? entity.enabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureAudioListener2D(entity); if (target) target.enabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "sprite2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get width() { return entity ? (entity.w !== undefined ? entity.w : 1) : 1; },\n";
        indent(); output << "set width(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.w = value; },\n";
        indent(); output << "get height() { return entity ? (entity.h !== undefined ? entity.h : 1) : 1; },\n";
        indent(); output << "set height(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.h = value; },\n";
        indent(); output << "get anchorX() { return entity ? (entity.anchorX !== undefined ? entity.anchorX : 0.5) : 0.5; },\n";
        indent(); output << "set anchorX(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.anchorX = value; },\n";
        indent(); output << "get anchorY() { return entity ? (entity.anchorY !== undefined ? entity.anchorY : 0.5) : 0.5; },\n";
        indent(); output << "set anchorY(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.anchorY = value; },\n";
        indent(); output << "get color() { return entity ? (entity.color !== undefined ? entity.color : 'white') : 'white'; },\n";
        indent(); output << "set color(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.color = value; },\n";
        indent(); output << "get texturePath() { return entity ? (entity.texturePath !== undefined ? entity.texturePath : '') : ''; },\n";
        indent(); output << "set texturePath(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.texturePath = value; },\n";
        indent(); output << "get texture() { return self._zenithTextureHandle(entity ? entity.texturePath : ''); },\n";
        indent(); output << "set texture(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.texturePath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get sortOrder() { return entity ? (entity.sortOrder !== undefined ? entity.sortOrder : 0) : 0; },\n";
        indent(); output << "set sortOrder(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.sortOrder = value; },\n";
        indent(); output << "get visible() { return entity ? entity.visible !== false : true; },\n";
        indent(); output << "set visible(value) { const target = self._zenithEnsureSprite2D(entity); if (target) target.visible = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "tilemap2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get columns() { return entity ? (entity.columns !== undefined ? entity.columns : 1) : 1; },\n";
        indent(); output << "set columns(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) self.resizeTilemap2D(target, value, target.rows !== undefined ? target.rows : 1, 0); },\n";
        indent(); output << "get rows() { return entity ? (entity.rows !== undefined ? entity.rows : 1) : 1; },\n";
        indent(); output << "set rows(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) self.resizeTilemap2D(target, target.columns !== undefined ? target.columns : 1, value, 0); },\n";
        indent(); output << "get tileWidth() { return entity ? (entity.tileWidth !== undefined ? entity.tileWidth : 1) : 1; },\n";
        indent(); output << "set tileWidth(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) target.tileWidth = value > 0 ? value : 1; },\n";
        indent(); output << "get tileHeight() { return entity ? (entity.tileHeight !== undefined ? entity.tileHeight : 1) : 1; },\n";
        indent(); output << "set tileHeight(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) target.tileHeight = value > 0 ? value : 1; },\n";
        indent(); output << "get anchorX() { return entity ? (entity.anchorX !== undefined ? entity.anchorX : 0) : 0; },\n";
        indent(); output << "set anchorX(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) target.anchorX = value; },\n";
        indent(); output << "get anchorY() { return entity ? (entity.anchorY !== undefined ? entity.anchorY : 0) : 0; },\n";
        indent(); output << "set anchorY(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) target.anchorY = value; },\n";
        indent(); output << "get sortOrder() { return entity ? (entity.sortOrder !== undefined ? entity.sortOrder : 0) : 0; },\n";
        indent(); output << "set sortOrder(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) target.sortOrder = value; },\n";
        indent(); output << "get visible() { return entity ? entity.visible !== false : true; },\n";
        indent(); output << "set visible(value) { const target = self._zenithEnsureTilemap2D(entity); if (target) target.visible = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "character2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.y = value; },\n";
        indent(); output << "get vx() { return entity ? (entity.vx !== undefined ? entity.vx : 0) : 0; },\n";
        indent(); output << "set vx(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.vx = value; },\n";
        indent(); output << "get vy() { return entity ? (entity.vy !== undefined ? entity.vy : 0) : 0; },\n";
        indent(); output << "set vy(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.vy = value; },\n";
        indent(); output << "get moveSpeed() { return entity ? (entity.moveSpeed !== undefined ? entity.moveSpeed : 10) : 10; },\n";
        indent(); output << "set moveSpeed(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.moveSpeed = value; },\n";
        indent(); output << "get jumpForce() { return entity ? (entity.jumpForce !== undefined ? entity.jumpForce : 12) : 12; },\n";
        indent(); output << "set jumpForce(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.jumpForce = value; },\n";
        indent(); output << "get isGrounded() { return entity ? entity.isGrounded === true : false; },\n";
        indent(); output << "set isGrounded(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.isGrounded = value === true; },\n";
        indent(); output << "get facingRight() { return entity ? entity.facingRight !== false : true; },\n";
        indent(); output << "set facingRight(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.facingRight = value !== false; },\n";
        indent(); output << "get texturePath() { return entity ? (entity.texturePath !== undefined ? entity.texturePath : '') : ''; },\n";
        indent(); output << "set texturePath(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.texturePath = value || ''; },\n";
        indent(); output << "get texture() { return self._zenithTextureHandle(entity ? entity.texturePath : ''); },\n";
        indent(); output << "set texture(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.texturePath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get color() { return entity ? (entity.color !== undefined ? entity.color : 'white') : 'white'; },\n";
        indent(); output << "set color(value) { const target = self._zenithEnsureCharacter2D(entity); if (target) target.color = value; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "audioSource2D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.y = value; },\n";
        indent(); output << "get clipPath() { return entity ? (entity.clipPath !== undefined ? entity.clipPath : '') : ''; },\n";
        indent(); output << "set clipPath(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.clipPath = value || ''; },\n";
        indent(); output << "get clip() { return self._zenithAudioHandle(entity ? entity.clipPath : '', false); },\n";
        indent(); output << "set clip(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.clipPath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get volume() { return entity ? (entity.volume !== undefined ? entity.volume : 1) : 1; },\n";
        indent(); output << "set volume(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.volume = value; },\n";
        indent(); output << "get pitch() { return entity ? (entity.pitch !== undefined ? entity.pitch : 1) : 1; },\n";
        indent(); output << "set pitch(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.pitch = value; },\n";
        indent(); output << "get loop() { return entity ? entity.loop === true : false; },\n";
        indent(); output << "set loop(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.loop = value === true; },\n";
        indent(); output << "get playOnAwake() { return entity ? entity.playOnAwake === true : false; },\n";
        indent(); output << "set playOnAwake(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.playOnAwake = value === true; },\n";
        indent(); output << "get isPlaying() { return entity ? entity.isPlaying === true : false; },\n";
        indent(); output << "set isPlaying(value) { const target = self._zenithEnsureAudioSource2D(entity); if (target) target.isPlaying = value === true; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "transform3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get rotationX() { return entity ? (entity.rotationX !== undefined ? entity.rotationX : 0) : 0; },\n";
        indent(); output << "set rotationX(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.rotationX = value; },\n";
        indent(); output << "get rotationY() { return entity ? (entity.rotationY !== undefined ? entity.rotationY : 0) : 0; },\n";
        indent(); output << "set rotationY(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.rotationY = value; },\n";
        indent(); output << "get rotationZ() { return entity ? (entity.rotationZ !== undefined ? entity.rotationZ : 0) : 0; },\n";
        indent(); output << "set rotationZ(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.rotationZ = value; },\n";
        indent(); output << "get scaleX() { return entity ? (entity.scaleX !== undefined ? entity.scaleX : 1) : 1; },\n";
        indent(); output << "set scaleX(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.scaleX = value; },\n";
        indent(); output << "get scaleY() { return entity ? (entity.scaleY !== undefined ? entity.scaleY : 1) : 1; },\n";
        indent(); output << "set scaleY(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.scaleY = value; },\n";
        indent(); output << "get scaleZ() { return entity ? (entity.scaleZ !== undefined ? entity.scaleZ : 1) : 1; },\n";
        indent(); output << "set scaleZ(value) { const target = self._zenithEnsureTransform3D(entity); if (target) target.scaleZ = value; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "body3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get vx() { return entity ? (entity.vx !== undefined ? entity.vx : 0) : 0; },\n";
        indent(); output << "set vx(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.vx = value; },\n";
        indent(); output << "get vy() { return entity ? (entity.vy !== undefined ? entity.vy : 0) : 0; },\n";
        indent(); output << "set vy(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.vy = value; },\n";
        indent(); output << "get vz() { return entity ? (entity.vz !== undefined ? entity.vz : 0) : 0; },\n";
        indent(); output << "set vz(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.vz = value; },\n";
        indent(); output << "get mass() { return entity ? (entity.mass !== undefined ? entity.mass : 1) : 1; },\n";
        indent(); output << "set mass(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.mass = value > 0 ? value : 1; },\n";
        indent(); output << "get gravityScale() { return entity ? (entity.gravityScale !== undefined ? entity.gravityScale : 1) : 1; },\n";
        indent(); output << "set gravityScale(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.gravityScale = value; },\n";
        indent(); output << "get friction() { return entity ? (entity.friction !== undefined ? entity.friction : 0.2) : 0.2; },\n";
        indent(); output << "set friction(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.friction = value; },\n";
        indent(); output << "get restitution() { return entity ? (entity.restitution !== undefined ? entity.restitution : 0) : 0; },\n";
        indent(); output << "set restitution(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.restitution = value; },\n";
        indent(); output << "get useGravity() { return entity ? entity.useGravity !== false : true; },\n";
        indent(); output << "set useGravity(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.useGravity = value !== false; },\n";
        indent(); output << "get isGrounded() { return entity ? entity.isGrounded === true : false; },\n";
        indent(); output << "set isGrounded(value) { const target = self._zenithEnsureBody3D(entity); if (target) target.isGrounded = value === true; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "boxCollider3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get offsetX() { return entity ? (entity.boxCollider3DOffsetX !== undefined ? entity.boxCollider3DOffsetX : 0) : 0; },\n";
        indent(); output << "set offsetX(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DOffsetX = value; },\n";
        indent(); output << "get offsetY() { return entity ? (entity.boxCollider3DOffsetY !== undefined ? entity.boxCollider3DOffsetY : 0) : 0; },\n";
        indent(); output << "set offsetY(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DOffsetY = value; },\n";
        indent(); output << "get offsetZ() { return entity ? (entity.boxCollider3DOffsetZ !== undefined ? entity.boxCollider3DOffsetZ : 0) : 0; },\n";
        indent(); output << "set offsetZ(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DOffsetZ = value; },\n";
        indent(); output << "get width() { return entity ? (entity.boxCollider3DWidth !== undefined ? entity.boxCollider3DWidth : 1) : 1; },\n";
        indent(); output << "set width(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DWidth = value > 0 ? value : 0.01; },\n";
        indent(); output << "get height() { return entity ? (entity.boxCollider3DHeight !== undefined ? entity.boxCollider3DHeight : 1) : 1; },\n";
        indent(); output << "set height(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DHeight = value > 0 ? value : 0.01; },\n";
        indent(); output << "get depth() { return entity ? (entity.boxCollider3DDepth !== undefined ? entity.boxCollider3DDepth : 1) : 1; },\n";
        indent(); output << "set depth(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DDepth = value > 0 ? value : 0.01; },\n";
        indent(); output << "get isTrigger() { return entity ? entity.boxCollider3DIsTrigger === true : false; },\n";
        indent(); output << "set isTrigger(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DIsTrigger = value === true; },\n";
        indent(); output << "get enabled() { return entity ? entity.boxCollider3DEnabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureBoxCollider3D(entity); if (target) target.boxCollider3DEnabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "sphereCollider3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get offsetX() { return entity ? (entity.sphereCollider3DOffsetX !== undefined ? entity.sphereCollider3DOffsetX : 0) : 0; },\n";
        indent(); output << "set offsetX(value) { const target = self._zenithEnsureSphereCollider3D(entity); if (target) target.sphereCollider3DOffsetX = value; },\n";
        indent(); output << "get offsetY() { return entity ? (entity.sphereCollider3DOffsetY !== undefined ? entity.sphereCollider3DOffsetY : 0) : 0; },\n";
        indent(); output << "set offsetY(value) { const target = self._zenithEnsureSphereCollider3D(entity); if (target) target.sphereCollider3DOffsetY = value; },\n";
        indent(); output << "get offsetZ() { return entity ? (entity.sphereCollider3DOffsetZ !== undefined ? entity.sphereCollider3DOffsetZ : 0) : 0; },\n";
        indent(); output << "set offsetZ(value) { const target = self._zenithEnsureSphereCollider3D(entity); if (target) target.sphereCollider3DOffsetZ = value; },\n";
        indent(); output << "get radius() { return entity ? (entity.sphereCollider3DRadius !== undefined ? entity.sphereCollider3DRadius : 0.5) : 0.5; },\n";
        indent(); output << "set radius(value) { const target = self._zenithEnsureSphereCollider3D(entity); if (target) target.sphereCollider3DRadius = value > 0 ? value : 0.01; },\n";
        indent(); output << "get isTrigger() { return entity ? entity.sphereCollider3DIsTrigger === true : false; },\n";
        indent(); output << "set isTrigger(value) { const target = self._zenithEnsureSphereCollider3D(entity); if (target) target.sphereCollider3DIsTrigger = value === true; },\n";
        indent(); output << "get enabled() { return entity ? entity.sphereCollider3DEnabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureSphereCollider3D(entity); if (target) target.sphereCollider3DEnabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "camera3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get rotationX() { return entity ? (entity.rotationX !== undefined ? entity.rotationX : 0) : 0; },\n";
        indent(); output << "set rotationX(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.rotationX = value; },\n";
        indent(); output << "get rotationY() { return entity ? (entity.rotationY !== undefined ? entity.rotationY : 0) : 0; },\n";
        indent(); output << "set rotationY(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.rotationY = value; },\n";
        indent(); output << "get rotationZ() { return entity ? (entity.rotationZ !== undefined ? entity.rotationZ : 0) : 0; },\n";
        indent(); output << "set rotationZ(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.rotationZ = value; },\n";
        indent(); output << "get fov() { return entity ? (entity.fov !== undefined ? entity.fov : 60) : 60; },\n";
        indent(); output << "set fov(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.fov = value; },\n";
        indent(); output << "get nearClip() { return entity ? (entity.nearClip !== undefined ? entity.nearClip : 0.1) : 0.1; },\n";
        indent(); output << "set nearClip(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.nearClip = value; },\n";
        indent(); output << "get farClip() { return entity ? (entity.farClip !== undefined ? entity.farClip : 1000) : 1000; },\n";
        indent(); output << "set farClip(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.farClip = value; },\n";
        indent(); output << "get primary() { return entity ? entity.primary === true : false; },\n";
        indent(); output << "set primary(value) {\n";
        indent_level++;
        indent(); output << "const target = self._zenithEnsureCamera3D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "if (value) {\n";
        indent_level++;
        indent(); output << "for (const existing of self._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing !== target && existing.kind === 'camera3d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "self._zenithPrimaryCamera3D = target;\n";
        indent_level--;
        indent(); output << "} else if (self._zenithPrimaryCamera3D === target) {\n";
        indent_level++;
        indent(); output << "self._zenithPrimaryCamera3D = null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "target.primary = value === true;\n";
        indent_level--;
        indent(); output << "},\n";
        indent(); output << "get viewportX() { return entity ? (entity.viewportX !== undefined ? entity.viewportX : 0) : 0; },\n";
        indent(); output << "set viewportX(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.viewportX = value; },\n";
        indent(); output << "get viewportY() { return entity ? (entity.viewportY !== undefined ? entity.viewportY : 0) : 0; },\n";
        indent(); output << "set viewportY(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.viewportY = value; },\n";
        indent(); output << "get viewportWidth() { return entity ? (entity.viewportWidth !== undefined ? entity.viewportWidth : 1) : 1; },\n";
        indent(); output << "set viewportWidth(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.viewportWidth = value; },\n";
        indent(); output << "get viewportHeight() { return entity ? (entity.viewportHeight !== undefined ? entity.viewportHeight : 1) : 1; },\n";
        indent(); output << "set viewportHeight(value) { const target = self._zenithEnsureCamera3D(entity); if (target) target.viewportHeight = value; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "audioListener3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureAudioListener3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureAudioListener3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsureAudioListener3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get gain() { return entity ? (entity.gain !== undefined ? entity.gain : 1) : 1; },\n";
        indent(); output << "set gain(value) { const target = self._zenithEnsureAudioListener3D(entity); if (target) target.gain = value; },\n";
        indent(); output << "get primary() { return entity ? entity.primary === true : false; },\n";
        indent(); output << "set primary(value) {\n";
        indent_level++;
        indent(); output << "const target = self._zenithEnsureAudioListener3D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "if (value) {\n";
        indent_level++;
        indent(); output << "for (const existing of self._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (existing !== target && existing.kind === 'listener3d') existing.primary = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "self._zenithPrimaryAudioListener3D = target;\n";
        indent_level--;
        indent(); output << "} else if (self._zenithPrimaryAudioListener3D === target) {\n";
        indent_level++;
        indent(); output << "self._zenithPrimaryAudioListener3D = null;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "target.primary = value === true;\n";
        indent_level--;
        indent(); output << "},\n";
        indent(); output << "get enabled() { return entity ? entity.enabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureAudioListener3D(entity); if (target) target.enabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "pointLight3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsurePointLight3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsurePointLight3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsurePointLight3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get intensity() { return entity ? (entity.intensity !== undefined ? entity.intensity : 1) : 1; },\n";
        indent(); output << "set intensity(value) { const target = self._zenithEnsurePointLight3D(entity); if (target) target.intensity = value; },\n";
        indent(); output << "get range() { return entity ? (entity.range !== undefined ? entity.range : 10) : 10; },\n";
        indent(); output << "set range(value) { const target = self._zenithEnsurePointLight3D(entity); if (target) target.range = value; },\n";
        indent(); output << "get color() { return entity ? (entity.color !== undefined ? entity.color : 'white') : 'white'; },\n";
        indent(); output << "set color(value) { const target = self._zenithEnsurePointLight3D(entity); if (target) target.color = value || 'white'; },\n";
        indent(); output << "get enabled() { return entity ? entity.enabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsurePointLight3D(entity); if (target) target.enabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "directionalLight3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get directionX() { return entity ? (entity.directionX !== undefined ? entity.directionX : 0) : 0; },\n";
        indent(); output << "set directionX(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.directionX = value; },\n";
        indent(); output << "get directionY() { return entity ? (entity.directionY !== undefined ? entity.directionY : -1) : -1; },\n";
        indent(); output << "set directionY(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.directionY = value; },\n";
        indent(); output << "get directionZ() { return entity ? (entity.directionZ !== undefined ? entity.directionZ : 0) : 0; },\n";
        indent(); output << "set directionZ(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.directionZ = value; },\n";
        indent(); output << "get intensity() { return entity ? (entity.intensity !== undefined ? entity.intensity : 1) : 1; },\n";
        indent(); output << "set intensity(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.intensity = value; },\n";
        indent(); output << "get color() { return entity ? (entity.color !== undefined ? entity.color : 'white') : 'white'; },\n";
        indent(); output << "set color(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.color = value || 'white'; },\n";
        indent(); output << "get castShadows() { return entity ? entity.castShadows !== false : true; },\n";
        indent(); output << "set castShadows(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.castShadows = value !== false; },\n";
        indent(); output << "get enabled() { return entity ? entity.enabled !== false : true; },\n";
        indent(); output << "set enabled(value) { const target = self._zenithEnsureDirectionalLight3D(entity); if (target) target.enabled = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "mesh3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get meshPath() { return entity ? (entity.meshPath !== undefined ? entity.meshPath : '') : ''; },\n";
        indent(); output << "set meshPath(value) { const target = self._zenithEnsureMesh3D(entity); if (target) target.meshPath = value; },\n";
        indent(); output << "get shaderPath() { return entity ? (entity.shaderPath !== undefined ? entity.shaderPath : '') : ''; },\n";
        indent(); output << "set shaderPath(value) { const target = self._zenithEnsureMesh3D(entity); if (target) target.shaderPath = value; },\n";
        indent(); output << "get materialPath() { return entity ? (entity.materialPath !== undefined ? entity.materialPath : '') : ''; },\n";
        indent(); output << "set materialPath(value) { const target = self._zenithEnsureMesh3D(entity); if (target) { target.materialPath = value || ''; if (value) self._zenithEnsureMaterial(value); } },\n";
        indent(); output << "get mesh() { return self._zenithMeshHandle(entity ? entity.meshPath : ''); },\n";
        indent(); output << "set mesh(value) { const target = self._zenithEnsureMesh3D(entity); if (target) target.meshPath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get shader() { return self._zenithShaderHandle(entity ? entity.shaderPath : ''); },\n";
        indent(); output << "set shader(value) { const target = self._zenithEnsureMesh3D(entity); if (target) target.shaderPath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get material() { return self._zenithMaterialHandle(entity ? entity.materialPath : '', entity ? entity.shaderPath : ''); },\n";
        indent(); output << "set material(value) { const target = self._zenithEnsureMesh3D(entity); if (target) { target.materialPath = value && value.path ? String(value.path) : ''; const shaderPath = value && value.shaderPath ? String(value.shaderPath) : ''; if (target.materialPath) self._zenithEnsureMaterial(target.materialPath, shaderPath); } },\n";
        indent(); output << "get visible() { return entity ? entity.visible !== false : true; },\n";
        indent(); output << "set visible(value) { const target = self._zenithEnsureMesh3D(entity); if (target) target.visible = value !== false; },\n";
        indent(); output << "get castShadows() { return entity ? entity.castShadows !== false : true; },\n";
        indent(); output << "set castShadows(value) { const target = self._zenithEnsureMesh3D(entity); if (target) target.castShadows = value !== false; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "character3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get vx() { return entity ? (entity.vx !== undefined ? entity.vx : 0) : 0; },\n";
        indent(); output << "set vx(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.vx = value; },\n";
        indent(); output << "get vy() { return entity ? (entity.vy !== undefined ? entity.vy : 0) : 0; },\n";
        indent(); output << "set vy(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.vy = value; },\n";
        indent(); output << "get vz() { return entity ? (entity.vz !== undefined ? entity.vz : 0) : 0; },\n";
        indent(); output << "set vz(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.vz = value; },\n";
        indent(); output << "get moveSpeed() { return entity ? (entity.moveSpeed !== undefined ? entity.moveSpeed : 6) : 6; },\n";
        indent(); output << "set moveSpeed(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.moveSpeed = value; },\n";
        indent(); output << "get turnSpeed() { return entity ? (entity.turnSpeed !== undefined ? entity.turnSpeed : 4) : 4; },\n";
        indent(); output << "set turnSpeed(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.turnSpeed = value; },\n";
        indent(); output << "get jumpSpeed() { return entity ? (entity.jumpSpeed !== undefined ? entity.jumpSpeed : 7.5) : 7.5; },\n";
        indent(); output << "set jumpSpeed(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.jumpSpeed = Math.max(0, value); },\n";
        indent(); output << "get groundAcceleration() { return entity ? (entity.groundAcceleration !== undefined ? entity.groundAcceleration : 36) : 36; },\n";
        indent(); output << "set groundAcceleration(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.groundAcceleration = Math.max(0, value); },\n";
        indent(); output << "get airAcceleration() { return entity ? (entity.airAcceleration !== undefined ? entity.airAcceleration : 14) : 14; },\n";
        indent(); output << "set airAcceleration(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.airAcceleration = Math.max(0, value); },\n";
        indent(); output << "get groundFriction() { return entity ? (entity.groundFriction !== undefined ? entity.groundFriction : 20) : 20; },\n";
        indent(); output << "set groundFriction(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.groundFriction = Math.max(0, value); },\n";
        indent(); output << "get airControl() { return entity ? (entity.airControl !== undefined ? entity.airControl : 0.35) : 0.35; },\n";
        indent(); output << "set airControl(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.airControl = Math.max(0, Math.min(1, value)); },\n";
        indent(); output << "get groundSnapDistance() { return entity ? (entity.groundSnapDistance !== undefined ? entity.groundSnapDistance : 0.2) : 0.2; },\n";
        indent(); output << "set groundSnapDistance(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.groundSnapDistance = Math.max(0, value); },\n";
        indent(); output << "get maxSlopeAngle() { return entity ? (entity.maxSlopeAngle !== undefined ? entity.maxSlopeAngle : 55) : 55; },\n";
        indent(); output << "set maxSlopeAngle(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.maxSlopeAngle = Math.max(0, Math.min(89, value)); },\n";
        indent(); output << "get moveInputX() { return entity ? (entity.moveInputX !== undefined ? entity.moveInputX : 0) : 0; },\n";
        indent(); output << "set moveInputX(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.moveInputX = value; },\n";
        indent(); output << "get moveInputY() { return entity ? (entity.moveInputY !== undefined ? entity.moveInputY : 0) : 0; },\n";
        indent(); output << "set moveInputY(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.moveInputY = value; },\n";
        indent(); output << "get moveInputZ() { return entity ? (entity.moveInputZ !== undefined ? entity.moveInputZ : 0) : 0; },\n";
        indent(); output << "set moveInputZ(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.moveInputZ = value; },\n";
        indent(); output << "get isGrounded() { return entity ? entity.isGrounded === true : false; },\n";
        indent(); output << "set isGrounded(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.isGrounded = value === true; },\n";
        indent(); output << "get useGravity() { return entity ? entity.useGravity !== false : true; },\n";
        indent(); output << "set useGravity(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.useGravity = value !== false; },\n";
        indent(); output << "get meshPath() { return entity ? (entity.meshPath !== undefined ? entity.meshPath : '') : ''; },\n";
        indent(); output << "set meshPath(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.meshPath = value || ''; },\n";
        indent(); output << "get shaderPath() { return entity ? (entity.shaderPath !== undefined ? entity.shaderPath : '') : ''; },\n";
        indent(); output << "set shaderPath(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.shaderPath = value || ''; },\n";
        indent(); output << "get materialPath() { return entity ? (entity.materialPath !== undefined ? entity.materialPath : '') : ''; },\n";
        indent(); output << "set materialPath(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) { target.materialPath = value || ''; if (value) self._zenithEnsureMaterial(value); } },\n";
        indent(); output << "get mesh() { return self._zenithMeshHandle(entity ? entity.meshPath : ''); },\n";
        indent(); output << "set mesh(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.meshPath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get shader() { return self._zenithShaderHandle(entity ? entity.shaderPath : ''); },\n";
        indent(); output << "set shader(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) target.shaderPath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get material() { return self._zenithMaterialHandle(entity ? entity.materialPath : '', entity ? entity.shaderPath : ''); },\n";
        indent(); output << "set material(value) { const target = self._zenithEnsureCharacter3D(entity); if (target) { target.materialPath = value && value.path ? String(value.path) : ''; const shaderPath = value && value.shaderPath ? String(value.shaderPath) : ''; if (target.materialPath) self._zenithEnsureMaterial(target.materialPath, shaderPath); } }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "audioSource3D(entity) {\n";
        indent_level++;
        indent(); output << "const self = this;\n";
        indent(); output << "return {\n";
        indent_level++;
        indent(); output << "get x() { return entity ? (entity.x !== undefined ? entity.x : 0) : 0; },\n";
        indent(); output << "set x(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.x = value; },\n";
        indent(); output << "get y() { return entity ? (entity.y !== undefined ? entity.y : 0) : 0; },\n";
        indent(); output << "set y(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.y = value; },\n";
        indent(); output << "get z() { return entity ? (entity.z !== undefined ? entity.z : 0) : 0; },\n";
        indent(); output << "set z(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.z = value; },\n";
        indent(); output << "get clipPath() { return entity ? (entity.clipPath !== undefined ? entity.clipPath : '') : ''; },\n";
        indent(); output << "set clipPath(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.clipPath = value || ''; },\n";
        indent(); output << "get clip() { return self._zenithAudioHandle(entity ? entity.clipPath : '', true); },\n";
        indent(); output << "set clip(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.clipPath = value && value.path ? String(value.path) : ''; },\n";
        indent(); output << "get volume() { return entity ? (entity.volume !== undefined ? entity.volume : 1) : 1; },\n";
        indent(); output << "set volume(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.volume = value; },\n";
        indent(); output << "get pitch() { return entity ? (entity.pitch !== undefined ? entity.pitch : 1) : 1; },\n";
        indent(); output << "set pitch(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.pitch = value; },\n";
        indent(); output << "get loop() { return entity ? entity.loop === true : false; },\n";
        indent(); output << "set loop(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.loop = value === true; },\n";
        indent(); output << "get playOnAwake() { return entity ? entity.playOnAwake === true : false; },\n";
        indent(); output << "set playOnAwake(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.playOnAwake = value === true; },\n";
        indent(); output << "get isPlaying() { return entity ? entity.isPlaying === true : false; },\n";
        indent(); output << "set isPlaying(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.isPlaying = value === true; },\n";
        indent(); output << "get minDistance() { return entity ? (entity.minDistance !== undefined ? entity.minDistance : 1) : 1; },\n";
        indent(); output << "set minDistance(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.minDistance = value; },\n";
        indent(); output << "get maxDistance() { return entity ? (entity.maxDistance !== undefined ? entity.maxDistance : 20) : 20; },\n";
        indent(); output << "set maxDistance(value) { const target = self._zenithEnsureAudioSource3D(entity); if (target) target.maxDistance = value; }\n";
        indent_level--;
        indent(); output << "};\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setSpriteColor(entity, color) {\n";
        indent_level++;
        indent(); output << "if (entity) entity.color = color;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setSpriteTexture(entity, texturePath) {\n";
        indent_level++;
        indent(); output << "if (entity) entity.texturePath = texturePath;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "spriteTexturePath(entity) {\n";
        indent_level++;
        indent(); output << "return entity && entity.texturePath ? entity.texturePath : '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "attachBody2D(entity, mass, gravityScale, friction, restitution) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "entity.hasBody = true;\n";
        indent(); output << "entity.mass = mass > 0 ? mass : 1;\n";
        indent(); output << "entity.gravityScale = gravityScale;\n";
        indent(); output << "entity.friction = friction;\n";
        indent(); output << "entity.restitution = restitution;\n";
        indent(); output << "entity.vx = entity.vx || 0;\n";
        indent(); output << "entity.vy = entity.vy || 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "attachBoxCollider2D(entity, width, height, isTrigger) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureBoxCollider2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "target.boxColliderWidth = width > 0 ? width : 0.01;\n";
        indent(); output << "target.boxColliderHeight = height > 0 ? height : 0.01;\n";
        indent(); output << "target.boxColliderIsTrigger = isTrigger === true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "attachCircleCollider2D(entity, radius, isTrigger) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureCircleCollider2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "target.circleColliderRadius = radius > 0 ? radius : 0.01;\n";
        indent(); output << "target.circleColliderIsTrigger = isTrigger === true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "attachCapsuleCollider2D(entity, height, radius, isTrigger) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureCapsuleCollider2D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "target.capsuleColliderRadius = radius > 0 ? radius : 0.01;\n";
        indent(); output << "target.capsuleColliderHeight = Math.max(height, target.capsuleColliderRadius * 2);\n";
        indent(); output << "target.capsuleColliderIsTrigger = isTrigger === true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setBodyVelocity2D(entity, vx, vy) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "entity.vx = vx;\n";
        indent(); output << "entity.vy = vy;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "applyBodyImpulse2D(entity, ix, iy) {\n";
        indent_level++;
        indent(); output << "if (!entity) return;\n";
        indent(); output << "const mass = entity.mass && entity.mass > 0 ? entity.mass : 1;\n";
        indent(); output << "entity.vx = (entity.vx || 0) + (ix / mass);\n";
        indent(); output << "entity.vy = (entity.vy || 0) + (iy / mass);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "bodyVelocityX(entity) {\n";
        indent_level++;
        indent(); output << "return entity ? (entity.vx || 0) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "bodyVelocityY(entity) {\n";
        indent_level++;
        indent(); output << "return entity ? (entity.vy || 0) : 0;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "overlaps2D(first, second) {\n";
        indent_level++;
        indent(); output << "if (!first || !second) return false;\n";
        indent(); output << "if (!this._zenithCanEntitiesInteract(first, second)) return false;\n";
        indent(); output << "const firstBox = this._zenithBoxColliderBounds2D(first);\n";
        indent(); output << "const secondBox = this._zenithBoxColliderBounds2D(second);\n";
        indent(); output << "const firstCircle = this._zenithCircleColliderState2D(first);\n";
        indent(); output << "const secondCircle = this._zenithCircleColliderState2D(second);\n";
        indent(); output << "const firstCapsule = this._zenithCapsuleColliderState2D(first);\n";
        indent(); output << "const secondCapsule = this._zenithCapsuleColliderState2D(second);\n";
        indent(); output << "if (firstBox && secondBox) {\n";
        indent_level++;
        indent(); output << "if (firstBox.minX <= secondBox.maxX && firstBox.maxX >= secondBox.minX && firstBox.minY <= secondBox.maxY && firstBox.maxY >= secondBox.minY) return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (firstBox && secondCircle && this._zenithBoxIntersectsCircle2D(firstBox, secondCircle)) return true;\n";
        indent(); output << "if (firstBox && secondCapsule && this._zenithCapsuleIntersectsBox2D(secondCapsule, firstBox)) return true;\n";
        indent(); output << "if (firstCircle && secondBox && this._zenithBoxIntersectsCircle2D(secondBox, firstCircle)) return true;\n";
        indent(); output << "if (firstCircle && secondCapsule && this._zenithCapsuleIntersectsCircle2D(secondCapsule, firstCircle)) return true;\n";
        indent(); output << "if (firstCircle && secondCircle) {\n";
        indent_level++;
        indent(); output << "const dx = firstCircle.x - secondCircle.x;\n";
        indent(); output << "const dy = firstCircle.y - secondCircle.y;\n";
        indent(); output << "const radius = firstCircle.radius + secondCircle.radius;\n";
        indent(); output << "if ((dx * dx) + (dy * dy) <= (radius * radius)) return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (firstCapsule && secondBox && this._zenithCapsuleIntersectsBox2D(firstCapsule, secondBox)) return true;\n";
        indent(); output << "if (firstCapsule && secondCircle && this._zenithCapsuleIntersectsCircle2D(firstCapsule, secondCircle)) return true;\n";
        indent(); output << "if (firstCapsule && secondCapsule && this._zenithCapsuleIntersectsCapsule2D(firstCapsule, secondCapsule)) return true;\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "containsPoint2D(entity, x, y) {\n";
        indent_level++;
        indent(); output << "const box = this._zenithBoxColliderBounds2D(entity);\n";
        indent(); output << "if (box && x >= box.minX && x <= box.maxX && y >= box.minY && y <= box.maxY) return true;\n";
        indent(); output << "const circle = this._zenithCircleColliderState2D(entity);\n";
        indent(); output << "if (circle) {\n";
        indent_level++;
        indent(); output << "const dx = x - circle.x;\n";
        indent(); output << "const dy = y - circle.y;\n";
        indent(); output << "if ((dx * dx) + (dy * dy) <= (circle.radius * circle.radius)) return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const capsule = this._zenithCapsuleColliderState2D(entity);\n";
        indent(); output << "if (capsule && this._zenithCapsuleContainsPoint2D(capsule, x, y)) return true;\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithRaycastCircle2D(originX, originY, dirX, dirY, circle, maxDistance) {\n";
        indent_level++;
        indent(); output << "if (!circle || maxDistance < 0) return null;\n";
        indent(); output << "const ocX = originX - circle.x;\n";
        indent(); output << "const ocY = originY - circle.y;\n";
        indent(); output << "const b = (ocX * dirX) + (ocY * dirY);\n";
        indent(); output << "const c = (ocX * ocX) + (ocY * ocY) - (circle.radius * circle.radius);\n";
        indent(); output << "if (c > 0 && b > 0) return null;\n";
        indent(); output << "const discriminant = (b * b) - c;\n";
        indent(); output << "if (discriminant < 0) return null;\n";
        indent(); output << "let distance = -b - Math.sqrt(discriminant);\n";
        indent(); output << "if (distance < 0) distance = 0;\n";
        indent(); output << "if (distance > maxDistance) return null;\n";
        indent(); output << "const pointX = originX + (dirX * distance);\n";
        indent(); output << "const pointY = originY + (dirY * distance);\n";
        indent(); output << "let normalX = pointX - circle.x;\n";
        indent(); output << "let normalY = pointY - circle.y;\n";
        indent(); output << "const normalLength = Math.sqrt((normalX * normalX) + (normalY * normalY));\n";
        indent(); output << "if (normalLength > 0.0000001) {\n";
        indent_level++;
        indent(); output << "normalX /= normalLength;\n";
        indent(); output << "normalY /= normalLength;\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "normalX = -dirX;\n";
        indent(); output << "normalY = -dirY;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return { distance, pointX, pointY, normalX, normalY };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithRaycastBox2D(originX, originY, dirX, dirY, bounds, maxDistance) {\n";
        indent_level++;
        indent(); output << "if (!bounds || maxDistance < 0) return null;\n";
        indent(); output << "const dirEpsilon = 0.0000001;\n";
        indent(); output << "let tMin = 0;\n";
        indent(); output << "let tMax = maxDistance;\n";
        indent(); output << "let normalX = 0;\n";
        indent(); output << "let normalY = 0;\n";
        indent(); output << "const axisTest = (origin, dir, min, max, nx, ny) => {\n";
        indent_level++;
        indent(); output << "if (Math.abs(dir) <= dirEpsilon) {\n";
        indent_level++;
        indent(); output << "return origin >= min && origin <= max;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "let near = (min - origin) / dir;\n";
        indent(); output << "let far = (max - origin) / dir;\n";
        indent(); output << "let hitNx = nx;\n";
        indent(); output << "let hitNy = ny;\n";
        indent(); output << "if (near > far) {\n";
        indent_level++;
        indent(); output << "const swap = near;\n";
        indent(); output << "near = far;\n";
        indent(); output << "far = swap;\n";
        indent(); output << "hitNx = -nx;\n";
        indent(); output << "hitNy = -ny;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (near > tMin) {\n";
        indent_level++;
        indent(); output << "tMin = near;\n";
        indent(); output << "normalX = hitNx;\n";
        indent(); output << "normalY = hitNy;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (far < tMax) tMax = far;\n";
        indent(); output << "return tMin <= tMax;\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "if (!axisTest(originX, dirX, bounds.minX, bounds.maxX, -1, 0)) return null;\n";
        indent(); output << "if (!axisTest(originY, dirY, bounds.minY, bounds.maxY, 0, -1)) return null;\n";
        indent(); output << "if (tMax < 0) return null;\n";
        indent(); output << "const distance = tMin >= 0 ? tMin : 0;\n";
        indent(); output << "if (distance > maxDistance) return null;\n";
        indent(); output << "const pointX = originX + (dirX * distance);\n";
        indent(); output << "const pointY = originY + (dirY * distance);\n";
        indent(); output << "if (distance === 0 && normalX === 0 && normalY === 0) {\n";
        indent_level++;
        indent(); output << "normalX = -dirX;\n";
        indent(); output << "normalY = -dirY;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return { distance, pointX, pointY, normalX, normalY };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "raycast2D(originX, originY, directionX, directionY, maxDistance) {\n";
        indent_level++;
        indent(); output << "return this.raycast2DMask(originX, originY, directionX, directionY, maxDistance, 0xFFFFFFFF);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "raycast2DMask(originX, originY, directionX, directionY, maxDistance, layerMask) {\n";
        indent_level++;
        indent(); output << "const dirLength = Math.sqrt((directionX * directionX) + (directionY * directionY));\n";
        indent(); output << "if (dirLength <= 0.0000001 || maxDistance < 0) {\n";
        indent_level++;
        indent(); output << "return { hit: false, entity: null, distance: 0, pointX: 0, pointY: 0, normalX: 0, normalY: 0 };\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const dirX = directionX / dirLength;\n";
        indent(); output << "const dirY = directionY / dirLength;\n";
        indent(); output << "let bestHit = { hit: false, entity: null, distance: 0, pointX: 0, pointY: 0, normalX: 0, normalY: 0 };\n";
        indent(); output << "let bestDistance = maxDistance;\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity) continue;\n";
        indent(); output << "if (!this._zenithLayerMaskMatches(layerMask, this._zenithEntityLayer(entity))) continue;\n";
        indent(); output << "const box = this._zenithBoxColliderBounds2D(entity);\n";
        indent(); output << "if (box) {\n";
        indent_level++;
        indent(); output << "const hit = this._zenithRaycastBox2D(originX, originY, dirX, dirY, box, bestDistance);\n";
        indent(); output << "if (hit && hit.distance <= bestDistance) {\n";
        indent_level++;
        indent(); output << "bestDistance = hit.distance;\n";
        indent(); output << "bestHit = { hit: true, entity, distance: hit.distance, pointX: hit.pointX, pointY: hit.pointY, normalX: hit.normalX, normalY: hit.normalY };\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const circle = this._zenithCircleColliderState2D(entity);\n";
        indent(); output << "if (circle) {\n";
        indent_level++;
        indent(); output << "const hit = this._zenithRaycastCircle2D(originX, originY, dirX, dirY, circle, bestDistance);\n";
        indent(); output << "if (hit && hit.distance <= bestDistance) {\n";
        indent_level++;
        indent(); output << "bestDistance = hit.distance;\n";
        indent(); output << "bestHit = { hit: true, entity, distance: hit.distance, pointX: hit.pointX, pointY: hit.pointY, normalX: hit.normalX, normalY: hit.normalY };\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const capsule = this._zenithCapsuleColliderState2D(entity);\n";
        indent(); output << "if (capsule) {\n";
        indent_level++;
        indent(); output << "const hit = this._zenithRaycastCapsule2D(originX, originY, dirX, dirY, capsule, bestDistance);\n";
        indent(); output << "if (hit && hit.distance <= bestDistance) {\n";
        indent_level++;
        indent(); output << "bestDistance = hit.distance;\n";
        indent(); output << "bestHit = { hit: true, entity, distance: hit.distance, pointX: hit.pointX, pointY: hit.pointY, normalX: hit.normalX, normalY: hit.normalY };\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return bestHit;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "attachBoxCollider3D(entity, width, height, depth, isTrigger) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureBoxCollider3D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "target.boxCollider3DWidth = width > 0 ? width : 0.01;\n";
        indent(); output << "target.boxCollider3DHeight = height > 0 ? height : 0.01;\n";
        indent(); output << "target.boxCollider3DDepth = depth > 0 ? depth : 0.01;\n";
        indent(); output << "target.boxCollider3DIsTrigger = isTrigger === true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "attachSphereCollider3D(entity, radius, isTrigger) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureSphereCollider3D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "target.sphereCollider3DRadius = radius > 0 ? radius : 0.01;\n";
        indent(); output << "target.sphereCollider3DIsTrigger = isTrigger === true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setCharacterMove3D(entity, x, y, z) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureCharacter3D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "target.moveInputX = x;\n";
        indent(); output << "target.moveInputY = y;\n";
        indent(); output << "target.moveInputZ = z;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "jumpCharacter3D(entity) {\n";
        indent_level++;
        indent(); output << "const target = this._zenithEnsureCharacter3D(entity);\n";
        indent(); output << "if (!target) return;\n";
        indent(); output << "target.jumpQueued = true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "overlaps3D(first, second) {\n";
        indent_level++;
        indent(); output << "if (!first || !second) return false;\n";
        indent(); output << "if (!this._zenithCanEntitiesInteract(first, second)) return false;\n";
        indent(); output << "const firstBox = this._zenithBoxColliderBounds3D(first);\n";
        indent(); output << "const secondBox = this._zenithBoxColliderBounds3D(second);\n";
        indent(); output << "const firstSphere = this._zenithSphereColliderState3D(first);\n";
        indent(); output << "const secondSphere = this._zenithSphereColliderState3D(second);\n";
        indent(); output << "if (firstBox && secondBox) {\n";
        indent_level++;
        indent(); output << "if (firstBox.minX <= secondBox.maxX && firstBox.maxX >= secondBox.minX && firstBox.minY <= secondBox.maxY && firstBox.maxY >= secondBox.minY && firstBox.minZ <= secondBox.maxZ && firstBox.maxZ >= secondBox.minZ) return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (firstBox && secondSphere && this._zenithBoxIntersectsSphere3D(firstBox, secondSphere)) return true;\n";
        indent(); output << "if (firstSphere && secondBox && this._zenithBoxIntersectsSphere3D(secondBox, firstSphere)) return true;\n";
        indent(); output << "if (firstSphere && secondSphere) {\n";
        indent_level++;
        indent(); output << "const dx = firstSphere.x - secondSphere.x;\n";
        indent(); output << "const dy = firstSphere.y - secondSphere.y;\n";
        indent(); output << "const dz = firstSphere.z - secondSphere.z;\n";
        indent(); output << "const radius = firstSphere.radius + secondSphere.radius;\n";
        indent(); output << "if ((dx * dx) + (dy * dy) + (dz * dz) <= (radius * radius)) return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "containsPoint3D(entity, x, y, z) {\n";
        indent_level++;
        indent(); output << "const box = this._zenithBoxColliderBounds3D(entity);\n";
        indent(); output << "if (box && x >= box.minX && x <= box.maxX && y >= box.minY && y <= box.maxY && z >= box.minZ && z <= box.maxZ) return true;\n";
        indent(); output << "const sphere = this._zenithSphereColliderState3D(entity);\n";
        indent(); output << "if (sphere) {\n";
        indent_level++;
        indent(); output << "const dx = x - sphere.x;\n";
        indent(); output << "const dy = y - sphere.y;\n";
        indent(); output << "const dz = z - sphere.z;\n";
        indent(); output << "if ((dx * dx) + (dy * dy) + (dz * dz) <= (sphere.radius * sphere.radius)) return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "raycast3D(originX, originY, originZ, directionX, directionY, directionZ, maxDistance) {\n";
        indent_level++;
        indent(); output << "return this.raycast3DMask(originX, originY, originZ, directionX, directionY, directionZ, maxDistance, 0xFFFFFFFF);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "raycast3DMask(originX, originY, originZ, directionX, directionY, directionZ, maxDistance, layerMask) {\n";
        indent_level++;
        indent(); output << "const dirLength = Math.sqrt((directionX * directionX) + (directionY * directionY) + (directionZ * directionZ));\n";
        indent(); output << "if (dirLength <= 0.0000001 || maxDistance < 0) {\n";
        indent_level++;
        indent(); output << "return { hit: false, entity: null, distance: 0, pointX: 0, pointY: 0, pointZ: 0, normalX: 0, normalY: 0, normalZ: 0 };\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const dirX = directionX / dirLength;\n";
        indent(); output << "const dirY = directionY / dirLength;\n";
        indent(); output << "const dirZ = directionZ / dirLength;\n";
        indent(); output << "let bestHit = { hit: false, entity: null, distance: 0, pointX: 0, pointY: 0, pointZ: 0, normalX: 0, normalY: 0, normalZ: 0 };\n";
        indent(); output << "let bestDistance = maxDistance;\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity) continue;\n";
        indent(); output << "if (!this._zenithLayerMaskMatches(layerMask, this._zenithEntityLayer(entity))) continue;\n";
        indent(); output << "const box = this._zenithBoxColliderBounds3D(entity);\n";
        indent(); output << "if (box) {\n";
        indent_level++;
        indent(); output << "const hit = this._zenithRaycastBox3D(originX, originY, originZ, dirX, dirY, dirZ, box, bestDistance);\n";
        indent(); output << "if (hit && hit.distance <= bestDistance) {\n";
        indent_level++;
        indent(); output << "bestDistance = hit.distance;\n";
        indent(); output << "bestHit = { hit: true, entity, distance: hit.distance, pointX: hit.pointX, pointY: hit.pointY, pointZ: hit.pointZ, normalX: hit.normalX, normalY: hit.normalY, normalZ: hit.normalZ };\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const sphere = this._zenithSphereColliderState3D(entity);\n";
        indent(); output << "if (sphere) {\n";
        indent_level++;
        indent(); output << "const hit = this._zenithRaycastSphere3D(originX, originY, originZ, dirX, dirY, dirZ, sphere, bestDistance);\n";
        indent(); output << "if (hit && hit.distance <= bestDistance) {\n";
        indent_level++;
        indent(); output << "bestDistance = hit.distance;\n";
        indent(); output << "bestHit = { hit: true, entity, distance: hit.distance, pointX: hit.pointX, pointY: hit.pointY, pointZ: hit.pointZ, normalX: hit.normalX, normalY: hit.normalY, normalZ: hit.normalZ };\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return bestHit;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "followPrimaryCamera2D(target, offsetX, offsetY, smoothing) {\n";
        indent_level++;
        indent(); output << "if (!this._zenithPrimaryCamera || !target) return false;\n";
        indent(); output << "const blend = Math.max(0, Math.min(1, smoothing));\n";
        indent(); output << "const desiredX = (target.x || 0) + offsetX;\n";
        indent(); output << "const desiredY = (target.y || 0) + offsetY;\n";
        indent(); output << "this._zenithPrimaryCamera.x += (desiredX - (this._zenithPrimaryCamera.x || 0)) * blend;\n";
        indent(); output << "this._zenithPrimaryCamera.y += (desiredY - (this._zenithPrimaryCamera.y || 0)) * blend;\n";
        indent(); output << "if (blend >= 1) {\n";
        indent_level++;
        indent(); output << "this._zenithPrimaryCamera.x = desiredX;\n";
        indent(); output << "this._zenithPrimaryCamera.y = desiredY;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "followPrimaryCamera3D(target, offsetX, offsetY, offsetZ, smoothing) {\n";
        indent_level++;
        indent(); output << "if (!this._zenithPrimaryCamera3D || !target) return false;\n";
        indent(); output << "const blend = Math.max(0, Math.min(1, smoothing));\n";
        indent(); output << "const desiredX = (target.x || 0) + offsetX;\n";
        indent(); output << "const desiredY = (target.y || 0) + offsetY;\n";
        indent(); output << "const desiredZ = (target.z || 0) + offsetZ;\n";
        indent(); output << "this._zenithPrimaryCamera3D.x += (desiredX - (this._zenithPrimaryCamera3D.x || 0)) * blend;\n";
        indent(); output << "this._zenithPrimaryCamera3D.y += (desiredY - (this._zenithPrimaryCamera3D.y || 0)) * blend;\n";
        indent(); output << "this._zenithPrimaryCamera3D.z += (desiredZ - (this._zenithPrimaryCamera3D.z || 0)) * blend;\n";
        indent(); output << "if (blend >= 1) {\n";
        indent_level++;
        indent(); output << "this._zenithPrimaryCamera3D.x = desiredX;\n";
        indent(); output << "this._zenithPrimaryCamera3D.y = desiredY;\n";
        indent(); output << "this._zenithPrimaryCamera3D.z = desiredZ;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithWorld2DViewState(canvas) {\n";
        indent_level++;
        indent(); output << "const camera = this._zenithPrimaryCamera;\n";
        indent(); output << "const zoom = camera && camera.zoom > 0 ? camera.zoom : 1;\n";
        indent(); output << "const canvasWidth = canvas && canvas.width ? canvas.width : 80;\n";
        indent(); output << "const canvasHeight = canvas && canvas.height ? canvas.height : 24;\n";
        indent(); output << "const viewportX = camera ? canvasWidth * (camera.viewportX || 0) : 0;\n";
        indent(); output << "const viewportY = camera ? canvasHeight * (camera.viewportY || 0) : 0;\n";
        indent(); output << "const viewportWidth = camera ? Math.max(0, canvasWidth * (camera.viewportWidth !== undefined ? camera.viewportWidth : 1)) : canvasWidth;\n";
        indent(); output << "const viewportHeight = camera ? Math.max(0, canvasHeight * (camera.viewportHeight !== undefined ? camera.viewportHeight : 1)) : canvasHeight;\n";
        indent(); output << "const safeViewportWidth = viewportWidth > 0 ? viewportWidth : canvasWidth;\n";
        indent(); output << "const safeViewportHeight = viewportHeight > 0 ? viewportHeight : canvasHeight;\n";
        indent(); output << "const centerX = viewportX + (safeViewportWidth * 0.5);\n";
        indent(); output << "const centerY = viewportY + (safeViewportHeight * 0.5);\n";
        indent(); output << "const cameraX = camera ? (camera.x || 0) : 0;\n";
        indent(); output << "const cameraY = camera ? (camera.y || 0) : 0;\n";
        indent(); output << "const worldLeft = camera ? (cameraX - (safeViewportWidth / (2 * zoom))) : 0;\n";
        indent(); output << "const worldTop = camera ? (cameraY - (safeViewportHeight / (2 * zoom))) : 0;\n";
        indent(); output << "const worldRight = camera ? (cameraX + (safeViewportWidth / (2 * zoom))) : canvasWidth;\n";
        indent(); output << "const worldBottom = camera ? (cameraY + (safeViewportHeight / (2 * zoom))) : canvasHeight;\n";
        indent(); output << "return { camera, zoom, viewportX, viewportY, viewportWidth: safeViewportWidth, viewportHeight: safeViewportHeight, centerX, centerY, cameraX, cameraY, worldLeft, worldTop, worldRight, worldBottom };\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithProjectWorld2DX(view, worldX) {\n";
        indent_level++;
        indent(); output << "return view.camera ? (((worldX - view.cameraX) * view.zoom) + view.centerX) : worldX;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithProjectWorld2DY(view, worldY) {\n";
        indent_level++;
        indent(); output << "return view.camera ? (((worldY - view.cameraY) * view.zoom) + view.centerY) : worldY;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithScaleWorld2D(view, value) {\n";
        indent_level++;
        indent(); output << "return value * view.zoom;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithEntityVisible2D(camera, entity) {\n";
        indent_level++;
        indent(); output << "if (!camera || !entity) return true;\n";
        indent(); output << "return this._zenithLayerMaskMatches(this._zenithEntityMask(camera), this._zenithEntityLayer(entity));\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithDrawWorld2D(canvas) {\n";
        indent_level++;
        indent(); output << "const view = this._zenithWorld2DViewState(canvas);\n";
        indent(); output << "const renderables = [];\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity) continue;\n";
        indent(); output << "const kind = entity.kind || '';\n";
        indent(); output << "if (kind !== 'sprite2d' && kind !== 'character2d' && kind !== 'tilemap2d') continue;\n";
        indent(); output << "if (entity.visible === false) continue;\n";
        indent(); output << "if (!this._zenithEntityVisible2D(view.camera, entity)) continue;\n";
        indent(); output << "renderables.push({ entity, sortOrder: entity.sortOrder || 0, kindRank: kind === 'tilemap2d' ? 0 : 1 });\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "renderables.sort((lhs, rhs) => {\n";
        indent_level++;
        indent(); output << "if (lhs.sortOrder !== rhs.sortOrder) return lhs.sortOrder - rhs.sortOrder;\n";
        indent(); output << "if (lhs.kindRank !== rhs.kindRank) return lhs.kindRank - rhs.kindRank;\n";
        indent(); output << "return (lhs.entity.id || 0) - (rhs.entity.id || 0);\n";
        indent_level--;
        indent(); output << "});\n";
        indent(); output << "for (const entry of renderables) {\n";
        indent_level++;
        indent(); output << "const entity = entry.entity;\n";
        indent(); output << "if (entity.kind === 'tilemap2d') {\n";
        indent_level++;
        indent(); output << "const columns = entity.columns !== undefined ? Math.max(1, Math.trunc(entity.columns)) : 1;\n";
        indent(); output << "const rows = entity.rows !== undefined ? Math.max(1, Math.trunc(entity.rows)) : 1;\n";
        indent(); output << "const tileWidth = entity.tileWidth !== undefined && entity.tileWidth > 0 ? entity.tileWidth : 1;\n";
        indent(); output << "const tileHeight = entity.tileHeight !== undefined && entity.tileHeight > 0 ? entity.tileHeight : 1;\n";
        indent(); output << "const totalWidth = columns * tileWidth;\n";
        indent(); output << "const totalHeight = rows * tileHeight;\n";
        indent(); output << "const baseWorldX = (entity.x || 0) - (totalWidth * (entity.anchorX || 0));\n";
        indent(); output << "const baseWorldY = (entity.y || 0) - (totalHeight * (entity.anchorY || 0));\n";
        indent(); output << "const drawTileWidth = this._zenithScaleWorld2D(view, tileWidth);\n";
        indent(); output << "const drawTileHeight = this._zenithScaleWorld2D(view, tileHeight);\n";
        indent(); output << "const palette = Array.isArray(entity.palette) ? entity.palette : ['', 'white'];\n";
        indent(); output << "const cells = Array.isArray(entity.cells) ? entity.cells : [];\n";
        indent(); output << "for (let row = 0; row < rows; row += 1) {\n";
        indent_level++;
        indent(); output << "for (let column = 0; column < columns; column += 1) {\n";
        indent_level++;
        indent(); output << "const tileId = Number(cells[(row * columns) + column]) || 0;\n";
        indent(); output << "if (tileId === 0) continue;\n";
        indent(); output << "const color = palette[tileId] || '';\n";
        indent(); output << "if (!color) continue;\n";
        indent(); output << "const worldX = baseWorldX + (column * tileWidth);\n";
        indent(); output << "const worldY = baseWorldY + (row * tileHeight);\n";
        indent(); output << "canvas.drawRect(this._zenithProjectWorld2DX(view, worldX), this._zenithProjectWorld2DY(view, worldY), drawTileWidth, drawTileHeight, color);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this.drawEntityNames && entity.name) canvas.drawText(entity.name, this._zenithProjectWorld2DX(view, baseWorldX), this._zenithProjectWorld2DY(view, baseWorldY) - 1, 'white');\n";
        indent(); output << "continue;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const drawW = this._zenithScaleWorld2D(view, entity.w || 1);\n";
        indent(); output << "const drawH = this._zenithScaleWorld2D(view, entity.h || 1);\n";
        indent(); output << "let drawX = this._zenithProjectWorld2DX(view, entity.x || 0);\n";
        indent(); output << "let drawY = this._zenithProjectWorld2DY(view, entity.y || 0);\n";
        indent(); output << "drawX -= drawW * (entity.anchorX !== undefined ? entity.anchorX : 0.5);\n";
        indent(); output << "drawY -= drawH * (entity.anchorY !== undefined ? entity.anchorY : 0.5);\n";
        indent(); output << "canvas.drawRect(drawX, drawY, drawW, drawH, entity.color || 'white');\n";
        indent(); output << "if (this.drawEntityNames && entity.name) canvas.drawText(entity.name, drawX, drawY - 1, 'white');\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithDrawDebugOverlay2D(canvas) {\n";
        indent_level++;
        indent(); output << "const view = this._zenithWorld2DViewState(canvas);\n";
        indent(); output << "const overlayColor = this.debugOverlayColor || 'cyan';\n";
        indent(); output << "if (this.debugDrawGrid2D) {\n";
        indent_level++;
        indent(); output << "const cellWidth = Math.max(0.25, this.debugGridCellWidth || 1);\n";
        indent(); output << "const cellHeight = Math.max(0.25, this.debugGridCellHeight || 1);\n";
        indent(); output << "const startWorldX = Math.floor(view.worldLeft / cellWidth) * cellWidth;\n";
        indent(); output << "const startWorldY = Math.floor(view.worldTop / cellHeight) * cellHeight;\n";
        indent(); output << "const maxScreenX = view.viewportX + Math.max(1, view.viewportWidth - 1);\n";
        indent(); output << "const maxScreenY = view.viewportY + Math.max(1, view.viewportHeight - 1);\n";
        indent(); output << "for (let worldX = startWorldX; worldX <= view.worldRight + cellWidth; worldX += cellWidth) {\n";
        indent_level++;
        indent(); output << "const screenX = this._zenithProjectWorld2DX(view, worldX);\n";
        indent(); output << "canvas.drawLine(screenX, view.viewportY, screenX, maxScreenY, overlayColor);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "for (let worldY = startWorldY; worldY <= view.worldBottom + cellHeight; worldY += cellHeight) {\n";
        indent_level++;
        indent(); output << "const screenY = this._zenithProjectWorld2DY(view, worldY);\n";
        indent(); output << "canvas.drawLine(view.viewportX, screenY, maxScreenX, screenY, overlayColor);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this.debugDrawColliders2D) {\n";
        indent_level++;
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity || !this._zenithEntityVisible2D(view.camera, entity)) continue;\n";
        indent(); output << "if (entity.hasBoxCollider2D === true && entity.boxColliderEnabled !== false) {\n";
        indent_level++;
        indent(); output << "const width = entity.boxColliderWidth !== undefined && entity.boxColliderWidth > 0 ? entity.boxColliderWidth : 1;\n";
        indent(); output << "const height = entity.boxColliderHeight !== undefined && entity.boxColliderHeight > 0 ? entity.boxColliderHeight : 1;\n";
        indent(); output << "const centerX = (entity.x || 0) + (entity.boxColliderOffsetX || 0);\n";
        indent(); output << "const centerY = (entity.y || 0) + (entity.boxColliderOffsetY || 0);\n";
        indent(); output << "canvas.drawFrameRect(this._zenithProjectWorld2DX(view, centerX - (width * 0.5)), this._zenithProjectWorld2DY(view, centerY - (height * 0.5)), this._zenithScaleWorld2D(view, width), this._zenithScaleWorld2D(view, height), entity.boxColliderIsTrigger === true ? 'yellow' : overlayColor);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (entity.hasCircleCollider2D === true && entity.circleColliderEnabled !== false) {\n";
        indent_level++;
        indent(); output << "const radius = entity.circleColliderRadius !== undefined && entity.circleColliderRadius > 0 ? entity.circleColliderRadius : 0.5;\n";
        indent(); output << "const centerX = (entity.x || 0) + (entity.circleColliderOffsetX || 0);\n";
        indent(); output << "const centerY = (entity.y || 0) + (entity.circleColliderOffsetY || 0);\n";
        indent(); output << "canvas.drawCircleOutline(this._zenithProjectWorld2DX(view, centerX), this._zenithProjectWorld2DY(view, centerY), this._zenithScaleWorld2D(view, radius), entity.circleColliderIsTrigger === true ? 'yellow' : overlayColor);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (entity.hasCapsuleCollider2D === true && entity.capsuleColliderEnabled !== false) {\n";
        indent_level++;
        indent(); output << "const radius = entity.capsuleColliderRadius !== undefined && entity.capsuleColliderRadius > 0 ? entity.capsuleColliderRadius : 0.5;\n";
        indent(); output << "const height = entity.capsuleColliderHeight !== undefined ? Math.max(entity.capsuleColliderHeight, radius * 2) : Math.max(2, radius * 2);\n";
        indent(); output << "const halfSegment = Math.max(0, (height * 0.5) - radius);\n";
        indent(); output << "const centerX = (entity.x || 0) + (entity.capsuleColliderOffsetX || 0);\n";
        indent(); output << "const centerY = (entity.y || 0) + (entity.capsuleColliderOffsetY || 0);\n";
        indent(); output << "const color = entity.capsuleColliderIsTrigger === true ? 'yellow' : overlayColor;\n";
        indent(); output << "if (halfSegment > 0) {\n";
        indent_level++;
        indent(); output << "canvas.drawFrameRect(this._zenithProjectWorld2DX(view, centerX - radius), this._zenithProjectWorld2DY(view, centerY - halfSegment), this._zenithScaleWorld2D(view, radius * 2), this._zenithScaleWorld2D(view, halfSegment * 2), color);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "canvas.drawCircleOutline(this._zenithProjectWorld2DX(view, centerX), this._zenithProjectWorld2DY(view, centerY - halfSegment), this._zenithScaleWorld2D(view, radius), color);\n";
        indent(); output << "canvas.drawCircleOutline(this._zenithProjectWorld2DX(view, centerX), this._zenithProjectWorld2DY(view, centerY + halfSegment), this._zenithScaleWorld2D(view, radius), color);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this.debugDrawTransforms2D) {\n";
        indent_level++;
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity || !this._zenithEntityVisible2D(view.camera, entity)) continue;\n";
        indent(); output << "const screenX = this._zenithProjectWorld2DX(view, entity.x || 0);\n";
        indent(); output << "const screenY = this._zenithProjectWorld2DY(view, entity.y || 0);\n";
        indent(); output << "canvas.drawPoint(screenX, screenY, overlayColor);\n";
        indent(); output << "canvas.drawLine(screenX - 1, screenY, screenX + 1, screenY, overlayColor);\n";
        indent(); output << "canvas.drawLine(screenX, screenY - 1, screenX, screenY + 1, overlayColor);\n";
        indent(); output << "if (entity.hasBody === true) canvas.drawLine(screenX, screenY, this._zenithProjectWorld2DX(view, (entity.x || 0) + ((entity.vx || 0) * 0.15)), this._zenithProjectWorld2DY(view, (entity.y || 0) + ((entity.vy || 0) * 0.15)), 'green');\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this.debugDrawCameraBounds2D) {\n";
        indent_level++;
        indent(); output << "canvas.drawFrameRect(view.viewportX, view.viewportY, Math.max(1, view.viewportWidth - 1), Math.max(1, view.viewportHeight - 1), overlayColor);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this.debugDrawRuntimeStats) {\n";
        indent_level++;
        indent(); output << "let spriteCount = 0;\n";
        indent(); output << "let tilemapCount = 0;\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity) continue;\n";
        indent(); output << "if (entity.kind === 'tilemap2d') tilemapCount += 1;\n";
        indent(); output << "if (entity.kind === 'sprite2d' || entity.kind === 'character2d') spriteCount += 1;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const originX = view.viewportX + 1;\n";
        indent(); output << "let lineY = view.viewportY + 1;\n";
        indent(); output << "const frameMs = Math.round((this._zenithLastFrameDelta || 0) * 1000);\n";
        indent(); output << "const accumulatorMs = Math.round((this._zenithAccumulator || 0) * 1000);\n";
        indent(); output << "canvas.drawText('frame:' + this._zenithFrameCount + ' dt_ms:' + frameMs, originX, lineY, overlayColor);\n";
        indent(); output << "lineY += 1;\n";
        indent(); output << "canvas.drawText('fixed:' + this._zenithFixedStepCount + ' last:' + this._zenithLastFixedSteps + ' acc_ms:' + accumulatorMs, originX, lineY, overlayColor);\n";
        indent(); output << "lineY += 1;\n";
        indent(); output << "canvas.drawText('dropped:' + this._zenithDroppedStepFrames + ' entities:' + this._zenithEntities.length, originX, lineY, overlayColor);\n";
        indent(); output << "lineY += 1;\n";
        indent(); output << "canvas.drawText('sprites:' + spriteCount + ' tilemaps:' + tilemapCount, originX, lineY, overlayColor);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithClipOverlayText(text, maxChars) {\n";
        indent_level++;
        indent(); output << "const value = text === undefined || text === null ? '' : String(text);\n";
        indent(); output << "if (value.length <= maxChars) return value;\n";
        indent(); output << "if (maxChars <= 3) return value.slice(0, maxChars);\n";
        indent(); output << "return value.slice(0, maxChars - 3) + '...';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithInspectorPropertyValue(property) {\n";
        indent_level++;
        indent(); output << "if (!property) return '';\n";
        indent(); output << "if (property.kind === 'Number') return String(Number(property.numberValue || 0));\n";
        indent(); output << "if (property.kind === 'Toggle') return property.boolValue === true ? 'true' : 'false';\n";
        indent(); output << "return property.stringValue || '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "_zenithDrawInspector(canvas) {\n";
        indent_level++;
        indent(); output << "if (this.minimalInspectorEnabled !== true) return;\n";
        indent(); output << "const width = canvas && canvas.width ? canvas.width : 80;\n";
        indent(); output << "const height = canvas && canvas.height ? canvas.height : 24;\n";
        indent(); output << "const panelWidth = Math.max(24, Math.min(width - 2, 30));\n";
        indent(); output << "const panelHeight = Math.max(8, height - 2);\n";
        indent(); output << "if (panelWidth < 12 || panelHeight < 6) return;\n";
        indent(); output << "const borderColor = this.debugOverlayColor || 'cyan';\n";
        indent(); output << "const originX = Math.max(0, width - panelWidth - 1);\n";
        indent(); output << "const originY = 1;\n";
        indent(); output << "const innerWidth = Math.max(1, panelWidth - 2);\n";
        indent(); output << "let line = 1;\n";
        indent(); output << "const drawLine = (text, color = 'white') => {\n";
        indent_level++;
        indent(); output << "if (line >= panelHeight - 1) return false;\n";
        indent(); output << "canvas.drawText(this._zenithClipOverlayText(text, innerWidth), originX + 1, originY + line, color);\n";
        indent(); output << "line += 1;\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "canvas.drawFrameRect(originX, originY, panelWidth, panelHeight, borderColor);\n";
        indent(); output << "drawLine('Inspector', borderColor);\n";
        indent(); output << "drawLine('scene:' + (this.name || 'Scene'));\n";
        indent(); output << "let target = this.inspectedEntity();\n";
        indent(); output << "let materialPath = this.inspectedMaterialPath();\n";
        indent(); output << "if (target) {\n";
        indent_level++;
        indent(); output << "drawLine('entity:' + (target.name || 'unnamed'), 'yellow');\n";
        indent(); output << "drawLine('id:' + String(target.id || 0));\n";
        indent(); output << "if (target.tag) drawLine('tag:' + target.tag);\n";
        indent(); output << "drawLine('layer:' + this._zenithEntityLayer(target) + ' mask:' + this._zenithEntityMask(target));\n";
        indent(); output << "if (target.z !== undefined) drawLine('pos3:' + (target.x || 0) + ',' + (target.y || 0) + ',' + (target.z || 0));\n";
        indent(); output << "else drawLine('pos2:' + (target.x || 0) + ',' + (target.y || 0));\n";
        indent(); output << "if (target.vz !== undefined) drawLine('vel3:' + (target.vx || 0) + ',' + (target.vy || 0) + ',' + (target.vz || 0), 'green');\n";
        indent(); output << "else if (target.hasBody === true || target.vx !== undefined || target.vy !== undefined) drawLine('vel2:' + (target.vx || 0) + ',' + (target.vy || 0), 'green');\n";
        indent(); output << "if ((target.kind === 'sprite2d' || target.kind === 'character2d') && target.color) drawLine('sprite:' + target.color);\n";
        indent(); output << "if (target.texturePath) drawLine('tex:' + target.texturePath);\n";
        indent(); output << "if (target.meshPath) drawLine('mesh:' + target.meshPath);\n";
        indent(); output << "if (target.shaderPath) drawLine('shader:' + target.shaderPath);\n";
        indent(); output << "if (target.materialPath) {\n";
        indent_level++;
        indent(); output << "materialPath = target.materialPath;\n";
        indent(); output << "drawLine('mat:' + materialPath, 'magenta');\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (target.clipPath) drawLine('audio:' + target.clipPath);\n";
        indent_level--;
        indent(); output << "} else if (materialPath) {\n";
        indent_level++;
        indent(); output << "drawLine('material:' + materialPath, 'magenta');\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "let spriteCount = 0;\n";
        indent(); output << "let tilemapCount = 0;\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity) continue;\n";
        indent(); output << "if (entity.kind === 'tilemap2d') tilemapCount += 1;\n";
        indent(); output << "if (entity.kind === 'sprite2d' || entity.kind === 'character2d') spriteCount += 1;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "drawLine('entities:' + this._zenithEntities.length);\n";
        indent(); output << "drawLine('sprites:' + spriteCount);\n";
        indent(); output << "drawLine('tilemaps:' + tilemapCount);\n";
        indent(); output << "if (this._zenithEntities.length > 0) drawLine('next:' + (this._zenithEntities[0].name || 'entity'));\n";
        indent(); output << "drawLine('inspectEntity(...)', borderColor);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (materialPath) {\n";
        indent_level++;
        indent(); output << "const propertyCount = this.materialPropertyCount(materialPath);\n";
        indent(); output << "drawLine('props:' + propertyCount, borderColor);\n";
        indent(); output << "const visibleProperties = Math.min(propertyCount, Math.max(0, panelHeight - line - 1));\n";
        indent(); output << "for (let i = 0; i < visibleProperties; i += 1) {\n";
        indent_level++;
        indent(); output << "const property = this.materialPropertyAt(materialPath, i);\n";
        indent(); output << "if (!property || property.exists !== true) continue;\n";
        indent(); output << "drawLine(property.name + '=' + this._zenithInspectorPropertyValue(property));\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "load() {\n";
        indent_level++;
        indent(); output << "if (this._zenithLoaded) return;\n";
        indent(); output << "this._zenithLoaded = true;\n";
        indent(); output << "if (this.onLoad) this.onLoad();\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "updateFrame(frameDt) {\n";
        indent_level++;
        indent(); output << "this.load();\n";
        indent(); output << "if (frameDt < 0) frameDt = 0;\n";
        indent(); output << "if (frameDt > this.maxFrameDelta) frameDt = this.maxFrameDelta;\n";
        indent(); output << "this._zenithFrameCount += 1;\n";
        indent(); output << "this._zenithLastFrameDelta = frameDt;\n";
        indent(); output << "this._zenithLastFixedSteps = 0;\n";
        indent(); output << "if (this.onFrame) this.onFrame(frameDt);\n";
        indent(); output << "if (this.paused || this.fixedDeltaTime <= 0) return;\n";
        indent(); output << "this._zenithAccumulator += frameDt;\n";
        indent(); output << "while (this._zenithAccumulator + 0.000001 >= this.fixedDeltaTime && this._zenithLastFixedSteps < this.maxFixedStepsPerFrame) {\n";
        indent_level++;
        indent(); output << "this.simulateFixedStep(this.fixedDeltaTime);\n";
        indent(); output << "this._zenithAccumulator -= this.fixedDeltaTime;\n";
        indent(); output << "this._zenithLastFixedSteps += 1;\n";
        indent(); output << "this._zenithFixedStepCount += 1;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this._zenithLastFixedSteps === this.maxFixedStepsPerFrame && this._zenithAccumulator >= this.fixedDeltaTime) {\n";
        indent_level++;
        indent(); output << "this._zenithAccumulator = this._zenithAccumulator % this.fixedDeltaTime;\n";
        indent(); output << "this._zenithDroppedStepFrames += 1;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "simulateFixedStep(dt) {\n";
        indent_level++;
        indent(); output << "this.load();\n";
        indent(); output << "if (this.onFixedUpdate) this.onFixedUpdate(dt);\n";
        indent(); output << "if (this.autoSimulatePhysics) {\n";
        indent_level++;
        indent(); output << "const moveTowards = (current, target, maxDelta) => {\n";
        indent_level++;
        indent(); output << "if (maxDelta <= 0) return current;\n";
        indent(); output << "if (current < target) return Math.min(current + maxDelta, target);\n";
        indent(); output << "return Math.max(current - maxDelta, target);\n";
        indent_level--;
        indent(); output << "};\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (entity.moveSpeed === undefined || !entity.hasBody3D) continue;\n";
        indent(); output << "let inputX = entity.moveInputX || 0;\n";
        indent(); output << "let inputY = entity.moveInputY || 0;\n";
        indent(); output << "let inputZ = entity.moveInputZ || 0;\n";
        indent(); output << "const inputLength = Math.sqrt((inputX * inputX) + (inputY * inputY) + (inputZ * inputZ));\n";
        indent(); output << "if (inputLength > 1) {\n";
        indent_level++;
        indent(); output << "inputX /= inputLength;\n";
        indent(); output << "inputY /= inputLength;\n";
        indent(); output << "inputZ /= inputLength;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const useGravity = entity.useGravity !== false;\n";
        indent(); output << "if (useGravity) {\n";
        indent_level++;
        indent(); output << "const grounded = entity.isGrounded === true;\n";
        indent(); output << "const accel = grounded ? Math.max(0, entity.groundAcceleration ?? 36) : Math.max(0, (entity.airAcceleration ?? 14) * Math.max(0, Math.min(1, entity.airControl ?? 0.35)));\n";
        indent(); output << "entity.vx = moveTowards(entity.vx || 0, inputX * (entity.moveSpeed || 0), accel * dt);\n";
        indent(); output << "entity.vz = moveTowards(entity.vz || 0, inputZ * (entity.moveSpeed || 0), accel * dt);\n";
        indent(); output << "if (grounded && Math.abs(inputX) <= 0.0001 && Math.abs(inputZ) <= 0.0001) {\n";
        indent_level++;
        indent(); output << "const friction = Math.max(0, entity.groundFriction ?? 20) * dt;\n";
        indent(); output << "entity.vx = moveTowards(entity.vx || 0, 0, friction);\n";
        indent(); output << "entity.vz = moveTowards(entity.vz || 0, 0, friction);\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (entity.jumpQueued === true && grounded) {\n";
        indent_level++;
        indent(); output << "entity.vy = Math.max(entity.vy || 0, entity.jumpSpeed ?? 7.5);\n";
        indent(); output << "entity.isGrounded = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "} else {\n";
        indent_level++;
        indent(); output << "const accel = Math.max(0, entity.groundAcceleration ?? 36) * dt;\n";
        indent(); output << "entity.vx = moveTowards(entity.vx || 0, inputX * (entity.moveSpeed || 0), accel);\n";
        indent(); output << "entity.vy = moveTowards(entity.vy || 0, inputY * (entity.moveSpeed || 0), accel);\n";
        indent(); output << "entity.vz = moveTowards(entity.vz || 0, inputZ * (entity.moveSpeed || 0), accel);\n";
        indent(); output << "if (entity.jumpQueued === true) entity.vy = entity.jumpSpeed ?? 7.5;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "entity.jumpQueued = false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "for (const entity of this._zenithEntities) {\n";
        indent_level++;
        indent(); output << "if (!entity.hasBody) continue;\n";
        indent(); output << "if (entity.useGravity !== false) entity.vy = (entity.vy || 0) + (9.81 * (entity.gravityScale == null ? 1 : entity.gravityScale) * dt);\n";
        indent(); output << "entity.x = (entity.x || 0) + ((entity.vx || 0) * dt);\n";
        indent(); output << "entity.y = (entity.y || 0) + ((entity.vy || 0) * dt);\n";
        indent(); output << "entity.z = (entity.z || 0) + ((entity.vz || 0) * dt);\n";
        indent_level--;
        indent(); output << "}\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "if (this.onPostPhysics) this.onPostPhysics(dt);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "render(canvas) {\n";
        indent_level++;
        indent(); output << "this.load();\n";
        indent(); output << "if (this.onDraw) this.onDraw(canvas, this.interpolationAlpha());\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "setPaused(value) {\n";
        indent_level++;
        indent(); output << "this.paused = value;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "isLoaded() {\n";
        indent_level++;
        indent(); output << "return this._zenithLoaded;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "interpolationAlpha() {\n";
        indent_level++;
        indent(); output << "if (this.fixedDeltaTime <= 0) return 0.0;\n";
        indent(); output << "const alpha = this._zenithAccumulator / this.fixedDeltaTime;\n";
        indent(); output << "if (alpha < 0) return 0.0;\n";
        indent(); output << "if (alpha > 1) return 1.0;\n";
        indent(); output << "return alpha;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "totalFrames() {\n";
        indent_level++;
        indent(); output << "return this._zenithFrameCount;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "totalFixedSteps() {\n";
        indent_level++;
        indent(); output << "return this._zenithFixedStepCount;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "framesWithDroppedSteps() {\n";
        indent_level++;
        indent(); output << "return this._zenithDroppedStepFrames;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "lastSubstepCount() {\n";
        indent_level++;
        indent(); output << "return this._zenithLastFixedSteps;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "accumulatedTime() {\n";
        indent_level++;
        indent(); output << "return this._zenithAccumulator;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "frameDelta() {\n";
        indent_level++;
        indent(); output << "return this._zenithLastFrameDelta;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "inspectEntity(entity) {\n";
        indent_level++;
        indent(); output << "if (entity && this._zenithEntities.includes(entity)) {\n";
        indent_level++;
        indent(); output << "this._zenithInspectorEntity = entity;\n";
        indent(); output << "this._zenithInspectorMaterialPath = '';\n";
        indent(); output << "return;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this._zenithInspectorEntity = null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "inspectedEntity() {\n";
        indent_level++;
        indent(); output << "if (this._zenithInspectorEntity && this._zenithEntities.includes(this._zenithInspectorEntity)) return this._zenithInspectorEntity;\n";
        indent(); output << "return null;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "inspectMaterial(materialPath) {\n";
        indent_level++;
        indent(); output << "this._zenithInspectorMaterialPath = materialPath === undefined || materialPath === null ? '' : String(materialPath);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "inspectedMaterialPath() {\n";
        indent_level++;
        indent(); output << "return this._zenithInspectorMaterialPath || '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "clearInspectorTarget() {\n";
        indent_level++;
        indent(); output << "this._zenithInspectorEntity = null;\n";
        indent(); output << "this._zenithInspectorMaterialPath = '';\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "selectNextInspectorEntity() {\n";
        indent_level++;
        indent(); output << "if (this._zenithEntities.length === 0) {\n";
        indent_level++;
        indent(); output << "this._zenithInspectorEntity = null;\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const current = this.inspectedEntity();\n";
        indent(); output << "if (!current) {\n";
        indent_level++;
        indent(); output << "this.inspectEntity(this._zenithEntities[0]);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const index = this._zenithEntities.indexOf(current);\n";
        indent(); output << "if (index < 0 || index + 1 >= this._zenithEntities.length) {\n";
        indent_level++;
        indent(); output << "this.inspectEntity(this._zenithEntities[0]);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this.inspectEntity(this._zenithEntities[index + 1]);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "selectPreviousInspectorEntity() {\n";
        indent_level++;
        indent(); output << "if (this._zenithEntities.length === 0) {\n";
        indent_level++;
        indent(); output << "this._zenithInspectorEntity = null;\n";
        indent(); output << "return false;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const current = this.inspectedEntity();\n";
        indent(); output << "if (!current) {\n";
        indent_level++;
        indent(); output << "this.inspectEntity(this._zenithEntities[this._zenithEntities.length - 1]);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "const index = this._zenithEntities.indexOf(current);\n";
        indent(); output << "if (index <= 0) {\n";
        indent_level++;
        indent(); output << "this.inspectEntity(this._zenithEntities[this._zenithEntities.length - 1]);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";
        indent(); output << "this.inspectEntity(this._zenithEntities[index - 1]);\n";
        indent(); output << "return true;\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "init() {\n";
        indent_level++;
        indent(); output << "this.load();\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "update(dt) {\n";
        indent_level++;
        indent(); output << "this.updateFrame(dt);\n";
        indent_level--;
        indent(); output << "}\n";

        indent();
        output << "draw(canvas) {\n";
        indent_level++;
        indent(); output << "canvas.clear(this.clearColor || 'black');\n";
        indent(); output << "if (this.autoRenderWorld2D) this._zenithDrawWorld2D(canvas);\n";
        indent(); output << "this.render(canvas);\n";
        indent(); output << "if (this.debugOverlayEnabled === true && (this.debugDrawGrid2D || this.debugDrawColliders2D || this.debugDrawTransforms2D || this.debugDrawCameraBounds2D || this.debugDrawRuntimeStats)) this._zenithDrawDebugOverlay2D(canvas);\n";
        indent(); output << "if (this.minimalInspectorEnabled === true) this._zenithDrawInspector(canvas);\n";
        indent_level--;
        indent(); output << "}\n";
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
        output << "const active = document.activeElement;\n";
        indent();
        output << "const hasFocus = this.domElement.contains(active);\n";
        indent();
        output << "const path = hasFocus ? getElementPath(this.domElement, active) : null;\n";
        indent();
        output << "const selStart = hasFocus ? active.selectionStart : null;\n";
        indent();
        output << "const selEnd = hasFocus ? active.selectionEnd : null;\n";
        indent();
        output << "this.domElement.parentNode.replaceChild(newDom, this.domElement);\n";
        indent();
        output << "if (path) {\n";
        indent_level++;
        indent();
        output << "const elToFocus = getElementByPath(newDom, path);\n";
        indent();
        output << "if (elToFocus) {\n";
        indent_level++;
        indent();
        output << "elToFocus.focus();\n";
        indent();
        output << "if (selStart !== null && selEnd !== null) {\n";
        indent_level++;
        indent();
        output << "try { elToFocus.setSelectionRange(selStart, selEnd); } catch(e) {}\n";
        indent_level--;
        indent();
        output << "}\n";
        indent_level--;
        indent();
        output << "}\n";
        indent_level--;
        indent();
        output << "}\n";
        indent_level--;
        indent();
        output << "} else {\n";
        indent_level++;
        indent();
        output << "const root = document.getElementById('zenith-ui-root');\n";
        indent();
        output << "if (root) {\n";
        indent_level++;
        indent();
        output << "root.innerHTML = '';\n";
        indent();
        output << "root.appendChild(newDom);\n";
        indent_level--;
        indent();
        output << "}\n";
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
        std::string lib_path = node->attributes.count("library") ? node->attributes.at("library") : "";
        if (lib_path == "builtin") {
            indent();
            output << "function " << node->function_name << "(";
            for (size_t i = 0; i < node->parameters.size(); ++i) {
                output << node->parameters[i]->var_name;
                if (i < node->parameters.size() - 1) output << ", ";
            }
            output << ") {\n";
            indent_level++;
            std::string first_arg = node->parameters.empty() ? "arg0" : node->parameters[0]->var_name;
            std::string second_arg = node->parameters.size() > 1 ? node->parameters[1]->var_name : "arg1";
            if (node->function_name == "isKeyPressed") {
                indent(); output << "return !!window._zenith_keys[" << first_arg << "];\n";
            } else if (node->function_name == "wasKeyPressed") {
                indent(); output << "return !!window._zenith_justPressed[" << first_arg << "];\n";
            } else if (node->function_name == "wasKeyReleased") {
                indent(); output << "return !!window._zenith_justReleased[" << first_arg << "];\n";
            } else if (node->function_name == "getAxis") {
                indent(); output << "return (window._zenith_keys[" << second_arg << "] ? 1 : 0) - (window._zenith_keys[" << first_arg << "] ? 1 : 0);\n";
            } else if (node->function_name == "getMouseX") {
                indent(); output << "return window._zenith_mouseX || 0;\n";
            } else if (node->function_name == "getMouseY") {
                indent(); output << "return window._zenith_mouseY || 0;\n";
            } else {
                indent(); output << "console.warn('Unknown builtin foreign function: " << node->function_name << "');\n";
                indent(); output << "return 0;\n";
            }
            indent_level--;
            indent(); output << "}\n\n";
        }
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

std::string JSCodeGenerator::generate(ProgramNode* program, bool is_module_mode) {
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

    std::string page_title = "Zenith Web Application";
    std::string page_description = "Statically rendered Zenith web application.";
    std::string pre_rendered_html = is_module_mode ? "" : computePreRenderedApp(program, page_title, page_description);
    
    if (!is_module_mode) {
        // HTML Wrapper Shell
        output << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
        output << "    <meta charset=\"UTF-8\">\n";
        output << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        output << "    <title>" << escapeHtml(page_title) << "</title>\n";
        output << "    <meta name=\"description\" content=\"" << escapeAttribute(page_description) << "\">\n";
        output << "    <link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600;700&family=Fira+Code:wght@400;500&display=swap\" rel=\"stylesheet\">\n";
        output << "    <style>\n";
    } else {
        output << "// Zenith ES6 Module\n";
        output << "// Auto-generated by Zenith compiler\n\n";
        output << "if (typeof document !== 'undefined') {\n";
        output << "    const styleEl = document.createElement('style');\n";
        output << "    styleEl.innerHTML = `\n";
    }
    if (!is_module_mode) {
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
        output << "            box-sizing: border-box;\n";
        output << "        }\n";
        output << "        #zenith-ui-root {\n";
        output << "            display: flex;\n";
        output << "            justify-content: center;\n";
        output << "            align-items: center;\n";
        output << "            min-height: 250px;\n";
        output << "            width: 100%;\n";
        output << "        }\n";
    }
    output << "        .zenith-column {\n";
    output << "            display: flex;\n";
    output << "            flex-direction: column;\n";
    output << "            gap: 16px;\n";
    output << "            width: 100%;\n";
    output << "            box-sizing: border-box;\n";
    output << "        }\n";
    output << "        .zenith-row {\n";
    output << "            display: flex;\n";
    output << "            flex-direction: row;\n";
    output << "            gap: 16px;\n";
    output << "            align-items: center;\n";
    output << "            width: 100%;\n";
    output << "            flex-wrap: wrap;\n";
    output << "            box-sizing: border-box;\n";
    output << "        }\n";
    output << "        .zenith-text {\n";
    output << "            font-size: 1rem;\n";
    output << "            line-height: 1.5;\n";
    output << "        }\n";;
    output << "        .zenith-input {\n";
    output << "            background: rgba(15, 23, 42, 0.6);\n";
    output << "            border: 1px solid rgba(0, 242, 254, 0.3);\n";
    output << "            border-radius: 8px;\n";
    output << "            padding: 8px 12px;\n";
    output << "            color: #f8fafc;\n";
    output << "            font-family: inherit;\n";
    output << "            font-size: 0.95rem;\n";
    output << "            min-width: 0;\n";
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
    if (!is_module_mode) {
        output << "    </style>\n";
    } else {
        output << "    `;\n";
        output << "    document.head.appendChild(styleEl);\n";
        output << "}\n\n";
    }

    // ----------------------------------------------------------------
    // CDN/npm library injection — collect import cdn/npm from AST
    // ----------------------------------------------------------------
    for (const auto& stmt : program->statements) {
        if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            if (imp->isActiveFor("web")) {
                if (imp->kind == ImportNode::ImportKind::Cdn || imp->kind == ImportNode::ImportKind::Npm) {
                    std::string url = imp->cdn_url;
                    if (url.length() < 5 || url.substr(url.length() - 5) != ".wasm") {
                        if (!is_module_mode) {
                            // Emit <script src="..."> in <head> before any app code runs
                            output << "    <!-- Zenith Library: " << imp->module_name << " -->\n";
                            output << "    <script src=\"" << imp->cdn_url << "\" crossorigin=\"anonymous\"></script>\n";
                        } else {
                            output << "if (typeof document !== 'undefined') {\n";
                            output << "    const s = document.createElement('script');\n";
                            output << "    s.src = \"" << imp->cdn_url << "\";\n";
                            output << "    s.crossOrigin = 'anonymous';\n";
                            output << "    document.head.appendChild(s);\n";
                            output << "}\n";
                        }
                    }
                }
            }
        }
    }

    // WASM string/module loading runtime helper injections
    if (!is_module_mode) {
        output << "    <script>\n";
    }
    output << "        // Helper to compute element path for focus preservation\n";
    output << "        function getElementPath(root, el) {\n";
    output << "            const path = [];\n";
    output << "            let current = el;\n";
    output << "            while (current && current !== root) {\n";
    output << "                let parent = current.parentNode;\n";
    output << "                if (!parent) break;\n";
    output << "                let index = Array.prototype.indexOf.call(parent.childNodes, current);\n";
    output << "                path.unshift(index);\n";
    output << "                current = parent;\n";
    output << "            }\n";
    output << "            return current === root ? path : null;\n";
    output << "        }\n\n";
    output << "        // Helper to find element by path for focus preservation\n";
    output << "        function getElementByPath(root, path) {\n";
    output << "            if (!path) return null;\n";
    output << "            let current = root;\n";
    output << "            for (let index of path) {\n";
    output << "                if (!current || !current.childNodes || index >= current.childNodes.length) {\n";
    output << "                    return null;\n";
    output << "                }\n";
    output << "                current = current.childNodes[index];\n";
    output << "            }\n";
    output << "            return current;\n";
    output << "        }\n\n";
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
    output << "                console.log('[WASM] Raw WASM module loaded from ' + url);\n";
    output << "            } catch (err) {\n";
    output << "                console.error('[WASM Error] Failed to load ' + url + ': ' + err);\n";
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
    output << "                console.log('[WASM] Dart WASM module loaded from ' + wasmUrl);\n";
    output << "            } catch (err) {\n";
    output << "                console.warn('[WASM Warning] Failed to load Dart WASM via JS module, trying raw loader: ' + err);\n";
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
    if (!is_module_mode) {
        output << "    </script>\n";
        output << "</head>\n<body>\n";
        output << "    <div id=\"zenith-ui-root\">" << pre_rendered_html << "</div>\n\n";
        output << "    <script>\n";
    }
    output << "        // Platform Detection Constants\n";
    output << "        const isAndroid = false;\n";
    output << "        const isIos = false;\n";
    output << "        const isMac = false;\n";
    output << "        const isLinux = false;\n";
    output << "        const isWeb = true;\n";
    output << "        const isWindows = false;\n\n";
    output << "        // Form Validation Helpers\n";
    output << "        function isEmail(val) {\n";
    output << "            const re = /^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$/;\n";
    output << "            return re.test(val) ? \"\" : \"Invalid email address\";\n";
    output << "        }\n";
    output << "        function isRequired(val) {\n";
    output << "            return val && val.trim().length > 0 ? \"\" : \"This field is required\";\n";
    output << "        }\n\n";
    output << "        // Navigation Helper\n";
    output << "        function navigate(path) {\n";
    output << "            if (typeof window !== 'undefined' && window.zenithRouter) {\n";
    output << "                window.zenithRouter.navigate(path);\n";
    output << "            }\n";
    output << "        }\n\n";
    output << "        // SPA Client-Side Router\n";
    output << "        class ZenithRouter {\n";
    output << "            constructor() {\n";
    output << "                this.routes = {};\n";
    output << "                window.addEventListener('popstate', () => this.handleRoute());\n";
    output << "                document.addEventListener('click', (e) => {\n";
    output << "                    const link = e.target.closest('a');\n";
    output << "                    if (link && link.getAttribute('href') && link.getAttribute('href').startsWith('/')) {\n";
    output << "                        e.preventDefault();\n";
    output << "                        this.navigate(link.getAttribute('href'));\n";
    output << "                    }\n";
    output << "                });\n";
    output << "            }\n";
    output << "            register(path, componentClass) {\n";
    output << "                this.routes[path] = componentClass;\n";
    output << "            }\n";
    output << "            navigate(path) {\n";
    output << "                window.history.pushState({}, '', path);\n";
    output << "                this.handleRoute();\n";
    output << "            }\n";
    output << "            handleRoute() {\n";
    output << "                const path = window.location.pathname || '/';\n";
    output << "                const componentClass = this.routes[path] || this.routes['/'] || Object.values(this.routes)[0];\n";
    output << "                if (componentClass) {\n";
    output << "                    const inst = new componentClass();\n";
    output << "                    inst.render();\n";
    output << "                } else {\n";
    output << "                    console.warn('[Router] No routed class registered.');\n";
    output << "                }\n";
    output << "            }\n";
    output << "        }\n";
    output << "        window.zenithRouter = new ZenithRouter();\n\n";
    output << "        const zenith = {\n";
    output << "            runGameLoop: function(game) {\n";
    output << "                if (game.init) game.init();\n";
    output << "                const root = document.getElementById('zenith-ui-root');\n";
    output << "                root.innerHTML = '';\n";
    output << "                const canvasEl = document.createElement('canvas');\n";
    output << "                canvasEl.width = 640;\n";
    output << "                canvasEl.height = 480;\n";
    output << "                canvasEl.style.display = 'block';\n";
    output << "                canvasEl.style.margin = '20px auto';\n";
    output << "                canvasEl.style.background = '#000000';\n";
    output << "                canvasEl.style.borderRadius = '12px';\n";
    output << "                canvasEl.style.boxShadow = '0 10px 30px rgba(0, 0, 0, 0.5)';\n";
    output << "                root.appendChild(canvasEl);\n";
    output << "                const ctx = canvasEl.getContext('2d');\n";
    output << "                const canvas = {\n";
    output << "                    width: canvasEl.width / 8,\n";
    output << "                    height: canvasEl.height / 20,\n";
    output << "                    clear: function(color = 'black') {\n";
    output << "                        ctx.fillStyle = color;\n";
    output << "                        ctx.fillRect(0, 0, canvasEl.width, canvasEl.height);\n";
    output << "                    },\n";
    output << "                    drawRect: function(x, y, w, h, color = 'white') {\n";
    output << "                        ctx.fillStyle = color;\n";
    output << "                        ctx.fillRect(x * 8, y * 20, w * 8, h * 20);\n";
    output << "                    },\n";
    output << "                    drawCircle: function(cx, cy, r, color = 'white') {\n";
    output << "                        ctx.fillStyle = color;\n";
    output << "                        ctx.beginPath();\n";
    output << "                        ctx.arc(cx * 8, cy * 20, r * 14, 0, Math.PI * 2);\n";
    output << "                        ctx.fill();\n";
    output << "                    },\n";
    output << "                    drawPoint: function(x, y, color = 'white') {\n";
    output << "                        ctx.fillStyle = color;\n";
    output << "                        ctx.fillRect((x * 8) - 2, (y * 20) - 2, 4, 4);\n";
    output << "                    },\n";
    output << "                    drawLine: function(x1, y1, x2, y2, color = 'white') {\n";
    output << "                        ctx.strokeStyle = color;\n";
    output << "                        ctx.lineWidth = 1.5;\n";
    output << "                        ctx.beginPath();\n";
    output << "                        ctx.moveTo(x1 * 8, y1 * 20);\n";
    output << "                        ctx.lineTo(x2 * 8, y2 * 20);\n";
    output << "                        ctx.stroke();\n";
    output << "                    },\n";
    output << "                    drawFrameRect: function(x, y, w, h, color = 'white') {\n";
    output << "                        ctx.strokeStyle = color;\n";
    output << "                        ctx.lineWidth = 1.5;\n";
    output << "                        ctx.strokeRect(x * 8, y * 20, w * 8, h * 20);\n";
    output << "                    },\n";
    output << "                    drawCircleOutline: function(cx, cy, r, color = 'white') {\n";
    output << "                        ctx.strokeStyle = color;\n";
    output << "                        ctx.lineWidth = 1.5;\n";
    output << "                        ctx.beginPath();\n";
    output << "                        ctx.arc(cx * 8, cy * 20, r * 14, 0, Math.PI * 2);\n";
    output << "                        ctx.stroke();\n";
    output << "                    },\n";
    output << "                    drawText: function(text, x, y, color = 'white') {\n";
    output << "                        ctx.fillStyle = color;\n";
    output << "                        ctx.font = '16px monospace';\n";
    output << "                        ctx.fillText(text, x * 8, y * 20 + 12);\n";
    output << "                    },\n";
    output << "                    present: function() {}\n";
    output << "                };\n";
    output << "                window._zenith_keys = {};\n";
    output << "                window._zenith_justPressed = {};\n";
    output << "                window._zenith_justReleased = {};\n";
    output << "                window.addEventListener('keydown', (e) => {\n";
    output << "                    if (!window._zenith_keys[e.key]) window._zenith_justPressed[e.key] = true;\n";
    output << "                    window._zenith_keys[e.key] = true;\n";
    output << "                });\n";
    output << "                window.addEventListener('keyup', (e) => {\n";
    output << "                    if (window._zenith_keys[e.key]) window._zenith_justReleased[e.key] = true;\n";
    output << "                    window._zenith_keys[e.key] = false;\n";
    output << "                });\n";
    output << "                window._zenith_mouseX = 0;\n";
    output << "                window._zenith_mouseY = 0;\n";
    output << "                canvasEl.addEventListener('mousemove', (e) => {\n";
    output << "                    const rect = canvasEl.getBoundingClientRect();\n";
    output << "                    window._zenith_mouseX = (e.clientX - rect.left) / 8.0;\n";
    output << "                    window._zenith_mouseY = (e.clientY - rect.top) / 20.0;\n";
    output << "                });\n";
    output << "                let lastTime = performance.now();\n";
    output << "                const loop = (time) => {\n";
    output << "                    const dt = (time - lastTime) / 1000;\n";
    output << "                    lastTime = time;\n";
    output << "                    if (game.update) game.update(dt);\n";
    output << "                    canvas.clear('black');\n";
    output << "                    if (game.draw) game.draw(canvas);\n";
    output << "                    window._zenith_justPressed = {};\n";
    output << "                    window._zenith_justReleased = {};\n";
    output << "                    requestAnimationFrame(loop);\n";
    output << "                };\n";
    output << "                requestAnimationFrame(loop);\n";
    output << "            },\n";
    output << "            consoleElement: null,\n";
    output << "            print: function(msg) {\n";
    output << "                if (!this.consoleElement) this.consoleElement = document.getElementById('zenith-console');\n";
    output << "                if (this.consoleElement) {\n";
    output << "                    this.consoleElement.textContent += msg;\n";
    output << "                    this.consoleElement.scrollTop = this.consoleElement.scrollHeight;\n";
    output << "                }\n";
    output << "                console.log(msg);\n";
    output << "            },\n";
    output << "            println: function(msg) {\n";
    output << "                this.print(msg + '\\n');\n";
    output << "            },\n";
    output << "            makeVec2: function(x = 0, y = 0) {\n";
    output << "                return {\n";
    output << "                    __zenithType: 'Vec2',\n";
    output << "                    x: x,\n";
    output << "                    y: y,\n";
    output << "                    length: function() { return Math.hypot(this.x, this.y); },\n";
    output << "                    lengthSquared: function() { return this.x * this.x + this.y * this.y; },\n";
    output << "                    normalized: function() {\n";
    output << "                        const len = this.length();\n";
    output << "                        return len > 0.0001 ? zenith.makeVec2(this.x / len, this.y / len) : zenith.makeVec2(0, 0);\n";
    output << "                    }\n";
    output << "                };\n";
    output << "            },\n";
    output << "            makeVec3: function(x = 0, y = 0, z = 0) {\n";
    output << "                return {\n";
    output << "                    __zenithType: 'Vec3',\n";
    output << "                    x: x,\n";
    output << "                    y: y,\n";
    output << "                    z: z,\n";
    output << "                    length: function() { return Math.hypot(this.x, this.y, this.z); },\n";
    output << "                    lengthSquared: function() { return this.x * this.x + this.y * this.y + this.z * this.z; },\n";
    output << "                    normalized: function() {\n";
    output << "                        const len = this.length();\n";
    output << "                        return len > 0.0001 ? zenith.makeVec3(this.x / len, this.y / len, this.z / len) : zenith.makeVec3(0, 0, 0);\n";
    output << "                    }\n";
    output << "                };\n";
    output << "            },\n";
    output << "            makeVec4: function(x = 0, y = 0, z = 0, w = 0) {\n";
    output << "                return {\n";
    output << "                    __zenithType: 'Vec4',\n";
    output << "                    x: x,\n";
    output << "                    y: y,\n";
    output << "                    z: z,\n";
    output << "                    w: w,\n";
    output << "                    length: function() { return Math.hypot(this.x, this.y, this.z, this.w); },\n";
    output << "                    lengthSquared: function() { return this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w; },\n";
    output << "                    normalized: function() {\n";
    output << "                        const len = this.length();\n";
    output << "                        return len > 0.0001 ? zenith.makeVec4(this.x / len, this.y / len, this.z / len, this.w / len) : zenith.makeVec4(0, 0, 0, 0);\n";
    output << "                    }\n";
    output << "                };\n";
    output << "            },\n";
    output << "            makeMat4: function(m00 = 1, m01 = 0, m02 = 0, m03 = 0, m10 = 0, m11 = 1, m12 = 0, m13 = 0, m20 = 0, m21 = 0, m22 = 1, m23 = 0, m30 = 0, m31 = 0, m32 = 0, m33 = 1) {\n";
    output << "                return {\n";
    output << "                    __zenithType: 'Mat4',\n";
    output << "                    m00: m00, m01: m01, m02: m02, m03: m03,\n";
    output << "                    m10: m10, m11: m11, m12: m12, m13: m13,\n";
    output << "                    m20: m20, m21: m21, m22: m22, m23: m23,\n";
    output << "                    m30: m30, m31: m31, m32: m32, m33: m33\n";
    output << "                };\n";
    output << "            },\n";
    output << "            mathEqual: function(lhs, rhs) {\n";
    output << "                if (!lhs || !rhs || lhs.__zenithType !== rhs.__zenithType) return lhs === rhs;\n";
    output << "                if (lhs.__zenithType === 'Vec2') return lhs.x === rhs.x && lhs.y === rhs.y;\n";
    output << "                if (lhs.__zenithType === 'Vec3') return lhs.x === rhs.x && lhs.y === rhs.y && lhs.z === rhs.z;\n";
    output << "                if (lhs.__zenithType === 'Vec4') return lhs.x === rhs.x && lhs.y === rhs.y && lhs.z === rhs.z && lhs.w === rhs.w;\n";
    output << "                if (lhs.__zenithType === 'Mat4') {\n";
    output << "                    return lhs.m00 === rhs.m00 && lhs.m01 === rhs.m01 && lhs.m02 === rhs.m02 && lhs.m03 === rhs.m03 &&\n";
    output << "                           lhs.m10 === rhs.m10 && lhs.m11 === rhs.m11 && lhs.m12 === rhs.m12 && lhs.m13 === rhs.m13 &&\n";
    output << "                           lhs.m20 === rhs.m20 && lhs.m21 === rhs.m21 && lhs.m22 === rhs.m22 && lhs.m23 === rhs.m23 &&\n";
    output << "                           lhs.m30 === rhs.m30 && lhs.m31 === rhs.m31 && lhs.m32 === rhs.m32 && lhs.m33 === rhs.m33;\n";
    output << "                }\n";
    output << "                return lhs === rhs;\n";
    output << "            },\n";
    output << "            mathAdd: function(lhs, rhs) {\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Vec2' && rhs.__zenithType === 'Vec2') return zenith.makeVec2(lhs.x + rhs.x, lhs.y + rhs.y);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Vec3' && rhs.__zenithType === 'Vec3') return zenith.makeVec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Vec4' && rhs.__zenithType === 'Vec4') return zenith.makeVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Mat4' && rhs.__zenithType === 'Mat4') {\n";
    output << "                    return zenith.makeMat4(lhs.m00 + rhs.m00, lhs.m01 + rhs.m01, lhs.m02 + rhs.m02, lhs.m03 + rhs.m03, lhs.m10 + rhs.m10, lhs.m11 + rhs.m11, lhs.m12 + rhs.m12, lhs.m13 + rhs.m13, lhs.m20 + rhs.m20, lhs.m21 + rhs.m21, lhs.m22 + rhs.m22, lhs.m23 + rhs.m23, lhs.m30 + rhs.m30, lhs.m31 + rhs.m31, lhs.m32 + rhs.m32, lhs.m33 + rhs.m33);\n";
    output << "                }\n";
    output << "                return lhs + rhs;\n";
    output << "            },\n";
    output << "            mathSub: function(lhs, rhs) {\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Vec2' && rhs.__zenithType === 'Vec2') return zenith.makeVec2(lhs.x - rhs.x, lhs.y - rhs.y);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Vec3' && rhs.__zenithType === 'Vec3') return zenith.makeVec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Vec4' && rhs.__zenithType === 'Vec4') return zenith.makeVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Mat4' && rhs.__zenithType === 'Mat4') {\n";
    output << "                    return zenith.makeMat4(lhs.m00 - rhs.m00, lhs.m01 - rhs.m01, lhs.m02 - rhs.m02, lhs.m03 - rhs.m03, lhs.m10 - rhs.m10, lhs.m11 - rhs.m11, lhs.m12 - rhs.m12, lhs.m13 - rhs.m13, lhs.m20 - rhs.m20, lhs.m21 - rhs.m21, lhs.m22 - rhs.m22, lhs.m23 - rhs.m23, lhs.m30 - rhs.m30, lhs.m31 - rhs.m31, lhs.m32 - rhs.m32, lhs.m33 - rhs.m33);\n";
    output << "                }\n";
    output << "                return lhs - rhs;\n";
    output << "            },\n";
    output << "            mathMul: function(lhs, rhs) {\n";
    output << "                if (lhs && lhs.__zenithType === 'Vec2' && typeof rhs === 'number') return zenith.makeVec2(lhs.x * rhs, lhs.y * rhs);\n";
    output << "                if (typeof lhs === 'number' && rhs && rhs.__zenithType === 'Vec2') return zenith.makeVec2(lhs * rhs.x, lhs * rhs.y);\n";
    output << "                if (lhs && lhs.__zenithType === 'Vec3' && typeof rhs === 'number') return zenith.makeVec3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);\n";
    output << "                if (typeof lhs === 'number' && rhs && rhs.__zenithType === 'Vec3') return zenith.makeVec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);\n";
    output << "                if (lhs && lhs.__zenithType === 'Vec4' && typeof rhs === 'number') return zenith.makeVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);\n";
    output << "                if (typeof lhs === 'number' && rhs && rhs.__zenithType === 'Vec4') return zenith.makeVec4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);\n";
    output << "                if (lhs && lhs.__zenithType === 'Mat4' && typeof rhs === 'number') return zenith.makeMat4(lhs.m00 * rhs, lhs.m01 * rhs, lhs.m02 * rhs, lhs.m03 * rhs, lhs.m10 * rhs, lhs.m11 * rhs, lhs.m12 * rhs, lhs.m13 * rhs, lhs.m20 * rhs, lhs.m21 * rhs, lhs.m22 * rhs, lhs.m23 * rhs, lhs.m30 * rhs, lhs.m31 * rhs, lhs.m32 * rhs, lhs.m33 * rhs);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Mat4' && rhs.__zenithType === 'Vec4') return zenith.makeVec4(lhs.m00 * rhs.x + lhs.m01 * rhs.y + lhs.m02 * rhs.z + lhs.m03 * rhs.w, lhs.m10 * rhs.x + lhs.m11 * rhs.y + lhs.m12 * rhs.z + lhs.m13 * rhs.w, lhs.m20 * rhs.x + lhs.m21 * rhs.y + lhs.m22 * rhs.z + lhs.m23 * rhs.w, lhs.m30 * rhs.x + lhs.m31 * rhs.y + lhs.m32 * rhs.z + lhs.m33 * rhs.w);\n";
    output << "                if (lhs && rhs && lhs.__zenithType === 'Mat4' && rhs.__zenithType === 'Mat4') {\n";
    output << "                    return zenith.makeMat4(\n";
    output << "                        lhs.m00 * rhs.m00 + lhs.m01 * rhs.m10 + lhs.m02 * rhs.m20 + lhs.m03 * rhs.m30,\n";
    output << "                        lhs.m00 * rhs.m01 + lhs.m01 * rhs.m11 + lhs.m02 * rhs.m21 + lhs.m03 * rhs.m31,\n";
    output << "                        lhs.m00 * rhs.m02 + lhs.m01 * rhs.m12 + lhs.m02 * rhs.m22 + lhs.m03 * rhs.m32,\n";
    output << "                        lhs.m00 * rhs.m03 + lhs.m01 * rhs.m13 + lhs.m02 * rhs.m23 + lhs.m03 * rhs.m33,\n";
    output << "                        lhs.m10 * rhs.m00 + lhs.m11 * rhs.m10 + lhs.m12 * rhs.m20 + lhs.m13 * rhs.m30,\n";
    output << "                        lhs.m10 * rhs.m01 + lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21 + lhs.m13 * rhs.m31,\n";
    output << "                        lhs.m10 * rhs.m02 + lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22 + lhs.m13 * rhs.m32,\n";
    output << "                        lhs.m10 * rhs.m03 + lhs.m11 * rhs.m13 + lhs.m12 * rhs.m23 + lhs.m13 * rhs.m33,\n";
    output << "                        lhs.m20 * rhs.m00 + lhs.m21 * rhs.m10 + lhs.m22 * rhs.m20 + lhs.m23 * rhs.m30,\n";
    output << "                        lhs.m20 * rhs.m01 + lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21 + lhs.m23 * rhs.m31,\n";
    output << "                        lhs.m20 * rhs.m02 + lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22 + lhs.m23 * rhs.m32,\n";
    output << "                        lhs.m20 * rhs.m03 + lhs.m21 * rhs.m13 + lhs.m22 * rhs.m23 + lhs.m23 * rhs.m33,\n";
    output << "                        lhs.m30 * rhs.m00 + lhs.m31 * rhs.m10 + lhs.m32 * rhs.m20 + lhs.m33 * rhs.m30,\n";
    output << "                        lhs.m30 * rhs.m01 + lhs.m31 * rhs.m11 + lhs.m32 * rhs.m21 + lhs.m33 * rhs.m31,\n";
    output << "                        lhs.m30 * rhs.m02 + lhs.m31 * rhs.m12 + lhs.m32 * rhs.m22 + lhs.m33 * rhs.m32,\n";
    output << "                        lhs.m30 * rhs.m03 + lhs.m31 * rhs.m13 + lhs.m32 * rhs.m23 + lhs.m33 * rhs.m33\n";
    output << "                    );\n";
    output << "                }\n";
    output << "                return lhs * rhs;\n";
    output << "            },\n";
    output << "            mathDiv: function(lhs, rhs) {\n";
    output << "                if (lhs && lhs.__zenithType === 'Vec2' && typeof rhs === 'number') return zenith.makeVec2(lhs.x / rhs, lhs.y / rhs);\n";
    output << "                if (lhs && lhs.__zenithType === 'Vec3' && typeof rhs === 'number') return zenith.makeVec3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);\n";
    output << "                if (lhs && lhs.__zenithType === 'Vec4' && typeof rhs === 'number') return zenith.makeVec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);\n";
    output << "                if (lhs && lhs.__zenithType === 'Mat4' && typeof rhs === 'number') return zenith.makeMat4(lhs.m00 / rhs, lhs.m01 / rhs, lhs.m02 / rhs, lhs.m03 / rhs, lhs.m10 / rhs, lhs.m11 / rhs, lhs.m12 / rhs, lhs.m13 / rhs, lhs.m20 / rhs, lhs.m21 / rhs, lhs.m22 / rhs, lhs.m23 / rhs, lhs.m30 / rhs, lhs.m31 / rhs, lhs.m32 / rhs, lhs.m33 / rhs);\n";
    output << "                return lhs / rhs;\n";
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
    output << "                if (attrs.fontSize) el.style.fontSize = typeof attrs.fontSize === 'number' ? attrs.fontSize + 'px' : attrs.fontSize;\n";
    output << "                if (attrs.borderRadius) el.style.borderRadius = typeof attrs.borderRadius === 'number' ? attrs.borderRadius + 'px' : attrs.borderRadius;\n";
    output << "                if (attrs.border) el.style.border = attrs.border;\n";
    output << "                if (attrs.opacity !== undefined) el.style.opacity = attrs.opacity;\n";
    output << "                if (attrs.cursor) el.style.cursor = attrs.cursor;\n";
    output << "                if (attrs.display) el.style.display = attrs.display;\n";
    output << "                if (attrs.boxShadow) el.style.boxShadow = attrs.boxShadow;\n";
    output << "                if (attrs.alignSelf) el.style.alignSelf = attrs.alignSelf;\n";
    output << "                if (attrs.fontFamily) el.style.fontFamily = attrs.fontFamily;\n";
    output << "                if (attrs.transition) el.style.transition = attrs.transition;\n";
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
    output << "                const container = document.createElement('div');\n";
    output << "                container.style.display = 'flex';\n";
    output << "                container.style.flexDirection = 'column';\n";
    output << "                container.style.gap = '4px';\n";
    output << "                container.style.width = '100%';\n";
    output << "                const el = document.createElement('input');\n";
    output << "                el.type = 'text';\n";
    output << "                el.placeholder = Array.isArray(placeholder) ? placeholder.join('') : placeholder;\n";
    output << "                el.className = 'zenith-input';\n";
    output << "                el.style.width = '100%';\n";
    output << "                if (attrs.value) el.value = attrs.value;\n";
    output << "                const errorSpan = document.createElement('span');\n";
    output << "                errorSpan.style.color = '#ef4444';\n";
    output << "                errorSpan.style.fontSize = '0.8rem';\n";
    output << "                errorSpan.style.display = 'none';\n";
    output << "                container.appendChild(el);\n";
    output << "                container.appendChild(errorSpan);\n";
    output << "                UI.applyStyles(el, attrs);\n";
    output << "                if (attrs.validator && typeof attrs.validator === 'function') {\n";
    output << "                    const validate = () => {\n";
    output << "                        const err = attrs.validator(el.value);\n";
    output << "                        if (err && typeof err === 'string' && err.length > 0) {\n";
    output << "                            el.style.borderColor = '#ef4444';\n";
    output << "                            el.style.boxShadow = '0 0 0 1px #ef4444';\n";
    output << "                            errorSpan.textContent = err;\n";
    output << "                            errorSpan.style.display = 'block';\n";
    output << "                        } else if (err === false) {\n";
    output << "                            el.style.borderColor = '#ef4444';\n";
    output << "                            el.style.boxShadow = '0 0 0 1px #ef4444';\n";
    output << "                            errorSpan.textContent = 'Invalid input';\n";
    output << "                            errorSpan.style.display = 'block';\n";
    output << "                        } else {\n";
    output << "                            el.style.borderColor = '';\n";
    output << "                            el.style.boxShadow = '';\n";
    output << "                            errorSpan.style.display = 'none';\n";
    output << "                            errorSpan.textContent = '';\n";
    output << "                        }\n";
    output << "                    };\n";
    output << "                    el.oninput = function() {\n";
    output << "                        validate();\n";
    output << "                        if (attrs.onChange && typeof attrs.onChange === 'function') {\n";
    output << "                            attrs.onChange(el.value);\n";
    output << "                        }\n";
    output << "                    };\n";
    output << "                    if (el.value) {\n";
    output << "                        validate();\n";
    output << "                    }\n";
    output << "                } else {\n";
    output << "                    if (attrs.onChange && typeof attrs.onChange === 'function') {\n";
    output << "                        el.oninput = function() { attrs.onChange(el.value); };\n";
    output << "                    }\n";
    output << "                }\n";
    output << "                container.render = function() {\n";
    output << "                    UI.render(container);\n";
    output << "                    return container;\n";
    output << "                };\n";
    output << "                return container;\n";
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
    
    // Register routed components
    std::vector<std::pair<std::string, std::string>> routes;
    for (const auto& stmt : program->statements) {
        if (auto* class_decl = dynamic_cast<ClassDeclNode*>(stmt.get())) {
            if (class_decl->is_routed) {
                routes.push_back({class_decl->route_path, class_decl->class_name});
            }
        }
    }
    
    if (!is_module_mode) {
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
        if (!routes.empty()) {
            for (const auto& route : routes) {
                output << "            window.zenithRouter.register(\"" << route.first << "\", " << route.second << ");\n";
            }
            output << "            window.zenithRouter.handleRoute();\n";
        } else {
            output << "            main();\n";
        }
        output << "        };\n";
        output << "    </script>\n";
        output << "</body>\n</html>\n";
    } else {
        output << "        // Define fallback mocks for modules\n";
        output << "        if (typeof window !== 'undefined') {\n";
        output << "            if (typeof window.encrypt === 'undefined') {\n";
        output << "                window.encrypt = function(plaintext, key) {\n";
        output << "                    let result = '';\n";
        output << "                    for (let i = 0; i < plaintext.length; i++) {\n";
        output << "                        result += String.fromCharCode(plaintext.charCodeAt(i) ^ key.charCodeAt(i % key.length));\n";
        output << "                    }\n";
        output << "                    return btoa(result);\n";
        output << "                };\n";
        output << "            }\n";
        output << "            if (typeof window.decrypt === 'undefined') {\n";
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
        output << "            if (typeof window.deriveKey === 'undefined') {\n";
        output << "                window.deriveKey = function(password, salt) {\n";
        output << "                    return btoa(password + ':' + salt);\n";
        output << "                };\n";
        output << "            }\n";
        output << "            if (typeof window.println === 'undefined') window.println = (msg) => zenith.println(msg);\n";
        output << "            if (typeof window.print === 'undefined') window.print = (msg) => zenith.print(msg);\n";
        output << "        }\n\n";

        output << "// ES6 Module Exports\n";
        output << "export { UI, zenith, httpGet, httpPost, isAndroid, isIos, isMac, isLinux, isWeb, isWindows";
        for (const auto& c : class_names) {
            output << ", " << c;
        }
        for (const auto& f : function_names) {
            output << ", " << f;
        }
        output << " };\n";
    }
    
    return output.str();
}
