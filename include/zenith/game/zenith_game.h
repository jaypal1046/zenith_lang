#ifndef ZENITH_GAME_H
#define ZENITH_GAME_H

#include "../common/zenith_common.h"
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <mutex>
#include <cmath>
#include <iostream>
#include <cctype>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/gl.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#else
#include <unistd.h>
#endif

namespace zenith {

// Thread-safe singleton for keyboard and mouse states
class InputState {
public:
    static InputState& getInstance() {
        static InputState instance;
        return instance;
    }
    
    void setKeyPressed(const std::string& key, bool pressed) {
        std::lock_guard<std::mutex> lock(mtx);
        keys[key] = pressed;
    }
    
    bool isKeyPressed(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = keys.find(key);
        if (it != keys.end()) {
            return it->second;
        }
        return false;
    }
    
    void setMousePos(float x, float y) {
        std::lock_guard<std::mutex> lock(mtx);
        mouseX = x;
        mouseY = y;
    }
    
    float getMouseX() {
        std::lock_guard<std::mutex> lock(mtx);
        return mouseX;
    }
    
    float getMouseY() {
        std::lock_guard<std::mutex> lock(mtx);
        return mouseY;
    }
    
private:
    std::unordered_map<std::string, bool> keys;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    std::mutex mtx;
    
    InputState() = default;
};

#ifdef _WIN32
inline std::string vkToString(WPARAM wParam) {
    switch (wParam) {
        case VK_LEFT:   return "ArrowLeft";
        case VK_RIGHT:  return "ArrowRight";
        case VK_UP:     return "ArrowUp";
        case VK_DOWN:   return "ArrowDown";
        case 'A':       return "a";
        case 'D':       return "d";
        case 'W':       return "w";
        case 'S':       return "s";
        case VK_ESCAPE: return "Escape";
        case VK_SPACE:  return "Space";
        default: {
            if (wParam >= 'A' && wParam <= 'Z') {
                return std::string(1, static_cast<char>(tolower(wParam)));
            }
            if (wParam >= '0' && wParam <= '9') {
                return std::string(1, static_cast<char>(wParam));
            }
            return "";
        }
    }
}

static HGLRC hRC = NULL;
static HDC hDC = NULL;
static HWND hWnd = NULL;
static GLuint fontListBase = 0;

inline void setupPixelFormat(HDC hDC) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int pf = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pf, &pfd);
}

inline LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            zenith::InputState::getInstance().setKeyPressed(zenith::vkToString(wParam), true);
            break;
        case WM_KEYUP:
            zenith::InputState::getInstance().setKeyPressed(zenith::vkToString(wParam), false);
            break;
        case WM_MOUSEMOVE: {
            float mouseX = static_cast<float>(LOWORD(lParam)) / 8.0f;
            float mouseY = static_cast<float>(HIWORD(lParam)) / 20.0f;
            zenith::InputState::getInstance().setMousePos(mouseX, mouseY);
            break;
        }
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}
#endif

#ifdef __ANDROID__
namespace android_support {
    // EGL and GLES dummy hooks or stubs for Android NDK target
    inline void initEGL() {}
    inline void swapEGLBuffers() {}
}
#endif

class Canvas {
public:
    int width = 80;
    int height = 24;
    bool use_opengl = false;
    TerminalBuffer buffer;
#ifdef _WIN32
    GLuint fontListBase = 0;
#endif
    
    Canvas(int w = 80, int h = 24, bool gl = false) 
        : width(w), height(h), use_opengl(gl), buffer(w, h) {
        clear("black");
    }
    
