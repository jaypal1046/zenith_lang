#ifndef ZENITH_LIGHT2D_H
#define ZENITH_LIGHT2D_H

#include "zenith_window.h"
#include <vector>
#include <cmath>

namespace zenith {

struct PointLight2D {
    float x = 0.0f;
    float y = 0.0f;
    float radius = 150.0f;
    Color color = Color::Yellow();
    float intensity = 1.0f;
};

class Lighting2D {
private:
    static inline Color s_ambientColor = Color{0.1f, 0.1f, 0.15f, 1.0f};

public:
    static void setAmbientColor(const Color& color) {
        s_ambientColor = color;
    }

    static Color getAmbientColor() { return s_ambientColor; }

    static void drawPointLight(const PointLight2D& light, int segments = 32) {
#ifdef _WIN32
        glColor4f(light.color.r * light.intensity, light.color.g * light.intensity, light.color.b * light.intensity, light.color.a * 0.5f);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(light.x, light.y);
            for (int i = 0; i <= segments; ++i) {
                float angle = i * 2.0f * 3.14159265f / segments;
                glVertex2f(light.x + cosf(angle) * light.radius, light.y + sinf(angle) * light.radius);
            }
        glEnd();
#else
        Renderer2D::drawCircle(light.x, light.y, light.radius, light.color);
#endif
    }
};

} // namespace zenith

#endif // ZENITH_LIGHT2D_H
