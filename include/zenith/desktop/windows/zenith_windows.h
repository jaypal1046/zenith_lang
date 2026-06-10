#ifndef ZENITH_WINDOWS_H
#define ZENITH_WINDOWS_H

#include "../../common/zenith_common.h"
#include "../../ui/native_window.h"
#include "../../ui/native_widget_factory.h"
#include <windows.h>
#include <winhttp.h>
#include <conio.h>
#include <io.h>
#include <thread>
#include <chrono>
#include <functional>
#include <memory>

namespace zenith {

inline char get_keyboard_char() {
    return (char)_getch();
}

inline bool winhttp_get(const std::string& host, int port, const std::string& path, std::string& out_response) {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0, dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;

    hSession = WinHttpOpen(L"ZenithRuntime/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession) {
        std::wstring w_host(host.begin(), host.end());
        hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
    }
    if (hConnect) {
        std::wstring w_path(path.begin(), path.end());
        hRequest = WinHttpOpenRequest(hConnect, L"GET", w_path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }
    if (hRequest) {
        bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }
    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    std::string response_data;
    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;
            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer) break;
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

inline bool winhttp_post(const std::string& host, int port, const std::string& path, const std::string& json_body, std::string& out_response) {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0, dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;

    hSession = WinHttpOpen(L"ZenithRuntime/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession) {
        std::wstring w_host(host.begin(), host.end());
        hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
    }
    if (hConnect) {
        std::wstring w_path(path.begin(), path.end());
        hRequest = WinHttpOpenRequest(hConnect, L"POST", w_path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }
    if (hRequest) {
        LPCWSTR pwszHeaders = L"Content-Type: application/json\r\n";
        bResults = WinHttpAddRequestHeaders(hRequest, pwszHeaders, (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
        if (bResults) {
            bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)json_body.c_str(), json_body.length(), json_body.length(), 0);
        }
    }
    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    std::string response_data;
    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;
            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer) break;
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

inline bool winhttp_post_stream(const std::string& host, int port, const std::string& path, const std::string& json_body, std::function<void(const std::string&)> callback) {
    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0, dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;

    hSession = WinHttpOpen(L"ZenithRuntime/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession) {
        std::wstring w_host(host.begin(), host.end());
        hConnect = WinHttpConnect(hSession, w_host.c_str(), port, 0);
    }
    if (hConnect) {
        std::wstring w_path(path.begin(), path.end());
        hRequest = WinHttpOpenRequest(hConnect, L"POST", w_path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    }
    if (hRequest) {
        LPCWSTR pwszHeaders = L"Content-Type: application/json\r\n";
        bResults = WinHttpAddRequestHeaders(hRequest, pwszHeaders, (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);
        if (bResults) {
            bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)json_body.c_str(), json_body.length(), json_body.length(), 0);
        }
    }
    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    std::string line_buffer;
    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;
            pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer) break;
            ZeroMemory(pszOutBuffer, dwSize + 1);
            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                delete[] pszOutBuffer;
                break;
            }
            
            // Process the downloaded chunk
            for (DWORD i = 0; i < dwDownloaded; ++i) {
                char c = pszOutBuffer[i];
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

            delete[] pszOutBuffer;
        } while (dwSize > 0);
    }

    // Flush any remaining content in the buffer (if it doesn't end with a newline)
    if (!line_buffer.empty()) {
        std::string response_chunk = extract_json_field(line_buffer, "response");
        if (!response_chunk.empty()) {
            callback(response_chunk);
        }
    }

    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return bResults;
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
    success = winhttp_get(host, port, path, response);
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
    success = winhttp_post(host, port, path, json_body, response);
#endif
    return success ? response : "";
}

