#ifndef ZENITH_VIEWPORT_H
#define ZENITH_VIEWPORT_H

#include "zenith_window.h"

namespace zenith {

struct Viewport2D {
    int x = 0;
    int y = 0;
    int width = 800;
    int height = 600;

    void apply() const {
#ifdef _WIN32
        glViewport(x, y, width, height);
        glScissor(x, y, width, height);
        glEnable(GL_SCISSOR_TEST);
#endif
    }

    static void reset(int windowWidth = 800, int windowHeight = 600) {
#ifdef _WIN32
        glViewport(0, 0, windowWidth, windowHeight);
        glDisable(GL_SCISSOR_TEST);
#endif
    }
};

} // namespace zenith

#endif // ZENITH_VIEWPORT_H
