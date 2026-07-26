#ifndef ZENITH_VIDEO_H
#define ZENITH_VIDEO_H

#include "zenith_window.h"
#include "zenith_texture.h"
#include "zenith_gui.h"
#include <vector>
#include <string>
#include <iostream>

namespace zenith {

// --- Animated Sprite Sequence (Frame-based Video & Cutscene playback) ---
class AnimatedSpriteWidget : public Widget {
public:
    std::vector<Texture2D> frames;
    float frameDuration = 0.0416f; // 24 FPS default
    bool looping = true;
    bool isPlaying = true;
    float width = 0.0f;
    float height = 0.0f;

private:
    std::size_t m_currentFrame = 0;
    float m_elapsedTime = 0.0f;

public:
    AnimatedSpriteWidget() = default;

    void addFrame(const Texture2D& frame) {
        frames.push_back(frame);
    }

    void play() { isPlaying = true; }
    void pause() { isPlaying = false; }
    void stop() { isPlaying = false; m_currentFrame = 0; m_elapsedTime = 0.0f; }

    void update(float dt) {
        if (!isPlaying || frames.empty()) return;

        m_elapsedTime += dt;
        if (m_elapsedTime >= frameDuration) {
            m_elapsedTime = 0.0f;
            m_currentFrame++;
            if (m_currentFrame >= frames.size()) {
                if (looping) {
                    m_currentFrame = 0;
                } else {
                    m_currentFrame = frames.size() - 1;
                    isPlaying = false;
                }
            }
        }
    }

    void render(float x, float y, float availableWidth, float availableHeight) override {
        if (frames.empty()) return;

        float renderW = (width > 0.0f) ? width : availableWidth;
        float renderH = (height > 0.0f) ? height : availableHeight;

        TextureManager::drawSprite(frames[m_currentFrame], x, y, renderW, renderH);
    }
};

// --- Raw Video Frame Stream Widget ---
class VideoStreamWidget : public Widget {
public:
    int videoWidth = 640;
    int videoHeight = 360;
    bool isPlaying = true;
    Texture2D frameTexture;

private:
    std::vector<unsigned char> m_rawFrameBuffer;

public:
    VideoStreamWidget() {
        frameTexture = TextureManager::createDummyProceduralTexture(videoWidth, videoHeight, Color::Black());
        m_rawFrameBuffer.resize(videoWidth * videoHeight * 4, 255);
    }

    void updateRawFrame(const unsigned char* pixelData, std::size_t bufferSize) {
        if (!pixelData || bufferSize < m_rawFrameBuffer.size()) return;
        std::copy(pixelData, pixelData + m_rawFrameBuffer.size(), m_rawFrameBuffer.begin());

#ifdef _WIN32
        if (frameTexture.id > 0) {
            glBindTexture(GL_TEXTURE_2D, frameTexture.id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoWidth, videoHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_rawFrameBuffer.data());
        }
#endif
    }

    void render(float x, float y, float availableWidth, float availableHeight) override {
        float renderW = (availableWidth > 0.0f) ? availableWidth : static_cast<float>(videoWidth);
        float renderH = (availableHeight > 0.0f) ? availableHeight : static_cast<float>(videoHeight);

        TextureManager::drawSprite(frameTexture, x, y, renderW, renderH);
    }
};

} // namespace zenith

#endif // ZENITH_VIDEO_H
