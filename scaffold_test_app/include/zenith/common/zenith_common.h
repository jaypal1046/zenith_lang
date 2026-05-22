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

namespace zenith {

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
        } else if (type == "Scrolling") {
            int max_w = 0;
            int total_h = 0;
            for (const auto& child : children) {
                child.measure();
                if (child.layout_width > max_w) max_w = child.layout_width;
                total_h += child.layout_height;
            }
            layout_width = max_w + 4 + padding * 2;
            int h_limit = 8;
            if (attributes.count("height")) { try { h_limit = std::stoi(attributes.at("height")); } catch(...) {} }
            layout_height = (total_h > h_limit ? h_limit : total_h) + 2 + padding * 2;
        } else if (type == "Card" || type == "Container" || type == "Column") {
            int max_w = 0;
            int total_h = 0;
            for (const auto& child : children) {
                child.measure();
                if (child.layout_width > max_w) {
                    max_w = child.layout_width;
                }
                total_h += child.layout_height;
            }
            layout_width = max_w + 2 + padding * 2;
            layout_height = total_h + 2 + padding * 2;
        } else if (type == "Row") {
            int total_w = 0;
            int max_h = 0;
            for (const auto& child : children) {
                child.measure();
                total_w += child.layout_width;
                if (child.layout_height > max_h) {
                    max_h = child.layout_height;
                }
            }
            layout_width = total_w + 2 + padding * 2;
            layout_height = max_h + 2 + padding * 2;
        }

        if (attributes.count("width")) {
            try { layout_width = std::stoi(attributes.at("width")); } catch(...) {}
        }
        if (attributes.count("height")) {
            try { layout_height = std::stoi(attributes.at("height")); } catch(...) {}
        }
    }

    void arrange(int x, int y) const {
        layout_x = x;
        layout_y = y;
        int padding = 0;
        if (attributes.count("padding")) {
            try { padding = std::stoi(attributes.at("padding")); } catch(...) {}
        }

        if (type == "Column" || type == "Card" || type == "Container" || type == "Scrolling") {
            int current_y = y + 1 + padding;
            for (const auto& child : children) {
                child.arrange(x + 1 + padding, current_y);
                current_y += child.layout_height;
            }
        } else if (type == "Row") {
            int current_x = x + 1 + padding;
            for (const auto& child : children) {
                child.arrange(current_x, y + 1 + padding);
                current_x += child.layout_width;
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
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[36m");
        } else if (type == "Row") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[35m");
        } else if (type == "Card") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "double", "\033[1;32m");
        } else if (type == "Container") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[34m");
        } else if (type == "Scrolling") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[33m");
            int track_h = layout_height - 2;
            for (int i = 0; i < track_h; ++i) {
                std::string thumb = (i == track_h / 3) ? "█" : "░";
                buffer.setCell(abs_x + layout_width - 1, abs_y + 1 + i, thumb, "\033[33m");
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
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "double", "\033[33m");
            buffer.writeString(abs_x + 2 + padding, abs_y + 1 + padding, text_content, "\033[1;33m" + bg_color);
        } else if (type == "Image") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[32m");
            buffer.writeString(abs_x + 2 + padding, abs_y + 1 + padding, "📷 IMAGE", "\033[1;32m");
            std::string url = text_content;
            if (url.length() > (size_t)layout_width - 4) url = url.substr(0, layout_width - 7) + "...";
            buffer.writeString(abs_x + 2 + padding, abs_y + 2 + padding, url, "\033[37m");
        } else if (type == "Video") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[31m");
            buffer.writeString(abs_x + 2 + padding, abs_y + 1 + padding, "🎬 VIDEO: ▶", "\033[1;31m");
            std::string url = text_content;
            if (url.length() > (size_t)layout_width - 4) url = url.substr(0, layout_width - 7) + "...";
            buffer.writeString(abs_x + 2 + padding, abs_y + 2 + padding, url, "\033[37m");
        } else {
            for (const auto& child : children) {
                child.drawToBuffer(buffer, offset_x, offset_y);
            }
        }
    }

    void render(int indent = 0) const {
        if (indent == 0) {
            measure();
            arrange(0, 0);
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
};

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
    inline UIElement Image(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Image(std::move(url), std::move(attrs));
    }
    inline UIElement Video(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement::Video(std::move(url), std::move(attrs));
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

} // namespace zenith

#endif // ZENITH_COMMON_H
