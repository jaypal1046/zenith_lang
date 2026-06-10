# Zenith Native C++ UI Backend - Gap Analysis & Implementation Plan

## Executive Summary

**Current State:** Zenith has a complete compiler pipeline, advanced type system, Yoga layout engine integration, and UI widget definitions, but **only renders to terminal using ASCII art**. There is **no native window creation** or **native widget mapping**.

**Critical Missing Components:**
1. No native window creation (HWND, NSWindow, GTK widgets)
2. No native event loop integration (Windows MSG, macOS NSApplication, X11/Wayland)
3. No UIElement-to-native-widget mapping layer
4. Build system doesn't link native UI libraries
5. No hardware rendering backend

---

## Current Architecture Analysis

### What Exists ✅

1. **Compiler Pipeline** (`src/frontend/`, `src/backend/`)
   - Lexer, Parser, Semantic Analysis, Formatter
   - Codegen for JS, WebAssembly
   - LSP support

2. **Type System** (`include/zenith/`)
   - Advanced type inference
   - FFI bindings for Python, C dynamic libs
   - Memory management (RC+GC)

3. **UI Infrastructure** (`include/zenith/ui/`)
   - `yoga_layout.h` - Complete Yoga flexbox integration
   - UIElement definitions with layout calculations
   - Widget types: Button, TextField, Checkbox, Slider, Toggle, Dropdown, Card, Row, Column, Text

4. **Platform Headers** (`include/zenith/desktop/`)
   - `windows/zenith_windows.h` - WinHTTP, terminal input
   - `mac/zenith_mac.h`, `linux/zenith_linux.h` - Platform detection
   - HTTP client, LLM client utilities

5. **Sample Application** (`todo_app/desktop/main.cpp`)
   - Reactive todo app with state management
   - Uses `zenith::runInteractiveLoop()` which only does terminal rendering

### What's Missing ❌

#### 1. Native Window Creation Layer
**Problem:** No actual OS windows are created. The `runInteractiveLoop()` function only prints ASCII art to terminal.

**Required:**
```cpp
// Missing interface
class NativeWindow {
public:
    virtual void create(const std::string& title, int width, int height) = 0;
    virtual void show() = 0;
    virtual void close() = 0;
    virtual bool shouldClose() const = 0;
    virtual void* getNativeHandle() = 0; // HWND, NSWindow*, GtkWindow*
};
```

#### 2. Native Event Loop Integration
**Problem:** Currently uses simple terminal keyboard input (`_getch()`). No integration with platform event loops.

**Required:**
- **Windows:** `GetMessage()/DispatchMessage()` message loop
- **macOS:** `NSApplication` run loop
- **Linux:** X11 event loop or Wayland event queue

#### 3. UIElement → Native Widget Mapping
**Problem:** Zenith UIElements are calculated with Yoga layout but never mapped to actual native controls.

**Required Mapping:**
```
Zenith UIElement     →   Native Control
─────────────────────────────────────────
Button              →   Win32: BUTTON, Cocoa: NSButton, GTK: GtkButton
TextField           →   Win32: EDIT, Cocoa: NSTextField, GTK: GtkEntry
Checkbox            →   Win32: BUTTON (BS_CHECKBOX), Cocoa: NSButton, GTK: GtkCheckButton
Slider              →   Win32: TRACKBAR_CLASS, Cocoa: NSSlider, GTK: GtkScale
Dropdown            →   Win32: COMBOBOX, Cocoa: NSComboBox, GTK: GtkComboBoxText
Card/Row/Column     →   Layout containers (mapped to native views)
Text                →   Static text labels
```

#### 4. Build System Gaps
**Problem:** Makefile doesn't link native UI libraries.

**Current Makefile:**
```makefile
LDFLAGS = -lws2_32  # Only WinHTTP
```

**Required:**
```makefile
# Windows: Link user32, gdi32, comctl32
LDFLAGS_WIN = -luser32 -lgdi32 -lcomctl32 -lws2_32

# macOS: Link Cocoa framework
LDFLAGS_MAC = -framework Cocoa

# Linux: Link GTK or X11
LDFLAGS_LINUX = $(shell pkg-config --libs gtk+-3.0)
```

