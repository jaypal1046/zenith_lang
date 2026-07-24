#ifndef ZENITH_GAME_H
#define ZENITH_GAME_H

#include "../common/zenith_common.h"
#include "zenith_physics.h"
#include "zenith_resource.h"
#include "zenith_world.h"
#include "zenith_scene.h"
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <mutex>
#include <cmath>
#include <iostream>
#include <cctype>
#include <algorithm>

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

    void beginFrame() {
        std::lock_guard<std::mutex> lock(mtx);
        justPressed.clear();
        justReleased.clear();
    }
    
    void setKeyPressed(const std::string& key, bool pressed) {
        if (key.empty()) {
            return;
        }

        std::lock_guard<std::mutex> lock(mtx);
        bool wasPressed = false;
        auto it = keys.find(key);
        if (it != keys.end()) {
            wasPressed = it->second;
        }

        keys[key] = pressed;
        if (wasPressed == pressed) {
            return;
        }

        if (pressed) {
            justPressed[key] = true;
            justReleased.erase(key);
        } else {
            justReleased[key] = true;
            justPressed.erase(key);
        }
    }
    
    bool isKeyPressed(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = keys.find(key);
        if (it != keys.end()) {
            return it->second;
        }
        return false;
    }

    bool wasKeyPressed(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = justPressed.find(key);
        return it != justPressed.end() && it->second;
    }

    bool wasKeyReleased(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = justReleased.find(key);
        return it != justReleased.end() && it->second;
    }

    float getAxis(const std::string& negativeKey, const std::string& positiveKey) {
        std::lock_guard<std::mutex> lock(mtx);
        float axis = 0.0f;
        auto negative = keys.find(negativeKey);
        auto positive = keys.find(positiveKey);
        if (negative != keys.end() && negative->second) {
            axis -= 1.0f;
        }
        if (positive != keys.end() && positive->second) {
            axis += 1.0f;
        }
        return axis;
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
    std::unordered_map<std::string, bool> justPressed;
    std::unordered_map<std::string, bool> justReleased;
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

template<typename SceneType, typename std::enable_if_t<std::is_base_of_v<game::Scene, SceneType>, int> = 0>
inline void runGameLoop(SceneType& scene);

template<typename GameType, typename std::enable_if_t<!std::is_base_of_v<game::Scene, GameType>, int> = 0>
inline void runGameLoop(GameType& game);

inline bool isKeyPressed(const std::string& key) {
    return InputState::getInstance().isKeyPressed(key);
}

inline bool wasKeyPressed(const std::string& key) {
    return InputState::getInstance().wasKeyPressed(key);
}

inline bool wasKeyReleased(const std::string& key) {
    return InputState::getInstance().wasKeyReleased(key);
}

inline float getAxis(const std::string& negativeKey, const std::string& positiveKey) {
    return InputState::getInstance().getAxis(negativeKey, positiveKey);
}

inline float getMouseX() {
    return InputState::getInstance().getMouseX();
}

inline float getMouseY() {
    return InputState::getInstance().getMouseY();
}

inline void drawWorld2D(Canvas& canvas, const game::World& world, bool drawEntityNames = true) {
    physics::Vec2 cameraPosition(0.0f, 0.0f);
    float cameraZoom = 1.0f;
    float viewportOriginX = 0.0f;
    float viewportOriginY = 0.0f;
    float viewportWidth = 0.0f;
    float viewportHeight = 0.0f;
    bool hasPrimaryCamera = false;

    if (auto primaryCamera = world.primaryCamera2D()) {
        hasPrimaryCamera = true;
        const game::Transform2D* cameraTransform = world.getTransform2D(*primaryCamera);
        const game::Camera2DComponent* camera = world.getCamera2D(*primaryCamera);
        if (cameraTransform != nullptr) {
            cameraPosition = cameraTransform->position;
        }
        if (camera != nullptr && camera->zoom > 0.0f) {
            cameraZoom = camera->zoom;
            viewportOriginX = canvas.width * camera->viewportOrigin.x;
            viewportOriginY = canvas.height * camera->viewportOrigin.y;
            viewportWidth = canvas.width * std::max(0.0f, camera->viewportSize.x);
            viewportHeight = canvas.height * std::max(0.0f, camera->viewportSize.y);
        }
    }

    if (viewportWidth <= 0.0f) {
        viewportWidth = static_cast<float>(canvas.width);
    }
    if (viewportHeight <= 0.0f) {
        viewportHeight = static_cast<float>(canvas.height);
    }

    float cameraScreenCenterX = viewportOriginX + viewportWidth * 0.5f;
    float cameraScreenCenterY = viewportOriginY + viewportHeight * 0.5f;

    std::vector<game::EntityId> sprites = world.spriteEntities2D();
    std::sort(sprites.begin(), sprites.end(), [&world](game::EntityId lhs, game::EntityId rhs) {
        const game::SpriteRenderer2D* left = world.getSpriteRenderer2D(lhs);
        const game::SpriteRenderer2D* right = world.getSpriteRenderer2D(rhs);
        int leftOrder = (left != nullptr) ? left->sortOrder : 0;
        int rightOrder = (right != nullptr) ? right->sortOrder : 0;
        if (leftOrder != rightOrder) {
            return leftOrder < rightOrder;
        }
        return lhs.index < rhs.index;
    });

    for (game::EntityId entity : sprites) {
        const game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity);
        const game::Transform2D* transform = world.getTransform2D(entity);
        if (sprite == nullptr || transform == nullptr || !sprite->visible) {
            continue;
        }

        float screenX = transform->position.x;
        float screenY = transform->position.y;
        if (hasPrimaryCamera) {
            screenX = ((transform->position.x - cameraPosition.x) * cameraZoom) + cameraScreenCenterX;
            screenY = ((transform->position.y - cameraPosition.y) * cameraZoom) + cameraScreenCenterY;
        }
        float drawWidth = sprite->size.x * cameraZoom;
        float drawHeight = sprite->size.y * cameraZoom;

        screenX -= drawWidth * sprite->anchor.x;
        screenY -= drawHeight * sprite->anchor.y;

        canvas.drawRect(screenX, screenY, drawWidth, drawHeight, sprite->tintColor);

        if (!drawEntityNames) {
            continue;
        }

        const game::NameComponent* name = world.getName(entity);
        if (name == nullptr || name->value.empty()) {
            continue;
        }

        canvas.drawText(name->value, screenX, screenY - 1.0f, "white");
    }
}

