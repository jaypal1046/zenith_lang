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
#if defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64)
#include <emmintrin.h>
#define ZENITH_HAS_SIMD 1
#endif

struct alignas(16) Vec4 {
    union {
        struct { float x, y, z, w; };
        float data[4];
#if ZENITH_HAS_SIMD
        __m128 simd_v;
#endif
    };

    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    Vec4 operator+(const Vec4& v) const {
#if ZENITH_HAS_SIMD
        Vec4 res;
        res.simd_v = _mm_add_ps(simd_v, v.simd_v);
        return res;
#else
        return Vec4(x + v.x, y + v.y, z + v.z, w + v.w);
#endif
    }
    Vec4 operator-(const Vec4& v) const {
#if ZENITH_HAS_SIMD
        Vec4 res;
        res.simd_v = _mm_sub_ps(simd_v, v.simd_v);
        return res;
#else
        return Vec4(x - v.x, y - v.y, z - v.z, w - v.w);
#endif
    }
    Vec4 operator*(float s) const {
#if ZENITH_HAS_SIMD
        Vec4 res;
        res.simd_v = _mm_mul_ps(simd_v, _mm_set1_ps(s));
        return res;
#else
        return Vec4(x * s, y * s, z * s, w * s);
#endif
    }
    Vec4 operator/(float s) const {
#if ZENITH_HAS_SIMD
        Vec4 res;
        res.simd_v = _mm_div_ps(simd_v, _mm_set1_ps(s));
        return res;
#else
        return Vec4(x / s, y / s, z / s, w / s);
#endif
    }

    Vec4& operator+=(const Vec4& v) { *this = *this + v; return *this; }
    Vec4& operator-=(const Vec4& v) { *this = *this - v; return *this; }
    Vec4& operator*=(float s) { *this = *this * s; return *this; }
    Vec4& operator/=(float s) { *this = *this / s; return *this; }

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

struct alignas(16) Mat4 {
    union {
        struct {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
        float data[16];
#if ZENITH_HAS_SIMD
        __m128 rows[4];
#endif
    };

    Mat4() : m00(1.0f), m01(0.0f), m02(0.0f), m03(0.0f),
             m10(0.0f), m11(1.0f), m12(0.0f), m13(0.0f),
             m20(0.0f), m21(0.0f), m22(1.0f), m23(0.0f),
             m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f) {}

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
#if ZENITH_HAS_SIMD
        Mat4 res;
        res.rows[0] = _mm_add_ps(rows[0], other.rows[0]);
        res.rows[1] = _mm_add_ps(rows[1], other.rows[1]);
        res.rows[2] = _mm_add_ps(rows[2], other.rows[2]);
        res.rows[3] = _mm_add_ps(rows[3], other.rows[3]);
        return res;
#else
        return Mat4(
            m00 + other.m00, m01 + other.m01, m02 + other.m02, m03 + other.m03,
            m10 + other.m10, m11 + other.m11, m12 + other.m12, m13 + other.m13,
            m20 + other.m20, m21 + other.m21, m22 + other.m22, m23 + other.m23,
            m30 + other.m30, m31 + other.m31, m32 + other.m32, m33 + other.m33
        );
#endif
    }

    Mat4 operator-(const Mat4& other) const {
#if ZENITH_HAS_SIMD
        Mat4 res;
        res.rows[0] = _mm_sub_ps(rows[0], other.rows[0]);
        res.rows[1] = _mm_sub_ps(rows[1], other.rows[1]);
        res.rows[2] = _mm_sub_ps(rows[2], other.rows[2]);
        res.rows[3] = _mm_sub_ps(rows[3], other.rows[3]);
        return res;
#else
        return Mat4(
            m00 - other.m00, m01 - other.m01, m02 - other.m02, m03 - other.m03,
            m10 - other.m10, m11 - other.m11, m12 - other.m12, m13 - other.m13,
            m20 - other.m20, m21 - other.m21, m22 - other.m22, m23 - other.m23,
            m30 - other.m30, m31 - other.m31, m32 - other.m32, m33 - other.m33
        );
#endif
    }

