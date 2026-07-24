# Zenith Game-First Pivot Plan

## Why This Pivot Is Needed

Zenith already has the beginning of a game-oriented runtime in:

- `include/zenith/game/zenith_game.h`
- `include/zenith/game/zenith_physics.h`
- `include/zenith/game/zenith_resource.h`

But much of the current product surface is still dominated by:

- app/web targets
- UI component syntax and layout work
- native app shells
- docs and examples centered on applications instead of games

That split makes the language feel unfocused. If Zenith is meant to become a serious game SDK and language, then the engine model must become the center, and app/web/UI work must move to optional modules.

## Product Positioning

Zenith should not try to compete with Unity by building another editor-heavy workflow.

Zenith should compete on:

- code-first authoring
- stable APIs that do not break when tools change
- fast compile and hot-reload loops
- direct property access through code
- strong resource management
- built-in 2D and 3D runtime concepts
- predictable performance through data-oriented runtime design

### Core Promise

"Everything important is visible and controllable in code."

That includes:

- character state
- transform and hierarchy
- camera
- physics bodies and colliders
- renderable materials and meshes
- animation state
- resource lifetime
- audio sources and listeners

## What To Remove From The Core Direction

These should not define the main identity of Zenith:

- UI-first language design
- app framework positioning
- web framework positioning
- editor/inspector dependency
- low-code or no-code workflow
- hidden engine behavior behind generated glue

This does not mean deleting all web or app support immediately.

It means:

- move app/web/UI features into optional targets or packages
- stop treating them as the primary roadmap
- keep the language syntax centered on gameplay/runtime systems

## Design Principles

### 1. Code Is The Source Of Truth

No scene must require a visual editor to exist.

Scenes, prefabs, entities, resources, and behaviors should all be creatable from code:

```zenith
entity player = spawn Character {
    transform = Transform3D(position: vec3(0, 1, 0))
    camera = FollowCamera(fov: 75)
    body = RigidBody3D.dynamic(mass: 70)
    collider = CapsuleCollider(height: 1.8, radius: 0.35)
    mesh = MeshRenderer(mesh: "hero.glb", material: "hero.mat")
}
```

### 2. 2D And 3D Should Share The Same Mental Model

Do not create two unrelated engines.

Use one world model with shared concepts:

- `Transform`
- `Camera`
- `Body`
- `Collider`
- `SpriteRenderer` and `MeshRenderer`
- `Light`
- `AudioSource`
- `Animator`

2D should feel like a constrained subset of the same engine, not a separate product.

### 3. Data-Oriented Runtime First

The runtime should optimize for cache-friendly updates and predictable frame timing.

Preferred direction:

- ECS or ECS-inspired storage
- structure-of-arrays layout for hot components
- explicit update phases
- typed handles instead of string lookups in frame-critical paths
- minimal heap churn during gameplay

### 4. Tooling Supports Code, Not Replaces It

Unity hides a lot of behavior behind editor workflows and generated bindings.

Zenith should do the opposite:

- tooling helps generate repetitive code
- runtime stays explicit
- generated code is inspectable
- no hidden object magic required for basic features

### 5. Copilot Should Reduce Boilerplate, Not Replace Architecture

Your idea of adding an interpreter/copilot layer is good if used correctly.

Use it for:

- generating entity setup code
- creating material definitions
- scaffolding state machines
- wiring animation graphs
- producing import manifests
- generating tests and sample scenes

Do not use it as a hidden runtime dependency for basic gameplay execution.

## Recommended Language Shape

Zenith should become a systems-oriented gameplay language with a small, strong core.

### Keep

- fast parsing
- strong static typing
- type inference where safe
- direct native compilation
- hot reload / interpreter for iteration
- FFI for platform and middleware integration

### Add

- first-class vector and matrix types
- deterministic value types for math and transforms
- components and systems as language-level patterns
- resource handle types
- compile-time asset references
- build profiles for debug, dev, release
- better package/module boundaries for engine subsystems

### Reduce In Importance

- UI component syntax as a flagship feature
- app-style widgets in the main language pitch
- DOM/web generation as a headline capability

