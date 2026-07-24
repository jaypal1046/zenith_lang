#ifndef ZENITH_PHYSICS_H
#define ZENITH_PHYSICS_H

#include "../common/zenith_common.h"
#include <cstdint>
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

struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4() = default;
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    Vec4 operator+(const Vec4& v) const { return Vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
    Vec4 operator-(const Vec4& v) const { return Vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
    Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
    Vec4 operator/(float s) const { return Vec4(x / s, y / s, z / s, w / s); }

    Vec4& operator+=(const Vec4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    Vec4& operator-=(const Vec4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    Vec4& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    Vec4& operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    float length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    float lengthSquared() const { return x * x + y * y + z * z + w * w; }
    Vec4 normalized() const {
        float len = length();
        return len > 0.0001f ? Vec4(x / len, y / len, z / len, w / len) : Vec4(0, 0, 0, 0);
    }

    static float dot(const Vec4& a, const Vec4& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }
};

struct Mat4 {
    float m00 = 1.0f; float m01 = 0.0f; float m02 = 0.0f; float m03 = 0.0f;
    float m10 = 0.0f; float m11 = 1.0f; float m12 = 0.0f; float m13 = 0.0f;
    float m20 = 0.0f; float m21 = 0.0f; float m22 = 1.0f; float m23 = 0.0f;
    float m30 = 0.0f; float m31 = 0.0f; float m32 = 0.0f; float m33 = 1.0f;

    Mat4() = default;
    Mat4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33)
        : m00(m00), m01(m01), m02(m02), m03(m03),
          m10(m10), m11(m11), m12(m12), m13(m13),
          m20(m20), m21(m21), m22(m22), m23(m23),
          m30(m30), m31(m31), m32(m32), m33(m33) {}

    static Mat4 identity() { return Mat4(); }

    Mat4 operator+(const Mat4& other) const {
        return Mat4(
            m00 + other.m00, m01 + other.m01, m02 + other.m02, m03 + other.m03,
            m10 + other.m10, m11 + other.m11, m12 + other.m12, m13 + other.m13,
            m20 + other.m20, m21 + other.m21, m22 + other.m22, m23 + other.m23,
            m30 + other.m30, m31 + other.m31, m32 + other.m32, m33 + other.m33
        );
    }

    Mat4 operator-(const Mat4& other) const {
        return Mat4(
            m00 - other.m00, m01 - other.m01, m02 - other.m02, m03 - other.m03,
            m10 - other.m10, m11 - other.m11, m12 - other.m12, m13 - other.m13,
            m20 - other.m20, m21 - other.m21, m22 - other.m22, m23 - other.m23,
            m30 - other.m30, m31 - other.m31, m32 - other.m32, m33 - other.m33
        );
    }

    Mat4 operator*(float scalar) const {
        return Mat4(
            m00 * scalar, m01 * scalar, m02 * scalar, m03 * scalar,
            m10 * scalar, m11 * scalar, m12 * scalar, m13 * scalar,
            m20 * scalar, m21 * scalar, m22 * scalar, m23 * scalar,
            m30 * scalar, m31 * scalar, m32 * scalar, m33 * scalar
        );
    }

    Mat4 operator/(float scalar) const {
        return Mat4(
            m00 / scalar, m01 / scalar, m02 / scalar, m03 / scalar,
            m10 / scalar, m11 / scalar, m12 / scalar, m13 / scalar,
            m20 / scalar, m21 / scalar, m22 / scalar, m23 / scalar,
            m30 / scalar, m31 / scalar, m32 / scalar, m33 / scalar
        );
    }

    Mat4 operator*(const Mat4& other) const {
        return Mat4(
            m00 * other.m00 + m01 * other.m10 + m02 * other.m20 + m03 * other.m30,
            m00 * other.m01 + m01 * other.m11 + m02 * other.m21 + m03 * other.m31,
            m00 * other.m02 + m01 * other.m12 + m02 * other.m22 + m03 * other.m32,
            m00 * other.m03 + m01 * other.m13 + m02 * other.m23 + m03 * other.m33,

            m10 * other.m00 + m11 * other.m10 + m12 * other.m20 + m13 * other.m30,
            m10 * other.m01 + m11 * other.m11 + m12 * other.m21 + m13 * other.m31,
            m10 * other.m02 + m11 * other.m12 + m12 * other.m22 + m13 * other.m32,
            m10 * other.m03 + m11 * other.m13 + m12 * other.m23 + m13 * other.m33,

            m20 * other.m00 + m21 * other.m10 + m22 * other.m20 + m23 * other.m30,
            m20 * other.m01 + m21 * other.m11 + m22 * other.m21 + m23 * other.m31,
            m20 * other.m02 + m21 * other.m12 + m22 * other.m22 + m23 * other.m32,
            m20 * other.m03 + m21 * other.m13 + m22 * other.m23 + m23 * other.m33,

            m30 * other.m00 + m31 * other.m10 + m32 * other.m20 + m33 * other.m30,
            m30 * other.m01 + m31 * other.m11 + m32 * other.m21 + m33 * other.m31,
            m30 * other.m02 + m31 * other.m12 + m32 * other.m22 + m33 * other.m32,
            m30 * other.m03 + m31 * other.m13 + m32 * other.m23 + m33 * other.m33
        );
    }

    Vec4 operator*(const Vec4& vec) const {
        return Vec4(
            m00 * vec.x + m01 * vec.y + m02 * vec.z + m03 * vec.w,
            m10 * vec.x + m11 * vec.y + m12 * vec.z + m13 * vec.w,
            m20 * vec.x + m21 * vec.y + m22 * vec.z + m23 * vec.w,
            m30 * vec.x + m31 * vec.y + m32 * vec.z + m33 * vec.w
        );
    }

    Mat4& operator+=(const Mat4& other) { *this = *this + other; return *this; }
    Mat4& operator-=(const Mat4& other) { *this = *this - other; return *this; }
    Mat4& operator*=(float scalar) { *this = *this * scalar; return *this; }
    Mat4& operator/=(float scalar) { *this = *this / scalar; return *this; }
    Mat4& operator*=(const Mat4& other) { *this = *this * other; return *this; }
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
    bool isGrounded = false;

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

struct CapsuleCollider2D {
    Vec2 offset;
    float height = 2.0f;
    float radius = 0.5f;
    bool isTrigger = false;
    bool enabled = true;
};

struct BoxCollider3D {
    Vec3 offset;
    Vec3 size = Vec3(1.0f, 1.0f, 1.0f);
    bool isTrigger = false;
    bool enabled = true;
};

struct SphereCollider3D {
    Vec3 offset;
    float radius = 0.5f;
    bool isTrigger = false;
    bool enabled = true;
};

struct CapsuleCollider3D {
    Vec3 offset;
    float height = 2.0f;
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

inline Vec2 centerOf(const CapsuleCollider2D& collider, const Vec2& position) {
    return position + collider.offset;
}

inline float capsuleHeight(const CapsuleCollider2D& collider) {
    const float radius = collider.radius <= 0.0f ? 0.01f : collider.radius;
    return std::max(collider.height, radius * 2.0f);
}

inline float capsuleHalfSegment(const CapsuleCollider2D& collider) {
    const float radius = collider.radius <= 0.0f ? 0.01f : collider.radius;
    return std::max(0.0f, (capsuleHeight(collider) * 0.5f) - radius);
}

inline BoxCollider2D middleOf(const CapsuleCollider2D& collider) {
    BoxCollider2D middle;
    middle.offset = collider.offset;
    middle.size = Vec2(
        std::max(collider.radius * 2.0f, 0.02f),
        std::max(capsuleHalfSegment(collider) * 2.0f, 0.0f));
    middle.isTrigger = collider.isTrigger;
    middle.enabled = collider.enabled;
    return middle;
}

inline CircleCollider2D topOf(const CapsuleCollider2D& collider) {
    CircleCollider2D cap;
    cap.offset = collider.offset + Vec2(0.0f, -capsuleHalfSegment(collider));
    cap.radius = collider.radius <= 0.0f ? 0.01f : collider.radius;
    cap.isTrigger = collider.isTrigger;
    cap.enabled = collider.enabled;
    return cap;
}

inline CircleCollider2D bottomOf(const CapsuleCollider2D& collider) {
    CircleCollider2D cap;
    cap.offset = collider.offset + Vec2(0.0f, capsuleHalfSegment(collider));
    cap.radius = collider.radius <= 0.0f ? 0.01f : collider.radius;
    cap.isTrigger = collider.isTrigger;
    cap.enabled = collider.enabled;
    return cap;
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

inline bool overlaps(const CapsuleCollider2D& capsule, const Vec2& capsulePosition, const BoxCollider2D& box, const Vec2& boxPosition) {
    if (!capsule.enabled || !box.enabled) {
        return false;
    }

    const BoxCollider2D middle = middleOf(capsule);
    if (middle.size.y > 0.000001f && overlaps(middle, capsulePosition, box, boxPosition)) {
        return true;
    }
    return overlaps(topOf(capsule), capsulePosition, box, boxPosition) ||
           overlaps(bottomOf(capsule), capsulePosition, box, boxPosition);
}

inline bool overlaps(const BoxCollider2D& box, const Vec2& boxPosition, const CapsuleCollider2D& capsule, const Vec2& capsulePosition) {
    return overlaps(capsule, capsulePosition, box, boxPosition);
}

inline bool overlaps(const CapsuleCollider2D& capsule, const Vec2& capsulePosition, const CircleCollider2D& circle, const Vec2& circlePosition) {
    if (!capsule.enabled || !circle.enabled) {
        return false;
    }

    const BoxCollider2D middle = middleOf(capsule);
    if (middle.size.y > 0.000001f && overlaps(middle, capsulePosition, circle, circlePosition)) {
        return true;
    }
    return overlaps(topOf(capsule), capsulePosition, circle, circlePosition) ||
           overlaps(bottomOf(capsule), capsulePosition, circle, circlePosition);
}

inline bool overlaps(const CircleCollider2D& circle, const Vec2& circlePosition, const CapsuleCollider2D& capsule, const Vec2& capsulePosition) {
    return overlaps(capsule, capsulePosition, circle, circlePosition);
}

inline bool overlaps(const CapsuleCollider2D& lhs, const Vec2& lhsPosition, const CapsuleCollider2D& rhs, const Vec2& rhsPosition) {
    if (!lhs.enabled || !rhs.enabled) {
        return false;
    }

    const BoxCollider2D lhsMiddle = middleOf(lhs);
    const BoxCollider2D rhsMiddle = middleOf(rhs);
    const CircleCollider2D lhsTop = topOf(lhs);
    const CircleCollider2D lhsBottom = bottomOf(lhs);
    const CircleCollider2D rhsTop = topOf(rhs);
    const CircleCollider2D rhsBottom = bottomOf(rhs);

    if (lhsMiddle.size.y > 0.000001f) {
        if (rhsMiddle.size.y > 0.000001f && overlaps(lhsMiddle, lhsPosition, rhsMiddle, rhsPosition)) {
            return true;
        }
        if (overlaps(lhsMiddle, lhsPosition, rhsTop, rhsPosition) ||
            overlaps(lhsMiddle, lhsPosition, rhsBottom, rhsPosition)) {
            return true;
        }
    }

    if (rhsMiddle.size.y > 0.000001f) {
        if (overlaps(rhsMiddle, rhsPosition, lhsTop, lhsPosition) ||
            overlaps(rhsMiddle, rhsPosition, lhsBottom, lhsPosition)) {
            return true;
        }
    }

    return overlaps(lhsTop, lhsPosition, rhsTop, rhsPosition) ||
           overlaps(lhsTop, lhsPosition, rhsBottom, rhsPosition) ||
           overlaps(lhsBottom, lhsPosition, rhsTop, rhsPosition) ||
           overlaps(lhsBottom, lhsPosition, rhsBottom, rhsPosition);
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

inline bool containsPoint(const CapsuleCollider2D& collider, const Vec2& position, const Vec2& point) {
    if (!collider.enabled) {
        return false;
    }

    const BoxCollider2D middle = middleOf(collider);
    if (middle.size.y > 0.000001f && containsPoint(middle, position, point)) {
        return true;
    }
    return containsPoint(topOf(collider), position, point) ||
           containsPoint(bottomOf(collider), position, point);
}

inline bool raycast(
    const Ray2D& ray,
    const CircleCollider2D& collider,
    const Vec2& position,
    float maxDistance,
    float& outDistance,
    Vec2& outPoint,
    Vec2& outNormal
) {
    if (!collider.enabled || maxDistance < 0.0f) {
        return false;
    }

    Vec2 direction = ray.direction.normalized();
    if (direction.lengthSquared() <= 0.0000001f) {
        return false;
    }

    const Vec2 center = centerOf(collider, position);
    const Vec2 m = ray.origin - center;
    const float b = Vec2::dot(m, direction);
    const float c = Vec2::dot(m, m) - (collider.radius * collider.radius);

    if (c > 0.0f && b > 0.0f) {
        return false;
    }

    const float discr = b * b - c;
    if (discr < 0.0f) {
        return false;
    }

    float t = -b - std::sqrt(discr);
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > maxDistance) {
        return false;
    }

    outDistance = t;
    outPoint = ray.origin + direction * t;
    outNormal = (outPoint - center).normalized();
    if (outNormal.lengthSquared() <= 0.0000001f) {
        outNormal = direction * -1.0f;
    }
    return true;
}

inline bool raycast(
    const Ray2D& ray,
    const BoxCollider2D& collider,
    const Vec2& position,
    float maxDistance,
    float& outDistance,
    Vec2& outPoint,
    Vec2& outNormal
) {
    if (!collider.enabled || maxDistance < 0.0f) {
        return false;
    }

    Vec2 direction = ray.direction.normalized();
    if (direction.lengthSquared() <= 0.0000001f) {
        return false;
    }

    const AABB2D bounds = boundsOf(collider, position);
    float tMin = 0.0f;
    float tMax = maxDistance;
    Vec2 hitNormal;

    const auto updateAxis = [&](float origin, float dir, float minBound, float maxBound, const Vec2& negativeNormal, const Vec2& positiveNormal) -> bool {
        if (std::fabs(dir) <= 0.000001f) {
            return origin >= minBound && origin <= maxBound;
        }

        float invDir = 1.0f / dir;
        float t1 = (minBound - origin) * invDir;
        float t2 = (maxBound - origin) * invDir;
        Vec2 axisNormal = negativeNormal;
        if (t1 > t2) {
            std::swap(t1, t2);
            axisNormal = positiveNormal;
        }

        if (t1 > tMin) {
            tMin = t1;
            hitNormal = axisNormal;
        }
        if (t2 < tMax) {
            tMax = t2;
        }
        return tMin <= tMax;
    };

    if (!updateAxis(ray.origin.x, direction.x, bounds.minBound.x, bounds.maxBound.x, Vec2(-1.0f, 0.0f), Vec2(1.0f, 0.0f)) ||
        !updateAxis(ray.origin.y, direction.y, bounds.minBound.y, bounds.maxBound.y, Vec2(0.0f, -1.0f), Vec2(0.0f, 1.0f))) {
        return false;
    }

    float t = tMin;
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > maxDistance) {
        return false;
    }

    outDistance = t;
    outPoint = ray.origin + direction * t;
    outNormal = hitNormal.lengthSquared() <= 0.0000001f ? direction * -1.0f : hitNormal;
    return true;
}

inline bool raycast(
    const Ray2D& ray,
    const CapsuleCollider2D& collider,
    const Vec2& position,
    float maxDistance,
    float& outDistance,
    Vec2& outPoint,
    Vec2& outNormal
) {
    if (!collider.enabled || maxDistance < 0.0f) {
        return false;
    }

    bool hit = false;
    float bestDistance = maxDistance;
    const auto consider = [&](const auto& partCollider) {
        float distance = 0.0f;
        Vec2 point;
        Vec2 normal;
        if (raycast(ray, partCollider, position, bestDistance, distance, point, normal)) {
            hit = true;
            bestDistance = distance;
            outDistance = distance;
            outPoint = point;
            outNormal = normal;
        }
    };

    const BoxCollider2D middle = middleOf(collider);
    if (middle.size.y > 0.000001f) {
        consider(middle);
    }
    consider(topOf(collider));
    consider(bottomOf(collider));
    return hit;
}

inline AABB3D boundsOf(const BoxCollider3D& collider, const Vec3& position) {
    const Vec3 center = position + collider.offset;
    const Vec3 halfExtent = collider.size * 0.5f;
    return AABB3D{center - halfExtent, center + halfExtent};
}

inline Vec3 centerOf(const SphereCollider3D& collider, const Vec3& position) {
    return position + collider.offset;
}

inline Vec3 centerOf(const CapsuleCollider3D& collider, const Vec3& position) {
    return position + collider.offset;
}

inline AABB3D boundsOf(const CapsuleCollider3D& collider, const Vec3& position) {
    const Vec3 center = position + collider.offset;
    const float halfH = std::max(collider.height * 0.5f, collider.radius);
    const Vec3 halfExtent(collider.radius, halfH, collider.radius);
    return AABB3D{center - halfExtent, center + halfExtent};
}

inline bool overlaps(const BoxCollider3D& lhs, const Vec3& lhsPosition, const BoxCollider3D& rhs, const Vec3& rhsPosition) {
    if (!lhs.enabled || !rhs.enabled) {
        return false;
    }
    return boundsOf(lhs, lhsPosition).intersects(boundsOf(rhs, rhsPosition));
}

inline bool overlaps(const SphereCollider3D& lhs, const Vec3& lhsPosition, const SphereCollider3D& rhs, const Vec3& rhsPosition) {
    if (!lhs.enabled || !rhs.enabled) {
        return false;
    }
    const Vec3 delta = centerOf(lhs, lhsPosition) - centerOf(rhs, rhsPosition);
    const float radius = lhs.radius + rhs.radius;
    return delta.lengthSquared() <= radius * radius;
}

inline bool overlaps(const BoxCollider3D& box, const Vec3& boxPosition, const SphereCollider3D& sphere, const Vec3& spherePosition) {
    if (!box.enabled || !sphere.enabled) {
        return false;
    }

    const AABB3D bounds = boundsOf(box, boxPosition);
    const Vec3 center = centerOf(sphere, spherePosition);
    const float closestX = std::clamp(center.x, bounds.minBound.x, bounds.maxBound.x);
    const float closestY = std::clamp(center.y, bounds.minBound.y, bounds.maxBound.y);
    const float closestZ = std::clamp(center.z, bounds.minBound.z, bounds.maxBound.z);
    const Vec3 delta = center - Vec3(closestX, closestY, closestZ);
    return delta.lengthSquared() <= sphere.radius * sphere.radius;
}

inline bool overlaps(const SphereCollider3D& sphere, const Vec3& spherePosition, const BoxCollider3D& box, const Vec3& boxPosition) {
    return overlaps(box, boxPosition, sphere, spherePosition);
}

inline bool containsPoint(const BoxCollider3D& collider, const Vec3& position, const Vec3& point) {
    if (!collider.enabled) {
        return false;
    }

    const AABB3D bounds = boundsOf(collider, position);
    return point.x >= bounds.minBound.x && point.x <= bounds.maxBound.x &&
           point.y >= bounds.minBound.y && point.y <= bounds.maxBound.y &&
           point.z >= bounds.minBound.z && point.z <= bounds.maxBound.z;
}

inline bool containsPoint(const SphereCollider3D& collider, const Vec3& position, const Vec3& point) {
    if (!collider.enabled) {
        return false;
    }

    const Vec3 delta = point - centerOf(collider, position);
    return delta.lengthSquared() <= collider.radius * collider.radius;
}

inline bool raycast(
    const Ray3D& ray,
    const SphereCollider3D& collider,
    const Vec3& position,
    float maxDistance,
    float& outDistance,
    Vec3& outPoint,
    Vec3& outNormal
) {
    if (!collider.enabled || maxDistance < 0.0f) {
        return false;
    }

    Vec3 direction = ray.direction.normalized();
    if (direction.lengthSquared() <= 0.0000001f) {
        return false;
    }

    const Vec3 center = centerOf(collider, position);
    const Vec3 m = ray.origin - center;
    const float b = Vec3::dot(m, direction);
    const float c = Vec3::dot(m, m) - (collider.radius * collider.radius);

    if (c > 0.0f && b > 0.0f) {
        return false;
    }

    const float discr = b * b - c;
    if (discr < 0.0f) {
        return false;
    }

    float t = -b - std::sqrt(discr);
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > maxDistance) {
        return false;
    }

    outDistance = t;
    outPoint = ray.origin + direction * t;
    outNormal = (outPoint - center).normalized();
    if (outNormal.lengthSquared() <= 0.0000001f) {
        outNormal = direction * -1.0f;
    }
    return true;
}

inline bool raycast(
    const Ray3D& ray,
    const BoxCollider3D& collider,
    const Vec3& position,
    float maxDistance,
    float& outDistance,
    Vec3& outPoint,
    Vec3& outNormal
) {
    if (!collider.enabled || maxDistance < 0.0f) {
        return false;
    }

    Vec3 direction = ray.direction.normalized();
    if (direction.lengthSquared() <= 0.0000001f) {
        return false;
    }

    const AABB3D bounds = boundsOf(collider, position);
    float tMin = 0.0f;
    float tMax = maxDistance;
    Vec3 hitNormal;

    const auto updateAxis = [&](float origin, float dir, float minBound, float maxBound, const Vec3& negativeNormal, const Vec3& positiveNormal) -> bool {
        if (std::fabs(dir) <= 0.000001f) {
            return origin >= minBound && origin <= maxBound;
        }

        float invDir = 1.0f / dir;
        float t1 = (minBound - origin) * invDir;
        float t2 = (maxBound - origin) * invDir;
        Vec3 axisNormal = negativeNormal;
        if (t1 > t2) {
            std::swap(t1, t2);
            axisNormal = positiveNormal;
        }

        if (t1 > tMin) {
            tMin = t1;
            hitNormal = axisNormal;
        }
        if (t2 < tMax) {
            tMax = t2;
        }
        return tMin <= tMax;
    };

    if (!updateAxis(ray.origin.x, direction.x, bounds.minBound.x, bounds.maxBound.x, Vec3(-1.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)) ||
        !updateAxis(ray.origin.y, direction.y, bounds.minBound.y, bounds.maxBound.y, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f)) ||
        !updateAxis(ray.origin.z, direction.z, bounds.minBound.z, bounds.maxBound.z, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 0.0f, 1.0f))) {
        return false;
    }

    float t = tMin;
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > maxDistance) {
        return false;
    }

    outDistance = t;
    outPoint = ray.origin + direction * t;
    outNormal = hitNormal.lengthSquared() <= 0.0000001f ? direction * -1.0f : hitNormal;
    return true;
}

