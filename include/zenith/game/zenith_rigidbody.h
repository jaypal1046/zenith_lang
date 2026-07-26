#ifndef ZENITH_RIGIDBODY_H
#define ZENITH_RIGIDBODY_H

#include "zenith_window.h"

namespace zenith {

class RigidBody2D {
public:
    float x = 0.0f;
    float y = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float mass = 1.0f;
    float gravityScale = 1.0f;
    float linearDamping = 0.05f;

    RigidBody2D() = default;

    void applyImpulse(float impulseX, float impulseY) {
        if (mass <= 0.0001f) return;
        velocityX += impulseX / mass;
        velocityY += impulseY / mass;
    }

    void update(float dt, float gravityAcc = 980.0f) {
        if (mass <= 0.0001f) return; // Static body

        velocityY += gravityAcc * gravityScale * dt;
        velocityX *= (1.0f - linearDamping * dt);
        velocityY *= (1.0f - linearDamping * dt);

        x += velocityX * dt;
        y += velocityY * dt;
    }
};

} // namespace zenith

#endif // ZENITH_RIGIDBODY_H
