# Zenith Game SDK Examples

These examples exercise the current code-first game runtime.

## Files

- `code_first_scene.zen`
  A pure Zenith `Scene` sample that uses builtin `Scene` and `Canvas` runtime types plus direct entity, camera, hierarchy, texture/resource, first-class `Character2D` scene properties, explicit 2D capsule/circle collider properties, overlap and `raycast2D` queries, and code-controlled 2D audio sources/listeners from code.
- `code_first_scene_3d.zen`  
  A pure Zenith `Scene` sample that uses the same code-first scene model for 3D character state, typed mesh/shader/material/audio handles for hot-path resource binding, camera state, rigid-body properties, explicit 3D sphere/box collider properties, overlap and `raycast3D` queries, 3D audio sources/listeners, point/directional lights, and code-defined material properties like text/radio/image/button/color plus typed `MaterialPropertyView` and `MaterialPropertyOptionView` access (`kind`, `label`, `options`, indexed names, direct property mutation, per-option labels/values, callback names, trigger counts), executable button callbacks through `triggerMaterialButton`, generic typed property dispatch through `notifyMaterialProperty`, and code-driven material cloning for fast variant authoring without relying on editor-side setup.
- `gameplay_input.zen`
  A Zenith game-loop sample that uses builtin input helpers like `isKeyPressed`, `wasKeyPressed`, `wasKeyReleased`, and `getAxis`.
- `playable_slice.zen`
  A small code-first playable slice that combines typed texture/audio handles, tilemap-backed scene setup, explicit top-down movement, code-side collision resolution against solid colliders, primary-camera follow, asset bundle and memory-budget wiring, optional inspector cycling, and always-available debug overlay rendering in one sample game loop.
- `follow_camera_scene.cpp`
  A C++ scene sample built on `zenith::game::Scene`, `World`, fixed-step simulation, prefab spawning, and primary-camera follow.

## Build The Scene Sample In Zenith

```bash
zenith examples/game_sdk/code_first_scene.zen -target cpp -o examples/game_sdk/code_first_scene.cpp
```

Optional native build on Windows:

```bash
g++ -std=c++17 -Iinclude examples/game_sdk/code_first_scene.cpp -o examples/game_sdk/code_first_scene.exe -lws2_32 -lwinhttp -lopengl32 -lgdi32 -luser32
```

## Build The 3D Scene Sample In Zenith

```bash
zenith examples/game_sdk/code_first_scene_3d.zen -target cpp -o examples/game_sdk/code_first_scene_3d.cpp
```

Optional native build on Windows:

```bash
g++ -std=c++17 -Iinclude examples/game_sdk/code_first_scene_3d.cpp -o examples/game_sdk/code_first_scene_3d.exe -lws2_32 -lwinhttp -lopengl32 -lgdi32 -luser32
```

## Build The Gameplay Input Sample

```bash
zenith examples/game_sdk/gameplay_input.zen -target cpp -o examples/game_sdk/gameplay_input.cpp
```

Optional native build on Windows:

```bash
g++ -std=c++17 -Iinclude examples/game_sdk/gameplay_input.cpp -o examples/game_sdk/gameplay_input.exe -lws2_32 -lwinhttp -lopengl32 -lgdi32 -luser32
```

## Build The Playable Slice Sample

```bash
zenith examples/game_sdk/playable_slice.zen -target cpp -o examples/game_sdk/playable_slice.cpp
```

Optional native build on Windows:

```bash
g++ -std=c++17 -Iinclude examples/game_sdk/playable_slice.cpp -o examples/game_sdk/playable_slice.exe -lws2_32 -lwinhttp -lopengl32 -lgdi32 -luser32
```

## Build The Scene Sample

```bash
g++ -std=c++17 -Iinclude examples/game_sdk/follow_camera_scene.cpp -o examples/game_sdk/follow_camera_scene.exe -lopengl32 -lgdi32 -luser32
```

## Controls

- `W`, `A`, `S`, `D` or arrow keys: move
- `Space`: flash the player yellow
- `Escape`: pause/resume in the C++ scene sample
- `Space` in `playable_slice.zen`: toggle debug overlay
- `Tab` in `playable_slice.zen`: cycle the minimal inspector target
- `R` in `playable_slice.zen`: restart the slice
