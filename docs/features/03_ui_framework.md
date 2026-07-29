# Zenith Feature Audit: Immediate-Mode Game Canvas & Native UI

## 1. Executive Overview
Zenith uses a code-first **Immediate-Mode Canvas Renderer** for game scenes (`Canvas.drawRect()`, `Canvas.drawFrameRect()`, `Canvas.drawText()`, `Canvas.drawTextWrapped()`, `Canvas.drawPoint()`, `pctX()`, `pctY()`), paired with a Win32 native OS windowing layer and Facebook Yoga flexbox integration for desktop desktop tools.

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **Immediate-Mode Canvas API** ([main.zen](file:///c:/Jay/_Plugin/zenith_lang/projects/nebula_novel/scenes/main.zen), [zenith_scene.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_scene.h)):
  - `onDraw(canvas: Canvas, alpha: Float)` scene rendering callback.
  - Primitive drawing: `drawRect`, `drawFrameRect`, `drawPoint`, `drawText`, `drawTextWrapped`.
  - Viewport-relative screen coordinate math: `canvas.mediaWidth()`, `canvas.mediaHeight()`, `canvas.pctX(percentage)`, `canvas.pctY(percentage)`.
- **Win32 Native Window Backend** ([win32_window.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/win32_window.h), [win32_window.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/ui/win32_window.cpp)):
  - Windows window creation (`HWND`, `CreateWindowEx`, `RegisterClassEx`).
  - Windows message loop (`GetMessage`, `DispatchMessage`, `WM_COMMAND`, `WM_PAINT`).
  - Abstract widget factories ([native_window.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/native_window.h), [native_widget_factory.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/native_widget_factory.h)).
- **Yoga Flexbox Adapter** ([yoga_layout.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/yoga_layout.h)):
  - Flexbox container layout calculation wrapper.

---

## 3. ❌ Missing / Incomplete Features
- **Unified Cross-Platform Canvas Wrapper (SDL2 / GLFW)**: Win32 native windowing works, but Mac/Linux canvas targets currently rely on terminal ASCII preview or web fallbacks.
- **Canvas Texture Batching**: GPU draw call batching for thousands of canvas sprite operations.

---

## 4. 💻 How to Use (Real Compiling Syntax)
```zenith
import std.io;

class HUDScene() implements Scene {
    String currentText = "Sensors detect energy anomaly ahead.";

    Void onDraw(canvas: Canvas, alpha: Float) {
        Float screenW = canvas.mediaWidth();
        Float screenH = canvas.mediaHeight();

        // 1. Draw Background
        canvas.drawRect(0.0, 0.0, screenW, screenH, "dark_slate");

        // 2. Draw Top Sci-Fi Header
        canvas.drawRect(0.0, 0.0, screenW, 2.0, "dark_navy");
        canvas.drawText("NEBULA CRISIS", 2.0, 0.5, "sky_blue");

        // 3. Draw Story Dialogue Panel
        Float dlgX = 2.0;
        Float dlgY = screenH - 10.0;
        Float dlgW = screenW - 4.0;
        Float dlgH = 9.0;

        canvas.drawRect(dlgX, dlgY, dlgW, dlgH, "dark_navy");
        canvas.drawFrameRect(dlgX, dlgY, dlgW, dlgH, "panel_border");
        canvas.drawTextWrapped(currentText, dlgX + 3.0, dlgY + 2.0, dlgW - 6.0, "white");
    }
}
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File | Implementation File |
| :--- | :--- | :--- |
| **Canvas Interop & Scene Lifecycle** | [zenith_scene.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_scene.h) | N/A (Header-only game engine runtime) |
| **Win32 Native Windowing** | [win32_window.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/win32_window.h) | [win32_window.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/ui/win32_window.cpp) |
| **Native Abstract Interfaces** | [native_window.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/native_window.h), [native_widget_factory.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/native_widget_factory.h) | N/A |
| **Yoga Flexbox Layout Adapter** | [yoga_layout.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/yoga_layout.h) | N/A |
