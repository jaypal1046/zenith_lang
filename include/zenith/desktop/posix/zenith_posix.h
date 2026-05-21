#ifndef ZENITH_POSIX_H
#define ZENITH_POSIX_H

#include "../../common/zenith_common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>

namespace zenith {

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

inline std::string httpGet(const std::string& url) {
    std::string response;
    bool success = false;
#ifdef USE_CURL
    success = curl_get(url, response);
#else
    std::string host, path;
    int port = 80;
    parse_url(url, host, port, path);
    success = posix_get(host, port, path, response);
#endif
    return success ? response : "";
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
    success = posix_post(host, port, path, json_body, response);
#endif
    return success ? response : "";
}

// On Android, runInteractiveLoop and LLMClient are provided by zenith_android.h
#ifndef __ANDROID__
template<typename AppType>
inline void runInteractiveLoop(AppType& app) {
    if (!isatty(0)) {
        zenith::UIElement root = app.build();
        root.measure();
        root.arrange(0, 0);
        
        std::cout << "\n=== Rendered UI Layout Tree ===\n";
        root.printTree(0);
        
        std::cout << "\n=== Terminal Visual Render (Wow Aesthetics) ===\n";
        TerminalBuffer buffer(root.layout_width, root.layout_height);
        root.drawToBuffer(buffer, 0, 0);
        buffer.print();
        std::cout << "\n";
        return;
    }

    while (true) {
        zenith::UIElement root = app.build();
        root.measure();
        root.arrange(0, 0);
        
        std::vector<const zenith::UIElement*> clickables;
        root.collectClickables(clickables);
        
        std::cout << "\033[2J\033[H";
        std::cout << "\n=== Rendered UI Layout Tree ===\n";
        root.printTree(0);
        
        std::cout << "\n=== Terminal Visual Render (Wow Aesthetics) ===\n";
        TerminalBuffer buffer(root.layout_width, root.layout_height);
        root.drawToBuffer(buffer, 0, 0);
        buffer.print();
        std::cout << "\n";
        
        if (clickables.empty()) break;
        
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
        if (choice == 'q' || choice == 'Q') break;
        
        int idx = choice - '1';
        if (idx >= 0 && idx < (int)clickables.size()) {
            std::string action = clickables[idx]->attributes.at("onClick");
            app.triggerCallback(action);
        }
    }
}

class LLMClient {
private:
    std::string endpoint;
public:
    LLMClient(std::string url) : endpoint(std::move(url)) {}
    std::string prompt(const std::string& prompt_str) {
        std::cout << "\n[POSIX LLM] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
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
        if (path == "/") path = "/api/generate";
        success = posix_post(host, port, path, json_body, raw_response);
#endif
        if (success) {
            std::string ai_response = extract_json_field(raw_response, "response");
            if (!ai_response.empty()) return ai_response;
        }
        return "POSIX LLM Offline Fallback";
    }
};

#endif // __ANDROID__

} // namespace zenith

#endif // ZENITH_POSIX_H