#### 5. No Hardware Rendering
**Problem:** Terminal-only rendering means no GPU acceleration, no custom drawing, no animations.

**Options:**
- **Dear ImGui** - Immediate mode GUI, easy integration
- **Skia** - 2D graphics library (used by Flutter, Chrome)
- **OpenGL/Vulkan** - Direct GPU access
- **DirectX** - Windows-only GPU rendering

---

## Implementation Priority

### Phase 1: Critical Foundation (Week 1-2)
**Goal:** Create actual native windows on Windows

1. **Create `NativeWindow` interface** (`include/zenith/ui/native_window.h`)
2. **Implement Win32 backend** (`src/ui/win32_window.cpp`)
   - Create HWND with `CreateWindowEx()`
   - Implement Windows message loop
   - Handle WM_PAINT, WM_MOUSEMOVE, WM_LBUTTONDOWN, etc.
3. **Update build system** to link user32, gdi32
4. **Replace terminal loop** with native window loop in `zenith_windows.h`

### Phase 2: Widget Mapping (Week 3-4)
**Goal:** Map Zenith UIElements to Win32 controls

1. **Create widget factory** (`include/zenith/ui/native_widget_factory.h`)
2. **Implement control creation**:
   ```cpp
   HWND createNativeButton(const UIElement& element, HWND parent);
   HWND createNativeTextField(const UIElement& element, HWND parent);
   ```
3. **Two-way binding** between Zenith state and native controls
4. **Event handling** - native events trigger Zenith callbacks

### Phase 3: Cross-Platform (Week 5-8)
**Goal:** Support macOS and Linux

1. **macOS backend** (`src/ui/cocoa_window.mm`)
   - NSWindow creation
   - NSButton, NSTextField, etc.
2. **Linux backend** (`src/ui/gtk_window.cpp`)
   - GtkWindow creation
   - GTK widget mapping
3. **Platform abstraction layer** for unified API

### Phase 4: Enhanced Rendering (Week 9-12)
**Goal:** Add hardware-accelerated custom rendering

1. **Integrate Dear ImGui** or **Skia**
2. **Custom widget rendering** for non-standard controls
3. **Animation support**
4. **Theme/custom styling system**

---

## Strategic Recommendations

### Option A: Wrap Existing Toolkit (Recommended)
**Use Dear ImGui + GLFW/SDL2**

**Pros:**
- Fast implementation (days, not months)
- Cross-platform from day one
- Hardware accelerated
- Active community, well-documented
- Custom look possible

**Cons:**
- Not truly "native" look (but can theme)
- Additional dependency

**Implementation:**
```cpp
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Map Zenith UI to ImGui calls
void renderNative(const UIElement& root) {
    ImGui::Begin("Zenith Window");
    for (const auto& child : root.children) {
        if (child.type == "Button") {
            if (ImGui::Button(child.text_content.c_str())) {
                // Trigger callback
            }
        } else if (child.type == "TextField") {
            // Use ImGui input field
        }
    }
    ImGui::End();
}
```

### Option B: Pure Native APIs
**Use raw Win32, Cocoa, GTK**

**Pros:**
- True native look & feel
- No additional dependencies
- Best accessibility support
- Smaller binary size

**Cons:**
- Much more code (months of work)
- Platform-specific maintenance burden
- Harder to add custom rendering

### Option C: Hybrid Approach (Best Long-term)
**Use native controls for standard widgets, custom rendering for complex UI**

**Implementation:**
- Standard widgets (Button, TextField) → Native controls
- Custom components → Dear ImGui/Skia rendering
- Layout → Yoga (already implemented)

---

## Code Changes Required

### 1. New Files to Create

```
include/zenith/ui/
├── native_window.h          # Abstract window interface
├── native_widget_factory.h  # Widget creation factory
├── event_handler.h          # Event dispatch system
└── render_backend.h         # Rendering abstraction

src/ui/
├── win32_window.cpp         # Windows implementation
├── win32_widgets.cpp        # Win32 control mapping
├── cocoa_window.mm          # macOS implementation
├── gtk_window.cpp           # Linux implementation
├── imgui_backend.cpp        # Optional: Dear ImGui integration
└── event_loop.cpp           # Cross-platform event handling
```

