#ifndef ZENITH_SCREEN_FREEZE2D_H
#define ZENITH_SCREEN_FREEZE2D_H

#include <vector>
#include <algorithm>
#include <cstdint>

namespace zenith {

class ScreenFreeze2D {
private:
    std::vector<uint32_t> snapshotBuffer;
    int snapshotWidth;
    int snapshotHeight;
    bool frozen;

public:
    ScreenFreeze2D() : snapshotWidth(0), snapshotHeight(0), frozen(false) {}

    void capture(const std::vector<uint32_t>& buffer, int width, int height) {
        snapshotBuffer = buffer;
        snapshotWidth = width;
        snapshotHeight = height;
        frozen = true;
    }

    void unfreeze() {
        frozen = false;
        snapshotBuffer.clear();
    }

    bool isFrozen() const { return frozen; }

    void applyFreezeFade(std::vector<uint32_t>& targetBuffer, float fadeAlpha = 0.5f) {
        if (!frozen || snapshotBuffer.empty() || targetBuffer.size() != snapshotBuffer.size()) return;

        float alpha = std::clamp(fadeAlpha, 0.0f, 1.0f);

        for (std::size_t i = 0; i < targetBuffer.size(); ++i) {
            uint32_t tP = targetBuffer[i];
            uint32_t sP = snapshotBuffer[i];

            uint32_t r = static_cast<uint32_t>(((tP >> 16) & 0xFF) * (1.0f - alpha) + ((sP >> 16) & 0xFF) * alpha);
            uint32_t g = static_cast<uint32_t>(((tP >> 8) & 0xFF)  * (1.0f - alpha) + ((sP >> 8) & 0xFF)  * alpha);
            uint32_t b = static_cast<uint32_t>((tP & 0xFF)         * (1.0f - alpha) + (sP & 0xFF)         * alpha);

            targetBuffer[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }
};

} // namespace zenith

#endif // ZENITH_SCREEN_FREEZE2D_H
