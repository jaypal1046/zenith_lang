#ifndef ZENITH_PHYSICS_H
#define ZENITH_PHYSICS_H

#include "../common/zenith_common.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <iostream>

namespace zenith {
namespace physics {

// ============================================================================
// 1. SIMD-FRIENDLY VECTOR & SPATIAL MATH
// ============================================================================

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(float s) const { return Vec2(x / s, y / s); }

    Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
    Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSquared() const { return x * x + y * y; }
    Vec2 normalized() const {
        float len = length();
        return len > 0.0001f ? Vec2(x / len, y / len) : Vec2(0, 0);
    }

    static float dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
    static float distance(const Vec2& a, const Vec2& b) { return (a - b).length(); }
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }

    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float lengthSquared() const { return x * x + y * y + z * z; }
    Vec3 normalized() const {
        float len = length();
        return len > 0.0001f ? Vec3(x / len, y / len, z / len) : Vec3(0, 0, 0);
    }

    static float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
    static float distance(const Vec3& a, const Vec3& b) { return (a - b).length(); }
};

struct AABB2D {
    Vec2 minBound;
    Vec2 maxBound;

    bool intersects(const AABB2D& other) const {
        return (minBound.x <= other.maxBound.x && maxBound.x >= other.minBound.x) &&
               (minBound.y <= other.maxBound.y && maxBound.y >= other.minBound.y);
    }
};

struct AABB3D {
    Vec3 minBound;
    Vec3 maxBound;

    bool intersects(const AABB3D& other) const {
        return (minBound.x <= other.maxBound.x && maxBound.x >= other.minBound.x) &&
               (minBound.y <= other.maxBound.y && maxBound.y >= other.minBound.y) &&
               (minBound.z <= other.maxBound.z && maxBound.z >= other.minBound.z);
    }
};

// ============================================================================
// 2. RAYCASTING & QUERIES
// ============================================================================

struct Ray2D {
    Vec2 origin;
    Vec2 direction;
};

struct Ray3D {
    Vec3 origin;
    Vec3 direction;
};

struct RaycastHit2D {
    bool hasHit = false;
    float distance = 0.0f;
    Vec2 point;
    Vec2 normal;
    void* entity = nullptr;
};

struct RaycastHit3D {
    bool hasHit = false;
    float distance = 0.0f;
    Vec3 point;
    Vec3 normal;
    void* entity = nullptr;
};

// ============================================================================
// 3. RIGIDBODY & COLLIDER DYNAMICS
// ============================================================================

enum class BodyType { Dynamic, Static, Kinematic };

class RigidBody2D {
public:
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    float mass = 1.0f;
    float gravityScale = 1.0f;
    float friction = 0.2f;
    float restitution = 0.0f; // Bounciness
    BodyType bodyType = BodyType::Dynamic;
    bool isGrounded = false;

    void applyForce(const Vec2& force) {
        if (bodyType != BodyType::Dynamic || mass <= 0.0f) return;
        acceleration += force / mass;
    }

    void applyImpulse(const Vec2& impulse) {
        if (bodyType != BodyType::Dynamic || mass <= 0.0f) return;
        velocity += impulse / mass;
    }

    void update(float dt, float globalGravity = 9.81f) {
        if (bodyType != BodyType::Dynamic) return;

        // Apply gravity
        acceleration.y += globalGravity * gravityScale;

        // Euler Integration
        velocity += acceleration * dt;
        position += velocity * dt;

        // Clear force accumulators
        acceleration = Vec2(0, 0);
    }
};

class RigidBody3D {
public:
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 rotation;
    float mass = 1.0f;
    float gravityScale = 1.0f;
    float friction = 0.2f;
    float restitution = 0.0f;
    BodyType bodyType = BodyType::Dynamic;
    bool useGravity = true;

    void applyForce(const Vec3& force) {
        if (bodyType != BodyType::Dynamic || mass <= 0.0f) return;
        acceleration += force / mass;
    }

    void applyImpulse(const Vec3& impulse) {
        if (bodyType != BodyType::Dynamic || mass <= 0.0f) return;
        velocity += impulse / mass;
    }

    void update(float dt, float globalGravity = -9.81f) {
        if (bodyType != BodyType::Dynamic) return;

        if (useGravity) {
            acceleration.y += globalGravity * gravityScale;
        }

        velocity += acceleration * dt;
        position += velocity * dt;

        acceleration = Vec3(0, 0, 0);
    }
};

struct BoxCollider2D {
    Vec2 offset;
    Vec2 size = Vec2(1.0f, 1.0f);
    bool isTrigger = false;
    bool enabled = true;
};

struct CircleCollider2D {
    Vec2 offset;
    float radius = 0.5f;
    bool isTrigger = false;
    bool enabled = true;
};

inline AABB2D boundsOf(const BoxCollider2D& collider, const Vec2& position) {
    const Vec2 center = position + collider.offset;
    const Vec2 halfExtent = collider.size * 0.5f;
    return AABB2D{center - halfExtent, center + halfExtent};
}

inline Vec2 centerOf(const CircleCollider2D& collider, const Vec2& position) {
    return position + collider.offset;
}

inline bool overlaps(const BoxCollider2D& lhs, const Vec2& lhsPosition, const BoxCollider2D& rhs, const Vec2& rhsPosition) {
    if (!lhs.enabled || !rhs.enabled) {
        return false;
    }
    return boundsOf(lhs, lhsPosition).intersects(boundsOf(rhs, rhsPosition));
}

