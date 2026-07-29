Language / Compiler

[x] Pattern & Struct Destructuring (match arms)
[x] Macro System (compile-time codegen)
[x] @deprecated Annotation + Compiler Warnings
[x] Semver Package Versioning

Codegen / Performance

[x] SIMD Vector Codegen (Vec2/Vec3/Vec4/Mat4)
[x] Direct LLVM IR Backend
[x] JIT Execution Engine
[x] Frame/Arena Allocator (Temp<T> scoped allocation)
[x] First-Class Object Pooling (Pool<T>)

Concurrency

[x] ECS Parallel System Scheduler (data-oriented job graph)
[x] Read/Write Component Access Declarations (per-system)

Rendering / Canvas

[x] Cross-Platform Canvas Backend (SDL2/GLFW)
[x] Canvas Texture Batching
[x] Texture Atlas / Sprite Sheet Packer
[x] Sprite Frame Animation / Flipbook
Node-Based Shader Graph Compiler

Physics Engine

[x] Collision Layer Matrix (code-defined)
[x] Physics Material System (friction/bounce presets)
[x] Trigger vs. Collision Event Separation
[x] Joint & Constraint System (hinge, spring, fixed)

Game Engine / ECS

Skeletal Animation & Blend Trees
[x] App-Level Game State Stack (menu/play/pause/gameover)
[x] World Snapshot / Restore
[x] Save / Load System

Asset Pipeline

[x] .zpak Asset Bundling
[x] ASTC/KTX2 Texture Compression
[x] Streaming Audio Chunks

Stdlib / FFI

[x] Rust FFI Bridge (crypto, network, compression)
[x] Unified Cross-Platform Networking Transport
[x] SQLite / Database Bridge

Testing

[x] Built-in Test Framework (test keyword + assertions)
[x] Headless Deterministic Execution Mode
Golden-Frame Visual Regression Testing
[x] Scripted Input Injection (for automated test scenes)

Tooling

[x] Structured Logging Framework (levels/tags/filters)
[x] In-Game Debug Console / Command System
[x] Gizmo / Debug Draw API
[x] zenith doctor (environment/SDK diagnostic command)
[x] Built-in Profiler (CPU/GPU/memory)
[x] Debug Adapter Protocol (verify/complete)
[x] Package Registry Client (zenith install)

Agentic AI

[x] Tool Sandbox / Permission Policies
[x] Multi-Agent Message Bus
[x] Local Model Runtime Bridge (GGUF/llama.cpp)

Language Semantics

[x] Nullable Types / Null Safety
[x] Tagged Unions / Sum Types
[x] Exhaustiveness Checking (match)
[x] Operator Overloading
[x] Traits / Mixins / Interfaces with Default Methods
[x] Extension Methods
[x] Generic Constraints (bounded generics)
[x] Tuple Types
[x] Named & Default Parameters
[x] Variadic Functions
[x] String Interpolation
[x] Range Expressions
[x] Closures with Explicit Capture
[x] Access Modifiers (public/private/internal)
[x] Design-by-Contract (require/ensure)
[x] Unsafe Blocks (opt-in raw memory access)
[x] Const Evaluation / const fn

Compiler / Build System

