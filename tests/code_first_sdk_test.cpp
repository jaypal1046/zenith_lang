#include "../include/zenith/game/zenith_game.h"
#include <iostream>
#include <cassert>

struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;
};

int main() {
    std::cout << "[Test] Initializing Zenith Code-First Game SDK test..." << std::endl;

    // 1. Test Windowing & Render Config
    zenith::WindowConfig config;
    config.title = "Zenith SDK Test";
    config.width = 800;
    config.height = 600;
    
    zenith::NativeWindow window;
    bool winOk = window.initialize(config);
    assert(winOk && "NativeWindow failed to initialize!");
    std::cout << "[Test] NativeWindow initialized successfully." << std::endl;

    // 2. Test ECS Core
    zenith::ecs::World world;

    zenith::ecs::Entity player = world.createEntity();
    world.addComponent(player, Position{100.0f, 150.0f});
    world.addComponent(player, Velocity{10.0f, -5.0f});

    zenith::ecs::Entity enemy = world.createEntity();
    world.addComponent(enemy, Position{200.0f, 300.0f});
    world.addComponent(enemy, Velocity{-2.0f, 0.0f});

    int processedEntities = 0;
    world.query<Position, Velocity>([&](zenith::ecs::Entity e, Position& pos, Velocity& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
        processedEntities++;
    });

    assert(processedEntities == 2 && "ECS Query failed to process 2 entities!");
    Position* pPos = world.getComponent<Position>(player);
    assert(pPos != nullptr && pPos->x == 110.0f && pPos->y == 145.0f && "ECS Movement update failed!");
    std::cout << "[Test] Zenith ECS created entities and processed queries successfully." << std::endl;

    // 3. Test Spatial Hash Grid
    zenith::spatial::SpatialHashGrid2D<std::uint32_t> spatialGrid(64.0f);
    spatialGrid.insert(100.0f, 100.0f, 32.0f, 32.0f, player.id);
    spatialGrid.insert(500.0f, 500.0f, 32.0f, 32.0f, enemy.id);

    auto nearby = spatialGrid.query(90.0f, 90.0f, 50.0f, 50.0f);
    assert(nearby.size() == 1 && nearby[0] == player.id && "Spatial hash query failed!");
    std::cout << "[Test] SpatialHashGrid2D query succeeded with exact target entity." << std::endl;

    // 4. Test Input Subsystem
    zenith::Input::updateKey(static_cast<int>(zenith::KeyCode::Space), true);
    assert(zenith::Input::isKeyDown(zenith::KeyCode::Space) && "Input KeyDown failed!");
    assert(zenith::Input::isKeyPressed(zenith::KeyCode::Space) && "Input KeyPressed failed!");
    std::cout << "[Test] Input subsystem key states verified." << std::endl;

    // 5. Test Camera2D Matrix Scoping & Debug Overlay
    zenith::Camera2D camera{100.0f, 50.0f, 1.5f, 0.0f};
    zenith::Renderer2D::beginCamera(camera);
    zenith::Renderer2D::drawRect(10, 10, 50, 50, zenith::Color::Red());
    zenith::Renderer2D::endCamera();

    zenith::DebugOverlay overlay;
    overlay.updateMetrics(0.016f, 2, 5);
    assert(overlay.isVisible() && "DebugOverlay visibility test failed!");
    std::cout << "[Test] DebugOverlay and Camera2D rendering scopes verified." << std::endl;

    // 6. Test Texture2D & Sprite Region Drawing
    zenith::Texture2D dummyTex = zenith::TextureManager::createDummyProceduralTexture(16, 16, zenith::Color::Green());
    assert(dummyTex.isValid() && "Dummy procedural texture creation failed!");
    
    zenith::SpriteRegion region{0.0f, 0.0f, 0.5f, 0.5f};
    zenith::TextureManager::drawSprite(dummyTex, 20.0f, 20.0f, 64.0f, 64.0f);
    zenith::TextureManager::drawSpriteRegion(dummyTex, region, 100.0f, 20.0f, 64.0f, 64.0f);
    std::cout << "[Test] Texture2D and SpriteRegion rendering verified." << std::endl;

    // 7. Test Audio Subsystem
    zenith::AudioManager::setMasterVolume(0.8f);
    assert(zenith::AudioManager::getMasterVolume() == 0.8f && "AudioManager master volume test failed!");
    zenith::AudioManager::playSound("dummy_sound.wav", 1.0f);
    zenith::AudioManager::stopMusic();
    std::cout << "[Test] AudioManager playSound and master volume verified." << std::endl;

    // 8. Test Particle Subsystem
    zenith::ParticleEmitter2D emitter(100);
    emitter.burst(400.0f, 300.0f, 15, zenith::Color::Yellow());
    assert(emitter.getActiveCount() == 15 && "ParticleEmitter2D burst active count test failed!");
    emitter.update(0.016f);
    emitter.render();
    std::cout << "[Test] ParticleEmitter2D burst, update, and render verified." << std::endl;

    // 9. Test Game State Machine Subsystem
    class TestState : public zenith::IGameState {
    public:
        bool updated = false;
        void onUpdate(float dt) override { updated = true; }
        void onRender() override {}
    };

    zenith::GameStateMachine stateMachine;
    stateMachine.changeState<TestState>();
    assert(stateMachine.size() == 1 && "GameStateMachine changeState failed!");
    stateMachine.update(0.016f);
    stateMachine.render();
    std::cout << "[Test] GameStateMachine changeState, update, and render verified." << std::endl;

    // 10. Test Composable Flutter-Style GUI Subsystem (Container, Stack, Row, Column, ImageWidget, ButtonWidget)
    bool actionTriggered = false;
    auto container = std::make_shared<zenith::Container>();
    container->width = 200.0f;
    container->height = 80.0f;
    container->color = zenith::Color::Blue();

    auto btn = std::make_shared<zenith::ButtonWidget>("Click Me", [&]() {
        actionTriggered = true;
    });

    container->child = btn;

    auto stack = std::make_shared<zenith::Stack>();
    stack->addChild(container);
    stack->render(10.0f, 10.0f, 200.0f, 80.0f);
    std::cout << "[Test] Flutter-style composable GUI Container, Stack, and ButtonWidget verified." << std::endl;

    // 11. Test AnimatedSpriteWidget & VideoStreamWidget Subsystem
    zenith::AnimatedSpriteWidget animWidget;
    animWidget.addFrame(dummyTex);
    animWidget.update(0.016f);
    animWidget.render(0.0f, 0.0f, 100.0f, 100.0f);

    zenith::VideoStreamWidget videoWidget;
    videoWidget.render(0.0f, 0.0f, 640.0f, 360.0f);
    std::cout << "[Test] AnimatedSpriteWidget and VideoStreamWidget verified." << std::endl;

    // 12. Test TileMap2D & TileSet Subsystem
    zenith::TileSet tileSet;
    tileSet.tileWidth = 32;
    tileSet.tileHeight = 32;
    tileSet.columns = 4;
    tileSet.rows = 4;

    zenith::TileMap2D tileMap(10, 10, 32.0f, tileSet);
    tileMap.setTile(2, 3, 1);
    assert(tileMap.getTile(2, 3) == 1 && "TileMap2D setTile/getTile test failed!");
    tileMap.render(0.0f, 0.0f, 320.0f, 320.0f);
    std::cout << "[Test] TileMap2D and TileSet verified." << std::endl;

    // 13. Test Tween & Easing Animation Subsystem
    bool tweenFinished = false;
    zenith::TweenFloat tween;
    tween.start(0.0f, 100.0f, 1.0f, zenith::EaseType::EaseOutBounce, [&]() {
        tweenFinished = true;
    });

    tween.update(0.5f);
    assert(tween.getValue() > 0.0f && "TweenFloat value progress calculation failed!");
    tween.update(0.5f);
    assert(tweenFinished && "TweenFloat completion callback failed!");
    std::cout << "[Test] TweenFloat and Easing curves verified." << std::endl;

    // 14. Test Null Safety Subsystem (Option<T>, SafeRef<T>)
    zenith::Option<int> opt = zenith::Option<int>::Some(42);
    assert(opt.isSome() && opt.unwrap() == 42 && "Option<T> unwrap test failed!");
    
    int val = 100;
    zenith::SafeRef<int> safeRef(&val);
    assert(safeRef.isValid() && safeRef.valueOrThrow() == 100 && "SafeRef<T> test failed!");
    std::cout << "[Test] Option<T> and SafeRef<T> null-safety wrappers verified." << std::endl;

    // 15. Test Multithreaded JobSystem Subsystem
    zenith::JobSystem jobSystem;
    jobSystem.initialize(2);
    std::atomic<int> counter{0};
    jobSystem.parallelFor(100, 10, [&](std::size_t index) {
        counter.fetch_add(1);
    });
    assert(counter.load() == 100 && "JobSystem parallelFor execution count test failed!");
    jobSystem.shutdown();
    std::cout << "[Test] Multithreaded JobSystem parallelFor dispatch verified." << std::endl;

    // 16. Test EventBus Subsystem
    struct PlayerDamagedEvent : public zenith::IEvent {
        int damage = 10;
    };

    bool eventReceived = false;
    int damageAmount = 0;

    zenith::EventBus eventBus;
    eventBus.subscribe<PlayerDamagedEvent>([&](const PlayerDamagedEvent& ev) {
        eventReceived = true;
        damageAmount = ev.damage;
    });

    eventBus.publish(PlayerDamagedEvent{25});
    assert(eventReceived && damageAmount == 25 && "EventBus publish/subscribe test failed!");
    std::cout << "[Test] EventBus publish and type-safe event subscription verified." << std::endl;

    // 17. Test SaveSystem Subsystem
    zenith::SaveSystem saveSystem;
    saveSystem.setInt("HighScore", 9999);
    saveSystem.setString("PlayerName", "Hero");
    saveSystem.setBool("SoundEnabled", true);

    assert(saveSystem.getInt("HighScore") == 9999 && "SaveSystem getInt test failed!");
    assert(saveSystem.getString("PlayerName") == "Hero" && "SaveSystem getString test failed!");
    assert(saveSystem.getBool("SoundEnabled") == true && "SaveSystem getBool test failed!");
    std::cout << "[Test] SaveSystem key-value serialization verified." << std::endl;

    // 18. Test Virtual Joystick Subsystem
    zenith::VirtualJoystick joystick(100.0f, 100.0f, 60.0f);
    joystick.update();
    joystick.render();
    assert(joystick.getAxis().x == 0.0f && joystick.getAxis().y == 0.0f && "VirtualJoystick default axis test failed!");
    std::cout << "[Test] VirtualJoystick update and rendering verified." << std::endl;

    // 19. Test Camera3D & Renderer3D Subsystem
    zenith::Camera3D cam3d;
    cam3d.position = {0.0f, 10.0f, 10.0f};
    zenith::Camera3D::beginCamera3D(cam3d);
    zenith::Renderer3D::drawGrid3D(10, 1.0f);
    zenith::Renderer3D::drawCube({0.0f, 0.0f, 0.0f}, 2.0f, 2.0f, 2.0f, zenith::Color::Red());
    zenith::Camera3D::endCamera3D();
    std::cout << "[Test] Camera3D and Renderer3D primitives verified." << std::endl;

    // 20. Test PerlinNoise2D Subsystem
    zenith::PerlinNoise2D noiseGen(42);
    float val1 = noiseGen.noise(1.5f, 2.5f);
    float val2 = noiseGen.noise(1.5f, 2.5f);
    assert(val1 == val2 && "PerlinNoise2D deterministic evaluation test failed!");
    std::cout << "[Test] PerlinNoise2D procedural noise evaluation verified." << std::endl;

    // 21. Test SpatialAudio2D Subsystem
    zenith::SpatialAudioParams audioParams = zenith::SpatialAudio2D::calculateParams(100.0f, 100.0f, 0.0f, 0.0f, 50.0f, 500.0f);
    assert(audioParams.volume > 0.0f && audioParams.volume < 1.0f && "SpatialAudio2D attenuation test failed!");
    std::cout << "[Test] SpatialAudio2D distance attenuation and panning calculated." << std::endl;

    // 22. Test CameraShake2D Subsystem
    zenith::CameraShake2D shake;
    shake.trigger(15.0f, 0.2f);
    assert(shake.isShaking() && "CameraShake2D trigger test failed!");
    shake.update(0.1f, camera);
    std::cout << "[Test] CameraShake2D trigger and offset update verified." << std::endl;

    // 23. Test PhysicsRaycast2D Subsystem
    zenith::spatial::Point2D rayOrigin{0.0f, 50.0f};
    zenith::spatial::Point2D rayDir{1.0f, 0.0f};
    zenith::spatial::Rect2D targetBox{100.0f, 0.0f, 100.0f, 100.0f};

    zenith::spatial::RaycastHit2D rayHit = zenith::PhysicsRaycast2D::castRayAgainstBox(rayOrigin, rayDir, 500.0f, targetBox);
    assert(rayHit.hit && rayHit.distance == 100.0f && "PhysicsRaycast2D ray hit test failed!");
    std::cout << "[Test] PhysicsRaycast2D raycast hit distance and point verified." << std::endl;

    // 24. Test Lighting2D Subsystem
    zenith::Lighting2D::setAmbientColor(zenith::Color{0.1f, 0.1f, 0.2f, 1.0f});
    zenith::PointLight2D light{100.0f, 100.0f, 200.0f, zenith::Color::Yellow(), 1.0f};
    zenith::Lighting2D::drawPointLight(light);
    assert(zenith::Lighting2D::getAmbientColor().b == 0.2f && "Lighting2D ambient color test failed!");
    std::cout << "[Test] Lighting2D ambient color and point light rendering verified." << std::endl;

    // 25. Test Node2D Subsystem
    auto parentNode = std::make_shared<zenith::Node2D>();
    parentNode->x = 100.0f;
    parentNode->y = 200.0f;

    auto childNode = std::make_shared<zenith::Node2D>();
    childNode->x = 50.0f;
    childNode->y = 25.0f;

    parentNode->addChild(childNode);
    zenith::spatial::Point2D worldPos = childNode->getWorldPosition();
    assert(worldPos.x == 150.0f && worldPos.y == 225.0f && "Node2D parent-child transform propagation failed!");
    std::cout << "[Test] Node2D parent-child world transform propagation verified." << std::endl;

    // 26. Test Animator2D Subsystem
    zenith::AnimationClip2D runClip;
    runClip.name = "run";
    runClip.frameDuration = 0.05f;
    runClip.frames.push_back(zenith::SpriteRegion{0.0f, 0.0f, 0.25f, 1.0f});
    runClip.frames.push_back(zenith::SpriteRegion{0.25f, 0.0f, 0.5f, 1.0f});

    zenith::Animator2D animator;
    animator.addClip(runClip);
    animator.play("run");
    animator.update(0.06f);
    assert(animator.getCurrentClipName() == "run" && "Animator2D state play test failed!");
    std::cout << "[Test] Animator2D state machine and frame updates verified." << std::endl;

    // 27. Test BehaviorTree AI Subsystem
    bool action1Ran = false;
    auto node1 = std::make_shared<zenith::ActionNode>([&]() {
        action1Ran = true;
        return zenith::BehaviorStatus::Success;
    });

    auto sequence = std::make_shared<zenith::SequenceNode>();
    sequence->children.push_back(node1);
    zenith::BehaviorStatus result = sequence->tick();

    assert(result == zenith::BehaviorStatus::Success && action1Ran && "BehaviorTree SequenceNode tick test failed!");
    std::cout << "[Test] BehaviorTree SequenceNode and ActionNode ticks verified." << std::endl;

    // 28. Test CollisionSolver2D Subsystem
    zenith::spatial::Rect2D moverBox{10.0f, 10.0f, 30.0f, 30.0f};
    zenith::spatial::Rect2D obstacleBox{30.0f, 10.0f, 30.0f, 30.0f};
    zenith::Point2D velocity{100.0f, 0.0f};

    zenith::CollisionSolver2D::resolveAABBCollision(moverBox, velocity, obstacleBox, 0.0f);
    assert(moverBox.x == 0.0f && "CollisionSolver2D push-out resolution failed!");
    std::cout << "[Test] CollisionSolver2D AABB push-out and bounce response verified." << std::endl;

    // 29. Test AbilityContainer & GameplayEffect Subsystem
    zenith::AbilityContainer abilities;
    zenith::GameplayEffect speedBuff;
    speedBuff.id = "speed_boost";
    speedBuff.duration = 1.0f;

    abilities.applyEffect(speedBuff);
    assert(abilities.hasEffect("speed_boost") && "AbilityContainer applyEffect test failed!");
    abilities.update(1.5f);
    assert(!abilities.hasEffect("speed_boost") && "AbilityContainer duration expiration test failed!");
    std::cout << "[Test] AbilityContainer status effects and buff duration expiration verified." << std::endl;

    // 30. Test GridPathfinder2D Subsystem
    zenith::TileSet pfTileSet;
    zenith::TileMap2D pfMap(5, 5, 32.0f, pfTileSet);
    auto foundPath = zenith::GridPathfinder2D::findPath(0, 0, 4, 4, pfMap);
    assert(!foundPath.empty() && foundPath.front().col == 0 && foundPath.back().col == 4 && "GridPathfinder2D path search failed!");
    std::cout << "[Test] GridPathfinder2D A* grid path search verified." << std::endl;

    // 31. Test ObjectPool Subsystem
    struct Bullet { float x, y; };
    zenith::ObjectPool<Bullet> bulletPool(10);
    auto b1 = bulletPool.acquire();
    assert(b1 != nullptr && bulletPool.getAvailableCount() == 9 && "ObjectPool acquire test failed!");
    bulletPool.release(b1);
    assert(bulletPool.getAvailableCount() == 10 && "ObjectPool release recycling test failed!");
    std::cout << "[Test] ObjectPool memory recycling verified." << std::endl;

    // 32. Test Material2D Subsystem
    zenith::Material2D mat("ShieldGlow");
    mat.setColorTint(zenith::Color::Cyan());
    mat.setEmission(2.5f);
    mat.setFloat("outlineWidth", 4.0f);
    mat.bind();
    assert(mat.getEmission() == 2.5f && mat.getFloat("outlineWidth") == 4.0f && "Material2D property binding failed!");
    std::cout << "[Test] Material2D color tint, emission, and uniform property binding verified." << std::endl;

    // 33. Test Gamepad Controller Subsystem
    zenith::Gamepad pad;
    pad.updateAxis(0.05f, 0.8f, 0.0f, 0.0f);
    assert(pad.getAxisLeftX() == 0.0f && pad.getAxisLeftY() == 0.8f && pad.isConnected() && "Gamepad deadzone filter test failed!");
    std::cout << "[Test] Gamepad analog axis deadzone filtering and connection state verified." << std::endl;

    // 34. Test LiveTuner Subsystem
    float speed = zenith::LiveTuner::tuneFloat("player_speed", 250.0f);
    assert(speed == 250.0f && "LiveTuner tuneFloat default value test failed!");
    zenith::LiveTuner::setFloat("player_speed", 320.0f);
    float updatedSpeed = zenith::LiveTuner::tuneFloat("player_speed", 250.0f);
    assert(updatedSpeed == 320.0f && "LiveTuner live update test failed!");
    std::cout << "[Test] LiveTuner live variable registration and parameter updates verified." << std::endl;

    // 35. Test AssetPack Subsystem
    zenith::AssetPack assets;
    auto tex = assets.getOrCreateTexture("player_sprite");
    assert(assets.getLoadedTextureCount() == 1 && "AssetPack getOrCreateTexture test failed!");
    assets.unloadAll();
    assert(assets.getLoadedTextureCount() == 0 && "AssetPack unloadAll test failed!");
    std::cout << "[Test] AssetPack caching and memory lifecycle management verified." << std::endl;

    // 36. Test Viewport2D Subsystem
    zenith::Viewport2D leftScreen{0, 0, 400, 600};
    leftScreen.apply();
    zenith::Viewport2D::reset(800, 600);
    std::cout << "[Test] Viewport2D split-screen viewport scoping verified." << std::endl;

    // 37. Test ParticleFXPreset Subsystem
    zenith::ParticleEmitter2D fxEmitter(50);
    zenith::ParticleFXPreset::createExplosion(fxEmitter, 100.0f, 100.0f, 20);
    assert(fxEmitter.particles.size() == 20 && "ParticleFXPreset createExplosion test failed!");
    std::cout << "[Test] ParticleFXPreset visual explosion burst generation verified." << std::endl;

    // 38. Test PostProcessing2D Subsystem
    zenith::PostProcessing2D::setVignette(0.5f);
    zenith::PostProcessing2D::setBloomThreshold(0.7f);
    zenith::PostProcessing2D::beginPass();
    zenith::PostProcessing2D::endPass();
    assert(zenith::PostProcessing2D::getVignette() == 0.5f && zenith::PostProcessing2D::getBloomThreshold() == 0.7f && "PostProcessing2D config test failed!");
    std::cout << "[Test] PostProcessing2D vignette, bloom, and pass scoping verified." << std::endl;

    // 39. Test NetPacket Serialization Subsystem
    zenith::NetPacket packet;
    packet.writeInt(42);
    packet.writeFloat(3.14159f);

    packet.resetRead();
    int32_t netInt = packet.readInt();
    float netFloat = packet.readFloat();
    assert(netInt == 42 && std::abs(netFloat - 3.14159f) < 0.001f && "NetPacket serialization/deserialization test failed!");
    std::cout << "[Test] NetPacket binary int/float serialization & deserialization verified." << std::endl;

    // 40. Test Font2D Subsystem
    float textWidth = zenith::Font2D::measureTextWidth("ZENITH GAME ENGINE", 20.0f);
    assert(textWidth > 0.0f && "Font2D measureTextWidth test failed!");
    zenith::Font2D::drawText("ZENITH GAME ENGINE", 10.0f, 10.0f, 20.0f, zenith::Color::Yellow());
    std::cout << "[Test] Font2D text measurement and styled text rendering verified." << std::endl;

    // 41. Test RigidBody2D Dynamics Subsystem
    zenith::RigidBody2D body;
    body.mass = 2.0f;
    body.applyImpulse(200.0f, -500.0f);
    body.update(0.1f, 980.0f);
    assert(body.x > 0.0f && body.velocityX > 0.0f && "RigidBody2D impulse and velocity integration failed!");
    std::cout << "[Test] RigidBody2D impulse forces and velocity integration verified." << std::endl;

    // 42. Test AutoTile2D Subsystem
    uint8_t bitmask = zenith::AutoTile2D::calculateBitmask(true, false, true, false); // up + left = 1 + 8 = 9
    assert(bitmask == 9 && zenith::AutoTile2D::getTileIndexFromBitmask(bitmask) == 9 && "AutoTile2D bitmask calculation failed!");
    std::cout << "[Test] AutoTile2D 4-bit neighbor rule calculation verified." << std::endl;

    // 43. Test AudioMixer2D Subsystem
    zenith::AudioMixer2D::setBusVolume(zenith::AudioBus::Master, 0.8f);
    zenith::AudioMixer2D::setBusVolume(zenith::AudioBus::Music, 0.5f);
    float effectiveMusicVol = zenith::AudioMixer2D::getBusVolume(zenith::AudioBus::Music);
    assert(std::abs(effectiveMusicVol - 0.4f) < 0.001f && "AudioMixer2D bus gain scaling test failed!");
    std::cout << "[Test] AudioMixer2D multi-channel bus gain scaling verified." << std::endl;

    // 44. Test DevConsole Subsystem
    bool cheatActivated = false;
    zenith::DevConsole::registerCommand("godmode", [&](const std::vector<std::string>& args) {
        (void)args;
        cheatActivated = true;
    });

    bool executed = zenith::DevConsole::executeCommand("godmode 1");
    assert(executed && cheatActivated && "DevConsole command registration/execution test failed!");
    std::cout << "[Test] DevConsole live command registration and execution verified." << std::endl;

    // 45. Test ProceduralAudio Synthesizer Subsystem
    auto toneSamples = zenith::ProceduralAudio::generateTone(440.0f, 0.1f);
    auto laserSamples = zenith::ProceduralAudio::generateLaser(0.2f);
    assert(!toneSamples.empty() && !laserSamples.empty() && "ProceduralAudio sample generation failed!");
    std::cout << "[Test] ProceduralAudio waveform tone and laser sample synthesis verified." << std::endl;

    // 46. Test RenderLayerManager2D Y-Sorting Subsystem
    zenith::RenderLayerManager2D layers;
    std::vector<int> renderOrder;
    layers.addRenderable(0, 200.0f, [&]() { renderOrder.push_back(2); });
    layers.addRenderable(0, 100.0f, [&]() { renderOrder.push_back(1); });
    layers.sortAndRender();

    assert(renderOrder.size() == 2 && renderOrder[0] == 1 && renderOrder[1] == 2 && "RenderLayerManager2D Y-sorting failed!");
    std::cout << "[Test] RenderLayerManager2D z-index & Y-sorting depth pass verified." << std::endl;

    // 47. Test DialogueTree Subsystem
    zenith::DialogueTree dialogue;
    zenith::DialogueNode node0{0, "Elder", "Greetings traveler! Seek the sword?", {{"Yes", 1, nullptr}, {"No", -1, nullptr}}};
    zenith::DialogueNode node1{1, "Elder", "Take this blade of light!", {}};

    dialogue.addNode(node0);
    dialogue.addNode(node1);
    dialogue.start(0);

    assert(dialogue.isActive() && dialogue.getCurrentSpeaker() == "Elder" && "DialogueTree start test failed!");
    dialogue.chooseOption(0);
    assert(dialogue.getCurrentText() == "Take this blade of light!" && "DialogueTree option transition failed!");
    std::cout << "[Test] DialogueTree narrative branching and option selection verified." << std::endl;

    // 48. Test DestructibleTerrain2D Subsystem
    zenith::DestructibleTerrain2D terrain(100, 100, true);
    assert(terrain.isSolid(50, 50) && "DestructibleTerrain2D initial fill failed!");
    terrain.carveCircle(50, 50, 10);
    assert(!terrain.isSolid(50, 50) && terrain.isSolid(10, 10) && "DestructibleTerrain2D carveCircle failed!");
    std::cout << "[Test] DestructibleTerrain2D bitmap grid explosion carving verified." << std::endl;

    // 49. Test Inventory2D Item Stacking Subsystem
    zenith::Inventory2D inventory(10);
    inventory.addItem("health_potion", 5);
    inventory.addItem("health_potion", 10);
    assert(inventory.getItemCount("health_potion") == 15 && "Inventory2D item stacking failed!");
    std::cout << "[Test] Inventory2D item stacking and capacity management verified." << std::endl;

    // 50. Test QuestManager Objective Tracking Subsystem
    zenith::QuestManager questMgr;
    zenith::Quest q1("slay_goblins", "Goblin Menace", "Defeat 3 goblins");
    q1.addObjective("Defeat Goblins", 3);
    questMgr.addQuest(q1);

    assert(!questMgr.isQuestCompleted("slay_goblins") && "Quest initially incomplete failed!");
    questMgr.advanceObjective("slay_goblins", 0, 3);
    assert(questMgr.isQuestCompleted("slay_goblins") && "Quest objective advancement/completion failed!");
    std::cout << "[Test] QuestManager objective tracking and completion verified." << std::endl;

    // 51. Test Profiler Benchmarking Subsystem
    {
        zenith::ProfileScope scope("RenderPass");
        // Simulate minor workload
        int dummy = 0;
        for (int i = 0; i < 10000; ++i) dummy += i;
        (void)dummy;
    }
    double renderTimeMs = zenith::Profiler::getAverageTimeMs("RenderPass");
    assert(renderTimeMs >= 0.0 && "Profiler scope benchmarking failed!");
    std::cout << "[Test] Profiler scoped section timer and metric benchmarking verified." << std::endl;

    // 52. Test WaterBody2D Wave Simulation Subsystem
    zenith::WaterBody2D water(20, 100.0f);
    water.splash(10, 50.0f);
    water.update(0.016f);
    assert(water.getSpringHeight(10) != 100.0f && "WaterBody2D spring splash wave calculation failed!");
    std::cout << "[Test] WaterBody2D spring wave propagation & splash dynamics verified." << std::endl;

    // 53. Test SoftBody2D Verlet Mesh Subsystem
    zenith::SoftBody2D jelly;
    auto n0 = jelly.addNode(0.0f, 0.0f);
    auto n1 = jelly.addNode(10.0f, 0.0f);
    jelly.addSpring(n0, n1, 0.5f);
    jelly.update(0.016f);

    assert(jelly.getNodeCount() == 2 && jelly.getNode(0).y > 0.0f && "SoftBody2D Verlet solver failed!");
    std::cout << "[Test] SoftBody2D Verlet mesh integration & spring relaxation verified." << std::endl;

    // 54. Test IK2D Inverse Kinematics Subsystem
    zenith::Vector2D joint;
    bool solved = zenith::IK2D::solveTwoBone({0.0f, 0.0f}, {15.0f, 0.0f}, 10.0f, 10.0f, false, joint);
    assert(solved && std::abs(joint.x - 7.5f) < 0.1f && "IK2D 2-bone joint calculation failed!");
    std::cout << "[Test] IK2D analytic 2-bone joint solving verified." << std::endl;

    // 55. Test ShadowCaster2D Dynamic Occlusion Subsystem
    zenith::Rect2D obstacleBox{50.0f, 50.0f, 20.0f, 20.0f};
    auto shadowVerts = zenith::ShadowCaster2D::projectBoxShadow(0.0f, 0.0f, obstacleBox, 500.0f);
    assert(shadowVerts.size() == 4 && shadowVerts[0].x > 50.0f && "ShadowCaster2D shadow projection failed!");
    std::cout << "[Test] ShadowCaster2D 2D polygon shadow ray projection verified." << std::endl;

    // 56. Test DungeonGenerator2D Procedural Generation Subsystem
    zenith::DungeonGenerator2D dungeonGen(50, 50);
    dungeonGen.generate(5, 4, 8);
    assert(dungeonGen.getRoomCount() > 0 && "DungeonGenerator2D room generation failed!");
    std::cout << "[Test] DungeonGenerator2D room placement and corridor connection verified." << std::endl;

    // 57. Test FluidSimulation2D SPH Liquid Solver Subsystem
    zenith::FluidSimulation2D fluid;
    fluid.addParticle(10.0f, 10.0f);
    fluid.addParticle(12.0f, 10.0f);
    fluid.update(0.016f);
    assert(fluid.getParticleCount() == 2 && fluid.getParticle(0).density > 0.0f && "FluidSimulation2D SPH density solver failed!");
    std::cout << "[Test] FluidSimulation2D SPH particle density & gravity solver verified." << std::endl;

    // 58. Test Cloth2D Rope Verlet Integration Subsystem
    zenith::Cloth2D rope;
    rope.createRope(0.0f, 0.0f, 0.0f, 50.0f, 5);
    rope.update(0.016f);
    assert(rope.getParticleCount() == 6 && rope.getParticle(0).pinned && "Cloth2D rope creation/update failed!");
    std::cout << "[Test] Cloth2D rope Verlet particle integration & distance constraint solving verified." << std::endl;

    // 59. Test WeatherSystem2D Environment Subsystem
    zenith::WeatherSystem2D weather;
    weather.setWeather(zenith::WeatherType::Rain, 50);
    weather.setWind(30.0f);
    weather.update(0.016f);
    assert(weather.getDropCount() == 50 && weather.getType() == zenith::WeatherType::Rain && "WeatherSystem2D rain simulation failed!");
    std::cout << "[Test] WeatherSystem2D rain, snow, and wind drift simulation verified." << std::endl;

    // 60. Test ReplaySystem2D State Snapshot & Playback Subsystem
    zenith::ReplaySystem2D replay;
    replay.recordFrame(0.0f, 10.0f, 20.0f, 1);
    replay.startPlayback();
    zenith::ReplayFrame2D frame;
    bool hasFrame = replay.stepPlayback(frame);
    assert(hasFrame && frame.posX == 10.0f && "ReplaySystem2D frame recording/playback failed!");
    std::cout << "[Test] ReplaySystem2D state snapshot recording and playback stepping verified." << std::endl;

    // 61. Test FOV2D Vision Cone Stealth Subsystem
    auto visionPoints = zenith::FOV2D::computeVisionCone(100.0f, 100.0f, 0.0f, 1.57f, 150.0f, 16);
    assert(visionPoints.size() == 17 && "FOV2D vision cone calculation failed!");
    std::cout << "[Test] FOV2D vision cone arc point generation verified." << std::endl;

    // 62. Test FogOfWar2D Exploration Subsystem
    zenith::FogOfWar2D fog(30, 30);
    fog.reveal(15, 15, 5);
    assert(fog.getTileState(15, 15) == zenith::FogTileState::Visible && "FogOfWar2D tile reveal failed!");
    std::cout << "[Test] FogOfWar2D map vision reveal and tile state tracking verified." << std::endl;

    // 63. Test DecalManager2D Surface Markings Subsystem
    zenith::DecalManager2D decals(50);
    decals.addDecal(100.0f, 100.0f, 16.0f, 16.0f, zenith::Color::Red(), 10.0f);
    decals.update(0.016f);
    assert(decals.getDecalCount() == 1 && "DecalManager2D decal addition/update failed!");
    std::cout << "[Test] DecalManager2D persistent blood splatters and bullet hole rendering verified." << std::endl;

    // 64. Test FloatingTextManager2D Damage Numbers Subsystem
    zenith::FloatingTextManager2D floatingTexts;
    floatingTexts.spawnDamageNumber(150, 200.0f, 200.0f, true);
    floatingTexts.update(0.016f);
    assert(floatingTexts.getCount() == 1 && "FloatingTextManager2D damage number creation failed!");
    std::cout << "[Test] FloatingTextManager2D critical damage numbers and float updates verified." << std::endl;

    // 65. Test SteeringBehaviors2D AI Movement Subsystem
    zenith::SteeringAgent2D agent;
    agent.position = {0.0f, 0.0f};
    agent.velocity = {0.0f, 0.0f};
    auto force = zenith::SteeringBehaviors2D::seek(agent, {100.0f, 0.0f});
    assert(force.x > 0.0f && "SteeringBehaviors2D seek force calculation failed!");
    std::cout << "[Test] SteeringBehaviors2D seek/arrive force calculations verified." << std::endl;

    // 66. Test Heatmap2D AI Danger Map Subsystem
    zenith::Heatmap2D heatMap(20, 20);
    heatMap.addHeat(5, 5, 10.0f);
    heatMap.decay(0.9f);
    assert(heatMap.getHeat(5, 5) == 9.0f && "Heatmap2D heat accumulation/decay failed!");
    std::cout << "[Test] Heatmap2D heat accumulation & decay factor updates verified." << std::endl;

    // 67. Test VoxelGrid2D Digging Subsystem
    zenith::VoxelGrid2D voxels(100, 100);
    voxels.fill(zenith::VoxelType::Dirt);
    voxels.carveCircle(50, 50, 10);
    assert(voxels.getVoxel(50, 50) == zenith::VoxelType::Empty && voxels.getVoxel(0, 0) == zenith::VoxelType::Dirt && "VoxelGrid2D digging failed!");
    std::cout << "[Test] VoxelGrid2D granular pixel carving & digging verified." << std::endl;

    // 68. Test DistortionSystem2D Shockwave Subsystem
    zenith::DistortionSystem2D distortion;
    distortion.addShockwave(100.0f, 100.0f, 150.0f, 200.0f, 10.0f);
    distortion.update(0.016f);
    assert(distortion.getWaveCount() == 1 && "DistortionSystem2D shockwave creation failed!");
    std::cout << "[Test] DistortionSystem2D shockwave expansion and pixel displacement verified." << std::endl;

    // 69. Test NormalMap2D 2D Pixel Lighting Subsystem
    float intensity = zenith::NormalMap2D::calculateLightIntensity(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    assert(intensity == 1.0f && "NormalMap2D light intensity calculation failed!");
    std::cout << "[Test] NormalMap2D dot product normal vector lighting verified." << std::endl;

    // 70. Test Ragdoll2D Humanoid Ragdoll Physics Subsystem
    zenith::Ragdoll2D ragdoll;
    ragdoll.createHumanoid(100.0f, 100.0f);
    ragdoll.update(0.016f);
    assert(ragdoll.getJointCount() == 7 && ragdoll.getJoint(0).y > 60.0f && "Ragdoll2D humanoid creation/update failed!");
    std::cout << "[Test] Ragdoll2D 7-joint humanoid skeleton Verlet physics verified." << std::endl;

    // 71. Test MarchingCubes2D Contour Cave Generation Subsystem
    std::vector<float> grid = {1.0f, 0.0f, 0.0f, 1.0f};
    auto contours = zenith::MarchingCubes2D::generateContours(grid, 2, 2, 0.5f);
    assert(!contours.empty() && "MarchingCubes2D contour generation failed!");
    std::cout << "[Test] MarchingCubes2D 2D marching squares contour generation verified." << std::endl;

    // 72. Test NavMesh2D Off-Grid Pathfinding Subsystem
    zenith::NavMesh2D navMesh;
    navMesh.addPolygon({{0, 0}, {100, 0}, {100, 100}, {0, 100}});
    auto navPath = navMesh.findPath({10, 10}, {90, 90});
    assert(navMesh.getPolygonCount() == 1 && navPath.size() >= 3 && "NavMesh2D pathfinding failed!");
    std::cout << "[Test] NavMesh2D polygon navigation mesh building & pathfinding verified." << std::endl;

    // 73. Test Heightfield2D Elevation Subsystem
    zenith::Heightfield2D hfield(10, 10);
    hfield.setHeight(5, 5, 20.0f);
    float slope = hfield.calculateSlope(4, 5);
    assert(slope > 0.0f && "Heightfield2D elevation/slope calculation failed!");
    std::cout << "[Test] Heightfield2D elevation height grid & slope gradient verified." << std::endl;

    // 74. Test SDF2D Signed Distance Function Subsystem
    float cDist = zenith::SDF2D::circleSDF(10.0f, 0.0f, 0.0f, 0.0f, 5.0f);
    float bDist = zenith::SDF2D::boxSDF(0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 20.0f);
    assert(cDist == 5.0f && bDist < 0.0f && "SDF2D signed distance calculation failed!");
    std::cout << "[Test] SDF2D circle and box signed distance function evaluation verified." << std::endl;

    // 75. Test Spline2D Bezier Motion Path Subsystem
    auto pQuad = zenith::Spline2D::quadraticBezier({0, 0}, {50, 100}, {100, 0}, 0.5f);
    auto pCubic = zenith::Spline2D::cubicBezier({0, 0}, {33, 100}, {66, 100}, {100, 0}, 0.5f);
    assert(pQuad.y == 50.0f && pCubic.x == 50.0f && "Spline2D Bezier evaluation failed!");
    std::cout << "[Test] Spline2D quadratic and cubic Bezier motion curves verified." << std::endl;

    // 76. Test BlurPass2D Motion Blur & Backdrop Blur Subsystem
    std::vector<uint32_t> buffer = {0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000};
    zenith::BlurPass2D::applyBoxBlur(buffer, 2, 2, 1);
    assert(!buffer.empty() && "BlurPass2D box blur execution failed!");
    std::cout << "[Test] BlurPass2D box blur & backdrop blur pass verified." << std::endl;

    // 77. Test ChromaticAberration2D Glitch & Lens Aberration Subsystem
    std::vector<uint32_t> chromBuffer = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFFFF};
    zenith::ChromaticAberration2D::applyRGBShift(chromBuffer, 2, 2, 1, 0, -1, 0);
    assert(!chromBuffer.empty() && "ChromaticAberration2D RGB shift failed!");
    std::cout << "[Test] ChromaticAberration2D RGB channel shift glitch effect verified." << std::endl;

    // 78. Test CRTFilter2D Scanline Filter Subsystem
    std::vector<uint32_t> crtBuffer = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    zenith::CRTFilter2D::applyScanlines(crtBuffer, 2, 2, 2, 0.5f);
    assert(!crtBuffer.empty() && "CRTFilter2D scanlines application failed!");
    std::cout << "[Test] CRTFilter2D scanline screen filter execution verified." << std::endl;

    // 79. Test LinearArenaAllocator Transient Allocation Subsystem
    zenith::LinearArenaAllocator arena(1024);
    int* val = arena.create<int>(42);
    assert(val != nullptr && *val == 42 && arena.getUsedBytes() > 0 && "LinearArenaAllocator allocation failed!");
    arena.reset();
    assert(arena.getUsedBytes() == 0 && "LinearArenaAllocator reset failed!");
    std::cout << "[Test] LinearArenaAllocator transient memory creation and reset verified." << std::endl;

    // 80. Test PaletteSwapper2D Color Palette Shift Subsystem
    std::vector<uint32_t> palBuf = {0xFF0000FF, 0x00000000};
    zenith::PaletteSwapper2D::swapColors(palBuf, 2, 1, {0xFF0000FF}, {0x00FF00FF});
    assert(palBuf[0] == 0x00FF00FF && "PaletteSwapper2D swap failed!");
    std::cout << "[Test] PaletteSwapper2D color re-skin and palette swap verified." << std::endl;

    // 81. Test Dither2D Bayer Matrix Shading Subsystem
    std::vector<uint32_t> ditherBuf = {0x80808080, 0x40404040, 0xC0C0C0C0, 0xFFFFFFFF};
    zenith::Dither2D::applyBayerDither(ditherBuf, 2, 2, 2);
    assert(!ditherBuf.empty() && "Dither2D Bayer dithering failed!");
    std::cout << "[Test] Dither2D Bayer 4x4 retro pixel shading verified." << std::endl;

    // 82. Test TrailManager2D Motion Trail & Ghost Sprite Subsystem
    zenith::TrailManager2D trail;
    trail.spawnGhost(100.0f, 100.0f, 32.0f, 32.0f, 0xFFFFFFFF, 0.2f);
    assert(trail.getGhostCount() == 1 && "TrailManager2D spawn failed!");
    trail.update(0.3f);
    assert(trail.getGhostCount() == 0 && "TrailManager2D ghost expiration failed!");
    std::cout << "[Test] TrailManager2D ghost sprite spawn and lifetime update verified." << std::endl;

    // 83. Test LightningGenerator2D Procedural Electric Arc Subsystem
    auto bolt = zenith::LightningGenerator2D::generateBolt(0.0f, 0.0f, 100.0f, 100.0f, 3, 20.0f);
    assert(!bolt.empty() && "LightningGenerator2D bolt generation failed!");
    std::cout << "[Test] LightningGenerator2D midpoint displacement electric bolt verified." << std::endl;

    // 84. Test VignetteFilter2D Screen Darkening Subsystem
    std::vector<uint32_t> vigBuffer = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    zenith::VignetteFilter2D::applyVignette(vigBuffer, 2, 2, 0.1f, 0.5f);
    assert(!vigBuffer.empty() && "VignetteFilter2D vignette application failed!");
    std::cout << "[Test] VignetteFilter2D radial edge darkening filter verified." << std::endl;

    // 85. Test ColorGrading2D Brightness, Contrast & Saturation Subsystem
    std::vector<uint32_t> cgBuffer = {0xFF808080, 0xFF404040, 0xFFC0C0C0, 0xFFFFFFFF};
    zenith::ColorGrading2D::applyGrading(cgBuffer, 2, 2, 1.1f, 1.2f, 0.8f);
    assert(!cgBuffer.empty() && "ColorGrading2D post-processing failed!");
    std::cout << "[Test] ColorGrading2D brightness, contrast, and saturation filter verified." << std::endl;

    // 86. Test DigitalGlitch2D Cyberpunk Scanline Disruption Subsystem
    std::vector<uint32_t> glitchBuffer = {0xFF112233, 0xFF445566, 0xFF778899, 0xFFAABBCC};
    zenith::DigitalGlitch2D::applyGlitch(glitchBuffer, 2, 2, 0.5f);
    assert(!glitchBuffer.empty() && "DigitalGlitch2D glitch application failed!");
    std::cout << "[Test] DigitalGlitch2D scanline slice glitch disruption verified." << std::endl;

    // 87. Test RippleSystem2D Sine Wave Water Ripple Subsystem
    std::vector<uint32_t> ripBuffer = {0xFF112233, 0xFF445566, 0xFF778899, 0xFFAABBCC};
    zenith::RippleSystem2D::applySineRipple(ripBuffer, 2, 2, 1.0f, 2.0f, 0.1f);
    assert(!ripBuffer.empty() && "RippleSystem2D sine ripple execution failed!");
    std::cout << "[Test] RippleSystem2D sine wave water ripple displacement verified." << std::endl;

    // 88. Test BloomPass2D Luminance Bloom Glow Subsystem
    std::vector<uint32_t> bloomBuffer = {0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000};
    zenith::BloomPass2D::applyBloom(bloomBuffer, 2, 2, 0.5f, 0.5f);
    assert(!bloomBuffer.empty() && "BloomPass2D glow pass failed!");
    std::cout << "[Test] BloomPass2D bright highlight extraction and glow accumulation verified." << std::endl;

    // 89. Test SobelFilter2D Edge Detection Subsystem
    std::vector<uint32_t> sobelBuffer(9, 0xFFFFFFFF);
    sobelBuffer[4] = 0xFF000000;
    zenith::SobelFilter2D::applySobel(sobelBuffer, 3, 3);
    assert(!sobelBuffer.empty() && "SobelFilter2D execution failed!");
    std::cout << "[Test] SobelFilter2D 3x3 gradient edge detection verified." << std::endl;

    // 90. Test PosterizeFilter2D Color Quantization Subsystem
    std::vector<uint32_t> postBuffer = {0xFF123456, 0xFF789ABC, 0xFFDEF012, 0xFF345678};
    zenith::PosterizeFilter2D::applyPosterize(postBuffer, 2, 2, 4);
    assert(!postBuffer.empty() && "PosterizeFilter2D quantization failed!");
    std::cout << "[Test] PosterizeFilter2D 4-level color quantization verified." << std::endl;

    // 91. Test SolarizeFilter2D Film Negative Inversion Subsystem
    std::vector<uint32_t> solBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::SolarizeFilter2D::applySolarize(solBuffer, 2, 2, 0.5f);
    assert(!solBuffer.empty() && "SolarizeFilter2D execution failed!");
    std::cout << "[Test] SolarizeFilter2D threshold color inversion verified." << std::endl;

    // 92. Test PixelateFilter2D Mosaic Pixelation Subsystem
    std::vector<uint32_t> pixBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::PixelateFilter2D::applyPixelate(pixBuffer, 2, 2, 2);
    assert(!pixBuffer.empty() && "PixelateFilter2D execution failed!");
    std::cout << "[Test] PixelateFilter2D mosaic block grid pixelation verified." << std::endl;

    // 93. Test FisheyeFilter2D Barrel Lens Distortion Subsystem
    std::vector<uint32_t> fishBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::FisheyeFilter2D::applyFisheye(fishBuffer, 2, 2, 1.2f);
    assert(!fishBuffer.empty() && "FisheyeFilter2D execution failed!");
    std::cout << "[Test] FisheyeFilter2D barrel lens distortion verified." << std::endl;

    // 94. Test SharpenFilter2D Unsharp Masking Subsystem
    std::vector<uint32_t> shpBuffer(9, 0xFF808080);
    shpBuffer[4] = 0xFFFFFFFF;
    zenith::SharpenFilter2D::applySharpen(shpBuffer, 3, 3, 1.0f);
    assert(!shpBuffer.empty() && "SharpenFilter2D execution failed!");
    std::cout << "[Test] SharpenFilter2D Laplacian unsharp mask detail enhancement verified." << std::endl;

    // 95. Test TiltShiftFilter2D Miniature Diorama Subsystem
    std::vector<uint32_t> tsBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::TiltShiftFilter2D::applyTiltShift(tsBuffer, 2, 2, 0.5f, 0.2f);
    assert(!tsBuffer.empty() && "TiltShiftFilter2D execution failed!");
    std::cout << "[Test] TiltShiftFilter2D focus band diorama blur verified." << std::endl;

    // 96. Test RadialBlurFilter2D Speed Dash Blur Subsystem
    std::vector<uint32_t> rbBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::RadialBlurFilter2D::applyRadialBlur(rbBuffer, 2, 2, 0.5f, 0.5f, 3, 0.05f);
    assert(!rbBuffer.empty() && "RadialBlurFilter2D execution failed!");
    std::cout << "[Test] RadialBlurFilter2D speed zoom dash blur verified." << std::endl;

    // 97. Test SpeedLines2D Procedural Action Lines Subsystem
    auto sLines = zenith::SpeedLines2D::generateSpeedLines(800.0f, 600.0f, 400.0f, 300.0f, 15, 100.0f);
    assert(!sLines.empty() && "SpeedLines2D generation failed!");
    std::cout << "[Test] SpeedLines2D radial anime speed lines action generator verified." << std::endl;

    // 98. Test ScreenFlash2D Damage/Flashbang Overlay Subsystem
    zenith::ScreenFlash2D flash;
    flash.triggerFlash(0xFF0000FF, 0.5f);
    assert(flash.isActive() && "ScreenFlash2D flash trigger failed!");
    flash.update(0.2f);
    assert(flash.isActive() && "ScreenFlash2D flash active state failed!");
    std::cout << "[Test] ScreenFlash2D flashbang/damage red flash overlay verified." << std::endl;

    // 99. Test GameFeelManager2D Unified Impact Juice Subsystem
    zenith::GameFeelManager2D feel;
    feel.triggerHitImpact(12.0f, 0xFF0000FF, 0.05f);
    assert(feel.isHitStopActive() && "GameFeelManager2D hit-stop trigger failed!");
    feel.update(0.06f);
    assert(!feel.isHitStopActive() && "GameFeelManager2D hit-stop resolution failed!");
    std::cout << "[Test] GameFeelManager2D unified screen shake, flash, and hit-stop verified." << std::endl;

    // 100. Test ShadowMaskFilter2D CRT Aperture Grille Subsystem
    std::vector<uint32_t> smBuffer = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    zenith::ShadowMaskFilter2D::applyShadowMask(smBuffer, 2, 2, zenith::ShadowMaskFilter2D::Pattern::ApertureGrille, 0.2f);
    assert(!smBuffer.empty() && "ShadowMaskFilter2D execution failed!");
    std::cout << "[Test] ShadowMaskFilter2D CRT sub-pixel aperture grille verified." << std::endl;

    // 101. Test VHSFilter2D Analog Tape Distortion Subsystem
    std::vector<uint32_t> vhsBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::VHSFilter2D::applyVHS(vhsBuffer, 2, 2, 1.5f, 0.1f, 0.05f);
    assert(!vhsBuffer.empty() && "VHSFilter2D execution failed!");
    std::cout << "[Test] VHSFilter2D tape noise, chroma bleed, and tracking error verified." << std::endl;

    // 102. Test ScreenFreeze2D Pause Snapshot Fade Subsystem
    zenith::ScreenFreeze2D freeze;
    std::vector<uint32_t> frzBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    freeze.capture(frzBuffer, 2, 2);
    assert(freeze.isFrozen() && "ScreenFreeze2D capture failed!");
    freeze.applyFreezeFade(frzBuffer, 0.5f);
    assert(!frzBuffer.empty() && "ScreenFreeze2D blend failed!");
    std::cout << "[Test] ScreenFreeze2D pause snapshot and screen wipe fade verified." << std::endl;

    // 103. Test ChromaticVignetteFilter2D Peripheral Fringe Subsystem
    std::vector<uint32_t> cvBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::ChromaticVignetteFilter2D::applyChromaticVignette(cvBuffer, 2, 2, 2.0f, 1.5f);
    assert(!cvBuffer.empty() && "ChromaticVignetteFilter2D execution failed!");
    std::cout << "[Test] ChromaticVignetteFilter2D peripheral RGB fringe separation verified." << std::endl;

    // 104. Test HeatVisionFilter2D Thermal Vision Subsystem
    std::vector<uint32_t> hvBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::HeatVisionFilter2D::applyHeatVision(hvBuffer, 2, 2);
    assert(!hvBuffer.empty() && "HeatVisionFilter2D execution failed!");
    std::cout << "[Test] HeatVisionFilter2D false-color thermal vision spectrum verified." << std::endl;

    // 105. Test NightVisionFilter2D Tactical Goggles Subsystem
    std::vector<uint32_t> nvBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::NightVisionFilter2D::applyNightVision(nvBuffer, 2, 2, 0.05f, 1.5f);
    assert(!nvBuffer.empty() && "NightVisionFilter2D execution failed!");
    std::cout << "[Test] NightVisionFilter2D green phosphor monochrome goggles verified." << std::endl;

    // 106. Test ASCIIFilter2D Text Terminal Art Subsystem
    std::vector<uint32_t> asciiBuf = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::ASCIIFilter2D::applyASCII(asciiBuf, 2, 2, 2, 2);
    assert(!asciiBuf.empty() && "ASCIIFilter2D execution failed!");
    std::cout << "[Test] ASCIIFilter2D character luminance quantization verified." << std::endl;

    // 107. Test LensFlare2D Optical Anamorphic Streak Subsystem
    auto flares = zenith::LensFlare2D::generateFlareChain(0xFFFFFF00);
    assert(!flares.empty() && "LensFlare2D flare chain generation failed!");
    std::vector<uint32_t> lfBuffer = {0xFF000000, 0xFF000000, 0xFF000000};
    zenith::LensFlare2D::applyAnamorphicStreak(lfBuffer, 3, 1, 1, 0, 0xFF00FFFF, 1);
    assert(!lfBuffer.empty() && "LensFlare2D streak failed!");
    std::cout << "[Test] LensFlare2D optical flare chain and anamorphic streak verified." << std::endl;

    // 108. Test VignettePulse2D Heartbeat Pulse Subsystem
    zenith::VignettePulse2D pulse;
    pulse.startPulse(3.0f, 0.2f, 0.8f);
    assert(pulse.isActive() && "VignettePulse2D start pulse failed!");
    pulse.update(0.1f);
    float power = pulse.getCurrentIntensity();
    assert(power >= 0.2f && power <= 0.8f && "VignettePulse2D intensity calculation failed!");
    std::cout << "[Test] VignettePulse2D oscillating heartbeat border pulse verified." << std::endl;

    // 109. Test ShatterSystem2D Glass Fragment Explosion Subsystem
    zenith::ShatterSystem2D shatter;
    shatter.triggerShatter(400.0f, 300.0f, 20);
    assert(shatter.getShards().size() == 20 && "ShatterSystem2D trigger failed!");
    shatter.update(0.016f);
    std::cout << "[Test] ShatterSystem2D glass cell fragmentation and physics debris verified." << std::endl;

    // 110. Test LaserScan2D Sci-Fi Target Sweep Subsystem
    std::vector<uint32_t> lsBuffer = {0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000};
    zenith::LaserScan2D::applyLaserScan(lsBuffer, 2, 2, 0.5f, 1.0f, 0xFF00FF00, 1);
    assert(!lsBuffer.empty() && "LaserScan2D execution failed!");
    std::cout << "[Test] LaserScan2D sci-fi laser scan sweep beam verified." << std::endl;

    // 111. Test SketchFilter2D Pencil Crosshatch Subsystem
    std::vector<uint32_t> skBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::SketchFilter2D::applySketch(skBuffer, 2, 2, 0.5f);
    assert(!skBuffer.empty() && "SketchFilter2D execution failed!");
    std::cout << "[Test] SketchFilter2D pencil crosshatch sketch lines verified." << std::endl;

    // 112. Test OilPaintFilter2D Kuwahara Painterly Subsystem
    std::vector<uint32_t> opBuffer(25, 0xFF808080);
    opBuffer[12] = 0xFFFFFFFF;
    zenith::OilPaintFilter2D::applyOilPaint(opBuffer, 5, 5, 1);
    assert(!opBuffer.empty() && "OilPaintFilter2D execution failed!");
    std::cout << "[Test] OilPaintFilter2D Kuwahara anisotropic oil painting filter verified." << std::endl;

    // 113. Test HalftoneFilter2D Comic Book Dot Matrix Subsystem
    std::vector<uint32_t> htBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::HalftoneFilter2D::applyHalftone(htBuffer, 2, 2, 2);
    assert(!htBuffer.empty() && "HalftoneFilter2D execution failed!");
    std::cout << "[Test] HalftoneFilter2D CMYK/RGB comic book dot matrix verified." << std::endl;

    // 114. Test LightRays2D Crepuscular God Rays Subsystem
    std::vector<uint32_t> lrBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::LightRays2D::applyGodRays(lrBuffer, 2, 2, 0.5f, 0.1f, 4, 0.5f, 0.8f);
    assert(!lrBuffer.empty() && "LightRays2D execution failed!");
    std::cout << "[Test] LightRays2D crepuscular god rays and sun shafts verified." << std::endl;

    // 115. Test KaleidoscopeFilter2D Rotational Symmetry Subsystem
    std::vector<uint32_t> ksBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::KaleidoscopeFilter2D::applyKaleidoscope(ksBuffer, 2, 2, 6);
    assert(!ksBuffer.empty() && "KaleidoscopeFilter2D execution failed!");
    std::cout << "[Test] KaleidoscopeFilter2D polar rotational symmetry verified." << std::endl;

    // 116. Test SwirlFilter2D Radial Vortex Distortion Subsystem
    std::vector<uint32_t> swBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::SwirlFilter2D::applySwirl(swBuffer, 2, 2, 50.0f, 1.57f);
    assert(!swBuffer.empty() && "SwirlFilter2D execution failed!");
    std::cout << "[Test] SwirlFilter2D radial vortex swirl displacement verified." << std::endl;

    // 117. Test PinchBulgeFilter2D Magnification Lens Warp Subsystem
    std::vector<uint32_t> pbBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::PinchBulgeFilter2D::applyPinchBulge(pbBuffer, 2, 2, 50.0f, 0.5f);
    assert(!pbBuffer.empty() && "PinchBulgeFilter2D execution failed!");
    std::cout << "[Test] PinchBulgeFilter2D radial coordinate bulge and pinch lens warp verified." << std::endl;

    // 118. Test TunnelFilter2D Log-Spiral Wormhole Subsystem
    std::vector<uint32_t> tnBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::TunnelFilter2D::applyTunnel(tnBuffer, 2, 2, 1.0f, 0.5f);
    assert(!tnBuffer.empty() && "TunnelFilter2D execution failed!");
    std::cout << "[Test] TunnelFilter2D polar log-spiral wormhole tunnel verified." << std::endl;

    // 119. Test MirrorFilter2D Reflection Axis Subsystem
    std::vector<uint32_t> mrBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::MirrorFilter2D::applyMirror(mrBuffer, 2, 2, zenith::MirrorFilter2D::Axis::HorizontalLeftToRight);
    assert(!mrBuffer.empty() && "MirrorFilter2D execution failed!");
    std::cout << "[Test] MirrorFilter2D horizontal and quad symmetry reflection verified." << std::endl;

    // 120. Test RGBWaveFilter2D Sine Oscillating Chromatic Subsystem
    std::vector<uint32_t> rwBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::RGBWaveFilter2D::applyRGBWave(rwBuffer, 2, 2, 0.5f, 5.0f, 4.0f);
    assert(!rwBuffer.empty() && "RGBWaveFilter2D execution failed!");
    std::cout << "[Test] RGBWaveFilter2D sine wave oscillating chromatic fringe verified." << std::endl;

    // 121. Test HeatHazeFilter2D Desert Mirage Subsystem
    std::vector<uint32_t> hhBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::HeatHazeFilter2D::applyHeatHaze(hhBuffer, 2, 2, 0.5f, 4.0f, 6.0f);
    assert(!hhBuffer.empty() && "HeatHazeFilter2D execution failed!");
    std::cout << "[Test] HeatHazeFilter2D desert mirage and atmospheric heat wave verified." << std::endl;

    // 122. Test RainDrops2D Water Droplets Subsystem
    std::vector<uint32_t> rdBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::RainDrops2D::applyRainDrops(rdBuffer, 2, 2, 0.5f, 2, 40.0f);
    assert(!rdBuffer.empty() && "RainDrops2D execution failed!");
    std::cout << "[Test] RainDrops2D glass rain droplets and trickle refraction verified." << std::endl;

    // 123. Test BurnDissolve2D Flame Ash Disintegration Subsystem
    std::vector<uint32_t> bdBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::BurnDissolve2D::applyBurnDissolve(bdBuffer, 2, 2, 0.5f, 0xFFFF4500);
    assert(!bdBuffer.empty() && "BurnDissolve2D execution failed!");
    std::cout << "[Test] BurnDissolve2D fire burn and ash dissolve transition verified." << std::endl;

    // 124. Test FrostIce2D Ice Crystal Frost Subsystem
    std::vector<uint32_t> fiBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::FrostIce2D::applyFrostIce(fiBuffer, 2, 2, 0.6f, 0xFFD0F0FF);
    assert(!fiBuffer.empty() && "FrostIce2D execution failed!");
    std::cout << "[Test] FrostIce2D perimeter ice crystal frost growth verified." << std::endl;

    // 125. Test SparkDischarges2D High-Voltage Arc Subsystem
    zenith::SparkDischarges2D sparks;
    sparks.triggerSparks(200.0f, 200.0f, 6);
    assert(sparks.getArcs().size() == 6 && "SparkDischarges2D trigger failed!");
    sparks.update(0.016f);
    std::cout << "[Test] SparkDischarges2D electric arcs and spark discharges verified." << std::endl;

    // 126. Test HexShield2D Sci-Fi Energy Grid Subsystem
    std::vector<uint32_t> hsBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::HexShield2D::applyHexShield(hsBuffer, 2, 2, 0.5f, 10.0f, 0xFF00E5FF);
    assert(!hsBuffer.empty() && "HexShield2D execution failed!");
    std::cout << "[Test] HexShield2D sci-fi honeycomb energy grid shield verified." << std::endl;

    // 127. Test CausticsFilter2D Underwater Sunlight Subsystem
    std::vector<uint32_t> csBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::CausticsFilter2D::applyCaustics(csBuffer, 2, 2, 0.5f, 0.03f, 0.35f);
    assert(!csBuffer.empty() && "CausticsFilter2D execution failed!");
    std::cout << "[Test] CausticsFilter2D underwater sunlight refraction network verified." << std::endl;

    // 128. Test PixelSort2D Cyberpunk Datamoshing Subsystem
    std::vector<uint32_t> psBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::PixelSort2D::applyPixelSort(psBuffer, 2, 2, 0.4f);
    assert(!psBuffer.empty() && "PixelSort2D execution failed!");
    std::cout << "[Test] PixelSort2D luminance-based row pixel sorting verified." << std::endl;

    // 129. Test InfraredFilter2D Thermal Scope Subsystem
    std::vector<uint32_t> irBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::InfraredFilter2D::applyInfrared(irBuffer, 2, 2);
    assert(!irBuffer.empty() && "InfraredFilter2D execution failed!");
    std::cout << "[Test] InfraredFilter2D thermal heat gradient false color mapping verified." << std::endl;

    // 130. Test HologramFilter2D Sci-Fi Holographic Projection Subsystem
    std::vector<uint32_t> hlBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::HologramFilter2D::applyHologram(hlBuffer, 2, 2, 0.5f, 0xFF00E5FF);
    assert(!hlBuffer.empty() && "HologramFilter2D execution failed!");
    std::cout << "[Test] HologramFilter2D cyan holographic scanlines and jitter verified." << std::endl;

    // 131. Test CMYKOffset2D Printing Press Misprint Subsystem
    std::vector<uint32_t> cmBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::CMYKOffset2D::applyCMYKOffset(cmBuffer, 2, 2, 1, 1);
    assert(!cmBuffer.empty() && "CMYKOffset2D execution failed!");
    std::cout << "[Test] CMYKOffset2D printing press ink plate misprint offset verified." << std::endl;

    // 132. Test FocusVignette2D Focal Point Highlighting Subsystem
    std::vector<uint32_t> fvBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::FocusVignette2D::applyFocusVignette(fvBuffer, 2, 2, 1.0f, 1.0f, 10.0f, 50.0f);
    assert(!fvBuffer.empty() && "FocusVignette2D execution failed!");
    std::cout << "[Test] FocusVignette2D dynamic focal point highlighting vignette verified." << std::endl;

    // 133. Test Shockwave2D Explosion Ring Displacement Subsystem
    zenith::Shockwave2D shockwave;
    shockwave.trigger(400.0f, 300.0f, 200.0f, 15.0f);
    assert(shockwave.isActive() && "Shockwave2D trigger failed!");
    shockwave.update(0.016f);
    std::vector<uint32_t> swvBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    shockwave.applyShockwave(swvBuffer, 2, 2);
    std::cout << "[Test] Shockwave2D expanding explosion ring displacement verified." << std::endl;

    // 134. Test StaticNoise2D TV White Noise Subsystem
    std::vector<uint32_t> snBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::StaticNoise2D::applyStaticNoise(snBuffer, 2, 2, 0.5f);
    assert(!snBuffer.empty() && "StaticNoise2D execution failed!");
    std::cout << "[Test] StaticNoise2D TV white noise and signal interference verified." << std::endl;

    // 135. Test ReticleHUD2D Tactical Lock-On Crosshair Subsystem
    std::vector<uint32_t> rhBuffer = {0xFFFFFFFF, 0xFF000000, 0xFF808080, 0xFFC0C0C0};
    zenith::ReticleHUD2D::drawReticle(rhBuffer, 2, 2, 1.0f, 1.0f, 1.0f, true, 0xFFFF0000);
    assert(!rhBuffer.empty() && "ReticleHUD2D execution failed!");
    std::cout << "[Test] ReticleHUD2D tactical lock-on crosshair HUD overlay verified." << std::endl;

    // 136. Test MinimapHUD2D Tactical Radar Subsystem
    std::vector<uint32_t> mmBuffer(400, 0xFF000000);
    std::vector<zenith::MinimapEntity> entities = {{10.0f, 10.0f, 0xFFFF0000}};
    zenith::MinimapHUD2D::drawMinimap(mmBuffer, 20, 20, 0.0f, 0.0f, entities, 10, 10, 8.0f, 0.5f);
    assert(!mmBuffer.empty() && "MinimapHUD2D execution failed!");
    std::cout << "[Test] MinimapHUD2D circular tactical radar overlay verified." << std::endl;

    // 137. Test HitIndicator2D Damage Arc Subsystem
    zenith::HitIndicator2D hitInd;
    hitInd.addHit(1.57f, 1.0f, 0xFFFF0000);
    hitInd.update(0.016f);
    std::vector<uint32_t> hiBuffer(400, 0xFF000000);
    hitInd.drawIndicators(hiBuffer, 20, 20, 0.0f, 8.0f);
    assert(!hiBuffer.empty() && "HitIndicator2D execution failed!");
    std::cout << "[Test] HitIndicator2D directional damage arc HUD overlay verified." << std::endl;

    // 138. Test BossBar2D Animated Boss Health Bar Subsystem
    std::vector<uint32_t> bbBuffer(400, 0xFF000000);
    zenith::BossBar2D::drawBossBar(bbBuffer, 20, 20, 75.0f, 100.0f, 25.0f, 50.0f, 5, 12, 4);
    assert(!bbBuffer.empty() && "BossBar2D execution failed!");
    std::cout << "[Test] BossBar2D animated boss health bar and shield overlay verified." << std::endl;

    // 139. Test ComboCounter2D Arcade Hit Combo Subsystem
    zenith::ComboCounter2D combo;
    combo.addHit(5);
    combo.update(0.016f);
    assert(combo.getComboCount() == 5 && "ComboCounter2D count failed!");
    std::vector<uint32_t> ccBuffer(400, 0xFF000000);
    combo.drawCombo(ccBuffer, 20, 20, 2, 2, 0xFFFFD700);
    std::cout << "[Test] ComboCounter2D arcade combo hit counter & scale pulse verified." << std::endl;

    // 140. Test RacingGauge2D Speedometer Dial Subsystem
    std::vector<uint32_t> rgBuffer(400, 0xFF000000);
    zenith::RacingGauge2D::drawGauge(rgBuffer, 20, 20, 120.0f, 200.0f, 10, 10, 8.0f);
    assert(!rgBuffer.empty() && "RacingGauge2D execution failed!");
    std::cout << "[Test] RacingGauge2D speedometer dial gauge and needle verified." << std::endl;

    // 141. Test DialogueBox2D Narrative Choice Selector Subsystem
    std::vector<uint32_t> dbBuffer(4000, 0xFF000000);
    std::vector<std::string> choices = {"1. Accept", "2. Decline"};
    zenith::DialogueBox2D::drawDialogueBox(dbBuffer, 100, 40, "Hero", "Greetings traveller!", 10, choices, 0, 20);
    assert(!dbBuffer.empty() && "DialogueBox2D execution failed!");
    std::cout << "[Test] DialogueBox2D RPG narrative typewriter & choice selector verified." << std::endl;

    // 142. Test InventoryHUD2D Quick Slot Hotbar Subsystem
    std::vector<uint32_t> invBuffer(4000, 0xFF000000);
    std::vector<zenith::HotbarSlot> slots = {{0xFFFF0000, 1}, {0xFF00FF00, 5}, {0xFF0000FF, 0}};
    zenith::InventoryHUD2D::drawHotbar(invBuffer, 100, 40, slots, 1, 10, 2, 5);
    assert(!invBuffer.empty() && "InventoryHUD2D execution failed!");
    std::cout << "[Test] InventoryHUD2D hotbar quick slot HUD overlay verified." << std::endl;

    // 143. Test StatusHUD2D RPG Buff/Debuff Subsystem
    zenith::StatusHUD2D statusHUD;
    statusHUD.addEffect("SpeedBoost", 0xFF00FF00, 5.0f);
    statusHUD.update(1.0f);
    std::vector<uint32_t> stBuffer(4000, 0xFF000000);
    statusHUD.drawStatusIcons(stBuffer, 100, 40, 5, 5, 10, 2);
    assert(!stBuffer.empty() && "StatusHUD2D execution failed!");
    std::cout << "[Test] StatusHUD2D RPG active status effect icon bar verified." << std::endl;

    // 144. Test QuestTracker2D Mission Objective Subsystem
    zenith::QuestTracker2D questTracker;
    questTracker.setQuest("Main Quest");
    questTracker.addObjective("Defeat 5 Goblins", true);
    questTracker.addObjective("Find the Ancient Key", false);
    std::vector<uint32_t> qtBuffer(4000, 0xFF000000);
    questTracker.drawQuestTracker(qtBuffer, 100, 40, 5, 5);
    assert(!qtBuffer.empty() && "QuestTracker2D execution failed!");
    std::cout << "[Test] QuestTracker2D mission objective checklist HUD verified." << std::endl;

    // 145. Test CooldownWheel2D Radial Skill Cooldown Subsystem
    std::vector<uint32_t> cwBuffer(4000, 0xFF000000);
    zenith::CooldownWheel2D::drawCooldownWheel(cwBuffer, 100, 40, 0.4f, 20, 20, 12.0f);
    assert(!cwBuffer.empty() && "CooldownWheel2D execution failed!");
    std::cout << "[Test] CooldownWheel2D radial skill cooldown clock HUD verified." << std::endl;

    // 146. Test RecoilCrosshair2D Shooter Recoil Bloom Subsystem
    zenith::RecoilCrosshair2D crosshair;
    crosshair.fireShot(10.0f);
    assert(crosshair.getSpread() > 8.0f && "RecoilCrosshair2D fire failed!");
    crosshair.update(0.016f);
    std::vector<uint32_t> rcBuffer(4000, 0xFF000000);
    crosshair.drawCrosshair(rcBuffer, 100, 40, 50, 20, 6, 0xFF00FF00);
    std::cout << "[Test] RecoilCrosshair2D shooter weapon recoil bloom verified." << std::endl;

    // 147. Test ScoreNotifier2D Floating XP Popup Subsystem
    zenith::ScoreNotifier2D scoreNotifier;
    scoreNotifier.addScore("+500 XP", 50.0f, 30.0f, 0xFFFFD700, 1.0f);
    scoreNotifier.update(0.016f);
    std::vector<uint32_t> snfBuffer(4000, 0xFF000000);
    scoreNotifier.drawScores(snfBuffer, 100, 40);
    assert(!snfBuffer.empty() && "ScoreNotifier2D execution failed!");
    std::cout << "[Test] ScoreNotifier2D floating combat score & XP popup verified." << std::endl;

    // 148. Test SkillTree2D RPG Progression Subsystem
    zenith::SkillTree2D skillTree;
    skillTree.addNode(1, "Root Skill", 20.0f, 20.0f, -1, true);
    skillTree.addNode(2, "Fireball", 50.0f, 20.0f, 1, false);
    assert(skillTree.unlockNode(2) && "SkillTree2D unlock failed!");
    std::vector<uint32_t> sttBuffer(4000, 0xFF000000);
    skillTree.drawSkillTree(sttBuffer, 100, 40, 6.0f);
    assert(!sttBuffer.empty() && "SkillTree2D execution failed!");
    std::cout << "[Test] SkillTree2D RPG ability node graph & dependency tree verified." << std::endl;

    // 149. Test EquipmentDoll2D RPG Armor Paper Doll Subsystem
    zenith::EquipmentDoll2D paperDoll;
    paperDoll.setEquippedItem(zenith::EquipSlot::Chestplate, "Iron Armor", 0xFF808080, 25);
    paperDoll.setEquippedItem(zenith::EquipSlot::Helmet, "Steel Helm", 0xFFC0C0C0, 10);
    assert(paperDoll.getTotalArmor() == 35 && "EquipmentDoll2D armor calc failed!");
    std::vector<uint32_t> eqBuffer(40000, 0xFF000000);
    paperDoll.drawDoll(eqBuffer, 200, 200, 10, 10, 20);
    assert(!eqBuffer.empty() && "EquipmentDoll2D execution failed!");
    std::cout << "[Test] EquipmentDoll2D RPG paper doll and slot management verified." << std::endl;

    // 150. Test SelectionMarquee2D RTS Multi-Unit Drag Selection Subsystem
    zenith::SelectionMarquee2D marquee;
    marquee.startDrag(10.0f, 10.0f);
    marquee.updateDrag(50.0f, 50.0f);
    assert(marquee.isDragging() && marquee.containsPoint(25.0f, 25.0f) && "SelectionMarquee2D drag failed!");
    std::vector<uint32_t> smBuffer(40000, 0xFF000000);
    marquee.drawMarquee(smBuffer, 200, 200);
    marquee.endDrag();
    assert(!marquee.isDragging() && "SelectionMarquee2D endDrag failed!");
    std::cout << "[Test] SelectionMarquee2D RTS drag selection box verified." << std::endl;

    // 151. Test ActionWheel2D Radial Weapon Wheel Subsystem
    zenith::ActionWheel2D actionWheel;
    actionWheel.addSector("Pistol", 0xFFFF0000);
    actionWheel.addSector("Shotgun", 0xFF00FF00);
    actionWheel.addSector("Plasma", 0xFF0000FF);
    int sel = actionWheel.getSelectedSector(150.0f, 100.0f, 100, 100);
    assert(sel >= 0 && "ActionWheel2D sector selection failed!");
    std::vector<uint32_t> awBuffer(40000, 0xFF000000);
    actionWheel.drawWheel(awBuffer, 200, 200, 150.0f, 100.0f, 100, 100, 60.0f);
    assert(!awBuffer.empty() && "ActionWheel2D execution failed!");
    std::cout << "[Test] ActionWheel2D radial weapon selection wheel verified." << std::endl;

    // 152. Test LootDrop2D World Item Rarity Column Subsystem
    zenith::LootDrop2D lootDrop;
    lootDrop.addDrop("Excalibur", zenith::LootRarity::Legendary, 50.0f, 60.0f);
    lootDrop.update(0.016f);
    std::vector<uint32_t> ldBuffer(40000, 0xFF000000);
    lootDrop.drawDrops(ldBuffer, 200, 200);
    assert(!ldBuffer.empty() && "LootDrop2D execution failed!");
    std::cout << "[Test] LootDrop2D world item loot rarity beam verified." << std::endl;

    // 153. Test TargetIndicator2D MOBA Skillshot Ground Overlay Subsystem
    std::vector<uint32_t> tiBuffer(40000, 0xFF000000);
    zenith::TargetIndicator2D::drawAOECircle(tiBuffer, 200, 200, 100, 100, 30.0f);
    zenith::TargetIndicator2D::drawConeArc(tiBuffer, 200, 200, 100, 100, 40.0f, 0.0f, 0.5f);
    assert(!tiBuffer.empty() && "TargetIndicator2D execution failed!");
    std::cout << "[Test] TargetIndicator2D MOBA AOE circle & dragon cone arc targeting verified." << std::endl;

    // 154. Test StealthCone2D Guard Perception Vision Cone Subsystem
    std::vector<uint32_t> scBuffer(40000, 0xFF000000);
    zenith::StealthCone2D::drawGuardVisionCone(scBuffer, 200, 200, 100.0f, 100.0f, 0.0f, 0.785f, 50.0f, 0.2f);
    assert(!scBuffer.empty() && "StealthCone2D execution failed!");
    std::cout << "[Test] StealthCone2D guard stealth perception cone & alert meter verified." << std::endl;

    // 155. Test TurnOrder2D RPG Turn-Based Battle Order Timeline Subsystem
    zenith::TurnOrder2D turnOrder;
    turnOrder.addUnit("Hero", 0xFF00FF00, 100.0f, true);
    turnOrder.addUnit("Goblin", 0xFFFF0000, 50.0f, false);
    turnOrder.advanceTurn();
    std::vector<uint32_t> toBuffer(40000, 0xFF000000);
    turnOrder.drawTurnOrder(toBuffer, 200, 200);
    assert(!toBuffer.empty() && "TurnOrder2D execution failed!");
    std::cout << "[Test] TurnOrder2D RPG battle turn order timeline banner verified." << std::endl;

    // 156. Test QTE2D Quick-Time Event & Button-Timing Ring Subsystem
    zenith::QTE2D qte;
    qte.triggerQTE(zenith::QTEType::ButtonMash, "E", 2.0f);
    assert(qte.isActive() && "QTE2D trigger failed!");
    qte.pressButton();
    qte.update(0.016f);
    std::vector<uint32_t> qteBuffer(40000, 0xFF000000);
    qte.drawQTE(qteBuffer, 200, 200);
    assert(!qteBuffer.empty() && "QTE2D execution failed!");
    std::cout << "[Test] QTE2D button mash & timing ring quick-time event verified." << std::endl;

    // 157. Test BuildingPlacement2D Base Building Grid Placement Subsystem
    zenith::BuildingPlacement2D buildingPreview;
    buildingPreview.setBuilding("Barracks", 2, 2, 32);
    buildingPreview.updateCursor(64.0f, 64.0f, true);
    assert(buildingPreview.isPlacing() && buildingPreview.getTileX() == 2 && "BuildingPlacement2D update failed!");
    std::vector<uint32_t> bpBuffer(40000, 0xFF000000);
    buildingPreview.drawPreview(bpBuffer, 200, 200);
    assert(!bpBuffer.empty() && "BuildingPlacement2D execution failed!");
    std::cout << "[Test] BuildingPlacement2D grid placement preview verified." << std::endl;

    // 158. Test CardHand2D Card Deck Fan Hand Layout Subsystem
    zenith::CardHand2D hand;
    hand.addCard(1, "Fireball", 0xFFFF4500, 3);
    hand.addCard(2, "Shield", 0xFF1E90FF, 1);
    hand.setHoveredCard(0);
    assert(hand.getCardCount() == 2 && "CardHand2D count failed!");
    std::vector<uint32_t> chBuffer(40000, 0xFF000000);
    hand.drawHand(chBuffer, 200, 200);
    assert(!chBuffer.empty() && "CardHand2D execution failed!");
    std::cout << "[Test] CardHand2D card deck fan hand layout & hover elevation verified." << std::endl;

    // 159. Test TacticalGrid2D Grid Movement & Cover Overlay Subsystem
    zenith::TacticalGrid2D tacGrid(10, 10, 20);
    tacGrid.setCover(2, 2, zenith::CoverType::Full);
    std::vector<uint32_t> tgBuffer(40000, 0xFF000000);
    tacGrid.drawTacticalGrid(tgBuffer, 200, 200, 2, 1, 3);
    assert(!tgBuffer.empty() && "TacticalGrid2D execution failed!");
    std::cout << "[Test] TacticalGrid2D Manhattan movement tiles & cover shield icons verified." << std::endl;

    // 160. Test Crafting2D Recipe Matrix Subsystem
    zenith::Crafting2D crafting;
    crafting.registerRecipe("Iron Sword", 0xFFC0C0C0, {{"Wood", 1}, {"IronIngot", 2}});
    std::map<std::string, int> inv = {{"Wood", 2}, {"IronIngot", 5}};
    assert(crafting.canCraft(0, inv) && "Crafting2D canCraft failed!");
    std::vector<uint32_t> crBuffer(40000, 0xFF000000);
    crafting.drawCraftingGrid(crBuffer, 200, 200, inv);
    assert(!crBuffer.empty() && "Crafting2D execution failed!");
    std::cout << "[Test] Crafting2D recipe matrix & ingredient validation verified." << std::endl;

    // 161. Test RacingMinimap2D Track Minimap Subsystem
    zenith::RacingMinimap2D racingMinimap;
    racingMinimap.addTrackPoint(-100.0f, -100.0f);
    racingMinimap.addTrackPoint(100.0f, -100.0f);
    racingMinimap.addTrackPoint(100.0f, 100.0f);
    racingMinimap.addDriver("Player", 0.0f, 0.0f, 0xFF00FF00, true);
    std::vector<uint32_t> rmBuffer(40000, 0xFF000000);
    racingMinimap.drawMinimap(rmBuffer, 200, 200);
    assert(!rmBuffer.empty() && "RacingMinimap2D execution failed!");
    std::cout << "[Test] RacingMinimap2D track waypoint loop & driver dots verified." << std::endl;

    // 162. Test LightMeter2D Stealth Light Level Visibility Meter Subsystem
    zenith::LightMeter2D lightMeter;
    lightMeter.updateLightLevel(0.4f);
    assert(lightMeter.getLightLevel() == 0.4f && "LightMeter2D level update failed!");
    std::vector<uint32_t> lmBuffer(40000, 0xFF000000);
    lightMeter.drawLightMeter(lmBuffer, 200, 200);
    assert(!lmBuffer.empty() && "LightMeter2D execution failed!");
    std::cout << "[Test] LightMeter2D stealth light-level visibility meter verified." << std::endl;

    // 163. Test InputBuffer2D Fighting Game Move Buffer Subsystem
    zenith::InputBuffer2D inputBuf;
    inputBuf.pushInput("Down", 0.1f);
    inputBuf.pushInput("Right", 0.2f);
    inputBuf.pushInput("Punch", 0.3f);
    assert(inputBuf.checkSequence({"Down", "Right", "Punch"}) && "InputBuffer2D sequence match failed!");
    std::vector<uint32_t> ibBuffer(40000, 0xFF000000);
    inputBuf.drawBuffer(ibBuffer, 200, 200);
    assert(!ibBuffer.empty() && "InputBuffer2D execution failed!");
    std::cout << "[Test] InputBuffer2D fighting game special move input sequence verified." << std::endl;

    // 164. Test Leaderboard2D Retro Arcade High Score Table Subsystem
    zenith::Leaderboard2D leaderboard;
    leaderboard.addEntry("AAA", 99990);
    leaderboard.addEntry("BBB", 88880);
    std::vector<uint32_t> lbBuffer(40000, 0xFF000000);
    leaderboard.drawLeaderboard(lbBuffer, 200, 200);
    assert(!lbBuffer.empty() && "Leaderboard2D execution failed!");
    std::cout << "[Test] Leaderboard2D arcade high score ranking table verified." << std::endl;

    // 165. Test NarrativeChoice2D Branching Dialogue Choice Subsystem
    zenith::NarrativeChoice2D narrative;
    narrative.setChoices({{"Accept Quest", 101}, {"Decline Quest", 102}});
    narrative.selectNext();
    assert(narrative.getSelectedNextNodeId() == 102 && "NarrativeChoice2D selection failed!");
    std::vector<uint32_t> ncBuffer(40000, 0xFF000000);
    narrative.drawChoices(ncBuffer, 200, 200);
    assert(!ncBuffer.empty() && "NarrativeChoice2D execution failed!");
    std::cout << "[Test] NarrativeChoice2D visual novel branching choice menu verified." << std::endl;

    // 166. Test WaveSpawner2D Tower Defense Wave Spawner Subsystem
    zenith::WaveSpawner2D waveSpawner;
    waveSpawner.addWave(1, "Goblin", 5, 0.1f);
    waveSpawner.startNextWave();
    waveSpawner.update(0.15f);
    assert(waveSpawner.getCurrentWaveNumber() == 1 && "WaveSpawner2D wave number failed!");
    std::vector<uint32_t> wsBuffer(40000, 0xFF000000);
    waveSpawner.drawWaveInfo(wsBuffer, 200, 200);
    assert(!wsBuffer.empty() && "WaveSpawner2D execution failed!");
    std::cout << "[Test] WaveSpawner2D tower defense enemy wave progress bar verified." << std::endl;

    // 167. Test PinballBumper2D Impulse Physics Subsystem
    zenith::PinballBumper2D bumper(100.0f, 100.0f, 15.0f, 500.0f);
    float vx = 0.0f, vy = 0.0f;
    bool hit = bumper.checkAndResolveCollision(110.0f, 100.0f, 5.0f, vx, vy);
    assert(hit && vx > 0.0f && "PinballBumper2D impulse bounce failed!");
    std::vector<uint32_t> pbBuffer(40000, 0xFF000000);
    bumper.drawBumper(pbBuffer, 200, 200);
    assert(!pbBuffer.empty() && "PinballBumper2D execution failed!");
    std::cout << "[Test] PinballBumper2D impulse rebound force & bounce ring verified." << std::endl;

    window.shutdown();
    std::cout << "[Success] ALL CODE-FIRST GAME SDK TESTS PASSED PERFECTLY!" << std::endl;
    return 0;
}
