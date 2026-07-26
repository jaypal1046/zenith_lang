#ifndef ZENITH_NOISE_H
#define ZENITH_NOISE_H

#include <cmath>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>

namespace zenith {

class PerlinNoise2D {
private:
    std::vector<int> p;

    float fade(float t) const { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }
    float lerp(float t, float a, float b) const { return a + t * (b - a); }
    float grad(int hash, float x, float y) const {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

public:
    explicit PerlinNoise2D(unsigned int seed = 12345) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);
        p.insert(p.end(), p.begin(), p.end());
    }

    float noise(float x, float y) const {
        int X = static_cast<int>(floorf(x)) & 255;
        int Y = static_cast<int>(floorf(y)) & 255;

        x -= floorf(x);
        y -= floorf(y);

        float u = fade(x);
        float v = fade(y);

        int A = p[X] + Y;
        int B = p[X + 1] + Y;

        return lerp(v, lerp(u, grad(p[A], x, y), grad(p[B], x - 1.0f, y)),
                       lerp(u, grad(p[A + 1], x, y - 1.0f), grad(p[B + 1], x - 1.0f, y - 1.0f)));
    }
};

} // namespace zenith

#endif // ZENITH_NOISE_H