template<typename AppType>
inline void runNativeWindowLoop(AppType& app) {
#ifdef _WIN32
    // Create native window using Win32 backend
    auto window = zenith::ui::createNativeWindow();
    auto widgetFactory = zenith::ui::createNativeWidgetFactory();
    
    // Build initial UI
    zenith::UIElement root = app.build();
    root.measure();
    root.arrange(0, 0);
    
    // Create native window
    window->create("Zenith Application", 800, 600);
    
    // Store app pointer for callbacks
    AppType* appPtr = &app;
    
    // Set event callback to handle native events
    window->setEventCallback([appPtr, &root](const zenith::ui::NativeEvent& event) {
        if (event.type == zenith::ui::EventType::Command) {
            // Handle button clicks and other commands
            // In a full implementation, we'd map HWND IDs to actions
            // For now, this is a placeholder for event dispatch
        } else if (event.type == zenith::ui::EventType::Resize) {
            // Re-layout on resize
            root.measure();
            root.arrange(0, 0, event.width, event.height);
        }
    });
    
    // Create native widgets from UIElement tree
    std::vector<std::unique_ptr<zenith::ui::NativeWidget>> widgets;
    void* parentHandle = window->getNativeHandle();
    
    // Helper lambda to recursively create widgets
    std::function<void(const zenith::UIElement&)> createWidgets = [&](const zenith::UIElement& element) {
        std::unique_ptr<zenith::ui::NativeWidget> widget;
        
        if (element.type == "Button") {
            widget = widgetFactory->createButton(element, parentHandle);
        } else if (element.type == "TextField") {
            widget = widgetFactory->createTextField(element, parentHandle);
        } else if (element.type == "Checkbox") {
            widget = widgetFactory->createCheckbox(element, parentHandle);
        } else if (element.type == "Slider") {
            widget = widgetFactory->createSlider(element, parentHandle);
        } else if (element.type == "Label" || element.type == "Text") {
            widget = widgetFactory->createLabel(element, parentHandle);
        } else if (element.type == "Container" || element.type == "Column" || element.type == "Row") {
            widget = widgetFactory->createContainer(element, parentHandle);
        }
        
        if (widget) {
            // Position and size based on Yoga layout
            widget->setPosition(static_cast<int>(element.layout_x), 
                               static_cast<int>(element.layout_y));
            widget->setSize(static_cast<int>(element.layout_width), 
                           static_cast<int>(element.layout_height));
            
            // Store widget reference
            widgets.push_back(std::move(widget));
        }
        
        // Recursively process children
        for (const auto& child : element.children) {
            createWidgets(*child);
        }
    };
    
    // Create all widgets
    createWidgets(root);
    
    // Show window and run message loop
    window->show();
    
#else
    // Fallback to terminal mode on non-Windows platforms
    runInteractiveLoop(app);
#endif
}