    void clear(const std::string& color = "black") {
        if (use_opengl) {
            float r = 0.0f, g = 0.0f, b = 0.0f;
            if (color == "red") r = 1.0f;
            else if (color == "green") g = 1.0f;
            else if (color == "blue") b = 1.0f;
            else if (color == "yellow") { r = 1.0f; g = 1.0f; }
            else if (color == "cyan") { g = 1.0f; b = 1.0f; }
            else if (color == "magenta") { r = 1.0f; b = 1.0f; }
            else if (color == "white") { r = 1.0f; g = 1.0f; b = 1.0f; }
            
#ifdef _WIN32
            glClearColor(r, g, b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
#endif
            return;
        }
        
        std::string bg_style = "\033[40m"; // default black
        if (color == "red") bg_style = "\033[41m";
        else if (color == "green") bg_style = "\033[42m";
        else if (color == "yellow") bg_style = "\033[43m";
        else if (color == "blue") bg_style = "\033[44m";
        else if (color == "magenta") bg_style = "\033[45m";
        else if (color == "cyan") bg_style = "\033[46m";
        else if (color == "white") bg_style = "\033[47m";
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                buffer.setCell(x, y, " ", bg_style);
            }
        }
    }
    
    void drawRect(float x, float y, float w, float h, const std::string& color = "white") {
        if (use_opengl) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            if (color == "red") { r = 1.0f; g = 0.0f; b = 0.0f; }
            else if (color == "green") { r = 0.0f; g = 1.0f; b = 0.0f; }
            else if (color == "blue") { r = 0.0f; g = 0.0f; b = 1.0f; }
            else if (color == "yellow") { r = 1.0f; g = 1.0f; b = 0.0f; }
            else if (color == "cyan") { r = 0.0f; g = 1.0f; b = 1.0f; }
            else if (color == "magenta") { r = 1.0f; g = 0.0f; b = 1.0f; }
            else if (color == "black") { r = 0.0f; g = 0.0f; b = 0.0f; }
            
#ifdef _WIN32
            glColor3f(r, g, b);
            glBegin(GL_QUADS);
            glVertex2f(x * 8.0f, y * 20.0f);
            glVertex2f((x + w) * 8.0f, y * 20.0f);
            glVertex2f((x + w) * 8.0f, (y + h) * 20.0f);
            glVertex2f(x * 8.0f, (y + h) * 20.0f);
            glEnd();
#endif
            return;
        }
        
        std::string bg_style = "\033[47m"; // default white
        if (color == "red") bg_style = "\033[41m";
        else if (color == "green") bg_style = "\033[42m";
        else if (color == "yellow") bg_style = "\033[43m";
        else if (color == "blue") bg_style = "\033[44m";
        else if (color == "magenta") bg_style = "\033[45m";
        else if (color == "cyan") bg_style = "\033[46m";
        else if (color == "black") bg_style = "\033[40m";
        
        int ix = static_cast<int>(x);
        int iy = static_cast<int>(y);
        int iw = static_cast<int>(w);
        int ih = static_cast<int>(h);
        
        for (int curr_y = iy; curr_y < iy + ih; ++curr_y) {
            for (int curr_x = ix; curr_x < ix + iw; ++curr_x) {
                if (curr_x >= 0 && curr_x < width && curr_y >= 0 && curr_y < height) {
                    buffer.setCell(curr_x, curr_y, " ", bg_style);
                }
            }
        }
    }
    
    void drawCircle(float cx, float cy, float r, const std::string& color = "white") {
        if (use_opengl) {
            float r_col = 1.0f, g = 1.0f, b = 1.0f;
            if (color == "red") { r_col = 1.0f; g = 0.0f; b = 0.0f; }
            else if (color == "green") { r_col = 0.0f; g = 1.0f; b = 0.0f; }
            else if (color == "blue") { r_col = 0.0f; g = 0.0f; b = 1.0f; }
            else if (color == "yellow") { r_col = 1.0f; g = 1.0f; b = 0.0f; }
            else if (color == "cyan") { r_col = 0.0f; g = 1.0f; b = 1.0f; }
            else if (color == "magenta") { r_col = 1.0f; g = 0.0f; b = 1.0f; }
            else if (color == "black") { r_col = 0.0f; g = 0.0f; b = 0.0f; }
            
#ifdef _WIN32
            glColor3f(r_col, g, b);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx * 8.0f, cy * 20.0f);
            int num_segments = 32;
            for (int i = 0; i <= num_segments; ++i) {
                float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
                float px = (cx * 8.0f) + (r * 14.0f) * cosf(theta);
                float py = (cy * 20.0f) + (r * 14.0f) * sinf(theta);
                glVertex2f(px, py);
            }
            glEnd();
#endif
            return;
        }
        
        std::string bg_style = "\033[47m";
        if (color == "red") bg_style = "\033[41m";
        else if (color == "green") bg_style = "\033[42m";
        else if (color == "yellow") bg_style = "\033[43m";
        else if (color == "blue") bg_style = "\033[44m";
        else if (color == "magenta") bg_style = "\033[45m";
        else if (color == "cyan") bg_style = "\033[46m";
        else if (color == "black") bg_style = "\033[40m";
        
        int ix = static_cast<int>(cx);
        int iy = static_cast<int>(cy);
        int ir = static_cast<int>(r);
        
        for (int y = iy - ir; y <= iy + ir; ++y) {
            for (int x = ix - ir * 2; x <= ix + ir * 2; ++x) {
                float dx = (x - ix) / 2.0f;
                float dy = (y - iy);
                if (dx * dx + dy * dy <= ir * ir) {
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        buffer.setCell(x, y, "o", bg_style);
                    }
                }
            }
        }
    }
    
    void drawText(const std::string& text, float x, float y, const std::string& color = "white") {
        if (use_opengl) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            if (color == "red") { r = 1.0f; g = 0.0f; b = 0.0f; }
            else if (color == "green") { r = 0.0f; g = 1.0f; b = 0.0f; }
            else if (color == "blue") { r = 0.0f; g = 0.0f; b = 1.0f; }
            else if (color == "yellow") { r = 1.0f; g = 1.0f; b = 0.0f; }
            else if (color == "cyan") { r = 0.0f; g = 1.0f; b = 1.0f; }
            else if (color == "magenta") { r = 1.0f; g = 0.0f; b = 1.0f; }
            else if (color == "black") { r = 0.0f; g = 0.0f; b = 0.0f; }
            
#ifdef _WIN32
            glColor3f(r, g, b);
            glRasterPos2f(x * 8.0f, y * 20.0f + 12.0f);
            glListBase(fontListBase);
            glCallLists(static_cast<GLsizei>(text.length()), GL_UNSIGNED_BYTE, text.c_str());
#endif
            return;
        }
        
        std::string fg_style = "\033[37m";
        if (color == "red") fg_style = "\033[31m";
        else if (color == "green") fg_style = "\033[32m";
        else if (color == "yellow") fg_style = "\033[33m";
        else if (color == "blue") fg_style = "\033[34m";
        else if (color == "magenta") fg_style = "\033[35m";
        else if (color == "cyan") fg_style = "\033[36m";
        
        buffer.writeString(static_cast<int>(x), static_cast<int>(y), text, fg_style);
    }
    
    void present() {
        if (use_opengl) {
#ifdef _WIN32
            SwapBuffers(hDC);
#endif
            return;
        }
        
        std::cout << "\033[2J\033[H";
        buffer.print();
        std::cout << std::flush;
    }
};

