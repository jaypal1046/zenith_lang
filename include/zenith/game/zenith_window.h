#ifndef ZENITH_WINDOW_H
#define ZENITH_WINDOW_H

#include "../common/zenith_common.h"
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <cmath>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/gl.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

namespace zenith {

enum class KeyCode {
    Unknown = 0,
    Space = 32,
    A = 'A', B = 'B', C = 'C', D = 'D', E = 'E', F = 'F', G = 'G', H = 'H',
    I = 'I', J = 'J', K = 'K', L = 'L', M = 'M', N = 'N', O = 'O', P = 'P',
    Q = 'Q', R = 'R', S = 'S', T = 'T', U = 'U', V = 'V', W = 'W', X = 'X',
    Y = 'Y', Z = 'Z',
    Escape = 27,
    Enter = 13,
    Up = 38, Down = 40, Left = 37, Right = 39
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2
};

struct WindowConfig {
    std::string title = "Zenith Game Engine";
    int width = 1280;
    int height = 720;
    bool vsync = true;
    bool resizable = true;
    bool fullscreen = false;
    float targetFps = 60.0f;
};

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    static Color White() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static Color Black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static Color Red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static Color Green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static Color Blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static Color Yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static Color Cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static Color Magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }
    static Color Clear() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
};

struct Camera2D {
    float positionX = 0.0f;
    float positionY = 0.0f;
    float zoom = 1.0f;
    float rotation = 0.0f;
};

class Input {
private:
    static inline std::unordered_map<int, bool> s_keysDown;
    static inline std::unordered_map<int, bool> s_keysPressed;
    static inline float s_mouseX = 0.0f;
    static inline float s_mouseY = 0.0f;
    static inline bool s_mouseButtons[3] = {false, false, false};

public:
    static void updateKey(int key, bool pressed) {
        if (!s_keysDown[key] && pressed) {
            s_keysPressed[key] = true;
        }
        s_keysDown[key] = pressed;
    }

    static void updateMousePosition(float x, float y) {
        s_mouseX = x;
        s_mouseY = y;
    }

    static void updateMouseButton(int button, bool pressed) {
        if (button >= 0 && button < 3) {
            s_mouseButtons[button] = pressed;
        }
    }

    static void endFrame() {
        s_keysPressed.clear();
    }

    static bool isKeyDown(KeyCode key) {
        return s_keysDown[static_cast<int>(key)];
    }

    static bool isKeyPressed(KeyCode key) {
        return s_keysPressed[static_cast<int>(key)];
    }

    static float getMouseX() { return s_mouseX; }
    static float getMouseY() { return s_mouseY; }
    static bool isMouseButtonDown(MouseButton button) {
        int idx = static_cast<int>(button);
        return (idx >= 0 && idx < 3) ? s_mouseButtons[idx] : false;
    }
};

struct ActionBinding {
    KeyCode key = KeyCode::Unknown;
    MouseButton mouseButton = static_cast<MouseButton>(-1);
};

class InputMap {
public:
    static InputMap& instance() {
        static InputMap map;
        return map;
    }

    void bindKey(const std::string& actionName, KeyCode key) {
        m_bindings[actionName].key = key;
    }

    void bindMouseButton(const std::string& actionName, MouseButton button) {
        m_bindings[actionName].mouseButton = button;
    }

    bool isActionDown(const std::string& actionName) const {
        auto it = m_bindings.find(actionName);
        if (it == m_bindings.end()) return false;
        if (it->second.key != KeyCode::Unknown) {
            return Input::isKeyDown(it->second.key);
        }
        if (static_cast<int>(it->second.mouseButton) >= 0) {
            return Input::isMouseButtonDown(it->second.mouseButton);
        }
        return false;
    }

    bool isActionPressed(const std::string& actionName) const {
        auto it = m_bindings.find(actionName);
        if (it == m_bindings.end()) return false;
        if (it->second.key != KeyCode::Unknown) {
            return Input::isKeyPressed(it->second.key);
        }
        return false;
    }

private:
    std::unordered_map<std::string, ActionBinding> m_bindings;
};

class CameraShake {
public:
    float intensity = 0.0f;
    float duration = 0.0f;
    float timer = 0.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    void shake(float pIntensity, float pDuration) {
        intensity = pIntensity;
        duration = pDuration;
        timer = pDuration;
    }

    void update(float dt) {
        if (timer > 0.0f) {
            timer -= dt;
            float progress = timer / duration;
            float currentIntensity = intensity * progress;
            offsetX = ((float)(rand() % 2000 - 1000) / 1000.0f) * currentIntensity;
            offsetY = ((float)(rand() % 2000 - 1000) / 1000.0f) * currentIntensity;
        } else {
            offsetX = 0.0f;
            offsetY = 0.0f;
        }
    }
};

class TweenEngine {
public:
    static float easeLinear(float t) { return t; }
    static float easeInQuad(float t) { return t * t; }
    static float easeOutQuad(float t) { return t * (2.0f - t); }
    static float easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }

    static float interpolate(float start, float end, float progress, const std::string& ease = "linear") {
        float t = std::clamp(progress, 0.0f, 1.0f);
        float e = t;
        if (ease == "in") e = easeInQuad(t);
        else if (ease == "out") e = easeOutQuad(t);
        else if (ease == "in_out") e = easeInOutQuad(t);
        return start + (end - start) * e;
    }
};

