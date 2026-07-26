#ifndef ZENITH_GUI_H
#define ZENITH_GUI_H

#include "zenith_window.h"
#include "zenith_texture.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace zenith {

struct EdgeInsets {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;

    static EdgeInsets all(float value) { return {value, value, value, value}; }
    static EdgeInsets symmetric(float vertical, float horizontal) { return {horizontal, vertical, horizontal, vertical}; }
};

class Widget {
public:
    virtual ~Widget() = default;
    virtual void render(float x, float y, float availableWidth, float availableHeight) = 0;
};

using WidgetPtr = std::shared_ptr<Widget>;

// --- Container Widget (like Flutter Container) ---
class Container : public Widget {
public:
    float width = 0.0f;  // 0 = fit parent/child
    float height = 0.0f;
    Color color = Color::Clear();
    Color borderColor = Color::Clear();
    float borderWidth = 0.0f;
    EdgeInsets padding;
    EdgeInsets margin;
    WidgetPtr child = nullptr;

    Container() = default;

    void render(float x, float y, float availableWidth, float availableHeight) override {
        float renderX = x + margin.left;
        float renderY = y + margin.top;
        float renderW = (width > 0.0f) ? width : (availableWidth - margin.left - margin.right);
        float renderH = (height > 0.0f) ? height : (availableHeight - margin.top - margin.bottom);

        // Draw background
        if (color.a > 0.0f) {
            Renderer2D::drawRect(renderX, renderY, renderW, renderH, color);
        }
        // Draw border
        if (borderWidth > 0.0f && borderColor.a > 0.0f) {
            Renderer2D::drawRectOutline(renderX, renderY, renderW, renderH, borderColor, borderWidth);
        }

        // Render child inside padding
        if (child) {
            float childX = renderX + padding.left;
            float childY = renderY + padding.top;
            float childW = renderW - padding.left - padding.right;
            float childH = renderH - padding.top - padding.bottom;
            child->render(childX, childY, childW, childH);
        }
    }
};

// --- Stack Widget (Overlays children on top of each other) ---
class Stack : public Widget {
public:
    std::vector<WidgetPtr> children;

    Stack() = default;
    explicit Stack(const std::vector<WidgetPtr>& children) : children(children) {}

    void addChild(WidgetPtr child) { children.push_back(child); }

    void render(float x, float y, float availableWidth, float availableHeight) override {
        for (const auto& child : children) {
            if (child) {
                child->render(x, y, availableWidth, availableHeight);
            }
        }
    }
};

// --- Column Widget (Stacks children vertically) ---
class Column : public Widget {
public:
    std::vector<WidgetPtr> children;
    float spacing = 8.0f;

    Column() = default;
    explicit Column(const std::vector<WidgetPtr>& children, float spacing = 8.0f) : children(children), spacing(spacing) {}

    void addChild(WidgetPtr child) { children.push_back(child); }

    void render(float x, float y, float availableWidth, float availableHeight) override {
        float currentY = y;
        for (const auto& child : children) {
            if (child) {
                child->render(x, currentY, availableWidth, availableHeight);
                currentY += 40.0f + spacing; // estimated element height offset
            }
        }
    }
};

// --- Row Widget (Arranges children horizontally) ---
class Row : public Widget {
public:
    std::vector<WidgetPtr> children;
    float spacing = 8.0f;

    Row() = default;
    explicit Row(const std::vector<WidgetPtr>& children, float spacing = 8.0f) : children(children), spacing(spacing) {}

    void addChild(WidgetPtr child) { children.push_back(child); }

    void render(float x, float y, float availableWidth, float availableHeight) override {
        float currentX = x;
        for (const auto& child : children) {
            if (child) {
                child->render(currentX, y, availableWidth, availableHeight);
                currentX += 80.0f + spacing; // estimated element width offset
            }
        }
    }
};

// --- Image Widget ---
class ImageWidget : public Widget {
public:
    Texture2D texture;
    float width = 0.0f;
    float height = 0.0f;
    Color tint = Color::White();

    ImageWidget() = default;
    ImageWidget(Texture2D tex, float w, float h, Color tint = Color::White())
        : texture(tex), width(w), height(h), tint(tint) {}

    void render(float x, float y, float availableWidth, float availableHeight) override {
        float renderW = (width > 0.0f) ? width : availableWidth;
        float renderH = (height > 0.0f) ? height : availableHeight;
        TextureManager::drawSprite(texture, x, y, renderW, renderH, tint);
    }
};

// --- Button Widget with Action Handler ---
class ButtonWidget : public Widget {
public:
    std::string label;
    WidgetPtr child = nullptr;
    Color baseColor = Color{0.2f, 0.4f, 0.8f, 1.0f};
    std::function<void()> onClick;

    ButtonWidget() = default;
    ButtonWidget(const std::string& label, std::function<void()> onClick)
        : label(label), onClick(onClick) {}

    void render(float x, float y, float availableWidth, float availableHeight) override {
        float mouseX = Input::getMouseX();
        float mouseY = Input::getMouseY();

        float renderW = (availableWidth > 0.0f) ? availableWidth : 120.0f;
        float renderH = (availableHeight > 0.0f) ? availableHeight : 40.0f;

        bool isHovered = (mouseX >= x && mouseX <= x + renderW && mouseY >= y && mouseY <= y + renderH);
        bool isClicked = isHovered && Input::isMouseButtonDown(MouseButton::Left);

        Color renderColor = baseColor;
        if (isClicked) {
            renderColor = Color{baseColor.r * 0.7f, baseColor.g * 0.7f, baseColor.b * 0.7f, 1.0f};
            if (onClick) onClick();
        } else if (isHovered) {
            renderColor = Color{baseColor.r * 1.2f, baseColor.g * 1.2f, baseColor.b * 1.2f, 1.0f};
        }

        Renderer2D::drawRect(x, y, renderW, renderH, renderColor);
        Renderer2D::drawRectOutline(x, y, renderW, renderH, Color::White(), 1.5f);

        if (child) {
            child->render(x, y, renderW, renderH);
        }
    }
};

// Global Gui API for quick layout rendering
class Gui {
public:
    static bool button(const std::string& label, float x, float y, float width, float height, const Color& baseColor = Color{0.2f, 0.4f, 0.8f, 1.0f}) {
        ButtonWidget btn(label, nullptr);
        btn.baseColor = baseColor;
        btn.render(x, y, width, height);

        float mouseX = Input::getMouseX();
        float mouseY = Input::getMouseY();
        bool isHovered = (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height);
        return (isHovered && Input::isMouseButtonDown(MouseButton::Left));
    }
};

} // namespace zenith

#endif // ZENITH_GUI_H