template<typename AppType>
inline void runInteractiveLoop(AppType& app) {
    // Check if we should use native window mode
    bool useNativeWindow = true;  // Default to native window on Windows
    
#ifdef _WIN32
    // On Windows, prefer native window unless running in pure terminal mode
    if (_isatty(0)) {
        // Running in interactive terminal - can still use native window
        runNativeWindowLoop(app);
        return;
    } else {
        // Non-interactive mode (piped input) - render once to terminal
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
#else
    // On non-Windows platforms, use terminal mode until native backends are implemented
    if (!_isatty(0)) {
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
#endif

    // Terminal-based interactive loop (fallback)
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
        
        std::cout << "\n=== Interactive Control Panel ===\n";
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
            } else if (interactives[i]->type == "Checkbox") {
                std::string checked = "false";
                if (interactives[i]->attributes.count("checked")) checked = interactives[i]->attributes.at("checked");
                std::cout << "Checkbox: " << interactives[i]->text_content << " [" << (checked == "true" ? "X" : " ") << "]";
            } else if (interactives[i]->type == "Slider") {
                std::string val = "0", mn = "0", mx = "100";
                if (interactives[i]->attributes.count("value")) val = interactives[i]->attributes.at("value");
                if (interactives[i]->attributes.count("min")) mn = interactives[i]->attributes.at("min");
                if (interactives[i]->attributes.count("max")) mx = interactives[i]->attributes.at("max");
                std::cout << "Slider: [" << mn << ".." << mx << "] Current=" << val;
            } else if (interactives[i]->type == "Toggle") {
                std::string is_on = "false";
                if (interactives[i]->attributes.count("isOn")) is_on = interactives[i]->attributes.at("isOn");
                std::cout << "Toggle: " << interactives[i]->text_content << " [" << (is_on == "true" ? "ON" : "OFF") << "]";
            } else if (interactives[i]->type == "Dropdown") {
                std::string val = "";
                if (interactives[i]->attributes.count("value")) val = interactives[i]->attributes.at("value");
                std::cout << "Dropdown: options=\"" << interactives[i]->text_content << "\" Selected=\"" << val << "\"";
            }
            std::cout << "\n";
        }
        std::cout << "[Q] Quit Application\n";
        std::cout << "Choose an option: ";
        
        char choice = zenith::get_keyboard_char();
        if (choice == 'q' || choice == 'Q') break;
        
        int idx = choice - '1';
        if (idx >= 0 && idx < (int)interactives.size()) {
            const auto* el = interactives[idx];
            if (el->type == "Button") {
                std::string action = el->attributes.at("onClick");
                app.triggerCallback(action);
            } else if (el->type == "TextField") {
                std::string action = el->attributes.at("onChange");
                std::cout << "\nEnter new value: ";
                std::string input_val;
                std::getline(std::cin, input_val);
                app.triggerCallback(action, input_val);
            } else if (el->type == "Checkbox") {
                std::string action = el->attributes.count("onChange") ? el->attributes.at("onChange") : "";
                if (!action.empty()) {
                    // Toggle the current state
                    std::string cur = el->attributes.count("checked") ? el->attributes.at("checked") : "false";
                    app.triggerCallback(action, cur == "true" ? "false" : "true");
                }
            } else if (el->type == "Slider") {
                std::string action = el->attributes.count("onChange") ? el->attributes.at("onChange") : "";
                if (!action.empty()) {
                    std::cout << "\nEnter slider value: ";
                    std::string input_val;
                    std::getline(std::cin, input_val);
                    app.triggerCallback(action, input_val);
                }
            } else if (el->type == "Toggle") {
                std::string action = el->attributes.count("onChange") ? el->attributes.at("onChange") : "";
                if (!action.empty()) {
                    std::string cur = el->attributes.count("isOn") ? el->attributes.at("isOn") : "false";
                    app.triggerCallback(action, cur == "true" ? "false" : "true");
                }
            } else if (el->type == "Dropdown") {
                std::string action = el->attributes.count("onChange") ? el->attributes.at("onChange") : "";
                if (!action.empty()) {
                    std::cout << "\nEnter selection: ";
                    std::string input_val;
                    std::getline(std::cin, input_val);
                    app.triggerCallback(action, input_val);
                }
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
        std::cout << "\n[Windows LLM] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
        if (!image_path.empty()) {
            std::cout << "[Windows LLM] Attaching image: " << image_path << "\n";
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
        success = winhttp_post(host, port, path, json_body, raw_response);
#endif
        if (success) {
            std::string ai_response = extract_json_field(raw_response, "response");
            if (!ai_response.empty()) return ai_response;
        }
        
        // High-fidelity simulation mode when offline/failed
        std::cout << "[Windows LLM Simulation] (Offline Fallback)\n";
        std::string simulated_resp = "Simulated response for prompt: '" + prompt_str + "'";
        if (!image_path.empty()) {
            simulated_resp += " with image '" + image_path + "'";
        }
        return simulated_resp;
    }

    std::string promptStream(const std::string& prompt_str, std::function<void(const std::string&)> callback, const std::string& image_path = "") {
        std::cout << "\n[Windows LLM Stream] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
        if (!image_path.empty()) {
            std::cout << "[Windows LLM Stream] Attaching image: " << image_path << "\n";
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
        
        success = winhttp_post_stream(host, port, path, json_body, stream_callback);
        
        if (success && !accumulated.empty()) {
            return accumulated;
        }
        
        // High-fidelity simulation mode when offline/failed
        std::cout << "[Windows LLM Stream Simulation] (Offline Fallback)\n";
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

} // namespace zenith

#endif // ZENITH_WINDOWS_H
