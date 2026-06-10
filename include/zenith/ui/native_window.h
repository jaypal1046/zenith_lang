/**
 * Zenith Native Window Interface
 * 
 * Abstract interface for platform-native window creation and management.
 * Implementations exist for Win32, Cocoa (macOS), and GTK (Linux).
 */

#ifndef ZENITH_NATIVE_WINDOW_H
#define ZENITH_NATIVE_WINDOW_H

#include <string>
#include <functional>
#include <memory>

namespace zenith {
namespace ui {

/**
 * @brief Event types for native window events
 */
enum class EventType {
    MouseMove,
    MouseDown,
    MouseUp,
    KeyDown,
    KeyUp,
    Resize,
    Paint,
    Close,
    Command  // Button click, menu selection, etc.
};

/**
 * @brief Native event structure
 */
struct NativeEvent {
    EventType type;
    int x = 0;
    int y = 0;
    int button = 0;
    int keyCode = 0;
    int width = 0;
    int height = 0;
    void* nativeHandle = nullptr;  // HWND, NSView*, GtkWidget*, etc.
};

/**
 * @brief Callback type for event handling
 */
using EventCallback = std::function<void(const NativeEvent&)>;

/**
 * @brief Abstract native window interface
 * 
 * Platform-independent interface for creating and managing native windows.
 * Each platform (Windows, macOS, Linux) provides its own implementation.
 */
class NativeWindow {
public:
    virtual ~NativeWindow() = default;
    
    /**
     * @brief Create the native window
     * @param title Window title
     * @param width Initial width in pixels
     * @param height Initial height in pixels
     */
    virtual void create(const std::string& title, int width, int height) = 0;
    
    /**
     * @brief Show the window and start event loop
     */
    virtual void show() = 0;
    
    /**
     * @brief Close and destroy the window
     */
    virtual void close() = 0;
    
    /**
     * @brief Check if window should close
     */
    virtual bool shouldClose() const = 0;
    
    /**
     * @brief Get native window handle
     * @return Platform-specific handle (HWND, NSWindow*, GtkWindow*, etc.)
     */
    virtual void* getNativeHandle() = 0;
    
    /**
     * @brief Set event callback
     * @param callback Function to call when events occur
     */
    virtual void setEventCallback(EventCallback callback) = 0;
    
    /**
     * @brief Update window title
     */
    virtual void setTitle(const std::string& title) = 0;
    
    /**
     * @brief Resize window
     */
    virtual void resize(int width, int height) = 0;
    
    /**
     * @brief Request repaint
     */
    virtual void invalidate() = 0;
};

/**
 * @brief Factory function to create platform-appropriate window
 * @return Unique pointer to NativeWindow implementation
 */
std::unique_ptr<NativeWindow> createNativeWindow();

} // namespace ui
} // namespace zenith

#endif // ZENITH_NATIVE_WINDOW_H
