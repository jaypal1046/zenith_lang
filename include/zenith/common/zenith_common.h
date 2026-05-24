#ifndef ZENITH_COMMON_H
#define ZENITH_COMMON_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <functional>
#include <cstring>
#include <fstream>
#include <iterator>
#include <cmath>

#include <cstdio>

#ifdef YOGA_AVAILABLE
#include "zenith/ui/yoga_layout.h"
#include <memory>
#endif

namespace zenith {

inline std::string run_cmd(const std::string& cmd) {
    std::string result;
    char buffer[128];
#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) return "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return result;
}

template<typename T>
std::string toString(const T& val) {
    if constexpr (std::is_convertible_v<T, std::string>) {
        return std::string(val);
    } else if constexpr (std::is_same_v<T, bool>) {
        return val ? "true" : "false";
    } else {
        return std::to_string(val);
    }
}

template<typename T1, typename T2>
std::string concat(const T1& a, const T2& b) {
    return toString(a) + toString(b);
}

class TerminalBuffer {
public:
    int width = 0;
    int height = 0;
    std::vector<std::vector<std::string>> grid;
    std::vector<std::vector<std::string>> styles;

    TerminalBuffer(int w, int h) 
        : width(w), height(h), 
          grid(h, std::vector<std::string>(w, " ")),
          styles(h, std::vector<std::string>(w, "")) {}

    void setCell(int x, int y, const std::string& s, const std::string& style = "") {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x] = s;
            styles[y][x] = style;
        }
    }

    void writeString(int x, int y, const std::string& str, const std::string& style = "") {
        int current_x = x;
        int current_y = y;
        for (size_t i = 0; i < str.length(); ) {
            unsigned char c = str[i];
            std::string utf8_char;
            int len = 1;
            if (c == '\n') {
                current_y++;
                current_x = x;
                i++;
                continue;
            }
            if (c >= 0xf0) { len = 4; }
            else if (c >= 0xe0) { len = 3; }
            else if (c >= 0xc0) { len = 2; }
            
            if (i + len <= str.length()) {
                utf8_char = str.substr(i, len);
            } else {
                utf8_char = std::string(1, c);
            }
            
            setCell(current_x, current_y, utf8_char, style);
            current_x++;
            i += len;
        }
    }

    void drawBox(int x, int y, int w, int h, const std::string& border_style = "single", const std::string& style = "") {
        if (w <= 0 || h <= 0) return;
        
        std::string tl = "┌", tr = "┐", bl = "└", br = "┘", hz = "─", vt = "│";
        if (border_style == "double") {
            tl = "╔"; tr = "╗"; bl = "╚"; br = "╝"; hz = "═"; vt = "║";
        }
        
        // Draw horizontal borders
        for (int i = 1; i < w - 1; ++i) {
            setCell(x + i, y, hz, style);
            setCell(x + i, y + h - 1, hz, style);
        }
        // Draw vertical borders
        for (int j = 1; j < h - 1; ++j) {
            setCell(x, y + j, vt, style);
            setCell(x + w - 1, y + j, vt, style);
        }
        // Draw corners
        setCell(x, y, tl, style);
        setCell(x + w - 1, y, tr, style);
        setCell(x, y + h - 1, bl, style);
        setCell(x + w - 1, y + h - 1, br, style);
    }

    void print() const {
        for (int y = 0; y < height; ++y) {
            std::string current_style = "";
            for (int x = 0; x < width; ++x) {
                if (styles[y][x] != current_style) {
                    if (!current_style.empty()) {
                        std::cout << "\033[0m";
                    }
                    current_style = styles[y][x];
                    if (!current_style.empty()) {
                        std::cout << current_style;
                    }
                }
                std::cout << grid[y][x];
            }
            if (!current_style.empty()) {
                std::cout << "\033[0m";
            }
            std::cout << "\n";
        }
    }
};

class UIElement {
public:
    std::string type;
    std::vector<UIElement> children;
    std::unordered_map<std::string, std::string> attributes;
    std::string text_content;

    // Layout engine metrics
    mutable int layout_width = 0;
    mutable int layout_height = 0;
    mutable int layout_x = 0;
    mutable int layout_y = 0;

    UIElement() : type("Empty") {}
    UIElement(std::string t, std::vector<UIElement> c = {}, std::unordered_map<std::string, std::string> a = {})
        : type(std::move(t)), children(std::move(c)), attributes(std::move(a)) {}

