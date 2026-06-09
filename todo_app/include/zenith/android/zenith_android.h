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

    std::string prompt(const std::string& prompt_str, const std::string& image_path = "") {
        std::cout << "\n[Android LLM] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
        if (!image_path.empty()) {
            std::cout << "[Android LLM] Attaching image: " << image_path << "\n";
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
        std::cout << "[Android LLM Simulation] (Offline Fallback)\n";
        std::string simulated_resp = "Simulated response for prompt: '" + prompt_str + "'";
        if (!image_path.empty()) {
            simulated_resp += " with image '" + image_path + "'";
        }
        return simulated_resp;
    }

    std::string promptStream(const std::string& prompt_str, std::function<void(const std::string&)> callback, const std::string& image_path = "") {
        std::cout << "\n[Android LLM Stream] Sending prompt to local backend (" << endpoint << "): \"" << prompt_str << "\"\n";
        if (!image_path.empty()) {
            std::cout << "[Android LLM Stream] Attaching image: " << image_path << "\n";
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
        std::cout << "[Android LLM Stream Simulation] (Offline Fallback)\n";
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

// -- Convenience log helper -----------------------------------
inline void android_log(const std::string& msg) {
    LOGI("%s", msg.c_str());
}

// -- Global app instance for callbacks ------------------------
static void* g_zenith_app_instance = nullptr;
static int g_screen_width = 0;
static int g_screen_height = 0;

// -- Touch event handlers -------------------------------------
inline void zenith_android_on_touch(float x, float y, bool isDown) {
    LOGI("Touch event: %s at (%f, %f)", isDown ? "DOWN" : "UP", x, y);
    // TODO: Dispatch to Zenith app instance
    if (g_zenith_app_instance) {
        // Call app's onTouch method if available
    }
}

inline void zenith_android_on_touch_move(float x, float y) {
    LOGI("Touch move: (%f, %f)", x, y);
    // TODO: Dispatch to Zenith app instance
    if (g_zenith_app_instance) {
        // Call app's onTouchMove method if available
    }
}

// -- Initialize Android runtime -------------------------------
inline void zenith_android_init(int width, int height) {
    LOGI("Initializing Zenith Android runtime: %dx%d", width, height);
    g_screen_width = width;
    g_screen_height = height;
    
    // Redirect stdout/stderr to logcat
    redirect_stdout_to_logcat();
    
    LOGI("Zenith Android runtime initialized");
}

// -- Render function ------------------------------------------
inline void zenith_android_render() {
    // TODO: Implement proper GLES2 rendering
    // For now, just clear screen with black
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// -- Execute Zenith code --------------------------------------
inline std::string zenith_android_execute(const char* code) {
    LOGI("Executing Zenith code: %s", code);
    
    // TODO: Integrate with Zenith compiler/runtime
    // For now, return a simple response
    std::string result = "Executed: ";
    result += code;
    
    return result;
}

// -- Rust FFI integration -------------------------------------
inline void zenith_android_init_rust() {
    LOGI("Initializing Rust runtime");
    // TODO: Load and initialize Rust library
}

inline std::string zenith_android_call_rust(const char* functionName, const char* args) {
    LOGI("Calling Rust function: %s with args: %s", functionName, args);
    
    // TODO: Call actual Rust function via FFI
    std::string result = "Rust call result for ";
    result += functionName;
    
    return result;
}

// -- Dynamic library loading ----------------------------------
inline bool zenith_android_load_library(const char* libPath) {
    LOGI("Loading dynamic library: %s", libPath);
    
    // Use dlopen for Android
    void* handle = dlopen(libPath, RTLD_NOW);
    if (!handle) {
        LOGE("Failed to load library: %s", dlerror());
        return false;
    }
    
    LOGI("Library loaded successfully: %s", libPath);
    return true;
}

} // namespace zenith

#endif // ZENITH_ANDROID_H