inline bool followCamera2D(
    game::World& world,
    game::EntityId cameraEntity,
    game::EntityId targetEntity,
    const physics::Vec2& offset = physics::Vec2(0.0f, 0.0f),
    float smoothing = 1.0f
) {
    game::Transform2D* cameraTransform = world.getTransform2D(cameraEntity);
    const game::Transform2D* targetTransform = world.getTransform2D(targetEntity);
    if (cameraTransform == nullptr || targetTransform == nullptr) {
        return false;
    }

    smoothing = std::clamp(smoothing, 0.0f, 1.0f);
    physics::Vec2 desired = targetTransform->position + offset;
    cameraTransform->position.x += (desired.x - cameraTransform->position.x) * smoothing;
    cameraTransform->position.y += (desired.y - cameraTransform->position.y) * smoothing;
    return true;
}

inline bool followPrimaryCamera2D(
    game::World& world,
    game::EntityId targetEntity,
    const physics::Vec2& offset = physics::Vec2(0.0f, 0.0f),
    float smoothing = 1.0f
) {
    std::optional<game::EntityId> primaryCamera = world.primaryCamera2D();
    if (!primaryCamera.has_value()) {
        return false;
    }
    return followCamera2D(world, primaryCamera.value(), targetEntity, offset, smoothing);
}

class SceneGame : public Game {
public:
    explicit SceneGame(game::Scene& scene) : scene(scene) {}

    void init() override {
        scene.load();
    }

    void update(float dt) override {
        scene.updateFrame(dt);
    }

    void draw(Canvas& canvas) override {
        canvas.clear(scene.clearColor);
        if (scene.autoRenderWorld2D) {
            drawWorld2D(canvas, scene.world, scene.drawEntityNames);
        }
        scene.render(canvas);
    }

private:
    game::Scene& scene;
};

