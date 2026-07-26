#ifndef ZENITH_AUDIO_H
#define ZENITH_AUDIO_H

#include "../common/zenith_common.h"
#include <string>
#include <iostream>
#include <unordered_map>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

namespace zenith {

struct Sound {
    std::string path;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool isLoaded = false;
};

struct Music {
    std::string path;
    float volume = 1.0f;
    bool isPlaying = false;
    bool isLooping = true;
};

class AudioManager {
private:
    static inline float s_masterVolume = 1.0f;
    static inline std::string s_currentMusicPath;

public:
    static void setMasterVolume(float volume) {
        s_masterVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f ? 1.0f : volume);
    }

    static float getMasterVolume() { return s_masterVolume; }

    static bool playSound(const std::string& wavPath, float volume = 1.0f) {
#ifdef _WIN32
        // Play asynchronous WAV sound effect via Windows Multimedia API
        BOOL res = PlaySoundA(wavPath.c_str(), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
        return (res == TRUE);
#else
        std::cout << "[Audio] Playing sound: " << wavPath << " (vol=" << volume * s_masterVolume << ")" << std::endl;
        return true;
#endif
    }

    static bool playMusic(const std::string& wavPath, bool loop = true) {
        s_currentMusicPath = wavPath;
#ifdef _WIN32
        DWORD flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT;
        if (loop) flags |= SND_LOOP;
        BOOL res = PlaySoundA(wavPath.c_str(), NULL, flags);
        return (res == TRUE);
#else
        std::cout << "[Audio] Playing music: " << wavPath << " (loop=" << (loop ? "true" : "false") << ")" << std::endl;
        return true;
#endif
    }

    static void stopMusic() {
#ifdef _WIN32
        PlaySoundA(NULL, NULL, 0);
#else
        std::cout << "[Audio] Stopping music playback." << std::endl;
#endif
        s_currentMusicPath.clear();
    }
};

} // namespace zenith

#endif // ZENITH_AUDIO_H