    Mat4 operator*(float scalar) const {
#if ZENITH_HAS_SIMD
        Mat4 res;
        __m128 s = _mm_set1_ps(scalar);
        res.rows[0] = _mm_mul_ps(rows[0], s);
        res.rows[1] = _mm_mul_ps(rows[1], s);
        res.rows[2] = _mm_mul_ps(rows[2], s);
        res.rows[3] = _mm_mul_ps(rows[3], s);
        return res;
#else
        return Mat4(
            m00 * scalar, m01 * scalar, m02 * scalar, m03 * scalar,
            m10 * scalar, m11 * scalar, m12 * scalar, m13 * scalar,
            m20 * scalar, m21 * scalar, m22 * scalar, m23 * scalar,
            m30 * scalar, m31 * scalar, m32 * scalar, m33 * scalar
        );
#endif
    }

    Mat4 operator/(float scalar) const {
#if ZENITH_HAS_SIMD
        Mat4 res;
        __m128 s = _mm_set1_ps(scalar);
        res.rows[0] = _mm_div_ps(rows[0], s);
        res.rows[1] = _mm_div_ps(rows[1], s);
        res.rows[2] = _mm_div_ps(rows[2], s);
        res.rows[3] = _mm_div_ps(rows[3], s);
        return res;
#else
        return Mat4(
            m00 / scalar, m01 / scalar, m02 / scalar, m03 / scalar,
            m10 / scalar, m11 / scalar, m12 / scalar, m13 / scalar,
            m20 / scalar, m21 / scalar, m22 / scalar, m23 / scalar,
            m30 / scalar, m31 / scalar, m32 / scalar, m33 / scalar
        );
#endif
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

// ============================================================================
// 4. COLLISION LAYER MATRIX, PHYSICS MATERIALS, TRIGGERS & JOINTS
// ============================================================================

class CollisionLayerMatrix {
public:
    uint32_t matrix[32];

    CollisionLayerMatrix() {
        for (int i = 0; i < 32; ++i) matrix[i] = 0xFFFFFFFF;
    }

    void setCollision(int layerA, int layerB, bool canCollide) {
        if (layerA < 0 || layerA >= 32 || layerB < 0 || layerB >= 32) return;
        if (canCollide) {
            matrix[layerA] |= (1u << layerB);
            matrix[layerB] |= (1u << layerA);
        } else {
            matrix[layerA] &= ~(1u << layerB);
            matrix[layerB] &= ~(1u << layerA);
        }
    }

    bool canCollide(int layerA, int layerB) const {
        if (layerA < 0 || layerA >= 32 || layerB < 0 || layerB >= 32) return true;
        return (matrix[layerA] & (1u << layerB)) != 0;
    }
};

struct PhysicsMaterial {
    float friction = 0.4f;
    float restitution = 0.2f;

    static PhysicsMaterial Bouncy() { return {0.2f, 0.9f}; }
    static PhysicsMaterial Rubber() { return {0.8f, 0.7f}; }
    static PhysicsMaterial Metal() { return {0.3f, 0.1f}; }
    static PhysicsMaterial Ice() { return {0.02f, 0.05f}; }
    static PhysicsMaterial Frictionless() { return {0.0f, 0.0f}; }
};

struct PhysicsCollisionPair {
    uint32_t entityA;
    uint32_t entityB;
    bool isTrigger;
};

class PhysicsEventSystem {
public:
    std::function<void(uint32_t, uint32_t)> onCollisionEnter;
    std::function<void(uint32_t, uint32_t)> onCollisionExit;
    std::function<void(uint32_t, uint32_t)> onTriggerEnter;
    std::function<void(uint32_t, uint32_t)> onTriggerExit;

    void dispatchCollision(uint32_t e1, uint32_t e2, bool enter) {
        if (enter && onCollisionEnter) onCollisionEnter(e1, e2);
        else if (!enter && onCollisionExit) onCollisionExit(e1, e2);
    }

    void dispatchTrigger(uint32_t e1, uint32_t e2, bool enter) {
        if (enter && onTriggerEnter) onTriggerEnter(e1, e2);
        else if (!enter && onTriggerExit) onTriggerExit(e1, e2);
    }
};

enum class JointType { Hinge, Spring, Fixed };

struct PhysicsJoint2D {
    JointType type = JointType::Fixed;
    uint32_t entityA = 0;
    uint32_t entityB = 0;
    Vec2 anchorA;
    Vec2 anchorB;
    float stiffness = 100.0f;
    float damping = 10.0f;
    float restLength = 1.0f;
    bool enabled = true;
};

struct CcdConfig {
    bool enableCCD = true;
    float motionThreshold = 0.5f;
    int maxSubSteps = 4;
};

class CcdSolver {
public:
    static bool sweepCircle(Vec2 startPos, Vec2 endPos, float radius, Vec2 obstaclePos, float obstacleRadius, float& outHitTime, Vec2& outHitNormal) {
        Vec2 motion = endPos - startPos;
        Vec2 rel = startPos - obstaclePos;
        float combinedRadius = radius + obstacleRadius;

        float a = motion.x * motion.x + motion.y * motion.y;
        if (a <= 0.0001f) return false;

        float b = 2.0f * (rel.x * motion.x + rel.y * motion.y);
        float c = (rel.x * rel.x + rel.y * rel.y) - (combinedRadius * combinedRadius);

        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f) return false;

        float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
        if (t >= 0.0f && t <= 1.0f) {
            outHitTime = t;
            Vec2 hitPos = startPos + motion * t;
            outHitNormal = (hitPos - obstaclePos).normalized();
            return true;
        }
        return false;
    }
};

struct RagdollLimb {
    uint32_t entity = 0;
    std::string name;
    Vec2 position;
    Vec2 velocity;
    float rotation = 0.0f;
    float angularVelocity = 0.0f;
    float minAngle = -0.8f;
    float maxAngle = 0.8f;
};

class RagdollSkeleton {
public:
    std::vector<RagdollLimb> limbs;

