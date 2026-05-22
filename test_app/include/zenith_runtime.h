#ifndef ZENITH_RUNTIME_H
#define ZENITH_RUNTIME_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <type_traits>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <conio.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <termios.h>
#endif
#include <functional>

namespace zenith {

#ifdef _WIN32
inline char get_keyboard_char() {
    return (char)_getch();
}
#else
inline char get_keyboard_char() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) {}
    struct termios raw = old;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &raw) < 0) {}
    if (read(0, &buf, 1) < 0) {}
    if (tcsetattr(0, TCSADRAIN, &old) < 0) {}
    return buf;
}
#endif


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

    // For Text component
    static UIElement Text(std::string text, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Text");
        el.text_content = std::move(text);
        el.attributes = std::move(attrs);
        return el;
    }

    // For Button component
    static UIElement Button(std::string label, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Button");
        el.text_content = std::move(label);
        el.attributes = std::move(attrs);
        return el;
    }

    // For Column component
    static UIElement Column(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Column", std::move(children), std::move(attrs));
    }

    // For Row component
    static UIElement Row(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Row", std::move(children), std::move(attrs));
    }

    // For Image component
    static UIElement Image(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Image");
        el.text_content = std::move(url);
        el.attributes = std::move(attrs);
        return el;
    }

    // For Video component
    static UIElement Video(std::string url, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Video");
        el.text_content = std::move(url);
        el.attributes = std::move(attrs);
        return el;
    }

    // For Scrolling component
    static UIElement Scrolling(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Scrolling", std::move(children), std::move(attrs));
    }

    // For Card component
    static UIElement Card(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Card", std::move(children), std::move(attrs));
    }

    // For Container component
    static UIElement Container(std::vector<UIElement> children, std::unordered_map<std::string, std::string> attrs = {}) {
        return UIElement("Container", std::move(children), std::move(attrs));
    }

    // Compute layout sizes
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

        // Apply width/height overrides if explicitly set in attributes
        if (attributes.count("width")) {
            try { layout_width = std::stoi(attributes.at("width")); } catch(...) {}
        }
        if (attributes.count("height")) {
            try { layout_height = std::stoi(attributes.at("height")); } catch(...) {}
        }
    }

    // Position children recursively
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

        // Color and border styles mapping
        std::string border_style = "single";
        std::string border_color = "\033[36m"; // Cyan default
        std::string text_color = "\033[37m";   // White default
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
            // Draw a simulated scroll track & thumb on the right border
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

    // Render layout tree
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

// UI Namespace wrapper to match generated code zenith::UI::Column
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

template<typename AppType>
inline void runInteractiveLoop(AppType& app) {
    while (true) {
        // Build the current UI element tree from the app
        zenith::UIElement root = app.build();
        
        // Measure and arrange layout
        root.measure();
        root.arrange(0, 0);
        
        // Collect all clickable buttons
        std::vector<const zenith::UIElement*> clickables;
        root.collectClickables(clickables);
        
        // Clear terminal screen using ANSI escape sequences
        std::cout << "\033[2J\033[H";
        
        // Print the layout tree and terminal visual render
        std::cout << "\n=== Rendered UI Layout Tree ===\n";
        root.printTree(0);
        
        std::cout << "\n=== Terminal Visual Render (Wow Aesthetics) ===\n";
        TerminalBuffer buffer(root.layout_width, root.layout_height);
        root.drawToBuffer(buffer, 0, 0);
        buffer.print();
        std::cout << "\n";
        
        if (clickables.empty()) {
            break; // Exit loop if no interactive elements
        }
        
        std::cout << "=== Interactive Control Panel ===\n";
        for (size_t i = 0; i < clickables.size(); ++i) {
            std::cout << "[" << (i + 1) << "] " << clickables[i]->text_content;
            if (clickables[i]->attributes.count("onClick")) {
                std::cout << " (Action: " << clickables[i]->attributes.at("onClick") << ")";
            }
            std::cout << "\n";
        }
        std::cout << "[Q] Quit Application\n";
        std::cout << "Choose an option: ";
        
        char choice = zenith::get_keyboard_char();
        if (choice == 'q' || choice == 'Q') {
            break;
        }
        
        int idx = choice - '1';
        if (idx >= 0 && idx < (int)clickables.size()) {
            std::string action = clickables[idx]->attributes.at("onClick");
            app.triggerCallback(action);
        }
    }
}

// Helper to parse URL into host, port, and path
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
        port = 80; // default HTTP
    }
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

#ifdef _WIN32
// Helper to make a WinHTTP GET request
inline bool winhttp_get(const std::string& host, int port, const std::string& path, std::string& out_response) {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;

    hSession = WinHttpOpen(L"ZenithRuntime/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        std::wstring w_host(host.begin(), host.end());
        hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
    }

    if (hConnect) {
        std::wstring w_path(path.begin(), path.end());
        hRequest = WinHttpOpenRequest(hConnect, L"GET", w_path.c_str(),
                                      NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }

    if (hRequest) {
        bResults = WinHttpSendRequest(hRequest,
                                      WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      WINHTTP_NO_REQUEST_DATA, 0,
                                      0, 0);
    }

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    std::string response_data;
    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                break;
            }
            if (dwSize == 0) {
                break;
            }

            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer) {
                break;
            }

            ZeroMemory(pszOutBuffer, dwSize + 1);

            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                delete[] pszOutBuffer;
                break;
            }

            response_data.append(pszOutBuffer, dwDownloaded);
            delete[] pszOutBuffer;

        } while (dwSize > 0);
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    if (bResults && !response_data.empty()) {
        out_response = response_data;
        return true;
    }
    return false;
}

