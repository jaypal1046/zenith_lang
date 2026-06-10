/**
 * Zenith Win32 Native Window Implementation
 * 
 * Windows-specific implementation using Win32 API.
 */

#ifdef _WIN32

#include "zenith/ui/win32_window.h"
#include "zenith/ast/ast.h"
#include <commctrl.h>
#include <stdexcept>
#include <unordered_map>
#include <memory>

namespace zenith {
namespace ui {

// Static instance handle
static HINSTANCE g_hInstance = nullptr;

// Global widget registry for event routing: controlId -> (actionName, callback)
static std::unordered_map<UINT_PTR, std::pair<std::string, std::function<void(const std::string&, const std::string&)>>> g_widgetRegistry;
static UINT_PTR g_nextControlId = 1000;

HINSTANCE Win32Window::getInstance() {
    return g_hInstance ? g_hInstance : GetModuleHandle(NULL);
}

Win32Window::~Win32Window() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

void Win32Window::create(const std::string& windowTitle, int w, int h) {
    title = windowTitle;
    width = w;
    height = h;
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Register window class if not already registered
    const char* className = "ZenithWindowClass";
    WNDCLASSEX wc = {};
    if (!GetClassInfoEx(getInstance(), className, &wc)) {
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = getInstance();
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = className;
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        
        if (!RegisterClassEx(&wc)) {
            throw std::runtime_error("Failed to register window class");
        }
    }
    
    // Create window
    hwnd = CreateWindowEx(
        0,
        className,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        nullptr,
        nullptr,
        getInstance(),
        this  // Pass 'this' as LPARAM for WM_CREATE
    );
    
    if (!hwnd) {
        throw std::runtime_error("Failed to create window");
    }
}

void Win32Window::show() {
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // Message loop
    MSG msg;
    while (!closed && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Win32Window::close() {
    closed = true;
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

void Win32Window::setTitle(const std::string& newTitle) {
    title = newTitle;
    if (hwnd) {
        SetWindowTextA(hwnd, title.c_str());
    }
}

void Win32Window::resize(int w, int h) {
    width = w;
    height = h;
    if (hwnd) {
        SetWindowPos(hwnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Win32Window::invalidate() {
    if (hwnd) {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
}

LRESULT CALLBACK Win32Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Get window pointer from GWLP_USERDATA or from WM_CREATE
    Win32Window* window = reinterpret_cast<Win32Window*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA)
    );
    
    switch (msg) {
        case WM_CREATE: {
            // Store 'this' pointer in GWLP_USERDATA
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            Win32Window* pWnd = reinterpret_cast<Win32Window*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
            window = pWnd;
            return 0;
        }
        
        case WM_DESTROY: {
            if (window) {
                window->closed = true;
            }
            PostQuitMessage(0);
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // Fill background
            RECT rect;
            GetClientRect(hWnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            
            // TODO: Render UI elements here using Yoga layout
            
            EndPaint(hWnd, &ps);
            return 0;
        }
        
        case WM_SIZE: {
            if (window && window->eventCallback) {
                NativeEvent event;
                event.type = EventType::Resize;
                event.width = LOWORD(lParam);
                event.height = HIWORD(lParam);
                event.nativeHandle = hWnd;
                window->eventCallback(event);
            }
            return 0;
        }
        
        case WM_COMMAND: {
            if (window) {
                window->handleCommand(wParam);
            }
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE: {
            if (window) {
                window->handleMouse(msg, wParam, lParam);
            }
            return 0;
        }
        
        case WM_KEYDOWN:
        case WM_KEYUP: {
            if (window) {
                window->handleKeyboard(msg, wParam);
            }
            return 0;
        }
    }
    
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Win32Window::handleCommand(WPARAM wParam) {
    int wmId = LOWORD(wParam);
    int wmEvent = HIWORD(wParam);
    
    // Find the control that sent the message
    HWND ctrlHwnd = GetDlgItem(hwnd, wmId);
    
    if (eventCallback && ctrlHwnd) {
        NativeEvent event;
        event.type = EventType::Command;
        event.nativeHandle = ctrlHwnd;
        eventCallback(event);
    }
}

void Win32Window::handleMouse(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!eventCallback) return;
    
    NativeEvent event;
    event.x = GET_X_LPARAM(lParam);
    event.y = GET_Y_LPARAM(lParam);
    event.button = (wParam & MK_LBUTTON) ? 1 : 0;
    event.nativeHandle = hwnd;
    
    switch (msg) {
        case WM_LBUTTONDOWN:
            event.type = EventType::MouseDown;
            break;
        case WM_LBUTTONUP:
            event.type = EventType::MouseUp;
            break;
        case WM_MOUSEMOVE:
            event.type = EventType::MouseMove;
            break;
    }
    
    eventCallback(event);
}

void Win32Window::handleKeyboard(UINT msg, WPARAM wParam) {
    if (!eventCallback) return;
    
    NativeEvent event;
    event.keyCode = static_cast<int>(wParam);
    event.nativeHandle = hwnd;
    event.type = (msg == WM_KEYDOWN) ? EventType::KeyDown : EventType::KeyUp;
    
    eventCallback(event);
}

// ============================================================================
// Win32Widget Implementation
// ============================================================================

Win32Widget::~Win32Widget() {
    // Note: Don't destroy HWND here - parent window owns child controls
}

void Win32Widget::setPosition(int x, int y) {
    if (hwnd) {
        SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void Win32Widget::setSize(int w, int h) {
    if (hwnd) {
        SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Win32Widget::setEnabled(bool enabled) {
    if (hwnd) {
        EnableWindow(hwnd, enabled ? TRUE : FALSE);
    }
}

void Win32Widget::setVisible(bool visible) {
    if (hwnd) {
        ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);
    }
}

void Win32Widget::setText(const std::string& text) {
    if (hwnd) {
        SetWindowTextA(hwnd, text.c_str());
    }
}

std::string Win32Widget::getText() const {
    if (!hwnd) return "";
    
    int len = GetWindowTextLengthA(hwnd);
    if (len == 0) return "";
    
    std::string buffer(len + 1, '\0');
    GetWindowTextA(hwnd, &buffer[0], len + 1);
    buffer.resize(len);
    return buffer;
}

void Win32Widget::setCallback(std::function<void(const std::string&, const std::string&)> cb) {
    callback = cb;
}

// ============================================================================
// Win32Button Implementation
// ============================================================================

Win32Button::Win32Button(const UIElement& element, void* parent) {
    HWND parentHwnd = reinterpret_cast<HWND>(parent);
    
    // Extract button text and attributes
    std::string text = element.text_content;
    std::string onClickAction;
    
    auto it = element.attributes.find("onClick");
    if (it != element.attributes.end()) {
        onClickAction = it->second;
    }
    
    // Create native button
    hwnd = CreateWindowEx(
        0,
        "BUTTON",
        text.c_str(),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        static_cast<int>(element.layout_x),
        static_cast<int>(element.layout_y),
        static_cast<int>(element.layout_width),
        static_cast<int>(element.layout_height),
        parentHwnd,
        nullptr,  // Menu/ID - should be unique in real implementation
        Win32Window::getInstance(),
        nullptr
    );
    
    // Store action in GWLP_USERDATA for retrieval in command handler
    if (hwnd && !onClickAction.empty()) {
        // In a real implementation, we'd store a map of ID -> action
        // For now, this is simplified
    }
}

// ============================================================================
// Win32TextField Implementation
// ============================================================================

Win32TextField::Win32TextField(const UIElement& element, void* parent) {
    HWND parentHwnd = reinterpret_cast<HWND>(parent);
    
    std::string placeholder;
    std::string value;
    std::string onChangeAction;
    
    auto it = element.attributes.find("placeholder");
    if (it != element.attributes.end()) {
        placeholder = it->second;
    }
    
    it = element.attributes.find("value");
    if (it != element.attributes.end()) {
        value = it->second;
    }
    
    it = element.attributes.find("onChange");
    if (it != element.attributes.end()) {
        onChangeAction = it->second;
    }
    
    // Create native edit control
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "EDIT",
        value.c_str(),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        static_cast<int>(element.layout_x),
        static_cast<int>(element.layout_y),
        static_cast<int>(element.layout_width),
        static_cast<int>(element.layout_height),
        parentHwnd,
        nullptr,
        Win32Window::getInstance(),
        nullptr
    );
}

// ============================================================================
// Win32Checkbox Implementation
// ============================================================================

Win32Checkbox::Win32Checkbox(const UIElement& element, void* parent) {
    HWND parentHwnd = reinterpret_cast<HWND>(parent);
    
    std::string text = element.text_content;
    bool checked = false;
    
    auto it = element.attributes.find("checked");
    if (it != element.attributes.end() && it->second == "true") {
        checked = true;
    }
    
    // Create native checkbox
    hwnd = CreateWindowEx(
        0,
        "BUTTON",
        text.c_str(),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        static_cast<int>(element.layout_x),
        static_cast<int>(element.layout_y),
        static_cast<int>(element.layout_width),
        static_cast<int>(element.layout_height),
        parentHwnd,
        nullptr,
        Win32Window::getInstance(),
        nullptr
    );
    
    if (hwnd && checked) {
        SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
    }
}

// ============================================================================
// Win32Slider Implementation
// ============================================================================

Win32Slider::Win32Slider(const UIElement& element, void* parent) {
    HWND parentHwnd = reinterpret_cast<HWND>(parent);
    
    int minVal = 0, maxVal = 100, currentVal = 0;
    
    auto it = element.attributes.find("min");
    if (it != element.attributes.end()) {
        minVal = std::stoi(it->second);
    }
    
    it = element.attributes.find("max");
    if (it != element.attributes.end()) {
        maxVal = std::stoi(it->second);
    }
    
    it = element.attributes.find("value");
    if (it != element.attributes.end()) {
        currentVal = std::stoi(it->second);
    }
    
    // Create native trackbar (slider)
    hwnd = CreateWindowEx(
        0,
        TRACKBAR_CLASSA,
        "",
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_TOOLTIPS,
        static_cast<int>(element.layout_x),
        static_cast<int>(element.layout_y),
        static_cast<int>(element.layout_width),
        static_cast<int>(element.layout_height),
        parentHwnd,
        nullptr,
        Win32Window::getInstance(),
        nullptr
    );
    
    if (hwnd) {
        SendMessage(hwnd, TBM_SETRANGEMIN, TRUE, minVal);
        SendMessage(hwnd, TBM_SETRANGEMAX, TRUE, maxVal);
        SendMessage(hwnd, TBM_SETPOS, TRUE, currentVal);
    }
}

// ============================================================================
// Win32Label Implementation
// ============================================================================

Win32Label::Win32Label(const UIElement& element, void* parent) {
    HWND parentHwnd = reinterpret_cast<HWND>(parent);
    
    std::string text = element.text_content;
    
    // Create native static text control
    hwnd = CreateWindowEx(
        0,
        "STATIC",
        text.c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        static_cast<int>(element.layout_x),
        static_cast<int>(element.layout_y),
        static_cast<int>(element.layout_width),
        static_cast<int>(element.layout_height),
        parentHwnd,
        nullptr,
        Win32Window::getInstance(),
        nullptr
    );
}

} // namespace ui
} // namespace zenith

// Factory function implementations
namespace zenith {
namespace ui {

std::unique_ptr<NativeWindow> createNativeWindow() {
#ifdef _WIN32
    return std::make_unique<Win32Window>();
#else
    // Placeholder for other platforms
    return nullptr;
#endif
}

std::unique_ptr<NativeWidgetFactory> createNativeWidgetFactory() {
#ifdef _WIN32
    // Win32WidgetFactory implementation would go here
    // For now, we'll create a simple inline implementation
    class Win32WidgetFactory : public NativeWidgetFactory {
    public:
        std::unique_ptr<NativeWidget> createButton(const UIElement& element, void* parent) override {
            return std::make_unique<Win32Button>(element, parent);
        }
        
        std::unique_ptr<NativeWidget> createTextField(const UIElement& element, void* parent) override {
            return std::make_unique<Win32TextField>(element, parent);
        }
        
        std::unique_ptr<NativeWidget> createCheckbox(const UIElement& element, void* parent) override {
            return std::make_unique<Win32Checkbox>(element, parent);
        }
        
        std::unique_ptr<NativeWidget> createSlider(const UIElement& element, void* parent) override {
            return std::make_unique<Win32Slider>(element, parent);
        }
        
        std::unique_ptr<NativeWidget> createDropdown(const UIElement& element, void* parent) override {
            // TODO: Implement Win32 dropdown (ComboBox)
            return nullptr;
        }
        
        std::unique_ptr<NativeWidget> createLabel(const UIElement& element, void* parent) override {
            return std::make_unique<Win32Label>(element, parent);
        }
        
        std::unique_ptr<NativeWidget> createContainer(const UIElement& element, void* parent) override {
            // Containers are typically just positioning guides in native Win32
            // Child controls are positioned absolutely
            return nullptr;
        }
        
        void setCallback(NativeWidget* widget, std::function<void(const std::string&, const std::string&)> callback) override {
            if (widget) {
                Win32Widget* win32Widget = dynamic_cast<Win32Widget*>(widget);
                if (win32Widget) {
                    win32Widget->setCallback(callback);
                }
            }
        }
    };
    
    return std::make_unique<Win32WidgetFactory>();
#else
    return nullptr;
#endif
}

} // namespace ui
} // namespace zenith

#endif // _WIN32