class InputInjector {
public:
    static void injectKeyPress(KeyCode key) {
        Input::updateKey(static_cast<int>(key), true);
    }
    static void injectKeyRelease(KeyCode key) {
        Input::updateKey(static_cast<int>(key), false);
    }
    static void injectMouseMove(float x, float y) {
        Input::updateMousePosition(x, y);
    }
    static void injectMouseButton(MouseButton button, bool pressed) {
        Input::updateMouseButton(static_cast<int>(button), pressed);
    }
};

class HeadlessRunner {
public:
    static void runDeterministic(std::function<void(float fixedDt)> stepFunc, int frameCount = 60, float fixedStep = 1.0f / 60.0f) {
        for (int frame = 0; frame < frameCount; ++frame) {
            stepFunc(fixedStep);
            Input::endFrame();
        }
    }
};

class NativeWindow {
private:
    WindowConfig m_config;
    bool m_shouldClose = false;
    double m_deltaTime = 0.0;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    
#ifdef _WIN32
    HWND m_hwnd = nullptr;
    HDC m_hdc = nullptr;
    HGLRC m_hglrc = nullptr;
#endif

public:
    NativeWindow() = default;
    ~NativeWindow() { shutdown(); }

    bool initialize(const WindowConfig& config = WindowConfig()) {
        m_config = config;
        m_lastFrameTime = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSA wc = {};
        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
            switch (msg) {
                case WM_KEYDOWN:
                    Input::updateKey(static_cast<int>(wParam), true);
                    break;
                case WM_KEYUP:
                    Input::updateKey(static_cast<int>(wParam), false);
                    break;
                case WM_MOUSEMOVE:
                    Input::updateMousePosition(static_cast<float>(LOWORD(lParam)), static_cast<float>(HIWORD(lParam)));
                    break;
                case WM_LBUTTONDOWN:
                    Input::updateMouseButton(0, true);
                    break;
                case WM_LBUTTONUP:
                    Input::updateMouseButton(0, false);
                    break;
                case WM_RBUTTONDOWN:
                    Input::updateMouseButton(1, true);
                    break;
                case WM_RBUTTONUP:
                    Input::updateMouseButton(1, false);
                    break;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
            }
            return DefWindowProcA(hwnd, msg, wParam, lParam);
        };
        wc.hInstance = hInstance;
        wc.lpszClassName = "ZenithWindowClass";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

        RegisterClassA(&wc);

        RECT rect = {0, 0, m_config.width, m_config.height};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindowExA(
            0, "ZenithWindowClass", m_config.title.c_str(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL, NULL, hInstance, NULL
        );

        if (!m_hwnd) return false;

        m_hdc = GetDC(m_hwnd);

        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;

        int format = ChoosePixelFormat(m_hdc, &pfd);
        SetPixelFormat(m_hdc, format, &pfd);

        m_hglrc = wglCreateContext(m_hdc);
        wglMakeCurrent(m_hdc, m_hglrc);

        // Setup OpenGL 2D ortho state
        glViewport(0, 0, m_config.width, m_config.height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, m_config.width, m_config.height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
        return true;
    }

    void pollEvents() {
        Input::endFrame();
#ifdef _WIN32
        MSG msg;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_shouldClose = true;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
#endif
        auto now = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration<double>(now - m_lastFrameTime).count();
        m_lastFrameTime = now;
    }

    void clear(const Color& color = Color::Black()) {
#ifdef _WIN32
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    }

    void swapBuffers() {
#ifdef _WIN32
        SwapBuffers(m_hdc);
#endif
    }

    void shutdown() {
#ifdef _WIN32
        if (m_hglrc) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(m_hglrc);
            m_hglrc = nullptr;
        }
        if (m_hdc && m_hwnd) {
            ReleaseDC(m_hwnd, m_hdc);
            m_hdc = nullptr;
        }
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
#endif
    }

    bool shouldClose() const { return m_shouldClose; }
    void setShouldClose(bool close) { m_shouldClose = close; }
    float getDeltaTime() const { return static_cast<float>(m_deltaTime); }
    int getWidth() const { return m_config.width; }
    int getHeight() const { return m_config.height; }
};

// Immediate mode 2D Renderer for Code-First Game Development
class Renderer2D {
public:
    static void beginCamera(const Camera2D& camera) {
#ifdef _WIN32
        glPushMatrix();
        glTranslatef(-camera.positionX, -camera.positionY, 0.0f);
        glScalef(camera.zoom, camera.zoom, 1.0f);
        glRotatef(camera.rotation, 0.0f, 0.0f, 1.0f);
#endif
    }

    static void endCamera() {
#ifdef _WIN32
        glPopMatrix();
#endif
    }

    static void drawRect(float x, float y, float width, float height, const Color& color) {
#ifdef _WIN32
        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + width, y);
            glVertex2f(x + width, y + height);
            glVertex2f(x, y + height);
        glEnd();
#endif
    }

    static void drawRectOutline(float x, float y, float width, float height, const Color& color, float lineWidth = 1.0f) {
#ifdef _WIN32
        glLineWidth(lineWidth);
        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + width, y);
            glVertex2f(x + width, y + height);
            glVertex2f(x, y + height);
        glEnd();
#endif
    }

    static void drawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness = 1.0f) {
#ifdef _WIN32
        glLineWidth(thickness);
        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_LINES);
            glVertex2f(x1, y1);
            glVertex2f(x2, y2);
        glEnd();
#endif
    }

    static void drawCircle(float cx, float cy, float radius, const Color& color, int segments = 32) {
#ifdef _WIN32
        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx, cy);
            for (int i = 0; i <= segments; ++i) {
                float angle = i * 2.0f * 3.14159265f / segments;
                glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
            }
        glEnd();
#endif
    }
};

} // namespace zenith

#endif // ZENITH_WINDOW_H