    void addLimb(const std::string& name, Vec2 pos) {
        limbs.push_back({static_cast<uint32_t>(limbs.size() + 1), name, pos, Vec2(0, 0), 0.0f, 0.0f, -0.8f, 0.8f});
    }

    void simulate(float dt) {
        for (auto& limb : limbs) {
            limb.velocity.y -= 9.81f * 100.0f * dt; // gravity
            limb.position = limb.position + limb.velocity * dt;
            limb.rotation += limb.angularVelocity * dt;

            // Joint angle limits constraint
            limb.rotation = std::clamp(limb.rotation, limb.minAngle, limb.maxAngle);
        }
    }
};

class VehiclePhysics2D {
public:
    float enginePower = 1500.0f;
    float steeringAngle = 0.0f;
    float maxSteerAngle = 0.5f;
    float brakeForce = 1000.0f;
    float lateralFriction = 0.95f; // tire grip / drift resistance

    void update(Vec2& position, float& rotation, Vec2& velocity, float throttle, float steer, float brake, float dt) {
        steeringAngle = steer * maxSteerAngle;

        // Forward vector in vehicle local space
        Vec2 forward(std::cos(rotation), std::sin(rotation));
        Vec2 right(-std::sin(rotation), std::cos(rotation));

        // Engine acceleration along forward axis
        if (throttle != 0.0f) {
            velocity = velocity + forward * (throttle * enginePower * dt);
        }

        // Brake force deceleration
        if (brake > 0.0f) {
            velocity = velocity * (1.0f - std::min(1.0f, brake * brakeForce * dt * 0.001f));
        }

        // Lateral velocity cancellation for realistic tire grip
        float rightVel = velocity.x * right.x + velocity.y * right.y;
        Vec2 lateralVel = right * rightVel;
        velocity = velocity - lateralVel * lateralFriction;

        // Turn rotation based on forward speed and steering angle
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        rotation += steeringAngle * (speed * 0.002f) * dt;

        position = position + velocity * dt;
    }
};

struct CharacterControllerConfig {
    float maxSlopeAngleDegrees = 45.0f;
    float stepOffset = 0.4f;
    bool enableLedgeProtection = true;
    float skinWidth = 0.08f;
    float gravity = -9.81f * 100.0f;
};

class CharacterControllerSystem {
public:
    CharacterControllerConfig config;

    struct MoveResult {
        Vec2 finalPosition;
        bool isGrounded = false;
        bool isSliding = false;
        bool isLedgeAhead = false;
        bool steppedUp = false;
    };

