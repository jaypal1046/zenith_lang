// Zenith Native Android Entry Point (NDK)
// This is the main entry point for the Android NDK build
#include <android/native_activity.h>
#include <android/log.h>
#include <android/input.h>
#include <android/sensor.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "../include/zenith_runtime.h"
#include "../include/zenith_android.h"

#define LOG_TAG "ZenithApp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global state
static ANativeActivity* g_activity = nullptr;
static ANativeWindow* g_window = nullptr;
static bool g_running = false;
static pthread_t g_render_thread;

// EGL context
static EGLDisplay g_display = EGL_NO_DISPLAY;
static EGLSurface g_surface = EGL_NO_SURFACE;
static EGLContext g_context = EGL_NO_CONTEXT;

// Touch state
static float g_touch_x = 0.0f;
static float g_touch_y = 0.0f;
static bool g_touching = false;

// Forward declarations
static void engine_init_display();
static void engine_draw_frame();
static void engine_term_display();
static void* render_thread_func(void* arg);
static int32_t handle_input(AInputEvent* event);
static void handle_cmd(int32_t cmd);

// Input callback
static int32_t engine_handle_input(AInputEvent* event) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> 
                               AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        action &= AMOTION_EVENT_ACTION_MASK;
        
        switch (action) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                g_touch_x = AMotionEvent_getX(event, pointerIndex);
                g_touch_y = AMotionEvent_getY(event, pointerIndex);
                g_touching = true;
                LOGI("Touch DOWN: x=%f, y=%f", g_touch_x, g_touch_y);
                // Call Zenith touch handler
                zenith_android_on_touch(g_touch_x, g_touch_y, true);
                return 1;
                
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                g_touching = false;
                LOGI("Touch UP");
                zenith_android_on_touch(g_touch_x, g_touch_y, false);
                return 1;
                
            case AMOTION_EVENT_ACTION_MOVE:
                g_touch_x = AMotionEvent_getX(event, 0);
                g_touch_y = AMotionEvent_getY(event, 0);
                LOGI("Touch MOVE: x=%f, y=%f", g_touch_x, g_touch_y);
                zenith_android_on_touch_move(g_touch_x, g_touch_y);
                return 1;
        }
    }
    return 0;
}

// Command callback
static void engine_handle_cmd(int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (g_activity->window != NULL) {
                g_window = g_activity->window;
                engine_init_display();
                engine_draw_frame();
            }
            break;
            
        case APP_CMD_TERM_WINDOW:
            engine_term_display();
            break;
            
        case APP_CMD_GAINED_FOCUS:
            g_running = true;
            break;
            
        case APP_CMD_LOST_FOCUS:
            g_running = false;
            break;
            
        case APP_CMD_PAUSE:
            g_running = false;
            break;
            
        case APP_CMD_RESUME:
            g_running = true;
            break;
            
        case APP_CMD_DESTROY:
            g_running = false;
            break;
    }
}

// Initialize EGL display
static void engine_init_display() {
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };
    
    g_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_display == EGL_NO_DISPLAY) {
        LOGE("Failed to get EGL display");
        return;
    }
    
    if (!eglInitialize(g_display, NULL, NULL)) {
        LOGE("Failed to initialize EGL");
        g_display = EGL_NO_DISPLAY;
        return;
    }
    
    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(g_display, attribs, &config, 1, &numConfigs)) {
        LOGE("Failed to choose EGL config");
        engine_term_display();
        return;
    }
    
    g_surface = eglCreateWindowSurface(g_display, config, g_window, NULL);
    if (g_surface == EGL_NO_SURFACE) {
        LOGE("Failed to create EGL surface");
        engine_term_display();
        return;
    }
    
    const EGLint ctxAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    g_context = eglCreateContext(g_display, config, NULL, ctxAttribs);
    if (g_context == EGL_NO_CONTEXT) {
        LOGE("Failed to create EGL context");
        engine_term_display();
        return;
    }
    
    if (eglMakeCurrent(g_display, g_surface, g_surface, g_context) == EGL_FALSE) {
        LOGE("Failed to make EGL context current");
        engine_term_display();
        return;
    }
    
    LOGI("EGL initialized successfully");
    
    // Initialize Zenith Android runtime with window info
    int32_t width = ANativeWindow_getWidth(g_window);
    int32_t height = ANativeWindow_getHeight(g_window);
    zenith_android_init(width, height);
}

// Draw a frame
static void engine_draw_frame() {
    if (g_display == EGL_NO_DISPLAY) return;
    
    // Clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render Zenith UI
    zenith_android_render();
    
    eglSwapBuffers(g_display, g_surface);
}

