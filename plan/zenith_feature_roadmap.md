# Zenith Feature Roadmap

A build-order sort of every feature discussed, from what's already working to what's genuinely optional. Check items off as you close them.

**How to use this:** work top to bottom. Don't start Tier 2 items until Tier 1 is done — a code-first engine with polish but no working game loop, save system, or tests is worse than a rough one that runs.

---

## Tier 0 — Already Built (per your audits)

Verified-by-audit, treat as done unless you hit a bug:
- [x] Lexer, parser, AST, semantic analyzer, Hindley-Milner type inference, code formatter
- [x] C++17 / JS (ES6) / WebAssembly codegen backends
- [x] Immediate-mode Canvas API (`drawRect`, `drawText`, `drawTextWrapped`, `pctX/pctY`)
- [x] Win32 native windowing backend
- [x] ECS `ZenithWorld` — 21 dense component pools
- [x] 2D/3D physics — rigid bodies, raycast/sweep queries, spatial hashing
- [x] Scene lifecycle (`onLoad`/`onUpdate`/`onFixedUpdate`/`onDraw`), prefabs, material variants
- [x] Asset handles + `.meta` sidecars + `zenith assets import/list` + hot reload watcher
- [x] Concurrency thread pool (`concurrency.hpp`)
- [x] Memory: `Ref<T>`/`Weak<T>`, `@gc_root`/`@managed`
- [x] Dynamic library FFI + CPython FFI
- [x] Windows/Android/iOS platform headers
- [x] CLI (`create`/`serve`/`build`/`assets`/`lsp`), LSP (diagnostics/autocomplete/hover/goto-def), VS Code extension
- [x] `agent`/`prompt`/`tool` keywords + AST + WinHTTP LLM connector
- [x] Zenith packages: `math.zen`, `collections.zen`, `crypto.zen`, `datetime.zen`, `network.zen`, `validation.zen`

**Needs verification before you count it as done** (audits named the file but no test exercises it):
- [ ] 100+ gameplay mechanic headers (dialogue, inventory, pathfinding, behavior trees, crafting, quests, etc.)
- [ ] 20 shader FX headers (bloom, CRT, glitch, hologram, etc.)
- [ ] DAP server (`zenith_dap_server.py`) — run `test_dap_validation.py` and confirm breakpoints actually halt execution, not just respond to the protocol
- [ ] `build_bridge.py` / `convert_dart.py` — confirm what these actually generate before relying on them for the Rust FFI bridge work below

---

## Tier 1 — v1: Ship One Real Game

Everything here blocks shipping a real game end-to-end. Nothing in Tier 2 matters if this tier isn't solid.

**Language / Compiler**
- [x] Pattern & struct destructuring (match arms)
- [x] `@deprecated` annotation + compiler warnings
- [x] SIMD codegen for Vec2/Vec3/Vec4/Mat4 (backend-only change, no new syntax)

**Runtime / Memory / Concurrency**
- [x] Frame/Arena allocator (`Temp<T>` scoped allocation)
- [x] Object pooling (first-class)
- [x] ECS parallel system scheduler (read/write component declarations → auto-parallel job graph)

**Rendering**
- [x] Cross-platform canvas backend (SDL2/GLFW) — replaces Win32-only real backend
- [x] 2D sprite batching
- [x] Texture atlas / sprite sheet packer
- [x] Sprite frame animation / flipbook

**Physics**
- [x] Collision layer matrix (code-defined)
- [x] Physics material system (friction/bounce presets)
- [x] Trigger vs. collision event separation
- [x] Joint & constraint system (hinge, spring, fixed)

**Input**
- [x] Unified input abstraction (keyboard/mouse/gamepad/touch)
- [x] Input action mapping & rebinding

**Camera / Animation**
- [x] Camera shake
- [x] Tween / easing engine

**State & Persistence**
- [x] App-level game state stack (menu/play/pause/game-over)
- [x] World snapshot / restore
- [x] Save / load system

**Testing**
- [x] Built-in test framework (`test` keyword + `assert_*` in stdlib, run via `zenith test`)
- [x] Headless deterministic execution mode (fixed timestep, no window)
- [x] Scripted input injection for automated test scenes

**Tooling**
- [x] Structured logging framework (levels/tags/filters)
- [x] In-game debug console / command system
- [x] Gizmo / debug draw API
- [x] `zenith doctor` (environment/SDK diagnostic command)
- [x] Compile-time asset reference validation (catch missing texture/mesh paths at build time)

**Asset**
- [x] `.zpak` production asset bundling

**Time**
- [x] Time scale / slow motion
- [x] Fixed vs. variable timestep config

**Localization**
- [x] Basic string table / localization system

---

## Tier 2 — v2: Production-Ready

Needed to ship to real stores/players, not needed for a working prototype.

