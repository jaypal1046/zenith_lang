#ifndef ZENITH_SCREEN_FLASH2D_H
#define ZENITH_SCREEN_FLASH2D_H

#include <algorithm>
#include <cstdint>

namespace zenith {

class ScreenFlash2D {
private:
    uint32_t flashColor;
    float duration;
    float timer;
    bool active;

public:
    ScreenFlash2D() : flashColor(0xFFFFFFFF), duration(0.0f), timer(0.0f), active(false) {}

    void triggerFlash(uint32_t color = 0xFFFFFFFF, float flashDuration = 0.3f) {
        flashColor = color;
        duration = flashDuration;
        timer = flashDuration;
        active = true;
    }

    void update(float dt) {
        if (!active) return;

        timer -= dt;
        if (timer <= 0.0f) {
            timer = 0.0f;
            active = false;
        }
    }

    bool isActive() const { return active; }

    uint32_t getCurrentFlashColor() const {
        if (!active || duration <= 0.0f) return 0x00000000;

        float alphaRatio = timer / duration;
        uint32_t baseAlpha = (flashColor >> 24) & 0xFF;
        uint32_t currentAlpha = static_cast<uint32_t>(baseAlpha * alphaRatio);

        return (currentAlpha << 24) | (flashColor & 0x00FFFFFF);
    }
};

} // namespace zenith

#endif // ZENITH_SCREEN_FLASH2D_H