## Engine Architecture Direction

## 1. Core Runtime Layer

This should be the non-negotiable base:

- memory allocator interface
- job system
- input system
- time/frame scheduler
- logging and diagnostics
- file system abstraction
- asset database and hot reload watcher

## 2. World Layer

This is the gameplay model:

- entity IDs
- transforms
- parent/child hierarchy
- tags and layers
- scene loading and streaming
- prefab/archetype instancing

## 3. Simulation Layer

- rigid body 2D
- rigid body 3D
- collision queries
- raycast and sweep tests
- trigger volumes
- character controller
- navigation later, not first

Recommended order:

1. solid 2D physics
2. simple 3D collision and rigid bodies
3. advanced constraints later

## 4. Rendering Layer

Start narrow instead of chasing every target.

Phase 1 rendering goals:

- sprites
- tilemaps
- basic camera
- text rendering
- mesh rendering
- materials
- simple lighting
- debug draw

Avoid building a huge renderer before the gameplay model is stable.

## 5. Resource System

This is one of the most important differentiators.

Zenith should expose resources directly in code while still supporting fast workflows:

- typed asset handles
- async loading and streaming
- reference counting or explicit ownership
- resource groups/bundles
- hot reload in development
- memory budget tracking
- importer pipeline for textures, meshes, audio, shaders
- metadata sidecars generated at import time

Material definitions should also stay code-first. A material must be able to define named properties directly in code, including data/editor shapes such as text, number, toggle, radio, image, button/action, and color, so gameplay and tooling can both read the same explicit property model without hiding it behind an inspector.

That property model also needs explicit schema access from code: property count, indexed property names, property kind, label, and options must all be readable without depending on an editor-generated inspector layer.

Creating variants must stay code-first too. A scene should be able to clone a material, copy its property bag, remove a property, or clear a temporary schema directly from code when building fast runtime-driven material presets.

Radio-style properties need typed option access as well. Labels and values for each option should be reachable, mutable, and enumerable from code without collapsing back to a hidden inspector-only representation.

Button/action properties should also be executable from code. A material-defined action must be able to trigger a scene callback directly and expose its runtime trigger count so tooling stays optional while gameplay behavior remains explicit and inspectable.

The rest of the material property kinds should be first-class control channels too. Text, radio, image, color, toggle, and number properties should be able to declare explicit callbacks and notify scene code with their current typed value so teams can wire gameplay/runtime behavior in code instead of hiding it behind editor-generated inspector events.

Audio sources should follow the same rule: clips, volume, pitch, looping, play state, and 3D distance ranges must be attachable and mutable directly from gameplay code instead of being locked behind editor-only components.

Listeners should be just as explicit: primary listener selection, gain, enabled state, and listener placement must all remain visible in code so camera/audio behavior does not depend on fragile tool-side defaults.

Colliders need the same code-first treatment. Box, circle, capsule, offset, trigger, enabled, and overlap/query behavior should be attachable and inspectable from gameplay code instead of being hidden behind an editor inspector.

### Example Direction

```zenith
TextureHandle heroTex = assets.texture("characters/hero.png")
MeshHandle heroMesh = assets.mesh("characters/hero.glb")
AudioHandle jumpSfx = assets.audio("sfx/jump.wav")
```

Then at build time:

- validate paths
- bake import metadata
- compress or convert assets
- package target-specific runtime data

## 6. Camera Model

Camera should be a first-class engine concept, not an afterthought.

Need both:

- `Camera2D`
- `Camera3D`

Common properties:

- position
- rotation
- projection
- viewport
- clear mode
- layer mask
- follow/target behavior
- post-process stack later

This should be code-driven and attachable like any other component.

## 7. Character Model

A "character" should not be magic.

It should be a composition of:

- transform
- collider
- rigid body or kinematic body
- input controller
- animator
- renderer
- gameplay script
- audio source set

That will make both 2D and 3D characters predictable.

## Compiler And Runtime Strategy

Zenith should likely use a two-lane execution model:

### Lane A: Fast Iteration

