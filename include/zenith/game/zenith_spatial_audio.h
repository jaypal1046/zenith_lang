#ifndef ZENITH_SPATIAL_AUDIO_H
#define ZENITH_SPATIAL_AUDIO_H

#include "zenith_audio.h"
#include "zenith_camera3d.h"
#include <cmath>
#include <algorithm>

namespace zenith {

struct SpatialAudioParams {
    float volume = 1.0f;
    float pan = 0.0f; // -1.0 (left) to +1.0 (right)
};

class SpatialAudio2D {
public:
    static SpatialAudioParams calculateParams(
        float sourceX, float sourceY,
        float listenerX, float listenerY,
        float minDistance = 50.0f,
        float maxDistance = 1000.0f
    ) {
        float dx = sourceX - listenerX;
        float dy = sourceY - listenerY;
        float distance = sqrtf(dx * dx + dy * dy);

        SpatialAudioParams params;
        if (distance <= minDistance) {
            params.volume = 1.0f;
        } else if (distance >= maxDistance) {
            params.volume = 0.0f;
        } else {
            params.volume = 1.0f - ((distance - minDistance) / (maxDistance - minDistance));
        }

        // Calculate stereo panning based on horizontal offset relative to max distance
        params.pan = dx / (maxDistance * 0.5f);
        if (params.pan < -1.0f) params.pan = -1.0f;
        if (params.pan > 1.0f) params.pan = 1.0f;

        return params;
    }

    static bool play3DSound(const std::string& wavPath, float sourceX, float sourceY, float listenerX, float listenerY) {
        SpatialAudioParams params = calculateParams(sourceX, sourceY, listenerX, listenerY);
        if (params.volume <= 0.001f) return false;
        return AudioManager::playSound(wavPath, params.volume);
    }
};

} // namespace zenith

#endif // ZENITH_SPATIAL_AUDIO_H
