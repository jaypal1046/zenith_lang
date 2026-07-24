#include "zenith/game/zenith_game.h"

class FollowCameraScene : public zenith::game::Scene {
public:
    zenith::game::EntityId player;
    zenith::game::EntityId camera;
    std::string statusText = "Move with WASD or arrow keys. Space changes color.";

protected:
    void onLoad() override {
        name = "Follow Camera Scene";
        clearColor = "black";
        fixedDeltaTime = 1.0f / 60.0f;
        drawEntityNames = false;
        physics.gravity2D = 0.0f;

        registerPrefab("player", [](zenith::game::Scene& scene, zenith::game::EntityId entity) {
            zenith::game::Transform2D& transform = scene.world.addTransform2D(entity);
            transform.position = zenith::physics::Vec2(0.0f, 0.0f);

            zenith::physics::RigidBody2D& body = scene.world.addRigidBody2D(entity);
            body.gravityScale = 0.0f;

            zenith::game::SpriteRenderer2D& sprite = scene.world.addSpriteRenderer2D(entity);
            sprite.size = zenith::physics::Vec2(5.0f, 3.0f);
            sprite.anchor = zenith::physics::Vec2(0.5f, 0.5f);
            sprite.tintColor = "cyan";
        });

        player = instantiate("player", "player");

        camera = world.createEntity("main_camera");
        zenith::game::Transform2D& cameraTransform = world.addTransform2D(camera);
        cameraTransform.position = zenith::physics::Vec2(0.0f, 0.0f);

        zenith::game::Camera2DComponent& cameraComponent = world.addCamera2D(camera);
        cameraComponent.primary = true;
        cameraComponent.zoom = 1.0f;
    }

    void onFrame(float dt) override {
        if (zenith::wasKeyPressed("Escape")) {
            setPaused(!paused);
            if (paused) {
                statusText = "Paused. Press Escape again to resume.";
            } else {
                statusText = "Resumed. Move with WASD or arrow keys.";
            }
        }
        (void)dt;
    }

    void onFixedUpdate(float dt) override {
        zenith::physics::RigidBody2D* body = world.getRigidBody2D(player);
        zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(player);
        if (body == nullptr || sprite == nullptr) {
            return;
        }

        float moveX = zenith::getAxis("a", "d");
        float moveY = zenith::getAxis("w", "s");

        if (zenith::isKeyPressed("ArrowLeft")) moveX -= 1.0f;
        if (zenith::isKeyPressed("ArrowRight")) moveX += 1.0f;
        if (zenith::isKeyPressed("ArrowUp")) moveY -= 1.0f;
        if (zenith::isKeyPressed("ArrowDown")) moveY += 1.0f;

        body->velocity.x = moveX * 18.0f;
        body->velocity.y = moveY * 10.0f;

        if (zenith::wasKeyPressed("Space")) {
            sprite->tintColor = "yellow";
            statusText = "Space pressed this frame.";
        }
        if (zenith::wasKeyReleased("Space")) {
            sprite->tintColor = "cyan";
            statusText = "Space released this frame.";
        }

        (void)dt;
    }

    void onPostPhysics(float dt) override {
        zenith::followPrimaryCamera2D(world, player, zenith::physics::Vec2(0.0f, 0.0f), 0.18f);
        (void)dt;
    }

    void onDraw(zenith::Canvas& canvas, float alpha) override {
        zenith::game::Transform2D* playerTransform = world.getTransform2D(player);
        canvas.drawText("Zenith Game SDK Sample", 2.0f, 1.0f, "yellow");
        canvas.drawText(statusText, 2.0f, 3.0f, "white");
        if (playerTransform != nullptr) {
            canvas.drawText(
                "Player: " + zenith::toString(playerTransform->position.x) + ", " + zenith::toString(playerTransform->position.y),
                2.0f,
                5.0f,
                "green"
            );
        }
        canvas.drawText("Mouse: " + zenith::toString(zenith::getMouseX()) + ", " + zenith::toString(zenith::getMouseY()), 2.0f, 7.0f, "magenta");
        canvas.drawText("Alpha: " + zenith::toString(alpha), 2.0f, 9.0f, "cyan");
    }
};

int main() {
    FollowCameraScene scene;
    zenith::runScene(scene);
    return 0;
}
