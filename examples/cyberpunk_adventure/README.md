# Cyberpunk Action Adventure — Zenith Game Sample

A 3D Sci-Fi Cyberpunk Action Adventure game project built using Zenith Language & Engine capabilities.

## 🚀 Key Features Demonstrated
- **Player Controller & Cyber-Dash**: Character movement, energy management, weapon plasma rifle firing, and dodge dashing.
- **GOAP Enemy AI**: Cyber-droid enemy AI using GOAP state machines, sight/hearing perception sensors, and target engagement.
- **Neon City World Environment**: PBR metallic road reflection shaders, day/night clock cycles, acid rain weather volumes, and 3D HRTF spatial audio.
- **Netcode & Save Persistence**: Co-op networking, reliable-UDP state synchronization, and versioned checkpoint save migration.

## 🕹️ Project Structure
- [main.zen](file:///c:/Jay/_Plugin/zenith_lang/examples/cyberpunk_adventure/main.zen): Main game loop & application entry point.
- [player.zen](file:///c:/Jay/_Plugin/zenith_lang/examples/cyberpunk_adventure/player.zen): CyberPlayer entity controller & weapon system.
- [enemy_ai.zen](file:///c:/Jay/_Plugin/zenith_lang/examples/cyberpunk_adventure/enemy_ai.zen): CyberDroidEnemy GOAP AI & perception sensors.
- [world_level.zen](file:///c:/Jay/_Plugin/zenith_lang/examples/cyberpunk_adventure/world_level.zen): NeonCityWorld PBR environment & weather presets.

## 🎮 How to Run
```bash
zenith run examples/cyberpunk_adventure/main.zen
```
