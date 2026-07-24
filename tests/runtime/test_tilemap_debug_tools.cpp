#include <iostream>
#include "zenith/game/zenith_game.h"

static int countStyle(const zenith::Canvas& canvas, const std::string& style) {
    int count = 0;
    for (const auto& row : canvas.buffer.styles) {
        for (const std::string& cellStyle : row) {
            if (cellStyle == style) {
                ++count;
            }
        }
    }
    return count;
}

static int countGlyph(const zenith::Canvas& canvas, const std::string& glyph) {
    int count = 0;
    for (const auto& row : canvas.buffer.grid) {
        for (const std::string& cell : row) {
            if (cell == glyph) {
                ++count;
            }
        }
    }
    return count;
}

int main() {
    using namespace zenith::game;

    Scene scene;
    scene.autoRenderWorld2D = true;
    scene.drawEntityNames = false;
    scene.debugDrawGrid2D = true;
    scene.debugDrawColliders2D = true;
    scene.debugDrawTransforms2D = true;
    scene.debugDrawCameraBounds2D = true;
    scene.debugDrawRuntimeStats = true;
    scene.debugGridCellWidth = 2.0f;
    scene.debugGridCellHeight = 1.0f;
    scene.debugOverlayColor = "cyan";

    EntityId camera = scene.spawnCamera2D("MainCamera", 4.0f, 4.0f, 1.0f, true);
    scene.setEntityMask(camera, 0xFFFFFFFF);

    EntityId tilemap = scene.spawnTilemap2D("Ground", 4.0f, 4.0f, 4, 3, 2.0f, 1.0f, "blue");
    scene.setTilemapCell(tilemap, 0, 0, 1);
    scene.setTilemapCell(tilemap, 1, 0, 1);
    scene.setTilemapCell(tilemap, 2, 1, 2);
    scene.setTilemapPaletteColor(tilemap, 2, "green");

    EntityId player = scene.spawnSprite("Player", 5.0f, 5.0f, 2.0f, 2.0f, "red");
    scene.attachBoxCollider2D(player, 2.0f, 2.0f, false);
    zenith::physics::RigidBody2D& body = scene.world.addRigidBody2D(player);
    body.position = zenith::physics::Vec2(5.0f, 5.0f);
    body.velocity = zenith::physics::Vec2(3.0f, -1.0f);

    Tilemap2DView view = scene.tilemap2D(tilemap);
    view.anchorX = 0.5f;
    view.anchorY = 0.5f;
    view.sortOrder = -10;

    if (scene.tilemapCell(tilemap, 2, 1) != 2 || scene.tilemapPaletteColor(tilemap, 2) != "green") {
        std::cerr << "tilemap_view_state_failed" << std::endl;
        return 1;
    }

    zenith::Canvas worldCanvas(32, 18, false);
    zenith::drawWorld2D(worldCanvas, scene.world, false);

    const int blueCells = countStyle(worldCanvas, "\033[44m");
    const int greenCells = countStyle(worldCanvas, "\033[42m");
    const int redCells = countStyle(worldCanvas, "\033[41m");

    zenith::Canvas canvas(32, 18, false);
    zenith::SceneGame host(scene);
    host.draw(canvas);

    const int cyanCells = countStyle(canvas, "\033[36m");
    const int overlayGlyphs = countGlyph(canvas, "*") + countGlyph(canvas, "+");

    std::cout << "tile_blue=" << blueCells << std::endl;
    std::cout << "tile_green=" << greenCells << std::endl;
    std::cout << "sprite_red=" << redCells << std::endl;
    std::cout << "overlay_cyan=" << cyanCells << std::endl;
    std::cout << "overlay_marks=" << overlayGlyphs << std::endl;

    if (blueCells <= 0 || greenCells <= 0 || redCells <= 0 || cyanCells <= 0 || overlayGlyphs <= 0) {
        return 1;
    }

    return 0;
}