class Game {
public:
    virtual ~Game() = default;
    virtual void init() {}
    virtual void update(float dt) {}
    virtual void draw(Canvas& canvas) {}
};

template<typename GameType>
inline void runGameLoop(GameType& game) {
#ifdef _WIN32
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "ZenithGameWindow";
    
    if (RegisterClassExA(&wc)) {
        RECT rect = {0, 0, 640, 480};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        
        hWnd = CreateWindowExA(
            0,
            "ZenithGameWindow",
            "Zenith Game Engine",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL, NULL, GetModuleHandleA(NULL), NULL
        );
        
        if (hWnd) {
            hDC = GetDC(hWnd);
            setupPixelFormat(hDC);
            hRC = wglCreateContext(hDC);
            wglMakeCurrent(hDC, hRC);
            
            fontListBase = glGenLists(256);
            HFONT hFont = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
                                      ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Courier New");
            SelectObject(hDC, hFont);
            wglUseFontBitmapsA(hDC, 0, 256, fontListBase);
            
            glViewport(0, 0, 640, 480);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, 640.0, 480.0, 0.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            Canvas canvas(80, 24, true);
            canvas.fontListBase = fontListBase;
            
            game.init();
            
            auto last_time = std::chrono::high_resolution_clock::now();
            MSG msg = {0};
            bool running = true;
            
            while (running) {
                while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) {
                        running = false;
                        break;
                    }
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }
                
                if (!running) break;
                
                auto current_time = std::chrono::high_resolution_clock::now();
                float dt = std::chrono::duration<float>(current_time - last_time).count();
                last_time = current_time;
                
                game.update(dt);
                canvas.clear("black");
                game.draw(canvas);
                canvas.present();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hRC);
            ReleaseDC(hWnd, hDC);
            DestroyWindow(hWnd);
            return;
        }
    }
#endif

    game.init();
    Canvas canvas(80, 24, false);
    
    auto last_time = std::chrono::high_resolution_clock::now();
    int frames = 0;
    bool infinite = true;
    
    #ifdef __ANDROID__
    infinite = false;
    #endif
    
    bool is_tty = false;
    #ifdef _WIN32
    is_tty = _isatty(0) != 0;
    #else
    is_tty = isatty(0) != 0;
    #endif
    
    if (!infinite || !is_tty) {
        for (int i = 0; i < 20; ++i) {
            auto current_time = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;
            
            game.update(dt);
            canvas.clear("black");
            game.draw(canvas);
            canvas.present();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return;
    }
    
    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        game.update(dt);
        canvas.clear("black");
        game.draw(canvas);
        canvas.present();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
        frames++;
        if (frames > 100) {
            break;
        }
    }
}

} // namespace zenith

// Global C FFI Wrappers prefixed with _zenith_builtin_
extern "C" {
    inline bool _zenith_builtin_isKeyPressed(const char* key) {
        return zenith::InputState::getInstance().isKeyPressed(key);
    }
    
    inline float _zenith_builtin_getMouseX() {
        return zenith::InputState::getInstance().getMouseX();
    }
    
    inline float _zenith_builtin_getMouseY() {
        return zenith::InputState::getInstance().getMouseY();
    }
}

#endif // ZENITH_GAME_H