    MoveResult move(Vec2 currentPos, Vec2 wishVelocity, Vec2 groundNormal, float groundY, float aheadGroundY, float dt) {
        MoveResult res;
        res.finalPosition = currentPos;

        // 1. Slope Angle Check
        float slopeAngleRad = std::acos(std::clamp(groundNormal.y, -1.0f, 1.0f));
        float slopeAngleDeg = slopeAngleRad * (180.0f / 3.14159265f);

        if (slopeAngleDeg > config.maxSlopeAngleDegrees) {
            // Slope too steep -> slide down slope
            res.isSliding = true;
            Vec2 slideDir(groundNormal.x, -std::abs(groundNormal.y));
            wishVelocity = wishVelocity + slideDir * 300.0f * dt;
        }

        // 2. Ledge Detection Check
        if (config.enableLedgeProtection) {
            float dropDepth = currentPos.y - aheadGroundY;
            if (dropDepth > config.stepOffset * 2.0f && std::abs(wishVelocity.x) > 0.0f) {
                res.isLedgeAhead = true;
                wishVelocity.x = 0.0f; // Halt horizontal motion at cliff edge
            }
        }

        // 3. Step Offset Check
        float obstacleHeight = groundY - currentPos.y;
        if (obstacleHeight > 0.0f && obstacleHeight <= config.stepOffset) {
            res.steppedUp = true;
            res.finalPosition.y += obstacleHeight; // Auto step-up over stair curb
        }

        // Integrate Position
        res.finalPosition = res.finalPosition + wishVelocity * dt;
        if (res.finalPosition.y <= groundY) {
            res.finalPosition.y = groundY;
            res.isGrounded = true;
        }

        return res;
    }
};

struct NavMeshNode {
    int id = 0;
    Vec2 position;
    std::vector<std::pair<int, float>> neighbors; // neighborId, edgeCost
};

class NavMesh2D {
public:
    std::vector<NavMeshNode> nodes;

    int addNode(Vec2 point) {
        int id = static_cast<int>(nodes.size());
        nodes.push_back({id, point, {}});
        return id;
    }

    void connectBiDirectional(int idA, int idB) {
        if (idA < 0 || idA >= (int)nodes.size() || idB < 0 || idB >= (int)nodes.size()) return;
        float dist = std::sqrt((nodes[idA].position.x - nodes[idB].position.x) * (nodes[idA].position.x - nodes[idB].position.x) +
                               (nodes[idA].position.y - nodes[idB].position.y) * (nodes[idA].position.y - nodes[idB].position.y));
        nodes[idA].neighbors.push_back({idB, dist});
        nodes[idB].neighbors.push_back({idA, dist});
    }

    int findClosestNode(Vec2 point) const {
        if (nodes.empty()) return -1;
        int closestId = 0;
        float minSqDist = 1e9f;
        for (const auto& node : nodes) {
            float dx = node.position.x - point.x;
            float dy = node.position.y - point.y;
            float sqDist = dx * dx + dy * dy;
            if (sqDist < minSqDist) {
                minSqDist = sqDist;
                closestId = node.id;
            }
        }
        return closestId;
    }
};

class PathfinderAStar {
public:
    static std::vector<Vec2> findPath(const NavMesh2D& navMesh, Vec2 startPos, Vec2 goalPos) {
        if (navMesh.nodes.empty()) return {startPos, goalPos};

        int startId = navMesh.findClosestNode(startPos);
        int goalId = navMesh.findClosestNode(goalPos);

        if (startId == goalId) return {startPos, goalPos};

        size_t nodeCount = navMesh.nodes.size();
        std::vector<float> gCost(nodeCount, 1e9f);
        std::vector<int> parent(nodeCount, -1);
        std::vector<bool> closedSet(nodeCount, false);

        auto heuristic = [&](int id) {
            Vec2 p = navMesh.nodes[id].position;
            return std::sqrt((p.x - goalPos.x) * (p.x - goalPos.x) + (p.y - goalPos.y) * (p.y - goalPos.y));
        };

        struct QueueItem {
            int id;
            float fCost;
            bool operator>(const QueueItem& other) const { return fCost > other.fCost; }
        };

        std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> openSet;

        gCost[startId] = 0.0f;
        openSet.push({startId, heuristic(startId)});

        while (!openSet.empty()) {
            int current = openSet.top().id;
            openSet.pop();

            if (current == goalId) break;
            if (closedSet[current]) continue;
            closedSet[current] = true;

            for (const auto& edge : navMesh.nodes[current].neighbors) {
                int neighbor = edge.first;
                float edgeWeight = edge.second;

                if (closedSet[neighbor]) continue;

                float tentativeG = gCost[current] + edgeWeight;
                if (tentativeG < gCost[neighbor]) {
                    gCost[neighbor] = tentativeG;
                    parent[neighbor] = current;
                    openSet.push({neighbor, tentativeG + heuristic(neighbor)});
                }
            }
        }

        // Reconstruct path
        std::vector<Vec2> path;
        int curr = goalId;
        while (curr != -1) {
            path.push_back(navMesh.nodes[curr].position);
            curr = parent[curr];
        }
        std::reverse(path.begin(), path.end());
        if (path.empty()) path = {startPos, goalPos};
        return path;
    }
};

struct GoapAction {
    std::string name;
    float cost = 1.0f;
    std::unordered_map<std::string, bool> preconditions;
    std::unordered_map<std::string, bool> effects;
};

class GoapPlanner {
public:
    static std::vector<GoapAction> plan(
        const std::unordered_map<std::string, bool>& currentState,
        const std::unordered_map<std::string, bool>& goalState,
        const std::vector<GoapAction>& availableActions
    ) {
        // Find action sequence that satisfies goalState from currentState
        std::vector<GoapAction> actionPlan;
        auto state = currentState;

        for (const auto& action : availableActions) {
            bool preconditionsMet = true;
            for (const auto& [key, val] : action.preconditions) {
                if (state.find(key) == state.end() || state[key] != val) {
                    preconditionsMet = false;
                    break;
                }
            }
            if (preconditionsMet) {
                actionPlan.push_back(action);
                for (const auto& [key, val] : action.effects) {
                    state[key] = val;
                }
            }
        }
        return actionPlan;
    }
};

struct Boid2D {
    Vec2 position;
    Vec2 velocity;
};

class FlockingBoids2D {
public:
    float neighborRadius = 50.0f;
    float separationWeight = 1.5f;
    float alignmentWeight = 1.0f;
    float cohesionWeight = 1.0f;

