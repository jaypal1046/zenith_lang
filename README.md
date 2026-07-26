# Zenith

Code-first game SDK and language.

Zenith is being shaped around one main goal: build games through stable, explicit code instead of editor-heavy UI workflows. The center of the repo is the language, the game runtime, typed resources, fast iteration loops, and shared 2D/3D engine concepts.

App, web, and UI-first work still exists in the repository, but it is no longer the main product story. Those surfaces are secondary targets and compatibility layers, not the identity of the project.

## Direction

Zenith is aiming to compete on:

- code-first scene and gameplay authoring
- direct property access from language code
- typed runtime/resource handles instead of fragile string-heavy hot paths
- fast compile, watch, and hot-reload loops
- shared 2D and 3D mental models
- predictable performance through data-oriented runtime cleanup

Zenith is not aiming to become another editor-centric Unity clone or another app framework that also happens to do games.

## Current Focus

The repo is currently centered on:

- language frontend and static analysis
- native code generation
- game runtime in `include/zenith/game/`
- material/resource property modeling in code
- gameplay productivity tools such as watch/dev loops, test harnesses, and asset commands
- **Code-First Game SDK**:
  - Native GPU double-buffered windowing & 2D immediate renderer ([zenith_window.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_window.h))
  - Cache-friendly Entity Component System (ECS) ([zenith_ecs.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_ecs.h))
  - $O(N \log N)$ Spatial Hash Grid & Physics Raycasting ([zenith_spatial.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_spatial.h))
  - Flutter-style composable UI tree (`Container`, `Stack`, `Row`, `Column`, `Image`, `Button` with actions) ([zenith_gui.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_gui.h))
  - Cutscene & video stream playback ([zenith_video.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_video.h))
  - Particle emitter engine ([zenith_particles.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_particles.h))
  - Audio manager ([zenith_audio.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_audio.h))
  - Stack-based game state machine ([zenith_state.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_state.h))

Secondary or maintenance-only areas:

- app-shell positioning
- web-first marketing
- widget-library-first messaging
- SSR/site-oriented storytelling

## Quick Start

Build the compiler:

```bash
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang
build.bat
```

Create a game-oriented project:

```bash
zenith create my_game --template=game
cd my_game
desktop/build.bat
```

Run the gameplay harness:

```bash
tests/run_gameplay_harness.bat
```

Compile a Zenith source file directly:

```bash
zenith tests/language_tests/test_math_assets.zen -target cpp
```

Use the fast iteration loop:

```bash
zenith dev tests/language_tests/test_math_assets.zen --profile debug --flow transpile
```

## Language and Runtime Shape

Zenith is moving toward a compact systems-oriented gameplay language with:

- first-class math/value types such as `Vec2`, `Vec3`, `Vec4`, and `Mat4`
- typed asset references such as `texture(...)`, `audio(...)`, `mesh(...)`, `shader(...)`, and `material(...)`
- explicit scene/runtime APIs
- code-visible materials and resource properties
- code-first camera, transform, collider, and character behavior

Example:

```zenith
import std.io;

Void main() {
    let move2 = Vec2(3, 4);
    let offset = move2 + Vec2(1, 2);
    let tex = texture("asset://textures/player.png");

    println("offset.x = " + offset.x);
    println("offset.y = " + offset.y);
    println("length = " + move2.length());
    println("texture = " + tex.path);
}
```

## Core Commands

Primary workflow:

```bash
zenith <file.zen> [-target cpp|web|wasm] [--profile debug|dev|release] [-o <output>]
zenith watch <file.zen> [-target cpp|web|wasm] [--profile debug|dev|release] [--flow native|transpile]
zenith dev <file.zen> [-target cpp|web|wasm] [--profile debug|dev|release] [--flow native|transpile]
```

Game/project workflow:

```bash
zenith create <name|.> [--template=app|game|package]
zenith run <desktop|windows|linux|mac|web|wasm|android|ios>
zenith format [-w] <file.zen>
```

Asset workflow:

```bash
zenith assets import <source> [--out path] [--group name] [--bundle name] [--db path]
zenith assets bake [--out path] [--db path]
zenith assets list [--db path]
zenith assets bundle create <name> [--db path]
zenith assets bundle add <bundle> <asset> [--db path]
zenith assets budget status [--db path]
zenith assets reload --dirty [--db path]
zenith assets poll [--db path]
```

Tooling:

```bash
zenith lsp
zenith daemon start [-d <dir>]
zenith daemon stop
zenith daemon status
```

## Repository Map

Important directories:

- `src/frontend/` - lexer, parser, semantic analysis, formatter
- `src/backend/` - C++, JS, and WASM code generators
- `src/main.cpp` - CLI entry point
- `include/zenith/game/` - game runtime, physics, world, scene, resources
- `include/zenith/common/` - shared runtime support
- `tests/language_tests/` - language-level Zenith samples and regressions
- `tests/runtime/` - C++ runtime validation
- `plan/game_sdk_pivot.md` - product-direction document for the game-first pivot
- `plan/action_plan.md` - execution roadmap aligned to the pivot

## Roadmap

As of July 23, 2026, the active direction is:

1. strengthen typed language/runtime primitives
2. keep the entity/scene/resource model code-first
3. improve ECS/data-oriented runtime paths
4. expand streaming, characters, rendering, and asset workflow lanes
5. keep productivity tooling optional and lightweight

The main roadmap documents are:

- [plan/game_sdk_pivot.md](plan/game_sdk_pivot.md)
- [plan/action_plan.md](plan/action_plan.md)

## Optional Targets

Web, WASM, app-shell, and UI-oriented features are still present for experimentation, tooling, or compatibility work. They should be treated as optional targets, not the top-level identity of Zenith.

That means:

- game/runtime work should lead the roadmap
- README examples should stay gameplay-oriented first
- language evolution should prefer explicit engine/runtime concepts over widget-first syntax

## Contributing

When making roadmap or documentation changes, prefer language that reinforces:

- game SDK first
- code as the source of truth
- explicit runtime concepts
- optional tooling instead of tooling-owned workflows

If a change makes Zenith read like an app framework first, it is probably moving in the wrong direction.

## License

MIT. See [LICENSE](LICENSE).
