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
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#endif

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

    // For Text component
    static UIElement Text(std::string text, std::unordered_map<std::string, std::string> attrs = {}) {
        UIElement el("Text");
        el.text_content = std::move(text);
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

    // Compute layout sizes
    void measure() const {
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
            layout_width = max_w;
            layout_height = h;
        } else if (type == "Column") {
            int max_w = 0;
            int total_h = 0;
            for (const auto& child : children) {
                child.measure();
                if (child.layout_width > max_w) {
                    max_w = child.layout_width;
                }
                total_h += child.layout_height;
            }
            layout_width = max_w + 2;
            layout_height = total_h + 2;
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
            layout_width = total_w + 2;
            layout_height = max_h + 2;
        }
    }

    // Position children recursively
    void arrange(int x, int y) const {
        layout_x = x;
        layout_y = y;
        if (type == "Column") {
            int current_y = y + 1;
            for (const auto& child : children) {
                child.arrange(x + 1, current_y);
                current_y += child.layout_height;
            }
        } else if (type == "Row") {
            int current_x = x + 1;
            for (const auto& child : children) {
                child.arrange(current_x, y + 1);
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
        if (type == "Text") {
            std::cout << ">" << text_content << "</Text>\n";
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
        
        if (type == "Column") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[36m");
        } else if (type == "Row") {
            buffer.drawBox(abs_x, abs_y, layout_width, layout_height, "single", "\033[35m");
        }
        
        if (type == "Text") {
            std::string color = "\033[37m";
            if (attributes.count("fontWeight") && attributes.at("fontWeight") == "bold") {
                color = "\033[1;37m";
            } else if (text_content.find("[Status:") != std::string::npos) {
                color = "\033[1;32m";
            } else if (text_content.find("Counter Value:") != std::string::npos) {
                color = "\033[1;33m";
            } else if (text_content.find("- Zenith") != std::string::npos || text_content.find("- LLM") != std::string::npos) {
                color = "\033[32m";
            }
            buffer.writeString(abs_x, abs_y, text_content, color);
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

#ifdef _WIN32
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
        
        std::string host, path;
        int port = 80;
        parse_url(endpoint, host, port, path);
        if (path == "/") {
            path = "/api/generate";
        }

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
#ifdef _WIN32
        success = winhttp_post(host, port, path, json_body, raw_response);
#else
        success = posix_post(host, port, path, json_body, raw_response);
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