[x] Incremental Compilation
[x] Conditional Compilation (#if platform)
[x] Build Profiles (debug/release/profile)
[x] Dead Code Elimination
[x] Link-Time Optimization
[x] Static Analyzer / Linter
[x] Compiler Plugin API
[x] Reproducible Builds

Memory / Runtime

[x] Object Pooling (first-class)
[x] Custom Allocator API
[x] GC Tuning Controls
[x] RAII / Deterministic Destructors
[x] Weak-Reference Cycle Detector

Concurrency

[x] Coroutines (gameplay-scoped)
[x] Fiber-Based Cooperative Multitasking
[x] Lock-Free Queues / Ring Buffers
[x] GPU Compute Dispatch

Rendering

[x] 2D Sprite Batching
[x] 3D Instanced Rendering
[x] Multi-Pass Render Graph
[x] Camera Stack / Multi-Camera Compositing
[x] LOD (Level of Detail) System
[x] Frustum & Occlusion Culling
[x] Baked Lightmaps / GI
[x] GPU Particle Simulation

Audio

[x] 3D Spatial Audio (HRTF)
[x] Audio Mixer Bus Routing
[x] DSP Effects Chain
[x] Layered Music/Ambience System

Physics

[x] Joint & Constraint System (hinge, spring, fixed)
[x] Continuous Collision Detection (CCD)
[x] Trigger vs Collision Event Separation
[x] Ragdoll Physics

Input

[x] Unified Input Abstraction (KB/mouse/gamepad/touch)
[x] Input Action Mapping & Rebinding
[x] Split Local Multiplayer Input
[x] Touch Gesture Recognition

Networking / Multiplayer

[x] Client-Server Netcode Primitives
[x] State Snapshot & Interpolation
[x] Rollback Netcode
[x] RPC Syntax
[x] Lobby/Matchmaking Client

Save & Persistence

[x] Save/Load System
[x] Versioned Save Migration
[x] Cloud Save Sync

Localization / Accessibility

[x] String Table / Localization System
[x] Accessibility Hooks (subtitles, remap, colorblind modes)

Platform / Deployment

[x] App Signing & Packaging Pipeline
[x] Console Platform Headers (future)
[x] Crash Reporting & Symbolication
[x] Patch/Auto-Update Delivery

Tooling

[x] Built-in Profiler (CPU/GPU/memory)
[x] Frame/Draw-Call Debugger
[x] REPL / Interactive Shell
[x] Gameplay Code Hot Reload (native target)
[x] Doc Comment Generator
[x] Project Template System

Security

[x] Signed Shared Libraries
[x] Sandboxed Plugin Execution
[x] Anti-Tamper / Memory Integrity Hooks

Agentic AI

[x] Local Model Runtime Bridge (GGUF/llama.cpp)
[x] Agent Memory/Context Persistence
[x] Tool Call Retry & Fallback Policy
[x] Streaming Response Handling

Ecosystem

[x] Community Plugin API
[x] Asset/Package Marketplace Integration

Animation & Timeline

[x] 2D Sprite Frame Animation / Flipbook
[x] Tween / Easing Engine
[x] Cutscene / Timeline Sequencer
[x] Animation Event Callbacks

Procedural Generation

[x] Noise Library (Perlin / Simplex / Worley)
[x] Procedural Mesh Generation
[x] Terrain / Heightmap System
[x] Voxel World Support

In-Game UI / HUD (runtime, not editor)

[x] Retained-Mode Widget Layer (optional, atop Canvas)
[x] Rich Text Rendering (markup, color, effects)
[x] UI Transition/Animation Primitives
[x] Nine-Slice Sprite UI Rendering

Camera & Cinematics

[x] Camera Shake
[x] Cinematic Director / Cutscene Sequencer
[x] Multi-Camera Blend/Transition
[x] Split-Screen Rendering

AI & Planning

[x] Goal-Oriented Action Planning (GOAP)
[x] Utility AI Scoring System
[x] NavMesh Generation
[x] Flocking / Crowd Simulation

World & Level Streaming

[x] Async Level Streaming
[x] World Partitioning / Chunk Loading
[x] Terrain LOD Streaming

Replay & Determinism

[x] Deterministic Fixed-Point Math Mode
[x] Replay Recording & Playback
[x] Spectator Camera Mode

Platform Services

[x] Achievements / Leaderboards Bridge
[x] In-App Purchase API
[x] Ad SDK Integration
[x] Voice Chat Integration
[x] Dedicated/Headless Server Build Mode

Modding & Extensibility

[x] Mod Loader / Plugin Discovery
[x] Sandboxed Mod Script Execution
[x] Data-Driven Config Hot Reload (JSON/TOML)

DevOps / CI

[x] Automated Multi-Platform Build Pipeline
[x] CI Test Runner Hooks
[x] Binary Size / Perf Regression Tracking

Time & Simulation Control

[x] Time Scale / Slow-Motion
[x] Pause-Safe Update Groups
[x] Fixed vs Variable Timestep Config

Peripherals

[x] Controller Haptics / Vibration
[x] Gyroscope / Accelerometer Input
[x] Custom HID Device Bridge

Emerging Platforms

[x] XR (VR/AR) Platform Bridge
[x] Cloud/Streaming Play Bridge

World Simulation

[x] Day/Night Cycle System
[x] Weather System (rain, fog, wind)
[x] Buoyancy / Water Simulation
[x] Destructible Environment System

Level & Content Definition

[x] Data-Driven Level Format (code-first level DSL, not editor-authored)
[x] Checkpoint / Respawn System
[x] Trigger Zone / Volume System
[x] Environment Query System (spatial awareness for AI)

AI Perception

[x] Sensor System (sight/hearing/proximity)
[x] Line-of-Sight / Stealth Detection

Physics (extended)

[x] Vehicle Physics
[x] Character Controller Tuning (slope limit, step offset, ledge detection)
[x] Buoyancy Forces
[x] Wind/Force Field Volumes

Audio (extended)

[x] Reverb / Occlusion Zones
[x] Adaptive Music (state-based transitions)

Rendering (extended)

[x] Skybox / Environment System
[x] Fog Volumes
[x] HDR & Tone Mapping
[x] Color Grading LUT
[x] Decal System

Text & Localization (extended)

[x] Complex Script Text Shaping (RTL, Devanagari, CJK)
[x] BiDi Text Support
[x] Localization Fallback Chains

Debugging & Diagnostics

[x] Structured Logging Framework (levels, tags, filters)
[x] In-Game Debug Console / Command System
[x] Gizmo / Debug Draw API (code-based visual debugging)
[x] Memory Leak Detector
[x] Frame Pacing / VSync Control

Serialization

[x] Binary Serialization Framework
[x] Schema-Based Data Format (Protobuf-like)
[x] Save Data Encryption

Live Ops / Content Delivery

[x] Remote Config / Feature Flags
[x] A/B Testing Framework
[x] Hot Content Push (live-service asset updates)
[x] Telemetry / Analytics SDK

Platform Lifecycle

[x] App Lifecycle Hooks (pause/resume/background/foreground)
[x] Screenshot / Video Capture API
[x] Platform Account Linking / Cross-Play

Package & Dependency Management

[x] Dependency Resolver
[x] Lockfile System
[x] Package Version Conflict Detection

Testing (extended)

[x] Property-Based Testing
[x] Fuzzing Harness
[x] Performance Regression Benchmarking

Modding (extended)

[x] Asset Override / Layering System
[x] Mod Dependency Declaration

Low-Level Graphics

[x] Graphics API Abstraction Layer (Vulkan/Metal/DirectX12)
[x] GPU-Driven Indirect Rendering
[x] Compute Shader Pipeline
[x] Custom Shader Language / DSL
[x] Multi-threaded Command Buffer Recording
[x] Ray Tracing Support (future target)

Gameplay Economy Systems

[x] Loot Table / Random Drop System
[x] Currency & Economy Model
[x] Crafting Recipe System (data-driven)
[x] Skill/Talent Tree Runtime
[x] Quest/Objective State Machine

Multiplayer (extended)

[x] Dedicated Matchmaking Service Bridge
[x] Anti-Cheat Hooks
[x] Session/Lobby Reconnect Handling
[x] Server-Authoritative Simulation Mode

Stdlib Data Formats

[x] JSON/YAML/TOML Parser
[x] Binary Compression Library (zip/gzip)
[x] Image Codec Support (PNG/JPEG/WebP)
[x] Video Playback/Codec Bridge
[x] Font Rasterization & Text Shaping Engine

Developer Tooling (extended)

[x] Edit-and-Continue / Live Coding
[x] Distributed / Cached Compilation
[x] API Reference Site Generator
[x] Sample/Template Project Gallery
[x] Crash Dump Symbolication Tool

Platform Abstraction

[x] Unified Platform Capability Detection API
[x] Universal Binary Packaging
[x] Console Platform Bridge (future: Switch/PlayStation/Xbox)

Ecosystem (extended)

[x] Package Registry Hosting Infrastructure
[x] Plugin Marketplace Storefront
[x] License/Attribution Manifest Generator

Rendering (gaps)

[x] PBR Material System (albedo/metallic/roughness workflow) — core, code-defined
[x] Texture Atlas / Sprite Sheet Packer
[x] SDF Font Rendering (crisp text at any zoom/rotation)
[x] Quality Presets / Graphics Settings Tiers (Low/Medium/High)
[x] Dynamic Resolution Scaling
[x] Capability-Based Feature Fallback (detect missing GPU features, fall back gracefully)

Code-First Specific Advantage (worth building precisely because you're code-first)

[x] Compile-Time Asset Reference Validation — catch a missing texture/mesh path at compile time instead of runtime, something Unity's editor genuinely can't do as cleanly since it resolves references at scene-load. This is a direct, concrete payoff of your whole philosophy.
[x] Asset Budget/Lint Rules (fail build if poly count / texture size exceeds a declared budget)

Toolchain

[x] zenith doctor — environment/SDK diagnostic command (like flutter doctor), since you'll eventually juggle NDK, Xcode, Emscripten versions
[x] zenith upgrade — self-update for the CLI/compiler binary

Mobile Platform Services

[x] Push Notification API
[x] In-App Review Prompt API
[x] Deep Linking / URL Scheme Handling
[x] Battery/Thermal-Aware Performance Throttling

Localization (gaps)

[x] Pluralization / Grammar-Aware Localization Rules
[x] Localized Audio/Voice-Over Track Switching

Stdlib

[x] Native File Dialog / Clipboard API

Compliance

[x] Privacy Consent & Data Opt-Out Framework (GDPR/COPPA — needed to ship on mobile stores globally)

Rendering (further)
[x] Shadow Mapping / Cascaded Shadow Maps · [x] Screen-Space Reflections · [x] Ambient Occlusion (SSAO) · [x] Anti-Aliasing (MSAA/TAA/FXAA) · [x] Motion Blur · [x] Depth of Field · [x] Volumetric Lighting/Fog · [x] Terrain Sculpting API · [x] Water Rendering Shader · [x] Particle-World Collision · [x] Billboard Rendering · [x] Trail Renderer · [x] Reflection Probe Baking · [x] Light Probe Volumes · [x] GPU Mesh Skinning · [x] Morph Target / Blend Shape Support · [x] Portal-Based Occlusion Culling · [x] Octree/Quadtree Scene Partitioning

Animation (further)
[x] Inverse Kinematics (IK) Solver · [x] Animation Retargeting · [x] Motion Matching · [x] Procedural Foot/Look-At IK

Physics (further)
[x] Physics Material System (friction/bounce presets) · [x] Code-Defined Collision Layer Matrix · [x] Fracture / Breakable Physics · [x] Soft-Body Simulation

Audio (further)
[x] Audio Occlusion Raycasting · [x] Doppler Effect Simulation · [x] Audio Ducking · [x] Footstep/Foley Surface Detection · [x] Audio Streaming (voice/network) · [x] Compressed Codec Support (Vorbis/Opus)

Runtime UI (further)
[x] Drag-and-Drop API · [x] Virtual Scrolling / List Virtualization · [x] Gamepad UI Focus Navigation · [x] UI Theming/Skin System · [x] Safe-Area / Notch Handling

Networking (further)
[x] NAT Traversal / Hole Punching · [x] Voice Codec Bridge (Opus) · [x] Server Browser · [x] Ping/Latency Display API · [x] Packet Compression · [x] Delta-Compressed Snapshots · [x] WebSocket Transport Bridge · [x] Reliable-UDP Layer · [x] Cross-Platform Play Bridge

Save & Data (further)
[x] Incremental/Delta Save · [x] Save Slot Management · [x] Autosave Scheduler · [x] Local Key-Value Cache Layer · [x] Embedded In-Memory Database · [x] Local ORM Layer

Platform SDK Bridges
[x] Steam SDK Bridge · [x] Epic Online Services Bridge · [x] GOG Galaxy Bridge · [x] Controller Haptics Bridge (adaptive triggers) · [x] Native Achievement-UI Hook

Accessibility (further)
[x] Text-to-Speech Narration · [x] Colorblind Simulation Preview · [x] Remappable UI Scale · [x] One-Handed Mode Support · [x] Screen Reader Hook API

Localization (further)
[x] Translation Memory / CAT Export · [x] Machine Translation API Bridge · [x] i18n Number/Date/Currency Formatting · [x] Timezone Handling

Dev Tooling (further)
[x] Flame-Graph Profiler Timeline · [x] Memory Snapshot Diff Tool · [x] Asset Dependency Graph Visualizer · [x] Build Size Analyzer · [x] Race Condition Detector · [x] Deadlock Detector · [x] Time-Travel (Step-Backward) Debugging · [x] Conditional Breakpoints · [x] Watch Expressions · [x] Remote Device Debugging

Package Management (further)
[x] Private Registry Support · [x] Package Signing / Checksum Verification · [x] Monorepo/Workspace Support · [x] Plugin Dependency Version Negotiation

Testing (further)
[x] UI Snapshot Testing · [x] Mutation Testing · [x] Code Coverage Reporting · [x] Mock/Stub Framework · [x] Multi-Client Simulated Integration Test Harness

CI/CD (further)
[x] Automated Changelog Generation · [x] Semantic Release Automation · [x] Build Artifact Signing · [x] Crash-Free Session Rate Tracking

Documentation
[x] Doc Comment Linter · [x] Commit-to-Changelog Generator · [x] Migration Guide Generator

Extensibility
[x] Custom Asset Importer Plugin API · [x] Custom Codegen Backend Plugin API · [x] LSP Extension Point API

Performance
[x] GPU Profiler Markers · [x] Frame Budget Alerting · [x] Battery Usage Profiler · [x] Thermal Throttling Detection API

Security (further)
[x] Script/Bytecode Obfuscation · [x] License Key Validation System · [x] DRM Hook Points · [x] Mod Sandbox Resource Limits

AI (further)
[x] Blackboard System (shared AI memory) · [x] Squad/Group AI Coordination · [x] NPC Mood/Emotion State Simulation

Reflection & Metadata
[x] Runtime Type Reflection API · [x] Attribute-Based Metadata Queries · [x] Reflection-Driven Serialization

Genre-General Systems
[x] Grid-Based Movement System · [x] Turn-Based Game State/Turn Manager · [x] Procedural Quest Generation · [x] Procedural Dungeon Generation Library

Distribution
[x] Steam/itch.io Auto-Upload Pipeline · [x] Staged Rollout / Beta Channel Support · [x] Cloud Save Conflict Resolution Hooks

Onboarding / Ecosystem
[x] Interactive Tutorial/Playground Mode · [x] Online Sandbox / Try-It Compiler