#ifndef ZENITH_PARTICLE_FX_H
#define ZENITH_PARTICLE_FX_H

#include "zenith_particles.h"
#include <cmath>
#include <cstdlib>

namespace zenith {

class ParticleFXPreset {
public:
    static void createExplosion(ParticleEmitter2D& emitter, float x, float y, int count = 30, const Color& color = Color::Yellow()) {
        emitter.originX = x;
        emitter.originY = y;
        emitter.emit(count);
    }

    static void createSparkles(ParticleEmitter2D& emitter, float x, float y, int count = 15) {
        emitter.originX = x;
        emitter.originY = y;
        emitter.emit(count);
    }
};

} // namespace zenith

#endif // ZENITH_PARTICLE_FX_H