    Vec2 calculateSteering(const std::vector<Boid2D>& boids, size_t index) const {
        if (boids.empty() || index >= boids.size()) return Vec2(0, 0);

        Vec2 separation(0, 0);
        Vec2 alignment(0, 0);
        Vec2 cohesion(0, 0);
        int neighborCount = 0;

        Vec2 myPos = boids[index].position;
        Vec2 myVel = boids[index].velocity;

        for (size_t i = 0; i < boids.size(); ++i) {
            if (i == index) continue;
            float dist = std::sqrt((boids[i].position.x - myPos.x) * (boids[i].position.x - myPos.x) +
                                   (boids[i].position.y - myPos.y) * (boids[i].position.y - myPos.y));
            if (dist < neighborRadius && dist > 0.001f) {
                // 1. Separation
                separation = separation + (myPos - boids[i].position) / (dist * dist);
                // 2. Alignment
                alignment = alignment + boids[i].velocity;
                // 3. Cohesion
                cohesion = cohesion + boids[i].position;
                neighborCount++;
            }
        }

        if (neighborCount > 0) {
            alignment = (alignment / static_cast<float>(neighborCount)).normalized();
            cohesion = ((cohesion / static_cast<float>(neighborCount)) - myPos).normalized();
            separation = separation.normalized();
        }

        return separation * separationWeight + alignment * alignmentWeight + cohesion * cohesionWeight;
    }
};

class PerceptionSensorSystem {
public:
    float sightRadius = 150.0f;
    float sightFovDegrees = 90.0f;
    float hearingRadius = 250.0f;

    bool canSeeTarget(Vec2 sensorPos, float sensorFacingAngle, Vec2 targetPos) const {
        Vec2 dir = targetPos - sensorPos;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > sightRadius) return false;

        float targetAngle = std::atan2(dir.y, dir.x);
        float angleDiff = std::abs(targetAngle - sensorFacingAngle);
        float fovRad = (sightFovDegrees * 0.5f) * (3.14159265f / 180.0f);
        return angleDiff <= fovRad;
    }

    bool canHearTarget(Vec2 sensorPos, Vec2 soundPos, float soundLoudness) const {
        float dist = std::sqrt((soundPos.x - sensorPos.x) * (soundPos.x - sensorPos.x) +
                               (soundPos.y - sensorPos.y) * (soundPos.y - sensorPos.y));
        return dist <= (hearingRadius * soundLoudness);
    }
};

} // namespace physics
} // namespace zenith

#endif // ZENITH_PHYSICS_H
