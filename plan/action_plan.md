# Zenith Action Plan

Last updated: July 23, 2026

This is the execution roadmap for Zenith after the game-first repo pivot. The purpose of this document is to keep day-to-day development aligned with the main direction in `plan/game_sdk_pivot.md`.

## Main Identity

Zenith is a game SDK and language first.

The core of the project is:

- language frontend and static analysis
- native-first code generation
- explicit scene/entity/runtime APIs
- 2D and 3D gameplay systems
- typed resources and asset workflow
- fast developer iteration through watch/dev/hot-reload loops

The following are no longer main roadmap drivers:

- app-framework positioning
- web-framework positioning
- UI/widget-first language messaging
- SSR/site-oriented product storytelling
- low-code or no-code workflow goals

Web, app-shell, and UI-oriented surfaces can remain in the repo, but they should be treated as optional or compatibility lanes unless they directly support the game SDK direction.

## Current Baseline

Already present in the repo:

- compiler frontend with lexer, parser, semantic analysis, and formatter
- native, JS, and WASM code generators
- game runtime headers under `include/zenith/game/`
- scene, physics, resource, material-property, and productivity-layer work
- profile-aware compile/dev/watch flow
- typed language-side math and compile-time asset references

What still needs the most attention:

- cleaning up helper-style runtime paths into more direct data-oriented structures
- expanding the gameplay runtime until a small playable slice feels natural
- keeping docs and examples centered on gameplay instead of apps/widgets

## Active Priorities

### Priority 1: Language and Runtime Core

Goal: make Zenith feel like a real gameplay language, not a syntax layer over helper APIs.

Focus:

- first-class math/value types
- typed resource handles
- compile-time asset references
- explicit scene and component interaction
- direct property access in code

Success looks like:

- common gameplay code reads clearly without engine magic
- runtime-heavy code avoids string lookups in hot paths
- both native and web targets preserve the same language meaning for core value types

### Priority 2: World and Data Model

Goal: push the runtime toward a stronger shared 2D/3D world model.

Focus:

- entity IDs and hierarchy
- layer and mask rules
- scene streaming
- prefab/archetype instancing
- better ECS/data-oriented storage cleanup

Success looks like:

- 2D and 3D systems share vocabulary and lifecycle
- runtime state is inspectable from code without editor-only assumptions
- hot component paths become less helper-driven and more direct

### Priority 3: Simulation and Character Work

Goal: make gameplay movement and collision feel trustworthy.

Focus:

- stronger 2D and 3D body behavior
- capsule/box/sphere collider coverage
- overlap and ray query APIs
- full character-controller work, especially 3D movement/collision quality

Success looks like:

- a small sample game can move, collide, jump, and query the world using only code-first APIs
- player control does not depend on UI tooling or hidden setup

### Priority 4: Rendering and Visualization

Goal: support practical gameplay rendering without losing the code-first shape.

Focus:

- sprites
- tilemaps
- mesh rendering
- materials and shaders
- debug draw
- broader runtime visualization tools

Success looks like:

- 2D and 3D samples can render meaningful gameplay state
- runtime debug information is available without requiring a heavyweight editor

### Priority 5: Asset Pipeline and Memory Discipline

Goal: treat resources as first-class runtime systems.

Focus:

- import commands and metadata baking
- bundle/group workflow
- hot reload
- memory budgets
- typed material/resource properties

Success looks like:

- assets are addressable from code through stable handles
- build-time asset validation catches obvious mistakes early
- development reload loops are fast and visible

### Priority 6: Productivity Layer

Goal: make Zenith pleasant without becoming tool-owned.

Focus:

- CLI game scaffolding
- gameplay test harness
- optional debug overlay
- optional minimal inspector
- dev/watch flow

Success looks like:

- tooling saves time but is never required to understand the project
- the runtime still makes sense when used only from source code

## Optional and Secondary Work

These areas should stay behind the core gameplay/runtime lanes:

- DOM/web generation as headline marketing
- app-style widget systems as main language examples
- SSR/server-style demos
- app/demo shells that are not needed for gameplay runtime validation

Allowed work in these areas:

- compatibility fixes
- regression prevention
- optional target maintenance
- isolated experiments that do not drive the language identity

Not recommended:

- expanding the main README around widgets or app UX
- making web/app demos the canonical examples for Zenith
- building new roadmap phases around app-shell breadth before gameplay depth

## Immediate Execution Plan

### Phase 1: Repo Narrative Cleanup

Goal: make the repo say what the product is.

Tasks:

- keep the README game-first
- keep roadmap docs aligned with the pivot
- move app/web/UI-first messaging into clearly secondary sections
- prefer gameplay/runtime examples over widget examples

Deliverable:

- top-level docs consistently describe Zenith as a game SDK and language

### Phase 2: Minimal Playable Slice

Goal: prove the current runtime shape with a real game loop.

Tasks:

- one small playable sample
- scene setup through code
- movement and collision
- resource loading
- camera behavior
- debug visibility

Deliverable:

- a small game sample that uses only code-first gameplay APIs

### Phase 3: Runtime Hardening

Goal: reduce fragility and improve performance.

Tasks:

- typed handle propagation in hot paths
- ECS/data-oriented cleanup
- fewer helper-only indirection layers
- more consistent 2D/3D API surface

Deliverable:

- cleaner runtime ownership and more predictable gameplay update paths

### Phase 4: Tooling Polish

Goal: improve iteration speed without shifting ownership away from code.

Tasks:

- stronger dev/watch workflows
- asset-db integration
- better gameplay test ergonomics
- optional runtime inspection tools

Deliverable:

- fast iteration tooling that helps, but does not define, the engine

## Decision Rules

When choosing between tasks, prefer the one that:

1. improves the code-first gameplay/runtime path
2. strengthens shared 2D/3D concepts
3. reduces hidden behavior
4. improves resource or performance visibility
5. keeps tooling optional

Defer work that primarily:

1. improves app-shell polish
2. makes widget syntax a larger part of the identity
3. adds web/app breadth without improving the gameplay core

## Short Version

Build the language and runtime around games.

Keep scenes, characters, cameras, physics, rendering, and resources explicit in code.

Let tooling reduce friction, but never let tooling become the source of truth.
