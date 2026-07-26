#ifndef ZENITH_INPUT_BUFFER2D_H
#define ZENITH_INPUT_BUFFER2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct TimedInput {
    std::string inputName; // "Up", "Down", "Left", "Right", "Punch", "Kick"
    float timestamp;
};

class InputBuffer2D {
private:
    std::vector<TimedInput> m_buffer;
    float m_maxAge = 1.0f; // Input buffer expires after 1 second

public:
    void pushInput(const std::string& inputName, float currentTime) {
        m_buffer.push_back({ inputName, currentTime });
    }

    void update(float currentTime) {
        for (auto it = m_buffer.begin(); it != m_buffer.end(); ) {
            if (currentTime - it->timestamp > m_maxAge) {
                it = m_buffer.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool checkSequence(const std::vector<std::string>& sequence) const {
        if (sequence.empty() || m_buffer.size() < sequence.size()) return false;

        size_t seqIdx = 0;
        for (const auto& in : m_buffer) {
            if (in.inputName == sequence[seqIdx]) {
                seqIdx++;
                if (seqIdx == sequence.size()) return true; // Full sequence matched!
            }
        }
        return false;
    }

    void drawBuffer(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 20, int posY = 200, int boxSize = 18, int spacing = 4) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_buffer.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (size_t i = 0; i < m_buffer.size(); ++i) {
            int iX = posX + static_cast<int>(i) * (boxSize + spacing);
            uint32_t boxCol = (m_buffer[i].inputName == "Punch" || m_buffer[i].inputName == "Kick") ? 0xFFFF4500 : 0xFF1E90FF;

            for (int y = posY; y < posY + boxSize; ++y) {
                for (int x = iX; x < iX + boxSize; ++x) {
                    if (x == iX || x == iX + boxSize - 1 || y == posY || y == posY + boxSize - 1) {
                        drawPixel(x, y, 0xFFFFFFFF);
                    } else {
                        drawPixel(x, y, boxCol);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_INPUT_BUFFER2D_H
