#ifndef ZENITH_STEERING2D_H
#define ZENITH_STEERING2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct Vector2DSteer {
    float x = 0.0f;
    float y = 0.0f;

    float length() const { return std::sqrt(x * x + y * y); }
    void normalize() {
        float len = length();
        if (len > 0.0001f) {
            x /= len;
            y /= len;
        }
    }
};

struct SteeringAgent2D {
    Vector2DSteer position;
    Vector2DSteer velocity;
    float maxSpeed = 200.0f;
    float maxForce = 500.0f;
};

class SteeringBehaviors2D {
public:
    static Vector2DSteer seek(const SteeringAgent2D& agent, Vector2DSteer target) {
        Vector2DSteer desired = {target.x - agent.position.x, target.y - agent.position.y};
        desired.normalize();
        desired.x *= agent.maxSpeed;
        desired.y *= agent.maxSpeed;

        Vector2DSteer steer = {desired.x - agent.velocity.x, desired.y - agent.velocity.y};
        float forceLen = steer.length();
        if (forceLen > agent.maxForce) {
            steer.normalize();
            steer.x *= agent.maxForce;
            steer.y *= agent.maxForce;
        }
        return steer;
    }

    static Vector2DSteer arrive(const SteeringAgent2D& agent, Vector2DSteer target, float slowingRadius = 100.0f) {
        Vector2DSteer desired = {target.x - agent.position.x, target.y - agent.position.y};
        float dist = desired.length();
        desired.normalize();

        if (dist < slowingRadius) {
            desired.x *= agent.maxSpeed * (dist / slowingRadius);
            desired.y *= agent.maxSpeed * (dist / slowingRadius);
        } else {
            desired.x *= agent.maxSpeed;
            desired.y *= agent.maxSpeed;
        }

        Vector2DSteer steer = {desired.x - agent.velocity.x, desired.y - agent.velocity.y};
        return steer;
    }
};

} // namespace zenith

#endif // ZENITH_STEERING2D_H
