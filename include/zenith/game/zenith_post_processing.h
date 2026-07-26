#ifndef ZENITH_POST_PROCESSING_H
#define ZENITH_POST_PROCESSING_H

#include "zenith_window.h"

namespace zenith {

class PostProcessing2D {
private:
    static inline float s_vignetteIntensity = 0.0f;
    static inline float s_bloomThreshold = 0.8f;
    static inline float s_chromaticAberration = 0.0f;

public:
    static void setVignette(float intensity) { s_vignetteIntensity = intensity; }
    static float getVignette() { return s_vignetteIntensity; }

    static void setBloomThreshold(float threshold) { s_bloomThreshold = threshold; }
    static float getBloomThreshold() { return s_bloomThreshold; }

    static void setChromaticAberration(float intensity) { s_chromaticAberration = intensity; }
    static float getChromaticAberration() { return s_chromaticAberration; }

    static void beginPass() {
        // Prepare post-processing offscreen FBO pass
    }

    static void endPass() {
        // Apply post-processing shaders and render quad to backbuffer
    }
};

} // namespace zenith

#endif // ZENITH_POST_PROCESSING_H