### 2. Modified Files

```
include/zenith/desktop/windows/zenith_windows.h
  - Replace runInteractiveLoop() with native window creation
  - Remove terminal rendering code
  - Add Win32 message loop

Makefile
  - Add platform-specific linker flags
  - Include new source files
  - Link native UI libraries

todo_app/desktop/main.cpp
  - Update to use native window instead of terminal loop
```

### 3. Minimal Win32 Implementation Example

```cpp
// src/ui/win32_window.cpp
#include <windows.h>
#include "zenith/ui/native_window.h"

namespace zenith { namespace ui {

class Win32Window : public NativeWindow {
private:
    HWND hwnd = nullptr;
    bool closed = false;
    
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                // Render UI here
                EndPaint(hWnd, &ps);
                return 0;
            }
            case WM_COMMAND: {
                // Handle button clicks, etc.
                int wmId = LOWORD(wParam);
                // Dispatch to Zenith callback system
                return 0;
            }
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

public:
    void create(const std::string& title, int width, int height) override {
        // Register window class
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "ZenithWindow";
        RegisterClassEx(&wc);
        
        // Create window
        hwnd = CreateWindowEx(
            0,
            "ZenithWindow",
            title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            width, height,
            nullptr, nullptr,
            GetModuleHandle(NULL),
            nullptr
        );
    }
    
    void show() override {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
        
        // Message loop
        MSG msg;
        while (!closed && GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    void close() override {
        closed = true;
        DestroyWindow(hwnd);
    }
    
    void* getNativeHandle() override {
        return hwnd;
    }
};

}} // namespace zenith::ui
```

---

## Testing Strategy

### Unit Tests
- Test native window creation/destruction
- Verify event dispatch correctness
- Validate widget property mapping

### Integration Tests
- Run todo_app with native window
- Test all widget interactions
- Verify state reactivity

### Platform Tests
- Windows 10/11
- macOS 12+
- Linux (Ubuntu, Fedora)

---

## Success Metrics

**Phase 1 Complete When:**
- [ ] Native window appears on Windows desktop
- [ ] Window can be moved, resized, closed
- [ ] No terminal output (except debug logs)

**Phase 2 Complete When:**
- [ ] All basic widgets render as native controls
- [ ] Button clicks trigger Zenith callbacks
- [ ] TextField input updates Zenith state
- [ ] Checkbox/Slider/Dropdown work correctly

**Phase 3 Complete When:**
- [ ] Same app runs on macOS with native Cocoa UI
- [ ] Same app runs on Linux with native GTK UI
- [ ] Consistent behavior across platforms

**Phase 4 Complete When:**
- [ ] Custom widgets can be rendered with GPU acceleration
- [ ] Animations run at 60fps
- [ ] Theme system allows customization

---

## Risk Mitigation

| Risk | Impact | Mitigation |
|------|--------|------------|
| Platform complexity | High | Start with Windows only, expand gradually |
| Native API changes | Medium | Use stable APIs, avoid deprecated features |
| Performance issues | Medium | Profile early, use hardware acceleration |
| Accessibility gaps | Medium | Follow platform guidelines, test with screen readers |
| Maintenance burden | High | Choose mature libraries (Dear ImGui, GTK) |

---

## Conclusion

**Immediate Next Step:** Create `NativeWindow` interface and implement minimal Win32 backend to prove the concept. This will transform Zenith from a terminal-based prototype into a real desktop application framework.

**Recommended Path:** Hybrid approach using native controls for standard widgets + Dear ImGui for custom rendering. This provides the best balance of native look, development speed, and flexibility.

**Timeline:** 
- MVP (Windows only): 2-3 weeks
- Cross-platform beta: 6-8 weeks  
- Production ready: 12+ weeks

The technical foundation is solid. The missing piece is connecting the Yoga layout calculations and UIElement tree to actual native windowing systems. Once this bridge is built, Zenith will have a significant advantage over Flutter (smaller binaries, true native controls) while maintaining cross-platform capabilities.
