#ifndef ZENITH_GAME_FEEL2D_H
#define ZENITH_GAME_FEEL2D_H

#include "zenith_camera_fx.h"
#include "zenith_screen_flash2d.h"
#include "zenith_speed_lines2d.h"

namespace zenith {

class GameFeelManager2D {
private:
    CameraShake2D cameraShake;
    ScreenFlash2D screenFlash;
    float hitStopTimer;
    bool inHitStop;

public:
    GameFeelManager2D() : hitStopTimer(0.0f), inHitStop(false) {}

    void triggerHitImpact(float shakeIntensity = 10.0f, uint32_t flashColor = 0xFFFFFFFF, float hitStopDuration = 0.05f) {
        cameraShake.trigger(shakeIntensity, 0.3f);
        screenFlash.triggerFlash(flashColor, 0.2f);
        if (hitStopDuration > 0.0f) {
            hitStopTimer = hitStopDuration;
            inHitStop = true;
        }
    }

    void update(float dt) {
        cameraShake.update(dt);
        screenFlash.update(dt);

        if (inHitStop) {
            hitStopTimer -= dt;
            if (hitStopTimer <= 0.0f) {
                hitStopTimer = 0.0f;
                inHitStop = false;
            }
        }
    }

    bool isHitStopActive() const { return inHitStop; }
    CameraShake2D& getCameraShake() { return cameraShake; }
    ScreenFlash2D& getScreenFlash() { return screenFlash; }
};

} // namespace zenith

#endif // ZENITH_GAME_FEEL2D_H