inline bool raycast(
    const Ray3D& ray,
    const CapsuleCollider3D& collider,
    const Vec3& position,
    float maxDistance,
    float& outDistance,
    Vec3& outPoint,
    Vec3& outNormal
) {
    if (!collider.enabled || maxDistance < 0.0f) return false;
    SphereCollider3D topSphere{collider.offset + Vec3(0.0f, std::max(0.0f, collider.height * 0.5f - collider.radius), 0.0f), collider.radius, collider.isTrigger, collider.enabled};
    SphereCollider3D botSphere{collider.offset - Vec3(0.0f, std::max(0.0f, collider.height * 0.5f - collider.radius), 0.0f), collider.radius, collider.isTrigger, collider.enabled};
    BoxCollider3D midCylinder{collider.offset, Vec3(collider.radius * 2.0f, std::max(0.01f, collider.height - collider.radius * 2.0f), collider.radius * 2.0f), collider.isTrigger, collider.enabled};

    float bestDist = maxDistance;
    bool hit = false;
    float dist = 0.0f; Vec3 pt, norm;

    if (raycast(ray, topSphere, position, bestDist, dist, pt, norm)) { hit = true; bestDist = dist; outDistance = dist; outPoint = pt; outNormal = norm; }
    if (raycast(ray, botSphere, position, bestDist, dist, pt, norm)) { hit = true; bestDist = dist; outDistance = dist; outPoint = pt; outNormal = norm; }
    if (raycast(ray, midCylinder, position, bestDist, dist, pt, norm)) { hit = true; bestDist = dist; outDistance = dist; outPoint = pt; outNormal = norm; }
    return hit;
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
