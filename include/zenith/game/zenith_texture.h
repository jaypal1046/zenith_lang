#ifndef ZENITH_TEXTURE_H
#define ZENITH_TEXTURE_H

#include "zenith_window.h"
#include <string>
#include <vector>
#include <iostream>

namespace zenith {

struct Texture2D {
    unsigned int id = 0;
    int width = 0;
    int height = 0;
    int channels = 4;
    std::string path;

    bool isValid() const { return id > 0 || width > 0; }
};

struct SpriteRegion {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
};

class TextureManager {
public:
    static Texture2D createDummyProceduralTexture(int width, int height, const Color& fillColor) {
        Texture2D tex;
        tex.width = width;
        tex.height = height;
        tex.channels = 4;
        tex.path = "procedural://dummy";

#ifdef _WIN32
        std::vector<unsigned char> pixels(width * height * 4);
        for (int i = 0; i < width * height; ++i) {
            pixels[i * 4 + 0] = static_cast<unsigned char>(fillColor.r * 255.0f);
            pixels[i * 4 + 1] = static_cast<unsigned char>(fillColor.g * 255.0f);
            pixels[i * 4 + 2] = static_cast<unsigned char>(fillColor.b * 255.0f);
            pixels[i * 4 + 3] = static_cast<unsigned char>(fillColor.a * 255.0f);
        }

        glGenTextures(1, &tex.id);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
#endif
        return tex;
    }

    static void drawSprite(const Texture2D& texture, float x, float y, float width, float height, const Color& tint = Color::White()) {
#ifdef _WIN32
        if (texture.id > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture.id);
        }

        glColor4f(tint.r, tint.g, tint.b, tint.a);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + height);
        glEnd();

        if (texture.id > 0) {
            glDisable(GL_TEXTURE_2D);
        }
#else
        Renderer2D::drawRect(x, y, width, height, tint);
#endif
    }

    static void drawSpriteRegion(const Texture2D& texture, const SpriteRegion& region, float x, float y, float width, float height, const Color& tint = Color::White()) {
#ifdef _WIN32
        if (texture.id > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture.id);
        }

        glColor4f(tint.r, tint.g, tint.b, tint.a);
        glBegin(GL_QUADS);
            glTexCoord2f(region.u0, region.v0); glVertex2f(x, y);
            glTexCoord2f(region.u1, region.v0); glVertex2f(x + width, y);
            glTexCoord2f(region.u1, region.v1); glVertex2f(x + width, y + height);
            glTexCoord2f(region.u0, region.v1); glVertex2f(x, y + height);
        glEnd();

        if (texture.id > 0) {
            glDisable(GL_TEXTURE_2D);
        }
#else
        Renderer2D::drawRect(x, y, width, height, tint);
#endif
    }
};

} // namespace zenith

#endif // ZENITH_TEXTURE_H
