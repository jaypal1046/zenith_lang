// zenith_plugin_host.cpp
// Implementation of the plugin host that bridges Zenith VM with Dart and Rust

#include "zenith_plugin_host.h"
#include <map>
#include <string>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <jni.h>
#include <android/log.h>

#define LOG_TAG "ZenithPluginHost"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ------------------------------------------------------------------
// Plugin Registry
// ------------------------------------------------------------------

struct PluginInfo {
    std::string dart_channel;
    std::string rust_prefix;
    bool is_dart;
};

static std::map<std::string, PluginInfo> g_plugin_registry;
static std::mutex g_registry_mutex;
static JavaVM* g_jvm = nullptr;
static jobject g_activity_instance = nullptr;

// ------------------------------------------------------------------
// Initialization
// ------------------------------------------------------------------

void zenith_plugin_host_init(JavaVM* jvm, jobject activity) {
    g_jvm = jvm;
    if (activity != nullptr) {
        JNIEnv* env;
        if (jvm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK) {
            g_activity_instance = env->NewGlobalRef(activity);
        }
    }
    LOGI("Plugin host initialized");
}

// ------------------------------------------------------------------
// Registration
// ------------------------------------------------------------------

bool zenith_register_dart_plugin(const char* name, const char* dart_channel) {
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    if (!name || !dart_channel) return false;
    
    PluginInfo info;
    info.dart_channel = dart_channel;
    info.is_dart = true;
    g_plugin_registry[name] = info;
    
    LOGI("Registered Dart plugin: %s (channel: %s)", name, dart_channel);
    return true;
}

bool zenith_register_rust_plugin(const char* name, const char* rust_prefix) {
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    if (!name || !rust_prefix) return false;
    
    PluginInfo info;
    info.rust_prefix = rust_prefix;
    info.is_dart = false;
    g_plugin_registry[name] = info;
    
    LOGI("Registered Rust plugin: %s (prefix: %s)", name, rust_prefix);
    return true;
}

// ------------------------------------------------------------------
// Dart Plugin Calls (via JNI)
// ------------------------------------------------------------------

char* zenith_call_dart_plugin(const char* plugin_name, const char* method, const char* args_json) {
    if (!g_jvm || !g_activity_instance) {
        LOGE("JVM or Activity not initialized");
        return strdup("{\"error\":\"Plugin host not initialized\"}");
    }
    
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    auto it = g_plugin_registry.find(plugin_name);
    if (it == g_plugin_registry.end() || !it->second.is_dart) {
        LOGE("Dart plugin not found: %s", plugin_name);
        return strdup("{\"error\":\"Plugin not registered\"}");
    }
    
    std::string channel = it->second.dart_channel;
    
    // Attach to JVM
    JNIEnv* env = nullptr;
    bool attached = false;
    if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            LOGE("Failed to attach to JVM");
            return strdup("{\"error\":\"JVM attachment failed\"}");
        }
        attached = true;
    }
    
    // Find Activity class and method
    jclass activity_class = env->GetObjectClass(g_activity_instance);
    jmethodID call_plugin_method = env->GetMethodID(activity_class, "callDartPlugin", 
                                                     "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    
    if (!call_plugin_method) {
        LOGE("Method callDartPlugin not found in Activity");
        if (attached) g_jvm->DetachCurrentThread();
        return strdup("{\"error\":\"Method not found\"}");
    }
    
    // Call the method
    jstring j_plugin = env->NewStringUTF(plugin_name);
    jstring j_method = env->NewStringUTF(method);
    jstring j_args = env->NewStringUTF(args_json);
    
    jstring j_result = (jstring)env->CallObjectMethod(g_activity_instance, call_plugin_method, 
                                                       j_plugin, j_method, j_args);
    
    // Get result
    const char* result_str = env->GetStringUTFChars(j_result, nullptr);
    char* result = strdup(result_str);
    env->ReleaseStringUTFChars(j_result, result_str);
    
    // Cleanup
    env->DeleteLocalRef(j_plugin);
    env->DeleteLocalRef(j_method);
    env->DeleteLocalRef(j_args);
    env->DeleteLocalRef(j_result);
    
    if (attached) g_jvm->DetachCurrentThread();
    
    LOGI("Called Dart plugin %s.%s -> %s", plugin_name, method, result);
    return result;
}

// ------------------------------------------------------------------
// Rust Plugin Calls (via direct FFI)
// ------------------------------------------------------------------

// Function pointer type for Rust plugin functions
typedef char* (*RustPluginFunc)(const char* args_json);

char* zenith_call_rust_plugin(const char* plugin_name, const char* function, const char* args_json) {
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    auto it = g_plugin_registry.find(plugin_name);
    if (it == g_plugin_registry.end() || it->second.is_dart) {
        LOGE("Rust plugin not found: %s", plugin_name);
        return strdup("{\"error\":\"Plugin not registered\"}");
    }
    
    std::string prefix = it->second.rust_prefix;
    std::string func_name = prefix + "_" + function;
    
    // In a real implementation, we would dlsym the function from the loaded .so
    // For now, we'll use a hardcoded example for demonstration
    
    // Example: If plugin_name="crypto" and function="hash", look for "crypto_hash"
    // This would be loaded via dlopen/dlsym in production
    
    LOGI("Calling Rust plugin: %s (function: %s)", func_name.c_str(), function);
    
    // TODO: Implement dynamic loading from Rust .so libraries
    // For now, return a mock response
    return strdup("{\"result\":\"mock_rust_response\",\"plugin\": \"") + plugin_name + "\"}";
}

// ------------------------------------------------------------------
// Event Pushing (From Native/Dart/Rust to Zenith VM)
// ------------------------------------------------------------------

// This function would be called by Dart/Rust plugins when they have events
// It needs to dispatch to the Zenith VM's event queue
extern void zenith_vm_dispatch_event(const char* event_name, const char* data_json);

void zenith_push_event_to_vm(const char* event_name, const char* data_json) {
    if (!event_name || !data_json) return;
    
    LOGI("Pushing event to VM: %s", event_name);
    
    // Dispatch to Zenith VM
    zenith_vm_dispatch_event(event_name, data_json);
}

// ------------------------------------------------------------------
// Cleanup
// ------------------------------------------------------------------

void zenith_plugin_host_shutdown() {
    std::lock_guard<std::mutex> lock(g_registry_mutex);
    g_plugin_registry.clear();
    
    if (g_activity_instance && g_jvm) {
        JNIEnv* env;
        if (g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK) {
            env->DeleteGlobalRef(g_activity_instance);
        }
        g_activity_instance = nullptr;
    }
    
    LOGI("Plugin host shutdown complete");
}
