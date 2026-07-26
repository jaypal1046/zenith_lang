#ifndef ZENITH_REPLAY2D_H
#define ZENITH_REPLAY2D_H

#include <vector>

namespace zenith {

struct ReplayFrame2D {
    float time = 0.0f;
    float posX = 0.0f;
    float posY = 0.0f;
    unsigned int inputFlags = 0;
};

class ReplaySystem2D {
private:
    std::vector<ReplayFrame2D> m_frames;
    std::size_t m_playbackIndex = 0;
    bool m_isRecording = true;

public:
    ReplaySystem2D() = default;

    void recordFrame(float time, float x, float y, unsigned int inputs = 0) {
        if (!m_isRecording) return;
        m_frames.push_back({time, x, y, inputs});
    }

    void startPlayback() {
        m_isRecording = false;
        m_playbackIndex = 0;
    }

    bool stepPlayback(ReplayFrame2D& outFrame) {
        if (m_playbackIndex >= m_frames.size()) return false;
        outFrame = m_frames[m_playbackIndex++];
        return true;
    }

    void rewind(std::size_t frameCount) {
        if (frameCount >= m_frames.size()) {
            m_frames.clear();
        } else {
            m_frames.resize(m_frames.size() - frameCount);
        }
    }

    std::size_t getFrameCount() const { return m_frames.size(); }
    bool isRecording() const { return m_isRecording; }
};

} // namespace zenith

#endif // ZENITH_REPLAY2D_H