// Helper to make a WinHTTP POST request
inline bool winhttp_post(const std::string& host, int port, const std::string& path, const std::string& json_body, std::string& out_response) {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;

    hSession = WinHttpOpen(L"ZenithRuntime/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        std::wstring w_host(host.begin(), host.end());
        hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
    }

    if (hConnect) {
        std::wstring w_path(path.begin(), path.end());
        hRequest = WinHttpOpenRequest(hConnect, L"POST", w_path.c_str(),
                                      NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }

    if (hRequest) {
        LPCWSTR pwszHeaders = L"Content-Type: application/json\r\n";
        bResults = WinHttpAddRequestHeaders(hRequest, pwszHeaders, (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);

        if (bResults) {
            bResults = WinHttpSendRequest(hRequest,
                                          WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                          (LPVOID)json_body.c_str(), json_body.length(),
                                          json_body.length(), 0);
        }
    }

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    std::string response_data;
    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                break;
            }
            if (dwSize == 0) {
                break;
            }

            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer) {
                break;
            }

            ZeroMemory(pszOutBuffer, dwSize + 1);

            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                delete[] pszOutBuffer;
                break;
            }

            response_data.append(pszOutBuffer, dwDownloaded);
            delete[] pszOutBuffer;

        } while (dwSize > 0);
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    if (bResults && !response_data.empty()) {
        out_response = response_data;
        return true;
    }
    return false;
}
#else
// Helper to make a POSIX socket GET request
inline bool posix_get(const std::string& host, int port, const std::string& path, std::string& out_response) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        close(sock);
        return false;
    }

    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return false;
    }

    std::stringstream req;
    req << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << ":" << port << "\r\n"
        << "Connection: close\r\n\r\n";

    std::string req_str = req.str();
    if (send(sock, req_str.c_str(), req_str.length(), 0) < 0) {
        close(sock);
        return false;
    }

    char buffer[4096];
    std::string response;
    int bytes_read;
    while ((bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        response.append(buffer, bytes_read);
    }

    close(sock);

    size_t body_pos = response.find("\r\n\r\n");
    if (body_pos != std::string::npos) {
        out_response = response.substr(body_pos + 4);
        return true;
    }
    return false;
}

// Helper to make a POSIX socket POST request
inline bool posix_post(const std::string& host, int port, const std::string& path, const std::string& json_body, std::string& out_response) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        close(sock);
        return false;
    }

    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return false;
    }

    std::stringstream req;
    req << "POST " << path << " HTTP/1.1\r\n"
        << "Host: " << host << ":" << port << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << json_body.length() << "\r\n"
        << "Connection: close\r\n\r\n"
        << json_body;

    std::string req_str = req.str();
    if (send(sock, req_str.c_str(), req_str.length(), 0) < 0) {
        close(sock);
        return false;
    }

    char buffer[4096];
    std::string response;
    int bytes_read;
    while ((bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        response.append(buffer, bytes_read);
    }

    close(sock);

    size_t body_pos = response.find("\r\n\r\n");
    if (body_pos != std::string::npos) {
        out_response = response.substr(body_pos + 4);
        return true;
    }
    return false;
}
#endif

inline std::string httpGet(const std::string& url) {
    std::string response;
    bool success = false;
#ifdef USE_CURL
    success = curl_get(url, response);
#else
    std::string host, path;
    int port = 80;
    parse_url(url, host, port, path);
#ifdef _WIN32
    success = winhttp_get(host, port, path, response);
#else
    success = posix_get(host, port, path, response);
#endif
#endif
    if (success) return response;
    return "";
}

inline std::string httpPost(const std::string& url, const std::string& json_body) {
    std::string response;
    bool success = false;
#ifdef USE_CURL
    success = curl_post(url, json_body, response);
#else
    std::string host, path;
    int port = 80;
    parse_url(url, host, port, path);
#ifdef _WIN32
    success = winhttp_post(host, port, path, json_body, response);
#else
    success = posix_post(host, port, path, json_body, response);
#endif
#endif
    if (success) return response;
    return "";
}

// Helper to extract a single string field from JSON response
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

class LLMClient {
private:
    std::string endpoint;
public:
    LLMClient(std::string url) : endpoint(std::move(url)) {}

    std::string prompt(const std::string& prompt_str) {
        std::cout << "\n[Runtime] LLMClient sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
        
        // Escape JSON quotes
        std::string escaped_prompt;
        for (char c : prompt_str) {
            if (c == '"') escaped_prompt += "\\\"";
            else if (c == '\\') escaped_prompt += "\\\\";
            else if (c == '\n') escaped_prompt += "\\n";
            else if (c == '\r') escaped_prompt += "\\r";
            else escaped_prompt += c;
        }

        std::string json_body = "{\"model\": \"llama3\", \"prompt\": \"" + escaped_prompt + "\", \"stream\": false}";

        std::string raw_response;
        bool success = false;
#ifdef USE_CURL
        success = curl_post(endpoint, json_body, raw_response);
#else
        std::string host, path;
        int port = 80;
        parse_url(endpoint, host, port, path);
        if (path == "/") {
            path = "/api/generate";
        }
#ifdef _WIN32
        success = winhttp_post(host, port, path, json_body, raw_response);
#else
        success = posix_post(host, port, path, json_body, raw_response);
#endif
#endif
        if (success) {
            std::string ai_response = extract_json_field(raw_response, "response");
            if (!ai_response.empty()) {
                return ai_response;
            }
        }

        std::cout << "[Runtime Warning] Ollama backend not reachable on " << endpoint << ". Falling back to simulated completion.\n";
        return "- Zenith compiles UI declarations straight to native bindings.\n"
               "- LLM prompts are statically validated at compile-time.\n"
               "- Zero runtime latency wrapper on top of pure C++ loops.";
    }
};

} // namespace zenith

#endif // ZENITH_RUNTIME_H
