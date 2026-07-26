#ifndef ZENITH_VIGNETTE_PULSE2D_H
#define ZENITH_VIGNETTE_PULSE2D_H

#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

class VignettePulse2D {
private:
    float timeAccumulator;
    float pulseFrequency; // Pulse speed in Hz
    float minIntensity;
    float maxIntensity;
    bool active;

public:
    VignettePulse2D()
        : timeAccumulator(0.0f), pulseFrequency(2.0f), minIntensity(0.2f), maxIntensity(0.8f), active(false) {}

    void startPulse(float frequency = 2.0f, float minPower = 0.2f, float maxPower = 0.8f) {
        pulseFrequency = frequency;
        minIntensity = minPower;
        maxIntensity = maxPower;
        active = true;
    }

    void stopPulse() {
        active = false;
    }

    void update(float dt) {
        if (!active) return;
        timeAccumulator += dt;
    }

    bool isActive() const { return active; }

    float getCurrentIntensity() const {
        if (!active) return 0.0f;
        float sineWave = (std::sin(timeAccumulator * pulseFrequency * 6.28318530718f) + 1.0f) * 0.5f;
        return minIntensity + sineWave * (maxIntensity - minIntensity);
    }
};

} // namespace zenith

#endif // ZENITH_VIGNETTE_PULSE2D_H
