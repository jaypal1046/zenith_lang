#ifndef ZENITH_PROCEDURAL_AUDIO_H
#define ZENITH_PROCEDURAL_AUDIO_H

#include "zenith_audio.h"
#include <vector>
#include <cmath>
#include <cstdint>

namespace zenith {

class ProceduralAudio {
public:
    static std::vector<int16_t> generateTone(float frequency, float duration, int sampleRate = 44100) {
        std::size_t numSamples = static_cast<std::size_t>(sampleRate * duration);
        std::vector<int16_t> samples(numSamples);

        for (std::size_t i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float sineWave = sinf(2.0f * 3.14159265f * frequency * t);
            float envelope = 1.0f - (t / duration); // Exponential decay
            samples[i] = static_cast<int16_t>(sineWave * envelope * 32767.0f * 0.5f);
        }
        return samples;
    }

    static std::vector<int16_t> generateLaser(float duration = 0.2f, int sampleRate = 44100) {
        std::size_t numSamples = static_cast<std::size_t>(sampleRate * duration);
        std::vector<int16_t> samples(numSamples);

        for (std::size_t i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float freq = 800.0f - (t / duration) * 600.0f; // Pitch sweep down
            float wave = sinf(2.0f * 3.14159265f * freq * t);
            float envelope = 1.0f - (t / duration);
            samples[i] = static_cast<int16_t>(wave * envelope * 32767.0f * 0.5f);
        }
        return samples;
    }
};

} // namespace zenith

#endif // ZENITH_PROCEDURAL_AUDIO_H
