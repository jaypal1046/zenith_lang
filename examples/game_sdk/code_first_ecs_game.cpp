#include "../../include/zenith/game/zenith_game.h"
#include <iostream>
#include <vector>

// Components
struct Position { float x = 0.0f, y = 0.0f; };
struct Velocity { float dx = 0.0f, dy = 0.0f; };
struct PlayerTag { bool isPlayer = true; };
struct EnemyTag { float speed = 100.0f; };
struct Renderable { zenith::Color color; float size = 32.0f; };

int main() {
    std::cout << "Starting Zenith Code-First ECS Game Example..." << std::endl;

    // 1. Initialize Window & Renderer
    zenith::WindowConfig config;
    config.title = "Zenith Code-First ECS Game";
    config.width = 1280;
    config.height = 720;
    config.vsync = true;

    zenith::NativeWindow window;
    if (!window.initialize(config)) {
        std::cerr << "Failed to initialize window!" << std::endl;
        return -1;
    }

    // 2. Initialize ECS & World
    zenith::ecs::World world;

    // Create Player Entity
    auto player = world.createEntity();
    world.addComponent(player, Position{640.0f, 360.0f});
    world.addComponent(player, Velocity{0.0f, 0.0f});
    world.addComponent(player, PlayerTag{true});
    world.addComponent(player, Renderable{zenith::Color::Cyan(), 40.0f});

    // Create Enemy Entities
    for (int i = 0; i < 5; ++i) {
        auto enemy = world.createEntity();
        world.addComponent(enemy, Position{100.0f + i * 150.0f, 100.0f});
        world.addComponent(enemy, Velocity{30.0f, 20.0f});
        world.addComponent(enemy, EnemyTag{50.0f});
        world.addComponent(enemy, Renderable{zenith::Color::Red(), 30.0f});
    }

    // Spatial Hash Grid for fast spatial queries
    zenith::spatial::SpatialHashGrid2D<zenith::ecs::EntityID> spatialGrid(64.0f);

    // Camera & Debug Overlay
    zenith::Camera2D camera{0.0f, 0.0f, 1.0f, 0.0f};
    zenith::DebugOverlay debugOverlay;

    std::cout << "Game Loop Started! Press ESC to Exit." << std::endl;

    // Main Gameplay Loop
    while (!window.shouldClose()) {
        window.pollEvents();
        float dt = window.getDeltaTime();

        if (zenith::Input::isKeyDown(zenith::KeyCode::Escape)) {
            window.setShouldClose(true);
        }

        // --- Player Input System ---
        Position* playerPos = world.getComponent<Position>(player);
        Velocity* playerVel = world.getComponent<Velocity>(player);
        if (playerPos && playerVel) {
            playerVel->dx = 0.0f;
            playerVel->dy = 0.0f;
            float speed = 250.0f;

            if (zenith::Input::isKeyDown(zenith::KeyCode::W) || zenith::Input::isKeyDown(zenith::KeyCode::Up)) playerVel->dy = -speed;
            if (zenith::Input::isKeyDown(zenith::KeyCode::S) || zenith::Input::isKeyDown(zenith::KeyCode::Down)) playerVel->dy = speed;
            if (zenith::Input::isKeyDown(zenith::KeyCode::A) || zenith::Input::isKeyDown(zenith::KeyCode::Left)) playerVel->dx = -speed;
            if (zenith::Input::isKeyDown(zenith::KeyCode::D) || zenith::Input::isKeyDown(zenith::KeyCode::Right)) playerVel->dx = speed;
        }

        // --- Movement & Physics System ---
        spatialGrid.clear();
        std::size_t entityCount = 0;

        world.query<Position, Velocity>([&](zenith::ecs::Entity e, Position& pos, Velocity& vel) {
            pos.x += vel.dx * dt;
            pos.y += vel.dy * dt;
            spatialGrid.insert(pos.x, pos.y, 32.0f, 32.0f, e.id);
            entityCount++;
        });

        // --- Camera Follow System ---
        if (playerPos) {
            camera.positionX = playerPos->x - config.width / 2.0f;
            camera.positionY = playerPos->y - config.height / 2.0f;
        }

        // --- Render System ---
        window.clear(zenith::Color{0.1f, 0.12f, 0.15f, 1.0f});

        zenith::Renderer2D::beginCamera(camera);

        // Draw Grid Lines in world space
        for (int x = -1000; x <= 2000; x += 100) {
            zenith::Renderer2D::drawLine(static_cast<float>(x), -1000.0f, static_cast<float>(x), 2000.0f, zenith::Color{0.2f, 0.25f, 0.3f, 0.5f});
        }
        for (int y = -1000; y <= 2000; y += 100) {
            zenith::Renderer2D::drawLine(-1000.0f, static_cast<float>(y), 2000.0f, static_cast<float>(y), zenith::Color{0.2f, 0.25f, 0.3f, 0.5f});
        }

        // Draw Entities
        world.query<Position, Renderable>([&](zenith::ecs::Entity e, Position& pos, Renderable& rend) {
            zenith::Renderer2D::drawRect(pos.x, pos.y, rend.size, rend.size, rend.color);
            zenith::Renderer2D::drawRectOutline(pos.x, pos.y, rend.size, rend.size, zenith::Color::White(), 1.5f);
        });

        zenith::Renderer2D::endCamera();

        // Render In-Game Debug Overlay
        debugOverlay.updateMetrics(dt, entityCount, 12);
        debugOverlay.render(config.width, config.height);

        window.swapBuffers();
    }

    window.shutdown();
    std::cout << "Zenith Code-First ECS Game Exited Gracefully." << std::endl;
    return 0;
}
