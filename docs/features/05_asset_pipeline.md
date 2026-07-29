# Zenith Feature Audit: Asset Pipeline & Resource Management

## 1. Executive Overview
Zenith uses a code-driven asset pipeline backed by numeric asset handles, memory budget tracking, JSON `.meta` sidecars, sqlite database registry (`zenith_assets.db`), asset pack definitions, and file-system watching for dev server hot-reloading.

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **Typed Resource Handles & Asset Database** ([zenith_resource.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_resource.h)):
  - Numeric handle types (`TextureHandle`, `MeshHandle`, `MaterialHandle`, `AudioHandle`, `ShaderHandle`).
  - Zero string lookup overhead inside hot frame loops.
  - VRAM/RAM memory budget tracking routines.
- **Asset Packing Specification** ([zenith_asset_pack.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_asset_pack.h)):
  - Asset pack container header definitions.
- **Hot Reload File Watcher** ([zenith_hot_reload.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_hot_reload.h)):
  - Monitors asset modifications on disk and triggers live dev server updates.
- **CLI Import Subcommand** ([main.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/main.cpp)):
  - `zenith assets import <dir>` scans directory, bakes `.meta` JSON sidecars, and registers handles in `zenith_assets.db`.
  - `zenith assets list` displays handle table and memory budget stats.

---

## 3. ❌ Missing / Incomplete Features & Rust FFI Strategy
- **Production `.zpak` Asset Bundling**: Use Rust `zstd` / `image` / `basis-universal` crates via thin C ABI (`cdylib`) FFI instead of hand-rolling C++ compression.
- **Streaming Audio Chunks**: Long audio files are loaded completely in memory instead of streamed from disk in chunks.

---

## 4. 💻 How to Use
```bash
# Import assets from directory and bake .meta sidecars
zenith assets import assets/

# List registered asset handles and memory budget stats
zenith assets list
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File | Implementation File / Source |
| :--- | :--- | :--- |
| **Resource Handles & Asset DB** | [zenith_resource.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_resource.h) | N/A (Header-only resource database) |
| **Asset Pack Data Structure** | [zenith_asset_pack.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_asset_pack.h) | N/A |
| **Hot Reload Watcher** | [zenith_hot_reload.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_hot_reload.h) | N/A |
| **CLI Asset Commands** | N/A | [main.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/main.cpp) |
