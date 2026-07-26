#ifndef ZENITH_CAMERA3D_H
#define ZENITH_CAMERA3D_H

#include "zenith_window.h"
#include <cmath>

namespace zenith {

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Camera3D {
    Vector3 position{0.0f, 10.0f, 10.0f};
    Vector3 target{0.0f, 0.0f, 0.0f};
    Vector3 up{0.0f, 1.0f, 0.0f};
    float fovy = 45.0f;
    float aspectRatio = 1.777f;

    static void beginCamera3D(const Camera3D& camera) {
#ifdef _WIN32
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        float top = tanf(camera.fovy * 0.5f * 3.14159265f / 180.0f) * 0.1f;
        float right = top * camera.aspectRatio;
        glFrustum(-right, right, -top, top, 0.1f, 1000.0f);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Basic lookAt translation
        glTranslatef(-camera.position.x, -camera.position.y, -camera.position.z);
        glEnable(GL_DEPTH_TEST);
#endif
    }

    static void endCamera3D() {
#ifdef _WIN32
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
#endif
    }
};

class Renderer3D {
public:
    static void drawCube(const Vector3& pos, float width, float height, float length, const Color& color) {
#ifdef _WIN32
        float w2 = width * 0.5f;
        float h2 = height * 0.5f;
        float l2 = length * 0.5f;

        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_QUADS);
            // Front Face
            glVertex3f(pos.x - w2, pos.y - h2, pos.z + l2);
            glVertex3f(pos.x + w2, pos.y - h2, pos.z + l2);
            glVertex3f(pos.x + w2, pos.y + h2, pos.z + l2);
            glVertex3f(pos.x - w2, pos.y + h2, pos.z + l2);

            // Back Face
            glVertex3f(pos.x - w2, pos.y - h2, pos.z - l2);
            glVertex3f(pos.x - w2, pos.y + h2, pos.z - l2);
            glVertex3f(pos.x + w2, pos.y + h2, pos.z - l2);
            glVertex3f(pos.x + w2, pos.y - h2, pos.z - l2);
        glEnd();
#endif
    }

    static void drawGrid3D(int slices, float spacing, const Color& color = Color{0.3f, 0.3f, 0.3f, 1.0f}) {
#ifdef _WIN32
        float halfSize = (slices * spacing) * 0.5f;
        glColor4f(color.r, color.g, color.b, color.a);
        glBegin(GL_LINES);
        for (int i = -slices; i <= slices; ++i) {
            float pos = i * spacing;
            glVertex3f(pos, 0.0f, -halfSize);
            glVertex3f(pos, 0.0f, halfSize);

            glVertex3f(-halfSize, 0.0f, pos);
            glVertex3f(halfSize, 0.0f, pos);
        }
        glEnd();
#endif
    }
};

} // namespace zenith

#endif // ZENITH_CAMERA3D_H