// Terminate EGL display
static void engine_term_display() {
    if (g_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_context != EGL_NO_CONTEXT) {
            eglDestroyContext(g_display, g_context);
        }
        if (g_surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_display, g_surface);
        }
        eglTerminate(g_display);
    }
    
    g_display = EGL_NO_DISPLAY;
    g_surface = EGL_NO_SURFACE;
    g_context = EGL_NO_CONTEXT;
}

// Render thread
static void* render_thread_func(void* arg) {
    LOGI("Render thread started");
    
    while (g_running) {
        engine_draw_frame();
        usleep(16000); // ~60 FPS
    }
    
    LOGI("Render thread stopped");
    return NULL;
}

// NativeActivity onCreate callback
extern "C" void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    LOGI("Zenith App Starting...");
    
    g_activity = activity;
    
    // Set callbacks
    activity->callbacks->onInputEvent = [](ANativeActivity* act, AInputEvent* event) -> int32_t {
        return engine_handle_input(event);
    };
    
    activity->callbacks->onContentRectChanged = [](ANativeActivity* act, const ARect* rect) {
        // Handle content rect changes
    };
    
    activity->callbacks->onWindowFocusChanged = [](ANativeActivity* act, int hasFocus) {
        if (hasFocus) {
            g_running = true;
        } else {
            g_running = false;
        }
    };
    
    activity->callbacks->onNativeWindowCreated = [](ANativeActivity* act, ANativeWindow* window) {
        g_window = window;
        engine_init_display();
        
        // Start render thread
        g_running = true;
        pthread_create(&g_render_thread, NULL, render_thread_func, NULL);
    };
    
    activity->callbacks->onNativeWindowDestroyed = [](ANativeActivity* act, ANativeWindow* window) {
        g_running = false;
        pthread_join(g_render_thread, NULL);
        engine_term_display();
        g_window = nullptr;
    };
    
    activity->callbacks->onPause = [](ANativeActivity* act) {
        g_running = false;
    };
    
    activity->callbacks->onResume = [](ANativeActivity* act) {
        g_running = true;
    };
    
    activity->callbacks->onDestroy = [](ANativeActivity* act) {
        g_running = false;
        if (g_render_thread) {
            pthread_join(g_render_thread, NULL);
        }
        engine_term_display();
        LOGI("Zenith App Destroyed");
    };
    
    LOGI("Zenith App initialized successfully");
}

// JNI functions for Dart/Rust communication
extern "C" {

// Called from Dart to execute Zenith code
JNIEXPORT jstring JNICALL Java_com_zenith_template_MainActivity_executeZenithCode(
    JNIEnv* env, jobject thiz, jstring code) {
    
    const char* codeStr = env->GetStringUTFChars(code, NULL);
    LOGI("Executing Zenith code from Dart: %s", codeStr);
    
    // Execute the code through Zenith runtime
    std::string result = zenith_android_execute(codeStr);
    
    env->ReleaseStringUTFChars(code, codeStr);
    return env->NewStringUTF(result.c_str());
}

// Called from Dart to call Rust function
JNIEXPORT jstring JNICALL Java_com_zenith_template_MainActivity_callRustFunction(
    JNIEnv* env, jobject thiz, jstring functionName, jstring args) {
    
    const char* funcName = env->GetStringUTFChars(functionName, NULL);
    const char* funcArgs = env->GetStringUTFChars(args, NULL);
    
    LOGI("Calling Rust function: %s with args: %s", funcName, funcArgs);
    
    // Call through Rust FFI
    std::string result = zenith_android_call_rust(funcName, funcArgs);
    
    env->ReleaseStringUTFChars(functionName, funcName);
    env->ReleaseStringUTFChars(args, funcArgs);
    
    return env->NewStringUTF(result.c_str());
}

// Initialize Rust runtime
JNIEXPORT void JNICALL Java_com_zenith_template_MainActivity_initRustRuntime(JNIEnv* env, jobject thiz) {
    LOGI("Initializing Rust runtime");
    zenith_android_init_rust();
}

// Load dynamic library
JNIEXPORT jboolean JNICALL Java_com_zenith_template_MainActivity_loadDynamicLibrary(
    JNIEnv* env, jobject thiz, jstring libPath) {
    
    const char* path = env->GetStringUTFChars(libPath, NULL);
    LOGI("Loading dynamic library: %s", path);
    
    bool success = zenith_android_load_library(path);
    
    env->ReleaseStringUTFChars(libPath, path);
    return success ? JNI_TRUE : JNI_FALSE;
}

} // extern "C"