**Rendering**
- [x] PBR material system (albedo/metallic/roughness)
- [x] Shadow mapping / cascaded shadow maps
- [x] Ambient occlusion (SSAO), anti-aliasing (MSAA/TAA/FXAA)
- [x] ASTC/KTX2 texture compression, SDF font rendering
- [x] Skeletal animation & blend trees, IK solver
- [x] Particle-world collision, decal system, skybox/fog volumes, color grading LUT
- [x] Quality presets, dynamic resolution scaling

**Audio**
- [x] 3D spatial audio (HRTF), reverb/occlusion zones
- [x] Adaptive music (state-based transitions)
- [x] Streaming audio chunks, compressed codec support (Vorbis/Opus)

**Physics / AI**
- [x] Continuous collision detection (CCD), ragdoll physics, vehicle physics
- [x] Character controller tuning (slope limit, step offset, ledge detection)
- [x] NavMesh generation, pathfinding
- [x] Utility AI / GOAP, flocking/crowd simulation, sensor/perception system

**Save / Networking**
- [x] Versioned save migration, incremental/delta save, save data encryption
- [x] Client-server netcode primitives, RPC syntax, state snapshot/interpolation
- [x] Reliable-UDP layer

**Platform Services**
- [x] Achievements/leaderboards bridge, in-app purchase API
- [x] Push notifications, deep linking
- [x] Crash reporting & symbolication
- [x] App lifecycle hooks (pause/resume/background/foreground)

**Dev Tooling**
- [x] Built-in profiler (CPU/GPU/memory)
- [x] Complete/verify DAP (real breakpoints, not just protocol responses)
- [x] Hot reload for native C++ target (via dev_watch.py & zenith run universal inline runner)

**Package Management**
- [x] `zenith install` / package registry client
- [x] Dependency resolver, lockfile system

**Accessibility / Security**
- [x] Subtitles, remappable controls, colorblind modes, text-to-speech
- [x] Signed shared libraries, script/bytecode obfuscation

**Serialization**
- [x] Binary serialization framework, schema-based data format

**Stdlib / FFI**
- [x] Rust FFI bridge — crypto/network/compression consolidated behind one cdylib (check `build_bridge.py` first)
- [x] Unified cross-platform networking transport (currently WinHTTP-only in practice)
- [x] SQLite / database bridge

---

## Tier 3 — Scale & Ecosystem

Matters once you have real games shipped and want Zenith to grow beyond you.

**Compiler / Advanced Rendering**
- [x] Direct LLVM IR backend, JIT execution engine
- [x] Macro system (compile-time codegen)
- [x] Node-based shader graph compiler, GPU compute dispatch, ray tracing support
- [x] Graphics API abstraction (Vulkan/Metal/DX12)

**Multiplayer**
- [x] Rollback netcode, dedicated/headless server build mode
- [x] Anti-cheat hooks, session/lobby reconnect handling

**Modding / Ecosystem**
- [x] Mod loader, sandboxed mod script execution
- [x] Community plugin API, package registry hosting infrastructure, plugin marketplace

**Live Ops**
- [x] Remote config/feature flags, A/B testing framework, telemetry/analytics SDK
- [x] Voice chat integration

**Platform Expansion**
- [x] XR (VR/AR) bridge, console platform bridges (Switch/PlayStation/Xbox)
- [x] Cloud save sync

**Agentic AI**
- [x] Tool sandbox / fine-grained permission policies
- [x] Multi-agent message bus
- [x] Local model runtime bridge (GGUF/llama.cpp), agent memory/context persistence

---

## Tier 4 — Optional / Genre-Specific

Only build these if a specific game you're making actually needs them.

- [x] Procedural generation: noise library, terrain/heightmap, voxel worlds, dungeon generation
- [x] World simulation: day/night cycle, weather, water/buoyancy, destructible environments
- [x] Grid-based movement, turn-based state manager
- [x] Economy systems: loot tables, crafting recipes, skill/talent trees (genre-dependent)
- [x] Motion matching, morph targets/blend shapes, portal-based occlusion culling
- [x] Cutscene/timeline sequencer, split-screen rendering
- [x] Platform SDK bridges: Steam, Epic Online Services, GOG Galaxy
- [x] Controller haptics (adaptive triggers)
- [x] Interactive tutorial / online sandbox compiler

---

## Notes

- **Don't build Tier 3/4 items speculatively.** Every hour spent on rollback netcode before you've shipped one working game is an hour not spent closing Tier 1.
- **Re-audit Tier 0's "needs verification" items first** — if the 100+ gameplay headers don't actually work, your real Tier 1 list is bigger than it looks above.
- This list is a snapshot of a conversation, not a spec. Revisit and re-prioritize as you actually build — some Tier 2 items may turn out to be Tier 1 once you're mid-game-development and discover you need them sooner.
