#include <cmath>
#include <iostream>
#include "zenith/game/zenith_scene.h"

int main() {
    using namespace zenith::game;

    Scene scene;
    scene.fixedDeltaTime = 1.0f / 60.0f;

    EntityId floor = scene.spawnMesh("Floor", "", "", 0.0f, -1.0f, 0.0f);
    scene.attachBoxCollider3D(floor, 30.0f, 2.0f, 30.0f, false);

    EntityId wall = scene.spawnMesh("Wall", "", "", 1.5f, 1.0f, 0.0f);
    scene.attachBoxCollider3D(wall, 1.0f, 4.0f, 40.0f, false);

    EntityId player = scene.spawnCharacter3D("Player", "", "", "", 0.0f, 4.0f, -1.0f);
    scene.attachSphereCollider3D(player, 0.5f, false);

    Character3DView character = scene.character3D(player);
    Body3DView body = scene.body3D(player);
    character.moveSpeed = 4.0f;
    character.jumpSpeed = 6.5f;
    character.groundAcceleration = 50.0f;
    character.airAcceleration = 20.0f;
    character.airControl = 0.6f;
    character.groundFriction = 30.0f;
    character.groundSnapDistance = 0.25f;
    character.maxSlopeAngle = 60.0f;

    for (int i = 0; i < 240; ++i) {
        scene.simulateFixedStep(scene.fixedDeltaTime);
    }

    const bool groundedAfterFall = static_cast<bool>(character.isGrounded);
    const float landedY = static_cast<float>(character.y);
    std::cout << "grounded_after_fall=" << groundedAfterFall << std::endl;
    std::cout << "landed_y=" << landedY << std::endl;

    scene.setCharacterMove3D(player, 1.0f, 0.0f, 1.0f);
    for (int i = 0; i < 120; ++i) {
        scene.simulateFixedStep(scene.fixedDeltaTime);
    }

    const float blockedX = static_cast<float>(character.x);
    const float slidZ = static_cast<float>(character.z);
    const bool wallBlocked = blockedX <= 0.6f;
    const bool slideAdvanced = slidZ > 1.5f;
    std::cout << "wall_blocked=" << wallBlocked << std::endl;
    std::cout << "slide_advanced=" << slideAdvanced << std::endl;
    std::cout << "blocked_x=" << blockedX << std::endl;
    std::cout << "slid_z=" << slidZ << std::endl;

    scene.setCharacterMove3D(player, 0.0f, 0.0f, 0.0f);
    const float jumpStartY = static_cast<float>(character.y);
    scene.jumpCharacter3D(player);
    scene.simulateFixedStep(scene.fixedDeltaTime);

    const bool airborneAfterJump = !static_cast<bool>(character.isGrounded) && static_cast<float>(body.vy) > 0.0f;
    std::cout << "airborne_after_jump=" << airborneAfterJump << std::endl;
    std::cout << "jump_vy=" << static_cast<float>(body.vy) << std::endl;

    for (int i = 0; i < 240; ++i) {
        scene.simulateFixedStep(scene.fixedDeltaTime);
    }

    const bool relanded = static_cast<bool>(character.isGrounded) && static_cast<bool>(body.isGrounded);
    const float finalY = static_cast<float>(character.y);
    std::cout << "relanded=" << relanded << std::endl;
    std::cout << "final_y=" << finalY << std::endl;

    const bool landedNearFloor = std::fabs(landedY - 0.5f) <= 0.15f;
    const bool finalNearFloor = std::fabs(finalY - 0.5f) <= 0.15f;
    const bool jumpLifted = finalY >= 0.0f && jumpStartY >= 0.0f;

    if (!groundedAfterFall || !landedNearFloor || !wallBlocked || !slideAdvanced || !airborneAfterJump || !relanded || !finalNearFloor || !jumpLifted) {
        return 1;
    }

    return 0;
}
