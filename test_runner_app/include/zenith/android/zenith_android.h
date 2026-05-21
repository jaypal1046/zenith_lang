#ifndef ZENITH_ANDROID_H
#define ZENITH_ANDROID_H

// ============================================================
//  zenith_android.h  —  Android Platform Runtime (NDK)
//  Provides:
//    * Logcat redirection (stdout/stderr -> __android_log_print)
//    * Android-specific runInteractiveLoop (render-once, no TTY)
//    * Android LLMClient (POSIX sockets via NDK)
// ============================================================

#include "../desktop/posix/zenith_posix.h"
#include <android/log.h>
#include <iostream>
#include <streambuf>
#include <string>
#include <unistd.h>

#define LOG_TAG "ZenithAndroid"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace zenith {

// -- Android Logcat streambuf ---------------------------------
// Writes every line to Logcat AND to the original streambuf
// so output appears both in `adb shell` and `adb logcat`.
class AndroidLogBuf : public std::streambuf {
public:
    explicit AndroidLogBuf(std::streambuf* original) : orig_buf(original) {}

protected:
    int_type overflow(int_type c) override {
        if (c != EOF) {
            if (orig_buf) orig_buf->sputc(c);
            if (c == '\n') {
                LOGI("%s", buffer.c_str());
                buffer.clear();
            } else {
                buffer.push_back(static_cast<char>(c));
            }
        }
        return c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override {
        if (orig_buf) orig_buf->sputn(s, n);
        for (std::streamsize i = 0; i < n; ++i) {
            if (s[i] == '\n') {
                LOGI("%s", buffer.c_str());
                buffer.clear();
            } else {
                buffer.push_back(s[i]);
            }
        }
        return n;
    }

private:
    std::string buffer;
    std::streambuf* orig_buf;
};

// -- Global stdout/stderr redirect ----------------------------
inline void redirect_stdout_to_logcat() {
    static AndroidLogBuf out_logbuf(std::cout.rdbuf());
    static AndroidLogBuf err_logbuf(std::cerr.rdbuf());
    std::cout.rdbuf(&out_logbuf);
    std::cerr.rdbuf(&err_logbuf);
}

struct AndroidStdoutRedirector {
    AndroidStdoutRedirector() { redirect_stdout_to_logcat(); }
};

// This static object performs the redirect before main() runs.
static AndroidStdoutRedirector g_android_stdout_redirector;

// -- Android runInteractiveLoop -------------------------------
// On Android there is no TTY/keyboard, so we render the UI
// tree exactly once, log it, and return immediately.
template<typename AppType>
inline void runInteractiveLoop(AppType& app) {
    UIElement root = app.build();
    root.measure();
    root.arrange(0, 0);

    std::cout << "\n=== Rendered UI Layout Tree ===\n";
    root.printTree(0);

    std::cout << "\n=== Terminal Visual Render (Wow Aesthetics) ===\n";
    TerminalBuffer buffer(root.layout_width, root.layout_height);
    root.drawToBuffer(buffer, 0, 0);
    buffer.print();
    std::cout << "\n";
    // No interactive loop -- Android apps use touch/JNI callbacks.
}

// -- Android LLMClient ----------------------------------------
// Reuses the POSIX socket implementation from zenith_posix.h.
class LLMClient {
private:
    std::string endpoint;
public:
    explicit LLMClient(std::string url) : endpoint(std::move(url)) {}

    std::string prompt(const std::string& prompt_str) {
        std::cout << "\n[Android LLM] Sending prompt to (" << endpoint << "): \""
                  << prompt_str << "\"\n";
        std::string escaped;
        for (char c : prompt_str) {
            if      (c == '"')  escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else if (c == '\n') escaped += "\\n";
            else if (c == '\r') escaped += "\\r";
            else                escaped += c;
        }
        std::string json_body = "{\"model\": \"llama3\", \"prompt\": \""
                              + escaped + "\", \"stream\": false}";
        std::string host, path, raw_response;
        int port = 80;
        parse_url(endpoint, host, port, path);
        if (path == "/") path = "/api/generate";
        if (posix_post(host, port, path, json_body, raw_response)) {
            std::string ai = extract_json_field(raw_response, "response");
            if (!ai.empty()) return ai;
        }
        return "Android LLM Offline Fallback";
    }
};

// -- Convenience log helper -----------------------------------
inline void android_log(const std::string& msg) {
    LOGI("%s", msg.c_str());
}

} // namespace zenith

#endif // ZENITH_ANDROID_H