- interpreter or JIT-like development mode
- hot reload for scripts and gameplay code
- live asset reload
- immediate diagnostics

### Lane B: Shipping Build

- ahead-of-time native compilation
- optimized code generation
- stripped diagnostics
- deterministic asset packaging

This is better than trying to make one runtime mode solve every need.

## Repository Refocus Plan

## Keep As Core

- compiler frontend
- native codegen/backend
- `include/zenith/game/*`
- runtime memory/concurrency foundations
- tests that validate language semantics and runtime correctness

## Move To Secondary/Optional Status

- UI-heavy syntax work
- web rendering backends
- app/demo shells
- SSR/site-oriented documentation
- widget libraries not needed for in-engine tooling

## Documentation Changes

Update the main story of the repo so the first impression is:

1. Zenith is a game SDK and language
2. everything important is code-first
3. 2D and 3D share one runtime model
4. web/app support is optional, not the center

## Suggested 4-Phase Roadmap

## Phase 1: Product Reset

Goal: stop roadmap drift.

Tasks:

- freeze new app/web/UI-first features
- define core engine vocabulary
- decide exact language identity
- write one canonical "Zenith game object model"
- mark non-core modules as experimental or optional

Deliverable:

- a revised README and roadmap aligned to game-first goals

## Phase 2: Minimal Vertical Slice

Goal: prove the architecture with one real playable example.

Build:

- scene
- entity/component model
- transform system
- camera
- sprite renderer
- basic 2D physics
- asset loading
- hot reload

Demo target:

- one small 2D game such as platformer, top-down action, or snake with real engine APIs

If this phase is not fun to use from code, the language design is not ready.

## Phase 3: Unified 3D Expansion

Goal: extend the same model into 3D without breaking 2D ergonomics.

Add:

- mesh renderer
- 3D transform and camera improvements
- material and shader system
- basic lighting
- 3D colliders
- character controller

Basic lighting should stay in the same code-first shape as the rest of the runtime. Point lights and directional lights need spawnable scene components with direct color, intensity, range, direction, enabled, and shadow properties instead of renderer-only hidden defaults.

3D colliders should follow the same rule: start with explicit box and sphere collider components whose size, offset, trigger state, enabled state, and overlap/point queries are all readable and writable from gameplay code.

Ray queries need to live in that same explicit model. A scene should be able to perform `raycast2D` and `raycast3D` directly from gameplay code and receive typed hit results with `hit`, `entity`, `distance`, `point`, and `normal` fields instead of pushing selection or picking logic into editor-only tools.

Character-shaped colliders should follow immediately after that. Zenith needs a code-first `CapsuleCollider2D` for player bodies so character collision stops being forced through box hacks and stays explicit in the same runtime API surface.

Demo target:

- one small 3D scene with moving camera, collisions, and asset streaming

## Phase 4: Productivity Layer

Goal: make the engine pleasant without becoming editor-dependent.

Add:

- CLI project scaffolding for game templates
- asset import commands
- codegen helpers for entities/resources
- testing harness for gameplay systems
- optional debug overlay
- optional minimal inspector for runtime debugging

Important:

The debug tools must remain optional. The engine must still make sense without them.

## Recommended Immediate Priorities For This Repo

Based on the current repository, the next best moves are:

1. Stop expanding the UI/app/web surface in the main roadmap.
2. Promote `include/zenith/game/*` into the center of the project.
3. Define a real entity-component runtime instead of loose helper classes.
4. Replace string-heavy runtime APIs with typed handles where performance matters.
5. Build one small playable sample that uses only code-first gameplay APIs.
6. Move documentation and examples away from widget-heavy messaging.
7. Keep the interpreter/hot reload idea, but treat it as a productivity feature, not the core runtime model.

## Final Recommendation

Your instinct is correct:

do not build another Unity clone with a different skin.

Build a language and SDK where:

- code is primary
- runtime concepts are explicit
- resources are first-class
- 2D and 3D share one model
- performance decisions are visible
- tooling helps, but never owns the project

If Zenith stays disciplined on that direction, it can become a much stronger identity than "another app framework that also tries to do games."
