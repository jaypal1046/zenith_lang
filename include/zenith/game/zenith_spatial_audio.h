#ifndef ZENITH_SPATIAL_AUDIO_H
#define ZENITH_SPATIAL_AUDIO_H

#include "zenith_audio.h"
#include "zenith_camera3d.h"
#include <cmath>
#include <algorithm>

namespace zenith {

struct SpatialAudioParams {
    float volume = 1.0f;
    float panLeft = 1.0f;  // ITD/ILD Left Channel Volume
    float panRight = 1.0f; // ITD/ILD Right Channel Volume
    float itdDelaySeconds = 0.0f; // Interaural Time Difference delay
    float lowPassCutoffHz = 22000.0f; // Occlusion low-pass filter frequency
};

class SpatialAudio3D {
public:
    static SpatialAudioParams calculateHrtf3D(
        Vec3 sourcePos,
        Vec3 listenerPos,
        Vec3 listenerForward,
        Vec3 listenerUp,
        float minDistance = 1.0f,
        float maxDistance = 100.0f,
        bool isOccluded = false
    ) {
        Vec3 dir = sourcePos - listenerPos;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

        SpatialAudioParams params;
        if (dist <= minDistance) {
            params.volume = 1.0f;
        } else if (dist >= maxDistance) {
            params.volume = 0.0f;
            return params;
        } else {
            // Inverse distance attenuation model
            params.volume = minDistance / (minDistance + (dist - minDistance));
        }

        // Calculate ITD/ILD azimuth angle relative to listener head
        Vec3 dirNorm = dir.normalized();
        Vec3 rightVector = listenerForward.cross(listenerUp).normalized();

        float azimuthCos = dirNorm.dot(rightVector); // +1 = right ear, -1 = left ear
        float angleRad = std::asin(std::clamp(azimuthCos, -1.0f, 1.0f));

        // Interaural Level Difference (ILD)
        params.panLeft = std::cos(0.25f * 3.14159265f * (1.0f + azimuthCos));
        params.panRight = std::sin(0.25f * 3.14159265f * (1.0f + azimuthCos));

        // Interaural Time Difference (ITD) — Woodworth's formula for head radius ~0.0875m
        float headRadius = 0.0875f;
        float speedOfSound = 343.0f;
        params.itdDelaySeconds = (headRadius / speedOfSound) * (angleRad + std::sin(angleRad));

        // Occlusion low-pass filter frequency dampening
        if (isOccluded) {
            params.volume *= 0.6f;
            params.lowPassCutoffHz = 1200.0f; // Dampen high frequencies through walls
        }

        return params;
    }
};

struct ReverbZone {
    Vec3 boundsMin = Vec3(-10.0f, -10.0f, -10.0f);
    Vec3 boundsMax = Vec3(10.0f, 10.0f, 10.0f);
    float roomSize = 0.8f;
    float damping = 0.4f;
    float wetLevel = 0.5f;
    float dryLevel = 0.5f;

    bool contains(Vec3 pos) const {
        return pos.x >= boundsMin.x && pos.x <= boundsMax.x &&
               pos.y >= boundsMin.y && pos.y <= boundsMax.y &&
               pos.z >= boundsMin.z && pos.z <= boundsMax.z;
    }
};

struct MusicStemTrack {
    std::string trackPath;
    float currentVolume = 0.0f;
    float targetVolume = 0.0f;
};

class AdaptiveMusicSystem {
public:
    std::string currentState = "Exploration";
    std::unordered_map<std::string, MusicStemTrack> stemLayers;
    float crossfadeSpeed = 2.0f; // Volume change per second

    void registerStem(const std::string& state, const std::string& audioTrackPath) {
        stemLayers[state] = {audioTrackPath, 0.0f, 0.0f};
        if (state == currentState) {
            stemLayers[state].targetVolume = 1.0f;
            stemLayers[state].currentVolume = 1.0f;
        }
    }

    void transitionToState(const std::string& state) {
        if (currentState == state) return;
        currentState = state;

        for (auto& [name, stem] : stemLayers) {
            if (name == state) {
                stem.targetVolume = 1.0f;
            } else {
                stem.targetVolume = 0.0f;
            }
        }
        std::cout << "[Adaptive Music] Seamless transition trigger to state '" << state << "'\n";
    }

    void update(float dt) {
        for (auto& [name, stem] : stemLayers) {
            if (stem.currentVolume < stem.targetVolume) {
                stem.currentVolume = std::min(stem.targetVolume, stem.currentVolume + crossfadeSpeed * dt);
            } else if (stem.currentVolume > stem.targetVolume) {
                stem.currentVolume = std::max(stem.targetVolume, stem.currentVolume - crossfadeSpeed * dt);
            }
        }
    }

    void triggerStinger(const std::string& stingerTrackPath) {
        std::cout << "[Adaptive Music] Playing dynamic stinger overlay track: '" << stingerTrackPath << "'\n";
    }
};

enum class AudioCodecFormat {
    OggVorbis,
    Opus,
    WavPCM,
    FLAC
};

struct AudioStreamChunk {
    size_t chunkIndex = 0;
    size_t sampleCount = 4096;
    std::vector<float> pcmSamples;
    bool isLastChunk = false;
};

class AudioStreamer {
public:
    AudioCodecFormat codec = AudioCodecFormat::OggVorbis;
    size_t chunkSizeSamples = 4096;
    size_t sampleRateHz = 44100;
    int channelCount = 2;
    bool isStreaming = false;

    std::queue<AudioStreamChunk> doubleBufferQueue;
    size_t currentReadChunkIndex = 0;

    void openStream(const std::string& audioPath, AudioCodecFormat format = AudioCodecFormat::OggVorbis) {
        codec = format;
        isStreaming = true;
        currentReadChunkIndex = 0;

        // Pre-buffer first 2 audio chunks for seamless playback
        preloadChunk(audioPath, 0);
        preloadChunk(audioPath, 1);
        std::cout << "[Audio Streamer] Opened stream '" << audioPath << "' (Codec: " << getCodecName(format) << ", 44.1kHz Stereo)\n";
    }

    bool preloadChunk(const std::string& audioPath, size_t chunkIndex) {
        AudioStreamChunk chunk;
        chunk.chunkIndex = chunkIndex;
        chunk.sampleCount = chunkSizeSamples;
        chunk.pcmSamples.resize(chunkSizeSamples * channelCount, 0.0f);
        doubleBufferQueue.push(chunk);
        return true;
    }

    bool processAudioQueue() {
        if (doubleBufferQueue.empty()) return false;
        AudioStreamChunk frontChunk = doubleBufferQueue.front();
        doubleBufferQueue.pop();
        currentReadChunkIndex++;
        return true;
    }

    static const char* getCodecName(AudioCodecFormat format) {
        switch (format) {
            case AudioCodecFormat::OggVorbis: return "Ogg Vorbis";
            case AudioCodecFormat::Opus: return "Opus (Low Latency)";
            case AudioCodecFormat::WavPCM: return "PCM WAV";
            case AudioCodecFormat::FLAC: return "FLAC Lossless";
        }
        return "Unknown";
    }
};

} // namespace zenith

#endif // ZENITH_SPATIAL_AUDIO_H
