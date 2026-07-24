#include <iostream>
#include "zenith/game/zenith_world.h"

int main() {
    using namespace zenith::game;

    World world;
    EntityId alpha = world.createEntity("Alpha");
    EntityId beta = world.createEntity("Beta");
    EntityId gamma = world.createEntity("Gamma");

    world.addTransform2D(alpha).position = zenith::physics::Vec2(2.0f, 3.0f);
    world.addTransform2D(beta).position = zenith::physics::Vec2(4.0f, 5.0f);
    world.addTransform2D(gamma).position = zenith::physics::Vec2(6.0f, 7.0f);

    world.addSpriteRenderer2D(alpha).sortOrder = 10;
    world.addSpriteRenderer2D(beta).sortOrder = 20;
    world.addSpriteRenderer2D(gamma).sortOrder = 30;

    world.addRigidBody2D(alpha);
    world.addRigidBody2D(gamma);

    std::cout << "sprites_before=" << world.spriteEntities2D().size() << std::endl;
    std::cout << "bodies_before=" << world.rigidBodyEntities2D().size() << std::endl;

    world.syncPhysicsFromTransforms2D();
    if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(alpha)) {
        std::cout << "alpha_body_x=" << body->position.x << std::endl;
    }

    bool destroyed_beta = world.destroyEntity(beta);
    std::cout << "destroyed_beta=" << destroyed_beta << std::endl;
    std::cout << "sprites_after=" << world.spriteEntities2D().size() << std::endl;
    std::cout << "beta_sprite_alive=" << world.hasSpriteRenderer2D(beta) << std::endl;

    EntityId delta = world.createEntity("Delta");
    world.addTransform2D(delta).position = zenith::physics::Vec2(9.0f, 1.0f);
    world.addSpriteRenderer2D(delta).sortOrder = 40;

    size_t dense_sprite_count = 0;
    world.forEachSpriteRenderer2D([&dense_sprite_count](EntityId, const SpriteRenderer2D&) {
        ++dense_sprite_count;
    });
    std::cout << "dense_sprite_count=" << dense_sprite_count << std::endl;

    size_t dense_body_count = 0;
    world.forEachRigidBody2D([&dense_body_count](EntityId, const zenith::physics::RigidBody2D&) {
        ++dense_body_count;
    });
    std::cout << "dense_body_count=" << dense_body_count << std::endl;

    std::optional<EntityId> found_delta = world.findByName("Delta");
    std::cout << "found_delta=" << found_delta.has_value() << std::endl;

    if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(alpha)) {
        body->position.x = 11.0f;
    }
    world.syncTransformsFromPhysics2D();
    if (const Transform2D* transform = world.getTransform2D(alpha)) {
        std::cout << "alpha_transform_x=" << transform->position.x << std::endl;
    }

    return 0;
}
