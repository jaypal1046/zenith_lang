#ifndef ZENITH_FONT_H
#define ZENITH_FONT_H

#include "zenith_window.h"
#include <string>

namespace zenith {

class Font2D {
public:
    static float measureTextWidth(const std::string& text, float fontSize) {
        return text.length() * (fontSize * 0.6f);
    }

    static void drawText(const std::string& text, float x, float y, float fontSize = 16.0f, const Color& color = Color::White()) {
#ifdef _WIN32
        // Immediate fallback text quad visualization for code-first game HUDs
        float currX = x;
        float charWidth = fontSize * 0.6f;
        float charHeight = fontSize;

        for (char c : text) {
            (void)c;
            Renderer2D::drawRect(currX, y, charWidth * 0.8f, charHeight, color);
            currX += charWidth;
        }
#endif
    }
};

} // namespace zenith

#endif // ZENITH_FONT_H