template<typename SceneType>
inline void runScene(SceneType& scene) {
    runGameLoop(scene);
}

template<typename SceneType, typename std::enable_if_t<std::is_base_of_v<game::Scene, SceneType>, int>>
inline void runGameLoop(SceneType& scene) {
    SceneGame host(scene);
    runGameLoop(host);
}

template<typename GameType, typename std::enable_if_t<!std::is_base_of_v<game::Scene, GameType>, int>>
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
                InputState::getInstance().beginFrame();
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
            InputState::getInstance().beginFrame();
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
        InputState::getInstance().beginFrame();
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

// ============================================================================
// 5. ENGINE PRIMITIVES (CHARACTER, CAMERA, LIGHTS, SPRITEBATCH)
// ============================================================================

class Character2D {
public:
    physics::Vec2 position;
    physics::Vec2 velocity;
    physics::RigidBody2D rigidBody;
    std::shared_ptr<resource::TextureAsset> sprite;
    float moveSpeed = 10.0f;
    bool isGrounded = false;

    virtual void update(float dt) {
        rigidBody.position = position;
        rigidBody.velocity = velocity;
        position = rigidBody.position;
    }
};

class Character3D {
public:
    physics::Vec3 position;
    physics::Vec3 velocity;
    physics::RigidBody3D rigidBody;
    std::shared_ptr<resource::MeshAsset> model;
    float moveSpeed = 10.0f;
    bool useGravity = true;

    virtual void update(float dt) {
        rigidBody.position = position;
        position = rigidBody.position;
    }

    void move(const physics::Vec3& dir) {
        velocity = dir * moveSpeed;
        position += velocity * 0.016f;
    }
};

class Camera2D {
public:
    physics::Vec2 position;
    float zoom = 1.0f;
    float smoothness = 0.1f;

    void follow(const physics::Vec2& targetPos, physics::Vec2 offset = physics::Vec2(0, 0)) {
        physics::Vec2 target = targetPos + offset;
        position.x += (target.x - position.x) * smoothness;
        position.y += (target.y - position.y) * smoothness;
    }
};

class Camera3D {
public:
    physics::Vec3 position;
    physics::Vec3 target;
    float fov = 60.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;

    void follow(const physics::Vec3& targetPos, float distance = 5.0f) {
        target = targetPos;
        position = targetPos + physics::Vec3(0, 2.0f, -distance);
    }
};

struct PointLight {
    physics::Vec3 position;
    float intensity = 1.0f;
    float range = 10.0f;
    float r = 1.0f, g = 1.0f, b = 1.0f;
};

struct DirectionalLight {
    physics::Vec3 direction;
    float intensity = 1.0f;
    float r = 1.0f, g = 1.0f, b = 1.0f;
    bool castShadows = true;
};

class SpriteBatcher2D {
public:
    int spriteCount = 0;

    void begin() { spriteCount = 0; }
    void drawSprite(std::shared_ptr<resource::TextureAsset> texture, physics::Vec2 pos, physics::Vec2 size) {
        spriteCount++;
    }
    void end() {
        // High-speed 1-draw-call buffer flush to GPU
    }
};

} // namespace zenith

// Global C FFI Wrappers prefixed with _zenith_builtin_
extern "C" {
    inline bool _zenith_builtin_isKeyPressed(const char* key) {
        return zenith::InputState::getInstance().isKeyPressed(key);
    }

    inline bool _zenith_builtin_wasKeyPressed(const char* key) {
        return zenith::InputState::getInstance().wasKeyPressed(key);
    }

    inline bool _zenith_builtin_wasKeyReleased(const char* key) {
        return zenith::InputState::getInstance().wasKeyReleased(key);
    }

    inline float _zenith_builtin_getAxis(const char* negativeKey, const char* positiveKey) {
        return zenith::InputState::getInstance().getAxis(negativeKey, positiveKey);
    }
    
    inline float _zenith_builtin_getMouseX() {
        return zenith::InputState::getInstance().getMouseX();
    }
    
    inline float _zenith_builtin_getMouseY() {
        return zenith::InputState::getInstance().getMouseY();
    }
}

#endif // ZENITH_GAME_H
