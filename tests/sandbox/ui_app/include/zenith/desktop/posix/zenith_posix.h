#ifndef ZENITH_POSIX_H
#define ZENITH_POSIX_H

#include "../../common/zenith_common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>
#include <thread>
#include <chrono>
#include <functional>

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

inline bool posix_post_stream(const std::string& host, int port, const std::string& path, const std::string& json_body, std::function<void(const std::string&)> callback) {
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
    tv.tv_sec = 5;
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
    std::string response_buffer;
    int bytes_read;
    bool in_body = false;
    std::string line_buffer;

    while ((bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        response_buffer.append(buffer, bytes_read);

        if (!in_body) {
            size_t body_pos_idx = response_buffer.find("\r\n\r\n");
            if (body_pos_idx != std::string::npos) {
                in_body = true;
                std::string body_part = response_buffer.substr(body_pos_idx + 4);
                response_buffer = body_part;
            }
        }

        if (in_body) {
            for (char c : response_buffer) {
                if (c == '\n') {
                    if (!line_buffer.empty()) {
                        std::string response_chunk = extract_json_field(line_buffer, "response");
                        if (!response_chunk.empty()) {
                            callback(response_chunk);
                        }
                        line_buffer.clear();
                    }
                } else {
                    line_buffer.push_back(c);
                }
            }
            response_buffer.clear();
        }
    }

    if (!line_buffer.empty()) {
        std::string response_chunk = extract_json_field(line_buffer, "response");
        if (!response_chunk.empty()) {
            callback(response_chunk);
        }
    }

    close(sock);
    return true;
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
        
        std::vector<const zenith::UIElement*> interactives;
        root.collectInteractives(interactives);
        
        std::cout << "\033[2J\033[H";
        std::cout << "\n=== Rendered UI Layout Tree ===\n";
        root.printTree(0);
        
        std::cout << "\n=== Terminal Visual Render (Wow Aesthetics) ===\n";
        TerminalBuffer buffer(root.layout_width, root.layout_height);
        root.drawToBuffer(buffer, 0, 0);
        buffer.print();
        std::cout << "\n";
        
        if (interactives.empty()) break;
        
        std::cout << "=== Interactive Control Panel ===\n";
        for (size_t i = 0; i < interactives.size(); ++i) {
            std::cout << "[" << (i + 1) << "] ";
            if (interactives[i]->type == "Button") {
                std::cout << "Button: " << interactives[i]->text_content;
                if (interactives[i]->attributes.count("onClick")) {
                    std::cout << " (Action: " << interactives[i]->attributes.at("onClick") << ")";
                }
            } else if (interactives[i]->type == "TextField") {
                std::cout << "Input: " << (interactives[i]->attributes.count("placeholder") ? interactives[i]->attributes.at("placeholder") : interactives[i]->text_content);
                std::string val = "";
                if (interactives[i]->attributes.count("value")) {
                    val = interactives[i]->attributes.at("value");
                }
                std::cout << " [Current: \"" << val << "\"]";
                if (interactives[i]->attributes.count("onChange")) {
                    std::cout << " (Action: " << interactives[i]->attributes.at("onChange") << ")";
                }
            }
            std::cout << "\n";
        }
        std::cout << "[Q] Quit Application\n";
        std::cout << "Choose an option: ";
        
        char choice = zenith::get_keyboard_char();
        if (choice == 'q' || choice == 'Q') break;
        
        int idx = choice - '1';
        if (idx >= 0 && idx < (int)interactives.size()) {
            if (interactives[idx]->type == "Button") {
                std::string action = interactives[idx]->attributes.at("onClick");
                app.triggerCallback(action);
            } else if (interactives[idx]->type == "TextField") {
                std::string action = interactives[idx]->attributes.at("onChange");
                std::cout << "\nEnter new value: ";
                std::string input_val;
                std::getline(std::cin, input_val);
                app.triggerCallback(action, input_val);
            }
        }
    }
}

class LLMClient {
private:
    std::string endpoint;
public:
    LLMClient(std::string url) : endpoint(std::move(url)) {}
    
    std::string prompt(const std::string& prompt_str, const std::string& image_path = "") {
        std::cout << "\n[POSIX LLM] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
        if (!image_path.empty()) {
            std::cout << "[POSIX LLM] Attaching image: " << image_path << "\n";
        }
        std::string escaped_prompt;
        for (char c : prompt_str) {
            if (c == '"') escaped_prompt += "\\\"";
            else if (c == '\\') escaped_prompt += "\\\\";
            else if (c == '\n') escaped_prompt += "\\n";
            else if (c == '\r') escaped_prompt += "\\r";
            else escaped_prompt += c;
        }
        
        std::string images_json = "";
        if (!image_path.empty()) {
            std::string b64 = zenith::base64_encode_file(image_path);
            if (!b64.empty()) {
                images_json = ", \"images\": [\"" + b64 + "\"]";
            }
        }
        
        std::string json_body = "{\"model\": \"llama3\", \"prompt\": \"" + escaped_prompt + "\", \"stream\": false" + images_json + "}";
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
        
        // High-fidelity simulation mode when offline/failed
        std::cout << "[POSIX LLM Simulation] (Offline Fallback)\n";
        std::string simulated_resp = "Simulated response for prompt: '" + prompt_str + "'";
        if (!image_path.empty()) {
            simulated_resp += " with image '" + image_path + "'";
        }
        return simulated_resp;
    }

    std::string promptStream(const std::string& prompt_str, std::function<void(const std::string&)> callback, const std::string& image_path = "") {
        std::cout << "\n[POSIX LLM Stream] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
        if (!image_path.empty()) {
            std::cout << "[POSIX LLM Stream] Attaching image: " << image_path << "\n";
        }
        std::string escaped_prompt;
        for (char c : prompt_str) {
            if (c == '"') escaped_prompt += "\\\"";
            else if (c == '\\') escaped_prompt += "\\\\";
            else if (c == '\n') escaped_prompt += "\\n";
            else if (c == '\r') escaped_prompt += "\\r";
            else escaped_prompt += c;
        }
        
        std::string images_json = "";
        if (!image_path.empty()) {
            std::string b64 = zenith::base64_encode_file(image_path);
            if (!b64.empty()) {
                images_json = ", \"images\": [\"" + b64 + "\"]";
            }
        }
        
        std::string json_body = "{\"model\": \"llama3\", \"prompt\": \"" + escaped_prompt + "\", \"stream\": true" + images_json + "}";
        
        std::string accumulated;
        auto stream_callback = [&](const std::string& chunk) {
            accumulated += chunk;
            callback(chunk);
        };
        
        bool success = false;
        std::string host, path;
        int port = 80;
        parse_url(endpoint, host, port, path);
        if (path == "/") path = "/api/generate";
        
        success = posix_post_stream(host, port, path, json_body, stream_callback);
        
        if (success && !accumulated.empty()) {
            return accumulated;
        }
        
        // High-fidelity simulation mode when offline/failed
        std::cout << "[POSIX LLM Stream Simulation] (Offline Fallback)\n";
        std::string simulated_resp = "Simulated streaming response for prompt: '" + prompt_str + "'";
        if (!image_path.empty()) {
            simulated_resp += " with image '" + image_path + "'";
        }
        
        // Output token by token with micro-delays
        std::string word;
        std::stringstream ss(simulated_resp);
        while (ss >> word) {
            callback(word + " ");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        callback("\n");
        return simulated_resp + "\n";
    }
};

#endif // __ANDROID__

} // namespace zenith

#endif // ZENITH_POSIX_H
