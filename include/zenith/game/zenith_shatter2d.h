#ifndef ZENITH_SHATTER2D_H
#define ZENITH_SHATTER2D_H

#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct GlassShard {
    float x, y;
    float vx, vy;
    float rotation;
    float angularVelocity;
    float size;
    uint32_t color;
    bool active;
};

class ShatterSystem2D {
private:
    std::vector<GlassShard> shards;

public:
    void triggerShatter(float impactX, float impactY, int shardCount = 30, uint32_t defaultColor = 0xFFFFFFFF) {
        shards.clear();
        shards.reserve(shardCount);

        for (int i = 0; i < shardCount; ++i) {
            GlassShard shard;
            shard.x = impactX;
            shard.y = impactY;

            float angle = (static_cast<float>(rand()) / RAND_MAX) * 6.28318530718f;
            float speed = 100.0f + (static_cast<float>(rand()) / RAND_MAX) * 300.0f;

            shard.vx = std::cos(angle) * speed;
            shard.vy = std::sin(angle) * speed;
            shard.rotation = 0.0f;
            shard.angularVelocity = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 10.0f;
            shard.size = 5.0f + (static_cast<float>(rand()) / RAND_MAX) * 15.0f;
            shard.color = defaultColor;
            shard.active = true;

            shards.push_back(shard);
        }
    }

    void update(float dt, float gravity = 500.0f) {
        for (auto& shard : shards) {
            if (!shard.active) continue;

            shard.x += shard.vx * dt;
            shard.y += shard.vy * dt;
            shard.vy += gravity * dt;
            shard.rotation += shard.angularVelocity * dt;
        }
    }

    const std::vector<GlassShard>& getShards() const { return shards; }
};

} // namespace zenith

#endif // ZENITH_SHATTER2D_H
