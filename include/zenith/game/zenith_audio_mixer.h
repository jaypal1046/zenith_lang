#ifndef ZENITH_AUDIO_MIXER_H
#define ZENITH_AUDIO_MIXER_H

#include <unordered_map>
#include <string>
#include <algorithm>

namespace zenith {

enum class AudioBus {
    Master,
    Sfx,
    Music,
    Voice,
    UI
};

class AudioMixer2D {
private:
    static inline float s_masterVolume = 1.0f;
    static inline float s_sfxVolume = 1.0f;
    static inline float s_musicVolume = 1.0f;
    static inline float s_voiceVolume = 1.0f;
    static inline float s_uiVolume = 1.0f;

public:
    static void setBusVolume(AudioBus bus, float volume) {
        float clamped = std::max(0.0f, std::min(1.0f, volume));
        switch (bus) {
            case AudioBus::Master: s_masterVolume = clamped; break;
            case AudioBus::Sfx:    s_sfxVolume = clamped; break;
            case AudioBus::Music:  s_musicVolume = clamped; break;
            case AudioBus::Voice:  s_voiceVolume = clamped; break;
            case AudioBus::UI:     s_uiVolume = clamped; break;
        }
    }

    static float getBusVolume(AudioBus bus) {
        switch (bus) {
            case AudioBus::Master: return s_masterVolume;
            case AudioBus::Sfx:    return s_sfxVolume * s_masterVolume;
            case AudioBus::Music:  return s_musicVolume * s_masterVolume;
            case AudioBus::Voice:  return s_voiceVolume * s_masterVolume;
            case AudioBus::UI:     return s_uiVolume * s_masterVolume;
        }
        return s_masterVolume;
    }
};

} // namespace zenith

#endif // ZENITH_AUDIO_MIXER_H
