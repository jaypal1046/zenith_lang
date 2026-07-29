# Zenith Feature Audit: Game Engine, ECS & Gameplay Systems

## 1. Executive Overview
Zenith contains a pure code-first game engine and Entity Component System (ECS) backed by continuous dense component pools (`ComponentPool<T>`), 2D/3D physics, spatial queries, scene lifecycle hooks, and over 100 specialized gameplay mechanic & 2D shader FX headers under `include/zenith/game/`.

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **High-Performance ECS (`ZenithWorld`)** ([zenith_world.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_world.h)):
  - 21 continuous dense component pools (`Transform2D/3D`, `RigidBody2D/3D`, `BoxCollider2D/3D`, `SphereCollider3D`, `CapsuleCollider2D/3D`, `SpriteRenderer2D`, `Tilemap2DComponent`, `Character2D/3DComponent`, `MeshRenderer3D`, `AudioSource2D/3D`, `Camera2D/3D`, `PointLight3D`, `DirectionalLight3D`, `RelationshipComponent`, `LayerMaskComponent`).
- **Physics Engine & Spatial Queries** ([zenith_physics.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_physics.h)):
  - 2D/3D rigid body dynamics, Euler integration, spatial hashing, raycast/sweep mask queries (`raycast2DMask`, `raycast3DMask`).
- **Scene Abstraction & View API** ([zenith_scene.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_scene.h)):
  - Lifecycle (`onLoad`, `onFixedUpdate`, `onUpdate`, `onDraw`), view proxies (`body2D`, `character3D`, etc.), prefabs (`instantiatePrefab2D/3D`), material variants (`cloneVariant`).
- **2D Gameplay Systems & Mechanics Subsystems**:
  - AI & Behavior: [zenith_behavior_tree.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_behavior_tree.h), [zenith_pathfinding.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_pathfinding.h), [zenith_steering2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_steering2d.h).
  - Inventory & Equipment: [zenith_inventory.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_inventory.h), [zenith_equipment_doll2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_equipment_doll2d.h), [zenith_crafting2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_crafting2d.h).
  - Quests & Dialogue: [zenith_dialogue.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_dialogue.h), [zenith_quest.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_quest.h), [zenith_skill_tree2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_skill_tree2d.h).
  - Audio & Sound Mixers: [zenith_audio.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_audio.h), [zenith_audio_mixer.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_audio_mixer.h), [zenith_procedural_audio.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_procedural_audio.h).
  - Physics & Simulation: [zenith_cloth2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_cloth2d.h), [zenith_ragdoll2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_ragdoll2d.h), [zenith_softbody2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_softbody2d.h), [zenith_fluid2d.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_fluid2d.h), [zenith_particles.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_particles.h).
- **Shader FX & Post-Processing Suite**:
  - Includes `zenith_bloom2d.h`, `zenith_blur2d.h`, `zenith_crt2d.h`, `zenith_digital_glitch2d.h`, `zenith_distortion2d.h`, `zenith_hologram2d.h`, `zenith_night_vision2d.h`, `zenith_oil_paint2d.h`, `zenith_pixel_sort2d.h`, `zenith_shockwave2d.h`, `zenith_vhs2d.h`, `zenith_vignette2d.h`.

---

## 3. ❌ Missing / Incomplete Features
- **Skeletal 3D Mesh Animation**: 3D skinned mesh skeletal bone hierarchy keyframe blending is missing.
- **Node-Based Shader Graph Compiler**: Custom shaders are bound by string handles rather than a node graph parser.

---

## 4. 💻 How to Use (Real Compiling Syntax)
```zenith
import std.io;

class GameLevel() implements Scene {
    Int playerEntity = 0;

    Void onLoad() {
        clearColor = "dark_slate";
    }

    Void onUpdate(dt: Float) {
        // Gameplay loop updates
    }
}
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File Location |
| :--- | :--- |
| **ECS Architecture & Component Pools** | [zenith_world.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_world.h) |
| **Physics & Query Engine** | [zenith_physics.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_physics.h) |
| **Scene Management & Views** | [zenith_scene.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_scene.h) |
| **Composite Game Header** | [zenith_game.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_game.h) |
| **Sample Game Reference** | [playable_slice.zen](file:///c:/Jay/_Plugin/zenith_lang/examples/game_sdk/playable_slice.zen) |
