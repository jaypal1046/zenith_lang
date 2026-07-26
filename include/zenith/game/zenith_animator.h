#ifndef ZENITH_ANIMATOR_H
#define ZENITH_ANIMATOR_H

#include "zenith_texture.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace zenith {

struct AnimationClip2D {
    std::string name;
    std::vector<SpriteRegion> frames;
    float frameDuration = 0.1f;
    bool looping = true;
};

class Animator2D {
private:
    std::unordered_map<std::string, AnimationClip2D> m_clips;
    std::string m_currentClipName;
    std::size_t m_currentFrameIndex = 0;
    float m_elapsedTime = 0.0f;
    bool m_isPlaying = false;

public:
    Animator2D() = default;

    void addClip(const AnimationClip2D& clip) {
        m_clips[clip.name] = clip;
        if (m_currentClipName.empty()) {
            play(clip.name);
        }
    }

    void play(const std::string& name) {
        if (m_currentClipName == name && m_isPlaying) return;

        auto it = m_clips.find(name);
        if (it != m_clips.end()) {
            m_currentClipName = name;
            m_currentFrameIndex = 0;
            m_elapsedTime = 0.0f;
            m_isPlaying = true;
        }
    }

    void update(float dt) {
        if (!m_isPlaying || m_currentClipName.empty()) return;

        auto it = m_clips.find(m_currentClipName);
        if (it == m_clips.end() || it->second.frames.empty()) return;

        const auto& clip = it->second;
        m_elapsedTime += dt;
        if (m_elapsedTime >= clip.frameDuration) {
            m_elapsedTime = 0.0f;
            m_currentFrameIndex++;
            if (m_currentFrameIndex >= clip.frames.size()) {
                if (clip.looping) {
                    m_currentFrameIndex = 0;
                } else {
                    m_currentFrameIndex = clip.frames.size() - 1;
                    m_isPlaying = false;
                }
            }
        }
    }

    SpriteRegion getCurrentRegion() const {
        auto it = m_clips.find(m_currentClipName);
        if (it != m_clips.end() && !it->second.frames.empty()) {
            return it->second.frames[m_currentFrameIndex];
        }
        return SpriteRegion{};
    }

    std::string getCurrentClipName() const { return m_currentClipName; }
    bool isPlaying() const { return m_isPlaying; }
};

} // namespace zenith

#endif // ZENITH_ANIMATOR_H