inline bool overlaps(const CircleCollider2D& lhs, const Vec2& lhsPosition, const CircleCollider2D& rhs, const Vec2& rhsPosition) {
    if (!lhs.enabled || !rhs.enabled) {
        return false;
    }
    const Vec2 delta = centerOf(lhs, lhsPosition) - centerOf(rhs, rhsPosition);
    const float radius = lhs.radius + rhs.radius;
    return delta.lengthSquared() <= radius * radius;
}

inline bool overlaps(const BoxCollider2D& box, const Vec2& boxPosition, const CircleCollider2D& circle, const Vec2& circlePosition) {
    if (!box.enabled || !circle.enabled) {
        return false;
    }

    const AABB2D bounds = boundsOf(box, boxPosition);
    const Vec2 center = centerOf(circle, circlePosition);
    const float closestX = std::clamp(center.x, bounds.minBound.x, bounds.maxBound.x);
    const float closestY = std::clamp(center.y, bounds.minBound.y, bounds.maxBound.y);
    const Vec2 delta = center - Vec2(closestX, closestY);
    return delta.lengthSquared() <= circle.radius * circle.radius;
}

inline bool overlaps(const CircleCollider2D& circle, const Vec2& circlePosition, const BoxCollider2D& box, const Vec2& boxPosition) {
    return overlaps(box, boxPosition, circle, circlePosition);
}

inline bool containsPoint(const BoxCollider2D& collider, const Vec2& position, const Vec2& point) {
    if (!collider.enabled) {
        return false;
    }

    const AABB2D bounds = boundsOf(collider, position);
    return point.x >= bounds.minBound.x && point.x <= bounds.maxBound.x &&
           point.y >= bounds.minBound.y && point.y <= bounds.maxBound.y;
}

inline bool containsPoint(const CircleCollider2D& collider, const Vec2& position, const Vec2& point) {
    if (!collider.enabled) {
        return false;
    }

    const Vec2 delta = point - centerOf(collider, position);
    return delta.lengthSquared() <= collider.radius * collider.radius;
}

// ============================================================================
// 4. SPATIAL PARTITIONING (SPATIAL HASHING & BVH)
// ============================================================================

class SpatialHash2D {
private:
    float cellSize;
    std::unordered_map<int64_t, std::vector<RigidBody2D*>> grid;

    int64_t getHashKey(int x, int y) const {
        return (static_cast<int64_t>(x) * 73856093) ^ (static_cast<int64_t>(y) * 19349663);
    }

public:
    SpatialHash2D(float cellSize = 64.0f) : cellSize(cellSize) {}

    void clear() { grid.clear(); }

    void insert(RigidBody2D* body) {
        int cellX = static_cast<int>(std::floor(body->position.x / cellSize));
        int cellY = static_cast<int>(std::floor(body->position.y / cellSize));
        grid[getHashKey(cellX, cellY)].push_back(body);
    }

    std::vector<RigidBody2D*> getNearby(const Vec2& pos) {
        std::vector<RigidBody2D*> nearby;
        int cellX = static_cast<int>(std::floor(pos.x / cellSize));
        int cellY = static_cast<int>(std::floor(pos.y / cellSize));

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                auto it = grid.find(getHashKey(cellX + dx, cellY + dy));
                if (it != grid.end()) {
                    nearby.insert(nearby.end(), it->second.begin(), it->second.end());
                }
            }
        }
        return nearby;
    }
};

class PhysicsWorld {
public:
    float gravity2D = 9.81f;
    float gravity3D = -9.81f;
    SpatialHash2D spatialHash2D;
    std::vector<RigidBody2D*> bodies2D;
    std::vector<RigidBody3D*> bodies3D;

    void registerBody(RigidBody2D* body) { bodies2D.push_back(body); }
    void registerBody(RigidBody3D* body) { bodies3D.push_back(body); }
    void clearBodies() {
        bodies2D.clear();
        bodies3D.clear();
    }

    void step(float dt) {
        spatialHash2D.clear();

        for (auto* body : bodies2D) {
            body->update(dt, gravity2D);
            spatialHash2D.insert(body);
        }

        for (auto* body : bodies3D) {
            body->update(dt, gravity3D);
        }
    }

    RaycastHit3D raycast3D(const Vec3& origin, const Vec3& direction, float maxDistance) {
        RaycastHit3D bestHit;
        bestHit.distance = maxDistance;
        Vec3 dirNorm = direction.normalized();

        for (auto* body : bodies3D) {
            // Sphere collision test for fast raycast
            Vec3 m = origin - body->position;
            float b = Vec3::dot(m, dirNorm);
            float c = Vec3::dot(m, m) - (1.0f * 1.0f); // default radius 1.0

            if (c > 0.0f && b > 0.0f) continue;
            float discr = b * b - c;
            if (discr < 0.0f) continue;

            float t = -b - std::sqrt(discr);
            if (t >= 0.0f && t < bestHit.distance) {
                bestHit.hasHit = true;
                bestHit.distance = t;
                bestHit.point = origin + dirNorm * t;
                bestHit.normal = (bestHit.point - body->position).normalized();
                bestHit.entity = body;
            }
        }
        return bestHit;
    }
};

} // namespace physics
} // namespace zenith

#endif // ZENITH_PHYSICS_H
