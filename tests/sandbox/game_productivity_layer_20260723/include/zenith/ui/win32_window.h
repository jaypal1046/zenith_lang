/**
 * Zenith Win32 Native Window Implementation
 * 
 * Windows-specific implementation of NativeWindow using Win32 API.
 * Creates actual HWND windows with native message loop.
 */

#ifndef ZENITH_WIN32_WINDOW_H
#define ZENITH_WIN32_WINDOW_H

#ifdef _WIN32

#include "native_window.h"
#include <windows.h>
#include <string>
#include <memory>

namespace zenith {
namespace ui {

/**
 * @brief Win32 implementation of NativeWindow
 */
class Win32Window : public NativeWindow {
private:
    HWND hwnd = nullptr;
    bool closed = false;
    std::string title;
    int width = 800;
    int height = 600;
    EventCallback eventCallback;
    
    /**
     * @brief Window procedure callback
     */
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    /**
     * @brief Handle WM_COMMAND (button clicks, etc.)
     */
    void handleCommand(WPARAM wParam);
    
    /**
     * @brief Handle mouse events
     */
    void handleMouse(UINT msg, WPARAM wParam, LPARAM lParam);
    
    /**
     * @brief Handle keyboard events
     */
    void handleKeyboard(UINT msg, WPARAM wParam);

public:
    Win32Window() = default;
    ~Win32Window();
    
    void create(const std::string& title, int width, int height) override;
    void show() override;
    void close() override;
    bool shouldClose() const override { return closed; }
    void* getNativeHandle() override { return hwnd; }
    void setEventCallback(EventCallback callback) override { eventCallback = callback; }
    void setTitle(const std::string& newTitle) override;
    void resize(int w, int h) override;
    void invalidate() override;
    
    /**
     * @brief Get HINSTANCE for this application
     */
    static HINSTANCE getInstance();
};

/**
 * @brief Win32 implementation of NativeWidget
 */
class Win32Widget : public NativeWidget {
protected:
    HWND hwnd = nullptr;
    std::function<void(const std::string&, const std::string&)> callback;
    
public:
    virtual ~Win32Widget();
    void* getNativeHandle() override { return hwnd; }
    void setPosition(int x, int y) override;
    void setSize(int width, int height) override;
    void setEnabled(bool enabled) override;
    void setVisible(bool visible) override;
    void setText(const std::string& text) override;
    std::string getText() const override;
    
    void setCallback(std::function<void(const std::string&, const std::string&)> cb);
    HWND getHwnd() const { return hwnd; }
};

/**
 * @brief Win32 button widget
 */
class Win32Button : public Win32Widget {
public:
    Win32Button(const UIElement& element, void* parent);
};

/**
 * @brief Win32 text field widget
 */
class Win32TextField : public Win32Widget {
public:
    Win32TextField(const UIElement& element, void* parent);
};

/**
 * @brief Win32 checkbox widget
 */
class Win32Checkbox : public Win32Widget {
public:
    Win32Checkbox(const UIElement& element, void* parent);
};

/**
 * @brief Win32 slider widget
 */
class Win32Slider : public Win32Widget {
public:
    Win32Slider(const UIElement& element, void* parent);
};

/**
 * @brief Win32 label widget
 */
class Win32Label : public Win32Widget {
public:
    Win32Label(const UIElement& element, void* parent);
};

} // namespace ui
} // namespace zenith

#endif // _WIN32
#endif // ZENITH_WIN32_WINDOW_H