    static UIElement Text(std::string text, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Text");
        el.text_content = std::move(text);
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement Button(std::string label, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Button");
        el.text_content = std::move(label);
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement TextField(std::string placeholder, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("TextField");
        el.text_content = std::move(placeholder);
        el.attributes = std::move(attrs);
        if (!el.attributes.count("placeholder")) {
            el.attributes["placeholder"] = el.text_content;
        }
        return el;
    }

    static UIElement Column(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Column", std::move(children), std::move(attrs));
    }

    static UIElement Row(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Row", std::move(children), std::move(attrs));
    }

    static UIElement Image(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Image");
        el.text_content = std::move(url);
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement Video(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Video");
        el.text_content = std::move(url);
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement Checkbox(std::string label, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Checkbox");
        el.text_content = std::move(label);
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement Slider(std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Slider");
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement Toggle(std::string label, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Toggle");
        el.text_content = std::move(label);
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement Dropdown(std::string options, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Dropdown");
        el.text_content = std::move(options);
        el.attributes = std::move(attrs);
        return el;
    }

    static UIElement Scrolling(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Scrolling", std::move(children), std::move(attrs));
    }

    static UIElement Card(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Card", std::move(children), std::move(attrs));
    }

    static UIElement Container(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Container", std::move(children), std::move(attrs));
    }

    void measure() const {
        int padding = 0;
        if (attributes.count("padding")) {
            try { padding = std::stoi(attributes.at("padding")); } catch(...) {}
        }

        if (type == "Text") {
            int max_w = 0;
            int current_w = 0;
            int h = 1;
            for (char c : text_content) {
                if (c == '\n') {
                    h++;
                    if (current_w > max_w) max_w = current_w;
                    current_w = 0;
                } else {
                    current_w++;
                }
            }
            if (current_w > max_w) max_w = current_w;
            layout_width = max_w + padding * 2;
            layout_height = h + padding * 2;
        } else if (type == "Button") {
            layout_width = text_content.length() + 4 + padding * 2;
            layout_height = 3 + padding * 2;
        } else if (type == "TextField") {
            int w = 24;
            int h = 3;
            if (attributes.count("width")) { try { w = std::stoi(attributes.at("width")); } catch(...) {} }
            if (attributes.count("height")) { try { h = std::stoi(attributes.at("height")); } catch(...) {} }
            layout_width = w + padding * 2;
            layout_height = h + padding * 2;
        } else if (type == "Image") {
            int w = 24;
            int h = 4;
            if (attributes.count("width")) { try { w = std::stoi(attributes.at("width")); } catch(...) {} }
            if (attributes.count("height")) { try { h = std::stoi(attributes.at("height")); } catch(...) {} }
            layout_width = w + padding * 2;
            layout_height = h + padding * 2;
        } else if (type == "Video") {
            int w = 28;
            int h = 5;
            if (attributes.count("width")) { try { w = std::stoi(attributes.at("width")); } catch(...) {} }
            if (attributes.count("height")) { try { h = std::stoi(attributes.at("height")); } catch(...) {} }
            layout_width = w + padding * 2;
            layout_height = h + padding * 2;
        } else if (type == "Checkbox") {
            layout_width = text_content.length() + 4 + padding * 2;
            layout_height = 1 + padding * 2;
        } else if (type == "Slider") {
            int w = 24;
            if (attributes.count("width")) { try { w = std::stoi(attributes.at("width")); } catch(...) {} }
            layout_width = w + padding * 2;
            layout_height = 1 + padding * 2;
        } else if (type == "Toggle") {
            layout_width = text_content.length() + 8 + padding * 2;
            layout_height = 1 + padding * 2;
        } else if (type == "Dropdown") {
            int w = 20;
            if (attributes.count("width")) { try { w = std::stoi(attributes.at("width")); } catch(...) {} }
            layout_width = w + padding * 2;
            layout_height = 3 + padding * 2;
        } else if (type == "Scrolling" || type == "Card" || type == "Container" || type == "Column" || type == "Row") {
            std::string flex_dir = (type == "Row") ? "row" : "column";
            if (attributes.count("flexDirection")) {
                flex_dir = attributes.at("flexDirection");
            }
            int gap = 0;
            if (attributes.count("gap")) {
                try { gap = std::stoi(attributes.at("gap")); } catch(...) {}
            }

            int content_w = 0;
            int content_h = 0;
            int num_children = children.size();

            for (const auto& child : children) {
                child.measure();
            }

            if (flex_dir == "row") {
                int total_child_w = 0;
                int max_child_h = 0;
                for (const auto& child : children) {
                    total_child_w += child.layout_width;
                    if (child.layout_height > max_child_h) {
                        max_child_h = child.layout_height;
                    }
                }
                content_w = total_child_w + (num_children > 1 ? gap * (num_children - 1) : 0);
                content_h = max_child_h;
            } else { // "column"
                int max_child_w = 0;
                int total_child_h = 0;
                for (const auto& child : children) {
                    if (child.layout_width > max_child_w) {
                        max_child_w = child.layout_width;
                    }
                    total_child_h += child.layout_height;
                }
                content_w = max_child_w;
                content_h = total_child_h + (num_children > 1 ? gap * (num_children - 1) : 0);
            }

            if (type == "Scrolling") {
                layout_width = content_w + 4 + padding * 2;
                int h_limit = 8;
                if (attributes.count("height")) { try { h_limit = std::stoi(attributes.at("height")); } catch(...) {} }
                layout_height = (content_h > h_limit ? h_limit : content_h) + 2 + padding * 2;
            } else {
                layout_width = content_w + 2 + padding * 2;
                layout_height = content_h + 2 + padding * 2;
            }
        }

        if (attributes.count("width")) {
            try { layout_width = std::stoi(attributes.at("width")); } catch(...) {}
        }
        if (attributes.count("height")) {
            try { layout_height = std::stoi(attributes.at("height")); } catch(...) {}
        }

        int margin = 0;
        if (attributes.count("margin")) {
            try { margin = std::stoi(attributes.at("margin")); } catch(...) {}
        }
        if (margin > 0) {
            layout_width += margin * 2;
            layout_height += margin * 2;
        }
    }

    void arrange(int x, int y) const {
        layout_x = x;
        layout_y = y;
        int padding = 0;
        if (attributes.count("padding")) {
            try { padding = std::stoi(attributes.at("padding")); } catch(...) {}
        }
        int margin = 0;
        if (attributes.count("margin")) {
            try { margin = std::stoi(attributes.at("margin")); } catch(...) {}
        }

        if (type == "Scrolling" || type == "Card" || type == "Container" || type == "Column" || type == "Row") {
            std::string flex_dir = (type == "Row") ? "row" : "column";
            if (attributes.count("flexDirection")) {
                flex_dir = attributes.at("flexDirection");
            }
            int gap = 0;
            if (attributes.count("gap")) {
                try { gap = std::stoi(attributes.at("gap")); } catch(...) {}
            }
            std::string justify_content = "flex-start";
            if (attributes.count("justifyContent")) {
                justify_content = attributes.at("justifyContent");
            }
            std::string align_items = "flex-start";
            if (attributes.count("alignItems")) {
                align_items = attributes.at("alignItems");
            }

            int content_start_x = x + margin + 1 + padding;
            int content_start_y = y + margin + 1 + padding;
            int content_w = layout_width - margin * 2 - 2 - padding * 2;
            int content_h = layout_height - margin * 2 - 2 - padding * 2;
            if (content_w < 0) content_w = 0;
            if (content_h < 0) content_h = 0;

            int total_base_main = 0;
            int total_flex_grow = 0;
            int num_children = children.size();

            for (const auto& child : children) {
                int child_grow = 0;
                if (child.attributes.count("flexGrow")) {
                    try { child_grow = std::stoi(child.attributes.at("flexGrow")); } catch(...) {}
                }
                total_flex_grow += child_grow;

                if (flex_dir == "row") {
                    total_base_main += child.layout_width;
                } else {
                    total_base_main += child.layout_height;
                }
            }

            int total_gap = (num_children > 1) ? gap * (num_children - 1) : 0;
            int remaining_space = 0;
            if (flex_dir == "row") {
                remaining_space = content_w - (total_base_main + total_gap);
            } else {
                remaining_space = content_h - (total_base_main + total_gap);
            }
            if (remaining_space < 0) remaining_space = 0;

            // Distribute flex grow space
            if (total_flex_grow > 0 && remaining_space > 0) {
                int distributed_space = 0;
                for (int i = 0; i < num_children; ++i) {
                    const auto& child = children[i];
                    int child_grow = 0;
                    if (child.attributes.count("flexGrow")) {
                        try { child_grow = std::stoi(child.attributes.at("flexGrow")); } catch(...) {}
                    }
                    if (child_grow > 0) {
                        int add_space = 0;
                        if (i == num_children - 1 || total_flex_grow == child_grow) {
                            add_space = remaining_space - distributed_space;
                        } else {
                            add_space = (remaining_space * child_grow) / total_flex_grow;
                        }
                        distributed_space += add_space;

                        if (flex_dir == "row") {
                            child.layout_width += add_space;
                        } else {
                            child.layout_height += add_space;
                        }
                    }
                }
            }

            // Apply align items stretch (runs before positioning to set child cross dimension)
            if (align_items == "stretch") {
                for (const auto& child : children) {
                    if (flex_dir == "row") {
                        child.layout_height = content_h;
                    } else {
                        child.layout_width = content_w;
                    }
                }
            }

            // Recalculate main size after grow space distribution
            int total_main_size = 0;
            for (const auto& child : children) {
                if (flex_dir == "row") {
                    total_main_size += child.layout_width;
                } else {
                    total_main_size += child.layout_height;
                }
            }
            int total_main_with_gap = total_main_size + total_gap;
            int main_free_space = 0;
            if (flex_dir == "row") {
                main_free_space = content_w - total_main_with_gap;
            } else {
                main_free_space = content_h - total_main_with_gap;
            }
            if (main_free_space < 0) main_free_space = 0;

            int current_main = (flex_dir == "row" ? content_start_x : content_start_y);

            // Apply justify content start offset
            if (justify_content == "flex-end") {
                current_main += main_free_space;
            } else if (justify_content == "center") {
                current_main += main_free_space / 2;
            } else if (justify_content == "space-around") {
                if (num_children > 0) {
                    int space_unit = main_free_space / (num_children * 2);
                    current_main += space_unit;
                }
            }

            for (int i = 0; i < num_children; ++i) {
                const auto& child = children[i];

                int child_cross_pos = 0;
                int child_cross_size = (flex_dir == "row" ? child.layout_height : child.layout_width);
                int cross_free_space = (flex_dir == "row" ? content_h : content_w) - child_cross_size;
                if (cross_free_space < 0) cross_free_space = 0;

                if (align_items == "flex-start" || align_items == "stretch") {
                    child_cross_pos = 0;
                } else if (align_items == "center") {
                    child_cross_pos = cross_free_space / 2;
                } else if (align_items == "flex-end") {
                    child_cross_pos = cross_free_space;
                }

                int child_x = 0, child_y = 0;
                if (flex_dir == "row") {
                    child_x = current_main;
                    child_y = content_start_y + child_cross_pos;
                } else {
                    child_x = content_start_x + child_cross_pos;
                    child_y = current_main;
                }

                child.arrange(child_x, child_y);

                int child_main_size = (flex_dir == "row" ? child.layout_width : child.layout_height);
                current_main += child_main_size;

                if (i < num_children - 1) {
                    current_main += gap;
                    if (justify_content == "space-between" && num_children > 1) {
                        current_main += main_free_space / (num_children - 1);
                    } else if (justify_content == "space-around" && num_children > 0) {
                        current_main += (main_free_space / (num_children * 2)) * 2;
                    }
                }
            }
        }
    }

    void printTree(int indent) const {
        std::string spaces(indent * 2, ' ');
        std::cout << spaces << "<" << type 
                  << " x=\"" << layout_x << "\" y=\"" << layout_y 
                  << "\" width=\"" << layout_width << "\" height=\"" << layout_height << "\"";
        for (const auto& attr : attributes) {
            std::cout << " " << attr.first << "=\"" << attr.second << "\"";
        }
        if (type == "Text" || type == "Image" || type == "Video") {
            std::cout << ">" << text_content << "</" << type << ">\n";
        } else {
            std::cout << ">\n";
            for (const auto& child : children) {
                child.printTree(indent + 1);
            }
            std::cout << spaces << "</" << type << ">\n";
        }
    }

    void drawToBuffer(TerminalBuffer& buffer, int offset_x, int offset_y) const {
        int abs_x = offset_x + layout_x;
        int abs_y = offset_y + layout_y;
        
        int padding = 0;
        if (attributes.count("padding")) {
            try { padding = std::stoi(attributes.at("padding")); } catch(...) {}
        }
        int margin = 0;
        if (attributes.count("margin")) {
            try { margin = std::stoi(attributes.at("margin")); } catch(...) {}
        }

        int draw_w = layout_width;
        int draw_h = layout_height;
        if (margin > 0) {
            abs_x += margin;
            abs_y += margin;
            draw_w -= margin * 2;
            draw_h -= margin * 2;
            if (draw_w < 0) draw_w = 0;
            if (draw_h < 0) draw_h = 0;
        }

        std::string border_style = "single";
        std::string border_color = "\033[36m";
        std::string text_color = "\033[37m";
        std::string bg_color = "";

        if (attributes.count("color")) {
            std::string c = attributes.at("color");
            if (c == "red") text_color = "\033[31m";
            else if (c == "green") text_color = "\033[32m";
            else if (c == "yellow") text_color = "\033[33m";
            else if (c == "blue") text_color = "\033[34m";
            else if (c == "magenta") text_color = "\033[35m";
            else if (c == "cyan") text_color = "\033[36m";
        }

        if (attributes.count("backgroundColor")) {
            std::string bg = attributes.at("backgroundColor");
            if (bg == "red") bg_color = "\033[41m";
            else if (bg == "green") bg_color = "\033[42m";
            else if (bg == "yellow") bg_color = "\033[43m";
            else if (bg == "blue") bg_color = "\033[44m";
            else if (bg == "magenta") bg_color = "\033[45m";
            else if (bg == "cyan") bg_color = "\033[46m";
        }

        std::string combined_style = bg_color + text_color;

        if (type == "Column") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[36m");
        } else if (type == "Row") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[35m");
        } else if (type == "Card") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "double", "\033[1;32m");
        } else if (type == "Container") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[34m");
        } else if (type == "Scrolling") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[33m");
            int track_h = draw_h - 2;
            for (int i = 0; i < track_h; ++i) {
                std::string thumb = (i == track_h / 3) ? "█" : "░";
                buffer.setCell(abs_x + draw_w - 1, abs_y + 1 + i, thumb, "\033[33m");
            }
        }
        
        if (type == "Text") {
            if (attributes.count("fontWeight") && attributes.at("fontWeight") == "bold") {
                combined_style = "\033[1m" + combined_style;
            }
            if (text_content.find("[Status:") != std::string::npos) {
                combined_style = "\033[1;32m" + bg_color;
            } else if (text_content.find("Counter Value:") != std::string::npos) {
                combined_style = "\033[1;33m" + bg_color;
            }
            buffer.writeString(abs_x + padding, abs_y + padding, text_content, combined_style);
        } else if (type == "Button") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "double", "\033[33m");
            buffer.writeString(abs_x + 2 + padding, abs_y + 1 + padding, text_content, "\033[1;33m" + bg_color);
        } else if (type == "TextField") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[36m");
            std::string display_text = "";
            std::string display_style = "\033[37m"; // default text color
            if (attributes.count("value") && !attributes.at("value").empty()) {
                display_text = attributes.at("value");
            } else if (attributes.count("placeholder") && !attributes.at("placeholder").empty()) {
                display_text = attributes.at("placeholder");
                display_style = "\033[90m"; // gray color
            } else if (!text_content.empty()) {
                display_text = text_content;
                display_style = "\033[90m";
            }
            if (display_text.length() > (size_t)draw_w - 4) {
                display_text = display_text.substr(0, draw_w - 7) + "...";
            }
            buffer.writeString(abs_x + 2 + padding, abs_y + 1 + padding, display_text, display_style + bg_color);
        } else if (type == "Image") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[32m");
            buffer.writeString(abs_x + 2 + padding, abs_y + 1 + padding, "📷 IMAGE", "\033[1;32m");
            std::string url = text_content;
            if (url.length() > (size_t)draw_w - 4) url = url.substr(0, draw_w - 7) + "...";
            buffer.writeString(abs_x + 2 + padding, abs_y + 2 + padding, url, "\033[37m");
        } else if (type == "Video") {
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[31m");
            buffer.writeString(abs_x + 2 + padding, abs_y + 1 + padding, "🎬 VIDEO: ▶", "\033[1;31m");
            std::string url = text_content;
            if (url.length() > (size_t)draw_w - 4) url = url.substr(0, draw_w - 7) + "...";
            buffer.writeString(abs_x + 2 + padding, abs_y + 2 + padding, url, "\033[37m");
        } else if (type == "Checkbox") {
            bool checked = (attributes.count("checked") && (attributes.at("checked") == "true" || attributes.at("checked") == "1"));
            std::string cb_str = checked ? "[X] " : "[ ] ";
            buffer.writeString(abs_x + padding, abs_y + padding, cb_str + text_content, "\033[36m" + bg_color);
        } else if (type == "Slider") {
            int min_val = 0;
            if (attributes.count("min")) { try { min_val = std::stoi(attributes.at("min")); } catch(...) {} }
            int max_val = 100;
            if (attributes.count("max")) { try { max_val = std::stoi(attributes.at("max")); } catch(...) {} }
            int cur_val = 0;
            if (attributes.count("value")) { try { cur_val = std::stoi(attributes.at("value")); } catch(...) {} }
            std::string track = "[";
            int inner_w = draw_w - 2;
            if (inner_w > 0) {
                int thumb_idx = -1;
                if (max_val > min_val) {
                    float pct = (float)(cur_val - min_val) / (max_val - min_val);
                    if (pct < 0.0f) pct = 0.0f;
                    if (pct > 1.0f) pct = 1.0f;
                    thumb_idx = static_cast<int>(std::round(pct * (inner_w - 1)));
                } else {
                    thumb_idx = 0;
                }
                for (int i = 0; i < inner_w; ++i) {
                    if (i == thumb_idx) {
                        track += "█";
                    } else {
                        track += "─";
                    }
                }
            }
            track += "]";
            buffer.writeString(abs_x + padding, abs_y + padding, track, "\033[33m" + bg_color);
        } else if (type == "Toggle") {
            bool is_on = (attributes.count("isOn") && (attributes.at("isOn") == "true" || attributes.at("isOn") == "1"));
            std::string toggle_style = is_on ? "\033[1;32m" : "\033[90m";
            std::string toggle_indicator = is_on ? "( •) ON " : "(• ) OFF ";
            buffer.writeString(abs_x + padding, abs_y + padding, toggle_indicator + text_content, toggle_style + bg_color);
        } else if (type == "Dropdown") {
            std::string selected = "Select Option";
            if (attributes.count("value") && !attributes.at("value").empty()) {
                selected = attributes.at("value");
            }
            std::string dropdown_text = "[ " + selected + " ▼ ]";
            if (dropdown_text.length() > (size_t)draw_w) {
                dropdown_text = dropdown_text.substr(0, draw_w - 3) + "...";
            }
            buffer.drawBox(abs_x, abs_y, draw_w, draw_h, "single", "\033[36m");
            buffer.writeString(abs_x + (draw_w - dropdown_text.length()) / 2 + padding, abs_y + 1 + padding, dropdown_text, "\033[1;36m" + bg_color);
        } else {
            for (const auto& child : children) {
                child.drawToBuffer(buffer, offset_x, offset_y);
            }
        }
    }

#ifdef YOGA_AVAILABLE
    static std::shared_ptr<zenith::ui::YogaNode> buildYogaTree(const UIElement& element, std::vector<std::pair<const UIElement*, std::shared_ptr<zenith::ui::YogaNode>>>& node_map) {
        auto node = std::make_shared<zenith::ui::YogaNode>();
        node_map.push_back({&element, node});
        
        if (element.type == "Row") {
            node->setFlexDirection(zenith::ui::FlexDirection::Row);
        } else {
            node->setFlexDirection(zenith::ui::FlexDirection::Column);
        }
        
        if (element.attributes.count("flexDirection")) {
            std::string fd = element.attributes.at("flexDirection");
            if (fd == "row") node->setFlexDirection(zenith::ui::FlexDirection::Row);
            else if (fd == "column") node->setFlexDirection(zenith::ui::FlexDirection::Column);
            else if (fd == "row-reverse") node->setFlexDirection(zenith::ui::FlexDirection::RowReverse);
            else if (fd == "column-reverse") node->setFlexDirection(zenith::ui::FlexDirection::ColumnReverse);
        }
        
        if (element.attributes.count("width")) {
            try {
                std::string w_str = element.attributes.at("width");
                if (!w_str.empty()) {
                    if (w_str.back() == '%') {
                        float val = std::stof(w_str.substr(0, w_str.length() - 1));
                        node->setWidth(zenith::ui::MeasureValue::percent(val));
                    } else if (w_str == "auto") {
                        node->setWidth(zenith::ui::MeasureValue::auto_value());
                    } else {
                        node->setWidth(zenith::ui::MeasureValue::points(std::stof(w_str)));
                    }
                }
            } catch(...) {}
        }
        if (element.attributes.count("height")) {
            try {
                std::string h_str = element.attributes.at("height");
                if (!h_str.empty()) {
                    if (h_str.back() == '%') {
                        float val = std::stof(h_str.substr(0, h_str.length() - 1));
                        node->setHeight(zenith::ui::MeasureValue::percent(val));
                    } else if (h_str == "auto") {
                        node->setHeight(zenith::ui::MeasureValue::auto_value());
                    } else {
                        node->setHeight(zenith::ui::MeasureValue::points(std::stof(h_str)));
                    }
                }
            } catch(...) {}
        }
        
        if (element.attributes.count("flexGrow")) {
            try { node->setFlexGrow(std::stof(element.attributes.at("flexGrow"))); } catch(...) {}
        }
        if (element.attributes.count("flexShrink")) {
            try { node->setFlexShrink(std::stof(element.attributes.at("flexShrink"))); } catch(...) {}
        }
        
        if (element.attributes.count("justifyContent")) {
            std::string jc = element.attributes.at("justifyContent");
            if (jc == "flex-start") node->setJustifyContent(zenith::ui::JustifyContent::FlexStart);
            else if (jc == "flex-end") node->setJustifyContent(zenith::ui::JustifyContent::FlexEnd);
            else if (jc == "center") node->setJustifyContent(zenith::ui::JustifyContent::Center);
            else if (jc == "space-between") node->setJustifyContent(zenith::ui::JustifyContent::SpaceBetween);
            else if (jc == "space-around") node->setJustifyContent(zenith::ui::JustifyContent::SpaceAround);
            else if (jc == "space-evenly") node->setJustifyContent(zenith::ui::JustifyContent::SpaceEvenly);
        }
        
        if (element.attributes.count("alignItems")) {
            std::string ai = element.attributes.at("alignItems");
            if (ai == "flex-start") node->setAlignItems(zenith::ui::AlignItems::FlexStart);
            else if (ai == "flex-end") node->setAlignItems(zenith::ui::AlignItems::FlexEnd);
            else if (ai == "center") node->setAlignItems(zenith::ui::AlignItems::Center);
            else if (ai == "baseline") node->setAlignItems(zenith::ui::AlignItems::Baseline);
            else if (ai == "stretch") node->setAlignItems(zenith::ui::AlignItems::Stretch);
        }
        
        if (element.attributes.count("padding")) {
            try {
                float p = std::stof(element.attributes.at("padding"));
                node->setPadding(zenith::ui::Edge::All, zenith::ui::MeasureValue::points(p));
            } catch(...) {}
        }
        if (element.attributes.count("margin")) {
            try {
                float m = std::stof(element.attributes.at("margin"));
                node->setMargin(zenith::ui::Edge::All, zenith::ui::MeasureValue::points(m));
            } catch(...) {}
        }
        
        if (element.type == "Column" || element.type == "Row" || element.type == "Card" || element.type == "Container" || element.type == "Scrolling") {
            node->setBorder(zenith::ui::Edge::All, 1.0f);
        }
        
        if (element.type == "Text" || element.type == "Button" || element.type == "TextField" || element.type == "Image" || element.type == "Video" ||
            element.type == "Checkbox" || element.type == "Slider" || element.type == "Toggle" || element.type == "Dropdown") {
            node->setMeasureFunction([type = element.type, text = element.text_content, attrs = element.attributes](float w_constraint, float h_constraint) -> std::pair<float, float> {
                (void)w_constraint;
                (void)h_constraint;
                float w = 0.0f;
                float h = 0.0f;
                
                int padding = 0;
                if (attrs.count("padding")) {
                    try { padding = std::stoi(attrs.at("padding")); } catch(...) {}
                }
                
                if (type == "Text") {
                    int max_w = 0;
                    int current_w = 0;
                    int line_h = 1;
                    for (char c : text) {
                        if (c == '\n') {
                            line_h++;
                            if (current_w > max_w) max_w = current_w;
                            current_w = 0;
                        } else {
                            current_w++;
                        }
                    }
                    if (current_w > max_w) max_w = current_w;
                    w = max_w + padding * 2;
                    h = line_h + padding * 2;
                } else if (type == "Button") {
                    w = text.length() + 4 + padding * 2;
                    h = 3 + padding * 2;
                } else if (type == "TextField") {
                    w = 24 + padding * 2;
                    h = 3 + padding * 2;
                    if (attrs.count("width")) { try { w = std::stof(attrs.at("width")) + padding * 2; } catch(...) {} }
                    if (attrs.count("height")) { try { h = std::stof(attrs.at("height")) + padding * 2; } catch(...) {} }
                } else if (type == "Image") {
                    w = 24 + padding * 2;
                    h = 4 + padding * 2;
                    if (attrs.count("width")) { try { w = std::stof(attrs.at("width")) + padding * 2; } catch(...) {} }
                    if (attrs.count("height")) { try { h = std::stof(attrs.at("height")) + padding * 2; } catch(...) {} }
                } else if (type == "Video") {
                    w = 28 + padding * 2;
                    h = 5 + padding * 2;
                    if (attrs.count("width")) { try { w = std::stof(attrs.at("width")) + padding * 2; } catch(...) {} }
                    if (attrs.count("height")) { try { h = std::stof(attrs.at("height")) + padding * 2; } catch(...) {} }
                } else if (type == "Checkbox") {
                    w = text.length() + 4 + padding * 2;
                    h = 1 + padding * 2;
                } else if (type == "Slider") {
                    w = 24 + padding * 2;
                    h = 1 + padding * 2;
                    if (attrs.count("width")) { try { w = std::stof(attrs.at("width")) + padding * 2; } catch(...) {} }
                } else if (type == "Toggle") {
                    w = text.length() + 8 + padding * 2;
                    h = 1 + padding * 2;
                } else if (type == "Dropdown") {
                    w = 20 + padding * 2;
                    h = 3 + padding * 2;
                    if (attrs.count("width")) { try { w = std::stof(attrs.at("width")) + padding * 2; } catch(...) {} }
                }
                return {w, h};
            });
        }
        
        for (const auto& child : element.children) {
            auto child_node = buildYogaTree(child, node_map);
            node->addChild(child_node);
        }
        
        return node;
    }

    static void applyYogaLayoutParallel(const UIElement& el, std::shared_ptr<zenith::ui::YogaNode> y_node, int parent_abs_x, int parent_abs_y) {
        if (!y_node) return;
        auto layout = y_node->getLayout();
        
        el.layout_width = static_cast<int>(std::round(layout.width));
        el.layout_height = static_cast<int>(std::round(layout.height));
        el.layout_x = parent_abs_x + static_cast<int>(std::round(layout.x));
        el.layout_y = parent_abs_y + static_cast<int>(std::round(layout.y));
        
        for (size_t i = 0; i < el.children.size(); ++i) {
            auto child_y_node = y_node->getChildAt(i);
            applyYogaLayoutParallel(el.children[i], child_y_node, el.layout_x, el.layout_y);
        }
    }

    void calculateYogaLayout() const {
        std::vector<std::pair<const UIElement*, std::shared_ptr<zenith::ui::YogaNode>>> node_map;
        auto root_node = buildYogaTree(*this, node_map);
        
        float w_constraint = NAN;
        float h_constraint = NAN;
        if (attributes.count("width")) {
            try { w_constraint = std::stof(attributes.at("width")); } catch(...) {}
        }
        if (attributes.count("height")) {
            try { h_constraint = std::stof(attributes.at("height")); } catch(...) {}
        }
        
        root_node->calculateLayout(w_constraint, h_constraint);
        applyYogaLayoutParallel(*this, root_node, 0, 0);
    }
#endif

    void render(int indent = 0) const {
        if (indent == 0) {
#ifdef YOGA_AVAILABLE
            calculateYogaLayout();
#else
            measure();
            arrange(0, 0);
#endif
            std::cout << "\n=== Rendered UI Layout Tree ===\n";
            printTree(0);
            
            std::cout << "\n=== Terminal Visual Render (Wow Aesthetics) ===\n";
            TerminalBuffer buffer(layout_width, layout_height);
            drawToBuffer(buffer, 0, 0);
            buffer.print();
            std::cout << "\n";
        }
    }

    void collectClickables(std::vector<const UIElement*>& clickables) const {
        if (type == "Button" && attributes.count("onClick")) {
            clickables.push_back(this);
        }
        for (const auto& child : children) {
            child.collectClickables(clickables);
        }
    }

    void collectInteractives(std::vector<const UIElement*>& interactives) const {
        if ((type == "Button" && attributes.count("onClick")) ||
            (type == "TextField" && attributes.count("onChange")) ||
            (type == "Checkbox" && attributes.count("onChange")) ||
            (type == "Slider" && attributes.count("onChange")) ||
            (type == "Toggle" && attributes.count("onChange")) ||
            (type == "Dropdown" && attributes.count("onChange"))) {
            interactives.push_back(this);
        }
        for (const auto& child : children) {
            child.collectInteractives(interactives);
        }
    }
};

template<typename T>
void flatten_helper(std::vector<UIElement>& result, T&& item) {
    if constexpr (std::is_same_v<std::decay_t<T>, UIElement>) {
        result.push_back(std::forward<T>(item));
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::vector<UIElement>>) {
        for (auto&& el : item) {
            result.push_back(std::move(el));
        }
    }
}

template<typename... Args>
std::vector<UIElement> make_children(Args&&... args) {
    std::vector<UIElement> result;
    (flatten_helper(result, std::forward<Args>(args)), ...);
    return result;
}

namespace UI {
    inline UIElement Column(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Column(std::move(children), std::move(attrs));
    }
    inline UIElement Row(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Row(std::move(children), std::move(attrs));
    }
    inline UIElement Text(std::string text, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Text(std::move(text), std::move(attrs));
    }
    inline UIElement Button(std::string label, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Button(std::move(label), std::move(attrs));
    }
    inline UIElement TextField(std::string placeholder, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::TextField(std::move(placeholder), std::move(attrs));
    }
    inline UIElement Image(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Image(std::move(url), std::move(attrs));
    }
    inline UIElement Video(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Video(std::move(url), std::move(attrs));
    }
    inline UIElement Checkbox(std::string label, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Checkbox(std::move(label), std::move(attrs));
    }
    inline UIElement Slider(std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Slider(std::move(attrs));
    }
    inline UIElement Toggle(std::string label, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Toggle(std::move(label), std::move(attrs));
    }
    inline UIElement Dropdown(std::string options, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Dropdown(std::move(options), std::move(attrs));
    }
    inline UIElement Scrolling(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Scrolling(std::move(children), std::move(attrs));
    }
    inline UIElement Card(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Card(std::move(children), std::move(attrs));
    }
    inline UIElement Container(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Container(std::move(children), std::move(attrs));
    }
}

inline void parse_url(const std::string& url, std::string& host, int& port, std::string& path) {
    std::string temp = url;
    if (temp.find("http://") == 0) {
        temp = temp.substr(7);
    } else if (temp.find("https://") == 0) {
        temp = temp.substr(8);
    }

    size_t colon = temp.find(':');
    size_t slash = temp.find('/');

    if (colon != std::string::npos && (slash == std::string::npos || colon < slash)) {
        host = temp.substr(0, colon);
        if (slash != std::string::npos) {
            port = std::stoi(temp.substr(colon + 1, slash - (colon + 1)));
            path = temp.substr(slash);
        } else {
            port = std::stoi(temp.substr(colon + 1));
            path = "/";
        }
    } else {
        if (slash != std::string::npos) {
            host = temp.substr(0, slash);
            path = temp.substr(slash);
        } else {
            host = temp;
            path = "/";
        }
        port = 80;
    }
}

inline std::string extract_json_field(const std::string& json, const std::string& field) {
    std::string key = "\"" + field + "\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos) return "";

    pos += key.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == ':' || json[pos] == '\t')) {
        pos++;
    }

    if (pos < json.length() && json[pos] == '"') {
        pos++;
        std::string val;
        while (pos < json.length()) {
            if (json[pos] == '\\' && pos + 1 < json.length() && json[pos + 1] == '"') {
                val += '"';
                pos += 2;
            } else if (json[pos] == '"') {
                break;
            } else if (json[pos] == '\\' && pos + 1 < json.length() && json[pos + 1] == 'n') {
                val += '\n';
                pos += 2;
            } else {
                val += json[pos];
                pos++;
            }
        }
        return val;
    }
    return "";
}

#ifdef USE_CURL
#include <curl/curl.h>

inline size_t curl_write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append((char*)contents, total_size);
    return total_size;
}

inline bool curl_post(const std::string& url, const std::string& json_body, std::string& out_response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

inline bool curl_get(const std::string& url, std::string& out_response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}
#endif

inline std::string base64_encode(const std::string& in) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

inline std::string base64_encode_file(const std::string& file_path) {
    // We already have headers in the file, but we should make sure we have <fstream> and <iterator>
    // These are standard, let's just use std::ifstream
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return base64_encode(data);
}

} // namespace zenith

#endif // ZENITH_COMMON_H
