#ifndef ZENITH_WINDOWS_H
#define ZENITH_WINDOWS_H

#include "../../common/zenith_common.h"
#include <windows.h>
#include <winhttp.h>
#include <conio.h>
#include <io.h>

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
inline void runInteractiveLoop(AppType& app) {
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
        std::cout << "\n[Windows LLM] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
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
        success = winhttp_post(host, port, path, json_body, raw_response);
#endif
        if (success) {
            std::string ai_response = extract_json_field(raw_response, "response");
            if (!ai_response.empty()) return ai_response;
        }
        return "Windows LLM Offline Fallback";
    }
};

} // namespace zenith

#endif // ZENITH_WINDOWS_H
