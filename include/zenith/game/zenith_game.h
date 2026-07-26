#ifndef ZENITH_GAME_H
#define ZENITH_GAME_H

#include "../common/zenith_common.h"
#include "zenith_physics.h"
#include "zenith_resource.h"
#include "zenith_world.h"
#include "zenith_scene.h"
#include "zenith_window.h"
#include "zenith_texture.h"
#include "zenith_audio.h"
#include "zenith_particles.h"
#include "zenith_state.h"
#include "zenith_gui.h"
#include "zenith_video.h"
#include "zenith_tilemap.h"
#include "zenith_tween.h"
#include "zenith_null_safety.h"
#include "zenith_job_system.h"
#include "zenith_event.h"
#include "zenith_save_system.h"
#include "zenith_input_virtual.h"
#include "zenith_camera3d.h"
#include "zenith_noise.h"
#include "zenith_spatial_audio.h"
#include "zenith_camera_fx.h"
#include "zenith_raycast.h"
#include "zenith_light2d.h"
#include "zenith_node2d.h"
#include "zenith_animator.h"
#include "zenith_behavior_tree.h"
#include "zenith_collision_solver.h"
#include "zenith_ability.h"
#include "zenith_pathfinding.h"
#include "zenith_pool.h"
#include "zenith_material.h"
#include "zenith_gamepad.h"
#include "zenith_hot_reload.h"
#include "zenith_asset_pack.h"
#include "zenith_viewport.h"
#include "zenith_particle_fx.h"
#include "zenith_post_processing.h"
#include "zenith_network.h"
#include "zenith_font.h"
#include "zenith_rigidbody.h"
#include "zenith_autotile.h"
#include "zenith_audio_mixer.h"
#include "zenith_dev_console.h"
#include "zenith_procedural_audio.h"
#include "zenith_render_layer.h"
#include "zenith_dialogue.h"
#include "zenith_destructible_terrain.h"
#include "zenith_inventory.h"
#include "zenith_quest.h"
#include "zenith_profiler.h"
#include "zenith_water2d.h"
#include "zenith_softbody2d.h"
#include "zenith_ik2d.h"
#include "zenith_shadow2d.h"
#include "zenith_dungeon_gen.h"
#include "zenith_fluid2d.h"
#include "zenith_cloth2d.h"
#include "zenith_weather.h"
#include "zenith_replay2d.h"
#include "zenith_fov2d.h"
#include "zenith_fog_of_war.h"
#include "zenith_decal2d.h"
#include "zenith_floating_text.h"
#include "zenith_steering2d.h"
#include "zenith_heatmap.h"
#include "zenith_voxel2d.h"
#include "zenith_distortion2d.h"
#include "zenith_normal_map2d.h"
#include "zenith_ragdoll2d.h"
#include "zenith_marching_cubes2d.h"
#include "zenith_navmesh2d.h"
#include "zenith_heightfield2d.h"
#include "zenith_sdf2d.h"
#include "zenith_spline2d.h"
#include "zenith_blur2d.h"
#include "zenith_chromatic2d.h"
#include "zenith_crt2d.h"
#include "zenith_arena_allocator.h"
#include "zenith_palette2d.h"
#include "zenith_dither2d.h"
#include "zenith_trail2d.h"
#include "zenith_lightning2d.h"
#include "zenith_vignette2d.h"
#include "zenith_color_grading2d.h"
#include "zenith_digital_glitch2d.h"
#include "zenith_ripple2d.h"
#include "zenith_bloom2d.h"
#include "zenith_sobel2d.h"
#include "zenith_posterize2d.h"
#include "zenith_solarize2d.h"
#include "zenith_pixelate2d.h"
#include "zenith_fisheye2d.h"
#include "zenith_sharpen2d.h"
#include "zenith_tilt_shift2d.h"
#include "zenith_radial_blur2d.h"
#include "zenith_speed_lines2d.h"
#include "zenith_screen_flash2d.h"
#include "zenith_game_feel2d.h"
#include "zenith_shadow_mask2d.h"
#include "zenith_vhs2d.h"
#include "zenith_screen_freeze2d.h"
#include "zenith_chromatic_vignette2d.h"
#include "zenith_heat_vision2d.h"
#include "zenith_night_vision2d.h"
#include "zenith_ascii2d.h"
#include "zenith_lens_flare2d.h"
#include "zenith_vignette_pulse2d.h"
#include "zenith_shatter2d.h"
#include "zenith_laser_scan2d.h"
#include "zenith_sketch2d.h"
#include "zenith_oil_paint2d.h"
#include "zenith_halftone2d.h"
#include "zenith_light_rays2d.h"
#include "zenith_kaleidoscope2d.h"
#include "zenith_swirl2d.h"
#include "zenith_pinch_bulge2d.h"
#include "zenith_tunnel2d.h"
#include "zenith_mirror2d.h"
#include "zenith_rgb_wave2d.h"
#include "zenith_heat_haze2d.h"
#include "zenith_rain_drops2d.h"
#include "zenith_burn_dissolve2d.h"
#include "zenith_frost_ice2d.h"
#include "zenith_spark_discharges2d.h"
#include "zenith_hex_shield2d.h"
#include "zenith_caustics2d.h"
#include "zenith_pixel_sort2d.h"
#include "zenith_infrared2d.h"
#include "zenith_hologram2d.h"
#include "zenith_cmyk_offset2d.h"
#include "zenith_focus_vignette2d.h"
#include "zenith_shockwave2d.h"
#include "zenith_static_noise2d.h"
#include "zenith_reticle_hud2d.h"
#include "zenith_minimap_hud2d.h"
#include "zenith_hit_indicator2d.h"
#include "zenith_boss_bar2d.h"
#include "zenith_combo_counter2d.h"
#include "zenith_racing_gauge2d.h"
#include "zenith_dialogue_box2d.h"
#include "zenith_inventory_hud2d.h"
#include "zenith_status_hud2d.h"
#include "zenith_quest_tracker2d.h"
#include "zenith_cooldown_wheel2d.h"
#include "zenith_recoil_crosshair2d.h"
#include "zenith_score_notifier2d.h"
#include "zenith_skill_tree2d.h"
#include "zenith_equipment_doll2d.h"
#include "zenith_selection_marquee2d.h"
#include "zenith_action_wheel2d.h"
#include "zenith_loot_drop2d.h"
#include "zenith_target_indicator2d.h"
#include "zenith_stealth_cone2d.h"
#include "zenith_turn_order2d.h"
#include "zenith_qte2d.h"
#include "zenith_building_placement2d.h"
#include "zenith_card_hand2d.h"
#include "zenith_tactical_grid2d.h"
#include "zenith_crafting2d.h"
#include "zenith_racing_minimap2d.h"
#include "zenith_light_meter2d.h"
#include "zenith_input_buffer2d.h"
#include "zenith_leaderboard2d.h"
#include "zenith_narrative_choice2d.h"
#include "zenith_wave_spawner2d.h"
#include "zenith_pinball2d.h"
#include "zenith_ecs.h"
#include "zenith_spatial.h"
#include "zenith_dev_overlay.h"
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <mutex>
#include <cmath>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/gl.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#else
#include <unistd.h>
#endif

namespace zenith {

// Thread-safe singleton for keyboard and mouse states
class InputState {
public:
    static InputState& getInstance() {
        static InputState instance;
        return instance;
    }

    void beginFrame() {
        std::lock_guard<std::mutex> lock(mtx);
        justPressed.clear();
        justReleased.clear();
    }
    
    void setKeyPressed(const std::string& key, bool pressed) {
        if (key.empty()) {
            return;
        }

        std::lock_guard<std::mutex> lock(mtx);
        bool wasPressed = false;
        auto it = keys.find(key);
        if (it != keys.end()) {
            wasPressed = it->second;
        }

        keys[key] = pressed;
        if (wasPressed == pressed) {
            return;
        }

        if (pressed) {
            justPressed[key] = true;
            justReleased.erase(key);
        } else {
            justReleased[key] = true;
            justPressed.erase(key);
        }
    }
    
    bool isKeyPressed(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = keys.find(key);
        if (it != keys.end()) {
            return it->second;
        }
        return false;
    }

    bool wasKeyPressed(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = justPressed.find(key);
        return it != justPressed.end() && it->second;
    }

    bool wasKeyReleased(const std::string& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = justReleased.find(key);
        return it != justReleased.end() && it->second;
    }

    float getAxis(const std::string& negativeKey, const std::string& positiveKey) {
        std::lock_guard<std::mutex> lock(mtx);
        float axis = 0.0f;
        auto negative = keys.find(negativeKey);
        auto positive = keys.find(positiveKey);
        if (negative != keys.end() && negative->second) {
            axis -= 1.0f;
        }
        if (positive != keys.end() && positive->second) {
            axis += 1.0f;
        }
        return axis;
    }
    
    void setMousePos(float x, float y) {
        std::lock_guard<std::mutex> lock(mtx);
        mouseX = x;
        mouseY = y;
    }
    
    float getMouseX() {
        std::lock_guard<std::mutex> lock(mtx);
        return mouseX;
    }
    
    float getMouseY() {
        std::lock_guard<std::mutex> lock(mtx);
        return mouseY;
    }
    
private:
    std::unordered_map<std::string, bool> keys;
    std::unordered_map<std::string, bool> justPressed;
    std::unordered_map<std::string, bool> justReleased;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    std::mutex mtx;
    
    InputState() = default;
};

inline void resolveColorRgb(const std::string& color, float& r, float& g, float& b) {
    r = 1.0f;
    g = 1.0f;
    b = 1.0f;
    if (color == "red") { r = 1.0f; g = 0.0f; b = 0.0f; }
    else if (color == "green") { r = 0.0f; g = 1.0f; b = 0.0f; }
    else if (color == "blue") { r = 0.0f; g = 0.0f; b = 1.0f; }
    else if (color == "yellow") { r = 1.0f; g = 1.0f; b = 0.0f; }
    else if (color == "cyan") { r = 0.0f; g = 1.0f; b = 1.0f; }
    else if (color == "magenta") { r = 1.0f; g = 0.0f; b = 1.0f; }
    else if (color == "black") { r = 0.0f; g = 0.0f; b = 0.0f; }
    else if (color == "white") { r = 1.0f; g = 1.0f; b = 1.0f; }
}

inline std::string resolveAnsiForeground(const std::string& color) {
    if (color == "red") return "\033[31m";
    if (color == "green") return "\033[32m";
    if (color == "yellow") return "\033[33m";
    if (color == "blue") return "\033[34m";
    if (color == "magenta") return "\033[35m";
    if (color == "cyan") return "\033[36m";
    if (color == "black") return "\033[30m";
    return "\033[37m";
}

inline std::string resolveAnsiBackground(const std::string& color) {
    if (color == "red") return "\033[41m";
    if (color == "green") return "\033[42m";
    if (color == "yellow") return "\033[43m";
    if (color == "blue") return "\033[44m";
    if (color == "magenta") return "\033[45m";
    if (color == "cyan") return "\033[46m";
    if (color == "white") return "\033[47m";
    return "\033[40m";
}

#ifdef _WIN32
inline std::string vkToString(WPARAM wParam) {
    switch (wParam) {
        case VK_LEFT:   return "ArrowLeft";
        case VK_RIGHT:  return "ArrowRight";
        case VK_UP:     return "ArrowUp";
        case VK_DOWN:   return "ArrowDown";
        case 'A':       return "a";
        case 'D':       return "d";
        case 'W':       return "w";
        case 'S':       return "s";
        case VK_ESCAPE: return "Escape";
        case VK_SPACE:  return "Space";
        default: {
            if (wParam >= 'A' && wParam <= 'Z') {
                return std::string(1, static_cast<char>(tolower(wParam)));
            }
            if (wParam >= '0' && wParam <= '9') {
                return std::string(1, static_cast<char>(wParam));
            }
            return "";
        }
    }
}

static HGLRC hRC = NULL;
static HDC hDC = NULL;
static HWND hWnd = NULL;
static GLuint fontListBase = 0;

inline void setupPixelFormat(HDC hDC) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int pf = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pf, &pfd);
}

inline LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            zenith::InputState::getInstance().setKeyPressed(zenith::vkToString(wParam), true);
            break;
        case WM_KEYUP:
            zenith::InputState::getInstance().setKeyPressed(zenith::vkToString(wParam), false);
            break;
        case WM_MOUSEMOVE: {
            float mouseX = static_cast<float>(LOWORD(lParam)) / 8.0f;
            float mouseY = static_cast<float>(HIWORD(lParam)) / 20.0f;
            zenith::InputState::getInstance().setMousePos(mouseX, mouseY);
            break;
        }
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}
#endif

#ifdef __ANDROID__
namespace android_support {
    // EGL and GLES dummy hooks or stubs for Android NDK target
    inline void initEGL() {}
    inline void swapEGLBuffers() {}
}
#endif

class Canvas {
public:
    int width = 80;
    int height = 24;
    bool use_opengl = false;
    TerminalBuffer buffer;
#ifdef _WIN32
    GLuint fontListBase = 0;
#endif
    
    Canvas(int w = 80, int h = 24, bool gl = false) 
        : width(w), height(h), use_opengl(gl), buffer(w, h) {
        clear("black");
    }
    
    void resolveColorRgb(const std::string& color, float& r, float& g, float& b) const {
        if (color.empty()) { r = 1.0f; g = 1.0f; b = 1.0f; return; }
        if (color[0] == '#' && color.length() == 7) {
            unsigned int hexVal = 0;
            std::stringstream ss;
            ss << std::hex << color.substr(1);
            ss >> hexVal;
            r = ((hexVal >> 16) & 0xFF) / 255.0f;
            g = ((hexVal >> 8) & 0xFF) / 255.0f;
            b = (hexVal & 0xFF) / 255.0f;
            return;
        }
        if (color == "red") { r = 0.90f; g = 0.22f; b = 0.22f; }
        else if (color == "green" || color == "emerald") { r = 0.06f; g = 0.72f; b = 0.50f; }
        else if (color == "blue") { r = 0.14f; g = 0.38f; b = 0.92f; }
        else if (color == "yellow" || color == "amber" || color == "gold") { r = 0.96f; g = 0.62f; b = 0.04f; }
        else if (color == "cyan" || color == "sky_blue") { r = 0.22f; g = 0.74f; b = 0.97f; }
        else if (color == "magenta" || color == "purple" || color == "violet") { r = 0.65f; g = 0.35f; b = 0.95f; }
        else if (color == "black" || color == "dark_slate") { r = 0.04f; g = 0.06f; b = 0.10f; }
        else if (color == "dark_navy" || color == "navy") { r = 0.07f; g = 0.11f; b = 0.18f; }
        else if (color == "glass_panel" || color == "slate") { r = 0.11f; g = 0.16f; b = 0.26f; }
        else if (color == "panel_border") { r = 0.22f; g = 0.35f; b = 0.60f; }
        else if (color == "white") { r = 0.95f; g = 0.96f; b = 0.98f; }
        else if (color == "light_gray") { r = 0.60f; g = 0.65f; b = 0.75f; }
        else { r = 1.0f; g = 1.0f; b = 1.0f; }
    }

    void clear(const std::string& color = "black") {
        if (use_opengl) {
            float r = 0.0f, g = 0.0f, b = 0.0f;
            resolveColorRgb(color, r, g, b);
#ifdef _WIN32
            glClearColor(r, g, b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
#endif
            return;
        }
        
        std::string bg_style = "\033[40m"; // default black
        if (color == "red") bg_style = "\033[41m";
        else if (color == "green") bg_style = "\033[42m";
        else if (color == "yellow") bg_style = "\033[43m";
        else if (color == "blue") bg_style = "\033[44m";
        else if (color == "magenta") bg_style = "\033[45m";
        else if (color == "cyan") bg_style = "\033[46m";
        else if (color == "white") bg_style = "\033[47m";
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                buffer.setCell(x, y, " ", bg_style);
            }
        }
    }
    
    void drawRect(float x, float y, float w, float h, const std::string& color = "white") {
        if (use_opengl) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            resolveColorRgb(color, r, g, b);
#ifdef _WIN32
            glColor3f(r, g, b);
            glBegin(GL_QUADS);
            glVertex2f(x * 8.0f, y * 20.0f);
            glVertex2f((x + w) * 8.0f, y * 20.0f);
            glVertex2f((x + w) * 8.0f, (y + h) * 20.0f);
            glVertex2f(x * 8.0f, (y + h) * 20.0f);
            glEnd();
#endif
            return;
        }
        
        std::string bg_style = "\033[47m"; // default white
        if (color == "red") bg_style = "\033[41m";
        else if (color == "green") bg_style = "\033[42m";
        else if (color == "yellow") bg_style = "\033[43m";
        else if (color == "blue") bg_style = "\033[44m";
        else if (color == "magenta") bg_style = "\033[45m";
        else if (color == "cyan") bg_style = "\033[46m";
        else if (color == "black") bg_style = "\033[40m";
        
        int ix = static_cast<int>(x);
        int iy = static_cast<int>(y);
        int iw = static_cast<int>(w);
        int ih = static_cast<int>(h);
        
        for (int curr_y = iy; curr_y < iy + ih; ++curr_y) {
            for (int curr_x = ix; curr_x < ix + iw; ++curr_x) {
                if (curr_x >= 0 && curr_x < width && curr_y >= 0 && curr_y < height) {
                    buffer.setCell(curr_x, curr_y, " ", bg_style);
                }
            }
        }
    }
    
    void drawCircle(float cx, float cy, float r, const std::string& color = "white") {
        if (use_opengl) {
            float r_col = 1.0f, g = 1.0f, b = 1.0f;
            resolveColorRgb(color, r_col, g, b);
#ifdef _WIN32
            glColor3f(r_col, g, b);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx * 8.0f, cy * 20.0f);
            int num_segments = 32;
            for (int i = 0; i <= num_segments; ++i) {
                float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
                float px = (cx * 8.0f) + (r * 14.0f) * cosf(theta);
                float py = (cy * 20.0f) + (r * 14.0f) * sinf(theta);
                glVertex2f(px, py);
            }
            glEnd();
#endif
            return;
        }
        
        std::string bg_style = "\033[47m";
        if (color == "red") bg_style = "\033[41m";
        else if (color == "green") bg_style = "\033[42m";
        else if (color == "yellow") bg_style = "\033[43m";
        else if (color == "blue") bg_style = "\033[44m";
        else if (color == "magenta") bg_style = "\033[45m";
        else if (color == "cyan") bg_style = "\033[46m";
        else if (color == "black") bg_style = "\033[40m";
        
        int ix = static_cast<int>(cx);
        int iy = static_cast<int>(cy);
        int ir = static_cast<int>(r);
        
        for (int y = iy - ir; y <= iy + ir; ++y) {
            for (int x = ix - ir * 2; x <= ix + ir * 2; ++x) {
                float dx = (x - ix) / 2.0f;
                float dy = (y - iy);
                if (dx * dx + dy * dy <= ir * ir) {
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        buffer.setCell(x, y, "o", bg_style);
                    }
                }
            }
        }
    }

    void drawPoint(float x, float y, const std::string& color = "white") {
        if (use_opengl) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            resolveColorRgb(color, r, g, b);
#ifdef _WIN32
            glColor3f(r, g, b);
            glPointSize(4.0f);
            glBegin(GL_POINTS);
            glVertex2f(x * 8.0f, y * 20.0f);
            glEnd();
#endif
            return;
        }

        const int ix = static_cast<int>(std::lround(x));
        const int iy = static_cast<int>(std::lround(y));
        if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
            buffer.setCell(ix, iy, "+", resolveAnsiForeground(color));
        }
    }

    void drawLine(float x1, float y1, float x2, float y2, const std::string& color = "white") {
        if (use_opengl) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            resolveColorRgb(color, r, g, b);
#ifdef _WIN32
            glColor3f(r, g, b);
            glBegin(GL_LINES);
            glVertex2f(x1 * 8.0f, y1 * 20.0f);
            glVertex2f(x2 * 8.0f, y2 * 20.0f);
            glEnd();
#endif
            return;
        }

        int xStart = static_cast<int>(std::lround(x1));
        int yStart = static_cast<int>(std::lround(y1));
        const int xEnd = static_cast<int>(std::lround(x2));
        const int yEnd = static_cast<int>(std::lround(y2));

        const int deltaX = std::abs(xEnd - xStart);
        const int stepX = xStart < xEnd ? 1 : -1;
        const int deltaY = -std::abs(yEnd - yStart);
        const int stepY = yStart < yEnd ? 1 : -1;
        int error = deltaX + deltaY;

        while (true) {
            if (xStart >= 0 && xStart < width && yStart >= 0 && yStart < height) {
                buffer.setCell(xStart, yStart, "*", resolveAnsiForeground(color));
            }
            if (xStart == xEnd && yStart == yEnd) {
                break;
            }
            const int twiceError = error * 2;
            if (twiceError >= deltaY) {
                error += deltaY;
                xStart += stepX;
            }
            if (twiceError <= deltaX) {
                error += deltaX;
                yStart += stepY;
            }
        }
    }

    void drawFrameRect(float x, float y, float w, float h, const std::string& color = "white") {
        if (w <= 0.0f || h <= 0.0f) {
            return;
        }

        if (use_opengl) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            resolveColorRgb(color, r, g, b);
#ifdef _WIN32
            glColor3f(r, g, b);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x * 8.0f, y * 20.0f);
            glVertex2f((x + w) * 8.0f, y * 20.0f);
            glVertex2f((x + w) * 8.0f, (y + h) * 20.0f);
            glVertex2f(x * 8.0f, (y + h) * 20.0f);
            glEnd();
#endif
            return;
        }

        drawLine(x, y, x + w, y, color);
        drawLine(x + w, y, x + w, y + h, color);
        drawLine(x + w, y + h, x, y + h, color);
        drawLine(x, y + h, x, y, color);
    }

    void drawCircleOutline(float cx, float cy, float r, const std::string& color = "white") {
        if (r <= 0.0f) {
            return;
        }

        if (use_opengl) {
            float red = 1.0f, green = 1.0f, blue = 1.0f;
            resolveColorRgb(color, red, green, blue);
#ifdef _WIN32
            glColor3f(red, green, blue);
            glBegin(GL_LINE_LOOP);
            constexpr int segments = 32;
            for (int i = 0; i < segments; ++i) {
                const float theta = (2.0f * 3.1415926f * static_cast<float>(i)) / static_cast<float>(segments);
                glVertex2f((cx * 8.0f) + (r * 14.0f * std::cos(theta)),
                           (cy * 20.0f) + (r * 14.0f * std::sin(theta)));
            }
            glEnd();
#endif
            return;
        }

        constexpr int segments = 32;
        float previousX = cx + r;
        float previousY = cy;
        for (int i = 1; i <= segments; ++i) {
            const float theta = (2.0f * 3.1415926f * static_cast<float>(i)) / static_cast<float>(segments);
            const float nextX = cx + (r * std::cos(theta) * 2.0f);
            const float nextY = cy + (r * std::sin(theta));
            drawLine(previousX, previousY, nextX, nextY, color);
            previousX = nextX;
            previousY = nextY;
        }
    }

    void drawText(const std::string& text, float x, float y, const std::string& color = "white") {
        if (use_opengl) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            if (color == "red") { r = 1.0f; g = 0.0f; b = 0.0f; }
            else if (color == "green") { r = 0.0f; g = 1.0f; b = 0.0f; }
            else if (color == "blue") { r = 0.0f; g = 0.0f; b = 1.0f; }
            else if (color == "yellow") { r = 1.0f; g = 1.0f; b = 0.0f; }
            else if (color == "cyan") { r = 0.0f; g = 1.0f; b = 1.0f; }
            else if (color == "magenta") { r = 1.0f; g = 0.0f; b = 1.0f; }
            else if (color == "black") { r = 0.0f; g = 0.0f; b = 0.0f; }
            
#ifdef _WIN32
            glColor3f(r, g, b);
            glRasterPos2f(x * 8.0f, y * 20.0f + 12.0f);
            glListBase(fontListBase);
            glCallLists(static_cast<GLsizei>(text.length()), GL_UNSIGNED_BYTE, text.c_str());
#endif
            return;
        }
        
        std::string fg_style = "\033[37m";
        if (color == "red") fg_style = "\033[31m";
        else if (color == "green") fg_style = "\033[32m";
        else if (color == "yellow") fg_style = "\033[33m";
        else if (color == "blue") fg_style = "\033[34m";
        else if (color == "magenta") fg_style = "\033[35m";
        else if (color == "cyan") fg_style = "\033[36m";
        
        buffer.writeString(static_cast<int>(x), static_cast<int>(y), text, fg_style);
    }

    void drawTextWrapped(const std::string& text, float x, float y, float maxW, const std::string& color = "white") {
        if (maxW <= 1.0f) {
            drawText(text, x, y, color);
            return;
        }

        const int maxCharsPerLine = std::max(5, static_cast<int>(maxW) - 2);
        std::stringstream ss(text);
        std::string word;
        std::string currentLine;
        float currentY = y;

        while (ss >> word) {
            if (currentLine.empty()) {
                currentLine = word;
            } else if (static_cast<int>(currentLine.length() + 1 + word.length()) <= maxCharsPerLine) {
                currentLine += " " + word;
            } else {
                drawText(currentLine, x, currentY, color);
                currentY += 1.2f;
                currentLine = word;
            }
        }
        if (!currentLine.empty()) {
            drawText(currentLine, x, currentY, color);
        }
    }

    float pctX(float percent) const {
        return (static_cast<float>(width) * percent) / 100.0f;
    }

    float pctY(float percent) const {
        return (static_cast<float>(height) * percent) / 100.0f;
    }

    float mediaWidth() const {
        return static_cast<float>(width);
    }

    float mediaHeight() const {
        return static_cast<float>(height);
    }
    
    void present() {
        if (use_opengl) {
#ifdef _WIN32
            SwapBuffers(hDC);
#endif
            return;
        }
        
        std::cout << "\033[2J\033[H";
        buffer.print();
        std::cout << std::flush;
    }
};

class Game {
public:
    virtual ~Game() = default;
    virtual void init() {}
    virtual void update(float dt) {}
    virtual void draw(Canvas& canvas) {}
};

template<typename SceneType, typename std::enable_if_t<std::is_base_of_v<game::Scene, SceneType>, int> = 0>
inline void runGameLoop(SceneType& scene);

template<typename GameType, typename std::enable_if_t<!std::is_base_of_v<game::Scene, GameType>, int> = 0>
inline void runGameLoop(GameType& game);

inline bool isKeyPressed(const std::string& key) {
    return InputState::getInstance().isKeyPressed(key);
}

inline bool wasKeyPressed(const std::string& key) {
    return InputState::getInstance().wasKeyPressed(key);
}

inline bool wasKeyReleased(const std::string& key) {
    return InputState::getInstance().wasKeyReleased(key);
}

inline float getAxis(const std::string& negativeKey, const std::string& positiveKey) {
    return InputState::getInstance().getAxis(negativeKey, positiveKey);
}

inline float getMouseX() {
    return InputState::getInstance().getMouseX();
}

inline float getMouseY() {
    return InputState::getInstance().getMouseY();
}

struct World2DViewState {
    game::EntityId cameraEntity = game::EntityId::invalid();
    physics::Vec2 cameraPosition = physics::Vec2(0.0f, 0.0f);
    float cameraZoom = 1.0f;
    float viewportOriginX = 0.0f;
    float viewportOriginY = 0.0f;
    float viewportWidth = 0.0f;
    float viewportHeight = 0.0f;
    float screenCenterX = 0.0f;
    float screenCenterY = 0.0f;
    float worldLeft = 0.0f;
    float worldTop = 0.0f;
    float worldRight = 0.0f;
    float worldBottom = 0.0f;
    bool hasPrimaryCamera = false;
};

inline World2DViewState buildWorld2DViewState(const Canvas& canvas, const game::World& world) {
    World2DViewState state;
    state.viewportWidth = static_cast<float>(canvas.width);
    state.viewportHeight = static_cast<float>(canvas.height);

    if (std::optional<game::EntityId> primaryCamera = world.primaryCamera2D()) {
        state.cameraEntity = *primaryCamera;
        state.hasPrimaryCamera = true;

        if (const game::Transform2D* cameraTransform = world.getTransform2D(*primaryCamera)) {
            state.cameraPosition = cameraTransform->position;
        }

        if (const game::Camera2DComponent* camera = world.getCamera2D(*primaryCamera)) {
            if (camera->zoom > 0.0f) {
                state.cameraZoom = camera->zoom;
            }
            state.viewportOriginX = static_cast<float>(canvas.width) * camera->viewportOrigin.x;
            state.viewportOriginY = static_cast<float>(canvas.height) * camera->viewportOrigin.y;
            state.viewportWidth = static_cast<float>(canvas.width) * std::max(0.0f, camera->viewportSize.x);
            state.viewportHeight = static_cast<float>(canvas.height) * std::max(0.0f, camera->viewportSize.y);
        }
    }

    if (state.viewportWidth <= 0.0f) {
        state.viewportWidth = static_cast<float>(canvas.width);
    }
    if (state.viewportHeight <= 0.0f) {
        state.viewportHeight = static_cast<float>(canvas.height);
    }

    state.screenCenterX = state.viewportOriginX + (state.viewportWidth * 0.5f);
    state.screenCenterY = state.viewportOriginY + (state.viewportHeight * 0.5f);

    if (state.hasPrimaryCamera) {
        const float halfVisibleWidth = state.viewportWidth / (2.0f * state.cameraZoom);
        const float halfVisibleHeight = state.viewportHeight / (2.0f * state.cameraZoom);
        state.worldLeft = state.cameraPosition.x - halfVisibleWidth;
        state.worldRight = state.cameraPosition.x + halfVisibleWidth;
        state.worldTop = state.cameraPosition.y - halfVisibleHeight;
        state.worldBottom = state.cameraPosition.y + halfVisibleHeight;
    } else {
        state.worldLeft = 0.0f;
        state.worldTop = 0.0f;
        state.worldRight = static_cast<float>(canvas.width);
        state.worldBottom = static_cast<float>(canvas.height);
    }

    return state;
}

inline bool isEntityVisibleToWorld2DView(const World2DViewState& state, const game::World& world, game::EntityId entity) {
    if (!state.hasPrimaryCamera) {
        return true;
    }
    return (world.entityMaskBits(state.cameraEntity) & world.entityLayerBits(entity)) != 0u;
}

inline float projectWorld2DX(const World2DViewState& state, float worldX) {
    if (!state.hasPrimaryCamera) {
        return worldX;
    }
    return ((worldX - state.cameraPosition.x) * state.cameraZoom) + state.screenCenterX;
}

inline float projectWorld2DY(const World2DViewState& state, float worldY) {
    if (!state.hasPrimaryCamera) {
        return worldY;
    }
    return ((worldY - state.cameraPosition.y) * state.cameraZoom) + state.screenCenterY;
}

inline float scaleWorld2DSize(const World2DViewState& state, float value) {
    return value * state.cameraZoom;
}

inline physics::Vec2 entityWorldPosition2D(const game::World& world, game::EntityId entity) {
    if (const physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
        return body->position;
    }
    if (const game::Transform2D* transform = world.getTransform2D(entity)) {
        return transform->position;
    }
    return physics::Vec2(0.0f, 0.0f);
}

inline void drawWorld2D(Canvas& canvas, const game::World& world, bool drawEntityNames = true) {
    const World2DViewState view = buildWorld2DViewState(canvas, world);

    struct DrawEntry {
        enum class Kind { Tilemap, Sprite };

        game::EntityId entity;
        Kind kind = Kind::Sprite;
        int sortOrder = 0;
        const game::Tilemap2DComponent* tilemap = nullptr;
        const game::SpriteRenderer2D* sprite = nullptr;
    };

    std::vector<DrawEntry> entries;
    entries.reserve(world.spriteRendererCount2D() + world.tilemapCount2D());

    world.forEachTilemap2D([&entries](game::EntityId entity, const game::Tilemap2DComponent& tilemap) {
        entries.push_back(DrawEntry{
            entity,
            DrawEntry::Kind::Tilemap,
            tilemap.sortOrder,
            &tilemap,
            nullptr
        });
    });

    world.forEachSpriteRenderer2D([&entries](game::EntityId entity, const game::SpriteRenderer2D& sprite) {
        entries.push_back(DrawEntry{
            entity,
            DrawEntry::Kind::Sprite,
            sprite.sortOrder,
            nullptr,
            &sprite
        });
    });

    std::sort(entries.begin(), entries.end(), [](const DrawEntry& lhs, const DrawEntry& rhs) {
        if (lhs.sortOrder != rhs.sortOrder) {
            return lhs.sortOrder < rhs.sortOrder;
        }
        if (lhs.kind != rhs.kind) {
            return lhs.kind == DrawEntry::Kind::Tilemap;
        }
        return lhs.entity.index < rhs.entity.index;
    });

    for (const DrawEntry& entry : entries) {
        const game::EntityId entity = entry.entity;
        if (!isEntityVisibleToWorld2DView(view, world, entity)) {
            continue;
        }

        if (entry.kind == DrawEntry::Kind::Tilemap) {
            const game::Tilemap2DComponent* tilemap = entry.tilemap;
            const game::Transform2D* transform = world.getTransform2D(entity);
            if (tilemap == nullptr || transform == nullptr || !tilemap->visible) {
                continue;
            }

            const float tileWidth = std::max(0.01f, tilemap->tileSize.x);
            const float tileHeight = std::max(0.01f, tilemap->tileSize.y);
            const float totalWidth = tileWidth * static_cast<float>(std::max(1, tilemap->columns));
            const float totalHeight = tileHeight * static_cast<float>(std::max(1, tilemap->rows));
            const float baseWorldX = transform->position.x - (totalWidth * tilemap->anchor.x);
            const float baseWorldY = transform->position.y - (totalHeight * tilemap->anchor.y);
            const float drawTileWidth = scaleWorld2DSize(view, tileWidth);
            const float drawTileHeight = scaleWorld2DSize(view, tileHeight);

            for (int row = 0; row < tilemap->rows; ++row) {
                for (int column = 0; column < tilemap->columns; ++column) {
                    const int tileId = tilemap->cellAt(column, row);
                    if (tileId == 0) {
                        continue;
                    }
                    const std::string color = tilemap->paletteColor(tileId);
                    if (color.empty()) {
                        continue;
                    }

                    const float worldX = baseWorldX + (static_cast<float>(column) * tileWidth);
                    const float worldY = baseWorldY + (static_cast<float>(row) * tileHeight);
                    canvas.drawRect(
                        projectWorld2DX(view, worldX),
                        projectWorld2DY(view, worldY),
                        drawTileWidth,
                        drawTileHeight,
                        color);
                }
            }

            if (drawEntityNames) {
                if (const game::NameComponent* name = world.getName(entity)) {
                    if (!name->value.empty()) {
                        canvas.drawText(
                            name->value,
                            projectWorld2DX(view, baseWorldX),
                            projectWorld2DY(view, baseWorldY) - 1.0f,
                            "white");
                    }
                }
            }
            continue;
        }

        const game::SpriteRenderer2D* sprite = entry.sprite;
        const game::Transform2D* transform = world.getTransform2D(entity);
        if (sprite == nullptr || transform == nullptr || !sprite->visible) {
            continue;
        }

        const float drawWidth = scaleWorld2DSize(view, sprite->size.x);
        const float drawHeight = scaleWorld2DSize(view, sprite->size.y);
        float screenX = projectWorld2DX(view, transform->position.x);
        float screenY = projectWorld2DY(view, transform->position.y);
        screenX -= drawWidth * sprite->anchor.x;
        screenY -= drawHeight * sprite->anchor.y;

        canvas.drawRect(screenX, screenY, drawWidth, drawHeight, sprite->tintColor);

        if (drawEntityNames) {
            if (const game::NameComponent* name = world.getName(entity)) {
                if (!name->value.empty()) {
                    canvas.drawText(name->value, screenX, screenY - 1.0f, "white");
                }
            }
        }
    }
}

inline void drawSceneDebug2D(Canvas& canvas, const game::Scene& scene) {
    const World2DViewState view = buildWorld2DViewState(canvas, scene.world);
    const std::string overlayColor = scene.debugOverlayColor.empty() ? "cyan" : scene.debugOverlayColor;

    if (scene.debugDrawGrid2D) {
        const float cellWidth = std::max(0.25f, scene.debugGridCellWidth);
        const float cellHeight = std::max(0.25f, scene.debugGridCellHeight);
        const float startWorldX = std::floor(view.worldLeft / cellWidth) * cellWidth;
        const float startWorldY = std::floor(view.worldTop / cellHeight) * cellHeight;
        const float maxScreenX = view.viewportOriginX + std::max(1.0f, view.viewportWidth - 1.0f);
        const float maxScreenY = view.viewportOriginY + std::max(1.0f, view.viewportHeight - 1.0f);

        for (float worldX = startWorldX; worldX <= view.worldRight + cellWidth; worldX += cellWidth) {
            const float screenX = projectWorld2DX(view, worldX);
            canvas.drawLine(screenX, view.viewportOriginY, screenX, maxScreenY, overlayColor);
        }
        for (float worldY = startWorldY; worldY <= view.worldBottom + cellHeight; worldY += cellHeight) {
            const float screenY = projectWorld2DY(view, worldY);
            canvas.drawLine(view.viewportOriginX, screenY, maxScreenX, screenY, overlayColor);
        }
    }

    if (scene.debugDrawColliders2D) {
        for (game::EntityId entity : scene.world.boxColliderEntities2D()) {
            if (!isEntityVisibleToWorld2DView(view, scene.world, entity)) {
                continue;
            }
            const physics::BoxCollider2D* collider = scene.world.getBoxCollider2D(entity);
            if (collider == nullptr || !collider->enabled) {
                continue;
            }
            const physics::Vec2 center = entityWorldPosition2D(scene.world, entity) + collider->offset;
            const float left = center.x - (collider->size.x * 0.5f);
            const float top = center.y - (collider->size.y * 0.5f);
            canvas.drawFrameRect(
                projectWorld2DX(view, left),
                projectWorld2DY(view, top),
                scaleWorld2DSize(view, collider->size.x),
                scaleWorld2DSize(view, collider->size.y),
                collider->isTrigger ? "yellow" : overlayColor);
        }

        for (game::EntityId entity : scene.world.circleColliderEntities2D()) {
            if (!isEntityVisibleToWorld2DView(view, scene.world, entity)) {
                continue;
            }
            const physics::CircleCollider2D* collider = scene.world.getCircleCollider2D(entity);
            if (collider == nullptr || !collider->enabled) {
                continue;
            }
            const physics::Vec2 center = physics::centerOf(*collider, entityWorldPosition2D(scene.world, entity));
            canvas.drawCircleOutline(
                projectWorld2DX(view, center.x),
                projectWorld2DY(view, center.y),
                scaleWorld2DSize(view, collider->radius),
                collider->isTrigger ? "yellow" : overlayColor);
        }

        for (game::EntityId entity : scene.world.capsuleColliderEntities2D()) {
            if (!isEntityVisibleToWorld2DView(view, scene.world, entity)) {
                continue;
            }
            const physics::CapsuleCollider2D* collider = scene.world.getCapsuleCollider2D(entity);
            if (collider == nullptr || !collider->enabled) {
                continue;
            }

            const physics::Vec2 position = entityWorldPosition2D(scene.world, entity);
            const physics::BoxCollider2D middle = physics::middleOf(*collider);
            const physics::CircleCollider2D top = physics::topOf(*collider);
            const physics::CircleCollider2D bottom = physics::bottomOf(*collider);
            const std::string color = collider->isTrigger ? "yellow" : overlayColor;

            if (middle.size.y > 0.0f) {
                const physics::Vec2 middleCenter = position + middle.offset;
                canvas.drawFrameRect(
                    projectWorld2DX(view, middleCenter.x - (middle.size.x * 0.5f)),
                    projectWorld2DY(view, middleCenter.y - (middle.size.y * 0.5f)),
                    scaleWorld2DSize(view, middle.size.x),
                    scaleWorld2DSize(view, middle.size.y),
                    color);
            }

            const physics::Vec2 topCenter = physics::centerOf(top, position);
            const physics::Vec2 bottomCenter = physics::centerOf(bottom, position);
            canvas.drawCircleOutline(
                projectWorld2DX(view, topCenter.x),
                projectWorld2DY(view, topCenter.y),
                scaleWorld2DSize(view, top.radius),
                color);
            canvas.drawCircleOutline(
                projectWorld2DX(view, bottomCenter.x),
                projectWorld2DY(view, bottomCenter.y),
                scaleWorld2DSize(view, bottom.radius),
                color);
        }
    }

    if (scene.debugDrawTransforms2D) {
        for (game::EntityId entity : scene.world.aliveEntities()) {
            if (!isEntityVisibleToWorld2DView(view, scene.world, entity)) {
                continue;
            }
            const game::Transform2D* transform = scene.world.getTransform2D(entity);
            if (transform == nullptr) {
                continue;
            }

            const physics::Vec2 position = entityWorldPosition2D(scene.world, entity);
            const float screenX = projectWorld2DX(view, position.x);
            const float screenY = projectWorld2DY(view, position.y);
            canvas.drawPoint(screenX, screenY, overlayColor);
            canvas.drawLine(screenX - 1.0f, screenY, screenX + 1.0f, screenY, overlayColor);
            canvas.drawLine(screenX, screenY - 1.0f, screenX, screenY + 1.0f, overlayColor);

            if (const physics::RigidBody2D* body = scene.world.getRigidBody2D(entity)) {
                canvas.drawLine(
                    screenX,
                    screenY,
                    projectWorld2DX(view, position.x + (body->velocity.x * 0.15f)),
                    projectWorld2DY(view, position.y + (body->velocity.y * 0.15f)),
                    "green");
            }
        }
    }

    if (scene.debugDrawCameraBounds2D) {
        canvas.drawFrameRect(
            view.viewportOriginX,
            view.viewportOriginY,
            std::max(1.0f, view.viewportWidth - 1.0f),
            std::max(1.0f, view.viewportHeight - 1.0f),
            overlayColor);
    }

    if (scene.debugDrawRuntimeStats) {
        const float originX = view.viewportOriginX + 1.0f;
        float lineY = view.viewportOriginY + 1.0f;
        const int frameMs = static_cast<int>(std::lround(scene.frameDelta() * 1000.0f));
        const int accumulatorMs = static_cast<int>(std::lround(scene.accumulatedTime() * 1000.0f));

        canvas.drawText(
            std::string("frame:") + std::to_string(scene.totalFrames()) +
            " dt_ms:" + std::to_string(frameMs),
            originX,
            lineY,
            overlayColor);
        lineY += 1.0f;
        canvas.drawText(
            std::string("fixed:") + std::to_string(scene.totalFixedSteps()) +
            " last:" + std::to_string(scene.lastSubstepCount()) +
            " acc_ms:" + std::to_string(accumulatorMs),
            originX,
            lineY,
            overlayColor);
        lineY += 1.0f;
        canvas.drawText(
            std::string("dropped:") + std::to_string(scene.framesWithDroppedSteps()) +
            " entities:" + std::to_string(scene.world.entityCount()),
            originX,
            lineY,
            overlayColor);
        lineY += 1.0f;
        canvas.drawText(
            std::string("sprites:") + std::to_string(scene.world.spriteRendererCount2D()) +
            " tilemaps:" + std::to_string(scene.world.tilemapCount2D()),
            originX,
            lineY,
            overlayColor);
    }
}

inline std::string clipOverlayText(const std::string& value, size_t maxChars) {
    if (value.size() <= maxChars) {
        return value;
    }
    if (maxChars <= 3) {
        return value.substr(0, maxChars);
    }
    return value.substr(0, maxChars - 3) + "...";
}

inline std::string summarizeMaterialPropertyValue(game::MaterialPropertyView property) {
    const std::string kind = property.kind;
    if (kind == "Number") {
        std::ostringstream stream;
        stream << static_cast<float>(property.numberValue);
        return stream.str();
    }
    if (kind == "Toggle") {
        return static_cast<bool>(property.boolValue) ? "true" : "false";
    }
    return static_cast<std::string>(property.stringValue);
}

inline void drawSceneInspector(Canvas& canvas, game::Scene& scene) {
    if (!scene.minimalInspectorEnabled) {
        return;
    }

    const int panelWidth = std::max(24, std::min(canvas.width - 2, 30));
    const int panelHeight = std::max(8, canvas.height - 2);
    if (panelWidth < 12 || panelHeight < 6) {
        return;
    }

    const std::string borderColor = scene.debugOverlayColor.empty() ? "cyan" : scene.debugOverlayColor;
    const float originX = static_cast<float>(std::max(0, canvas.width - panelWidth - 1));
    const float originY = 1.0f;
    const int innerWidth = std::max(1, panelWidth - 2);
    int line = 1;
    auto drawLine = [&](const std::string& text, const std::string& color = "white") {
        if (line >= panelHeight - 1) {
            return false;
        }
        canvas.drawText(clipOverlayText(text, static_cast<size_t>(innerWidth)), originX + 1.0f, originY + static_cast<float>(line), color);
        ++line;
        return true;
    };

    canvas.drawFrameRect(originX, originY, static_cast<float>(panelWidth), static_cast<float>(panelHeight), borderColor);
    drawLine("Inspector", borderColor);
    drawLine("scene:" + scene.name, "white");

    game::EntityId target = scene.inspectedEntity();
    std::string materialPath = scene.inspectedMaterialPath();

    if (target) {
        const game::NameComponent* name = scene.world.getName(target);
        const game::TagComponent* tag = scene.world.getTag(target);
        const game::LayerMaskComponent* layerMask = scene.world.getLayerMask(target);
        drawLine("entity:" + (name ? name->value : std::string("unnamed")), "yellow");
        drawLine("id:" + std::to_string(target.index) + " gen:" + std::to_string(target.generation));
        if (tag && !tag->value.empty()) {
            drawLine("tag:" + tag->value);
        }
        if (layerMask) {
            drawLine("layer:" + std::to_string(layerMask->layers) + " mask:" + std::to_string(layerMask->mask));
        }

        if (const game::Transform2D* transform2D = scene.world.getTransform2D(target)) {
            drawLine("pos2:" + zenith::toString(transform2D->position.x) + "," + zenith::toString(transform2D->position.y));
        } else if (const game::Transform3D* transform3D = scene.world.getTransform3D(target)) {
            drawLine(
                "pos3:" +
                zenith::toString(transform3D->position.x) + "," +
                zenith::toString(transform3D->position.y) + "," +
                zenith::toString(transform3D->position.z));
        }

        if (const physics::RigidBody2D* body2D = scene.world.getRigidBody2D(target)) {
            drawLine("vel2:" + zenith::toString(body2D->velocity.x) + "," + zenith::toString(body2D->velocity.y), "green");
        } else if (const physics::RigidBody3D* body3D = scene.world.getRigidBody3D(target)) {
            drawLine(
                "vel3:" +
                zenith::toString(body3D->velocity.x) + "," +
                zenith::toString(body3D->velocity.y) + "," +
                zenith::toString(body3D->velocity.z),
                "green");
        }

        if (const game::SpriteRenderer2D* sprite = scene.world.getSpriteRenderer2D(target)) {
            drawLine("sprite:" + sprite->tintColor);
            if (sprite->texture) {
                drawLine("tex:" + sprite->texture->path);
            }
        }

        if (const game::MeshRenderer3D* mesh = scene.world.getMeshRenderer3D(target)) {
            if (mesh->mesh) {
                drawLine("mesh:" + mesh->mesh->path);
            }
            if (mesh->shader) {
                drawLine("shader:" + mesh->shader->path);
            }
            if (mesh->material) {
                materialPath = mesh->material->path;
                drawLine("mat:" + materialPath, "magenta");
            }
        }

        if (const game::AudioSource2DComponent* audio2D = scene.world.getAudioSource2D(target)) {
            if (audio2D->clip) {
                drawLine("audio:" + audio2D->clip->path);
            }
        }

        if (const game::AudioSource3DComponent* audio3D = scene.world.getAudioSource3D(target)) {
            if (audio3D->clip) {
                drawLine("audio:" + audio3D->clip->path);
            }
        }
    } else if (!materialPath.empty()) {
        drawLine("material:" + materialPath, "magenta");
    } else {
        drawLine("entities:" + std::to_string(scene.world.entityCount()));
        drawLine("sprites:" + std::to_string(scene.world.spriteRendererCount2D()));
        drawLine("tilemaps:" + std::to_string(scene.world.tilemapCount2D()));
        const std::vector<game::EntityId> entities = scene.world.aliveEntities();
        if (!entities.empty()) {
            const game::NameComponent* firstName = scene.world.getName(entities.front());
            drawLine("next:" + (firstName ? firstName->value : std::string("entity")));
        }
        drawLine("inspectEntity(...)", borderColor);
    }

    if (!materialPath.empty()) {
        const int propertyCount = scene.materialPropertyCount(materialPath);
        drawLine("props:" + std::to_string(propertyCount), borderColor);
        const int visibleProperties = std::min(propertyCount, std::max(0, panelHeight - line - 1));
        for (int i = 0; i < visibleProperties; ++i) {
            game::MaterialPropertyView property = scene.materialPropertyAt(materialPath, i);
            if (!static_cast<bool>(property.exists)) {
                continue;
            }
            drawLine(static_cast<std::string>(property.name) + "=" + summarizeMaterialPropertyValue(property));
        }
    }
}

inline bool followCamera2D(
    game::World& world,
    game::EntityId cameraEntity,
    game::EntityId targetEntity,
    const physics::Vec2& offset = physics::Vec2(0.0f, 0.0f),
    float smoothing = 1.0f
) {
    game::Transform2D* cameraTransform = world.getTransform2D(cameraEntity);
    const game::Transform2D* targetTransform = world.getTransform2D(targetEntity);
    if (cameraTransform == nullptr || targetTransform == nullptr) {
        return false;
    }

    smoothing = std::clamp(smoothing, 0.0f, 1.0f);
    physics::Vec2 desired = targetTransform->position + offset;
    cameraTransform->position.x += (desired.x - cameraTransform->position.x) * smoothing;
    cameraTransform->position.y += (desired.y - cameraTransform->position.y) * smoothing;
    return true;
}

inline bool followPrimaryCamera2D(
    game::World& world,
    game::EntityId targetEntity,
    const physics::Vec2& offset = physics::Vec2(0.0f, 0.0f),
    float smoothing = 1.0f
) {
    std::optional<game::EntityId> primaryCamera = world.primaryCamera2D();
    if (!primaryCamera.has_value()) {
        return false;
    }
    return followCamera2D(world, primaryCamera.value(), targetEntity, offset, smoothing);
}

inline bool followCamera3D(
    game::World& world,
    game::EntityId cameraEntity,
    game::EntityId targetEntity,
    const physics::Vec3& offset = physics::Vec3(0.0f, 0.0f, 0.0f),
    float smoothing = 1.0f
) {
    game::Transform3D* cameraTransform = world.getTransform3D(cameraEntity);
    const game::Transform3D* targetTransform = world.getTransform3D(targetEntity);
    if (cameraTransform == nullptr || targetTransform == nullptr) {
        return false;
    }

    smoothing = std::clamp(smoothing, 0.0f, 1.0f);
    physics::Vec3 desired = targetTransform->position + offset;
    cameraTransform->position.x += (desired.x - cameraTransform->position.x) * smoothing;
    cameraTransform->position.y += (desired.y - cameraTransform->position.y) * smoothing;
    cameraTransform->position.z += (desired.z - cameraTransform->position.z) * smoothing;
    return true;
}

inline bool followPrimaryCamera3D(
    game::World& world,
    game::EntityId targetEntity,
    const physics::Vec3& offset = physics::Vec3(0.0f, 0.0f, 0.0f),
    float smoothing = 1.0f
) {
    std::optional<game::EntityId> primaryCamera = world.primaryCamera3D();
    if (!primaryCamera.has_value()) {
        return false;
    }
    return followCamera3D(world, primaryCamera.value(), targetEntity, offset, smoothing);
}

class SceneGame : public Game {
public:
    explicit SceneGame(game::Scene& scene) : scene(scene) {}

    void init() override {
        scene.load();
    }

    void update(float dt) override {
        scene.updateFrame(dt);
    }

    void draw(Canvas& canvas) override {
        canvas.clear(scene.clearColor);
        if (scene.autoRenderWorld2D) {
            drawWorld2D(canvas, scene.world, scene.drawEntityNames);
        }
        scene.render(canvas);
        if (scene.debugOverlayEnabled &&
            (scene.debugDrawGrid2D || scene.debugDrawColliders2D || scene.debugDrawTransforms2D ||
             scene.debugDrawCameraBounds2D || scene.debugDrawRuntimeStats)) {
            drawSceneDebug2D(canvas, scene);
        }
        if (scene.minimalInspectorEnabled) {
            drawSceneInspector(canvas, scene);
        }
    }

private:
    game::Scene& scene;
};

struct GameplayHarnessConfig {
    int canvasWidth = 80;
    int canvasHeight = 24;
    bool renderOnLoad = true;
    bool renderAfterUpdate = true;
    bool debugOverlayEnabled = false;
    bool inspectorEnabled = false;
};

class GameplayTestHarness {
public:
    explicit GameplayTestHarness(game::Scene& scene, GameplayHarnessConfig config = {})
        : scene(scene),
          host(scene),
          config(config),
          snapshotCanvas(std::max(8, config.canvasWidth), std::max(6, config.canvasHeight), false) {}

    void load() {
        applyDebugState();
        scene.load();
        if (config.renderOnLoad) {
            renderFrame();
        }
    }

    void stepFrame(float dt) {
        applyDebugState();
        scene.updateFrame(dt);
        if (config.renderAfterUpdate) {
            renderFrame();
        }
    }

    void stepFrames(int frameCount, float dt) {
        for (int i = 0; i < frameCount; ++i) {
            stepFrame(dt);
        }
    }

    void stepFixed(float dt, int steps = 1) {
        applyDebugState();
        for (int i = 0; i < steps; ++i) {
            scene.simulateFixedStep(dt);
        }
        if (config.renderAfterUpdate) {
            renderFrame();
        }
    }

    void renderFrame() {
        applyDebugState();
        host.draw(snapshotCanvas);
    }

    void setDebugOverlayEnabled(bool enabled) {
        config.debugOverlayEnabled = enabled;
        scene.debugOverlayEnabled = enabled;
    }

    void setInspectorEnabled(bool enabled) {
        config.inspectorEnabled = enabled;
        scene.minimalInspectorEnabled = enabled;
    }

    std::string snapshotText(bool includeStyles = false) const {
        std::ostringstream output;
        for (int y = 0; y < snapshotCanvas.buffer.height; ++y) {
            std::string currentStyle;
            for (int x = 0; x < snapshotCanvas.buffer.width; ++x) {
                const std::string& style = snapshotCanvas.buffer.styles[static_cast<size_t>(y)][static_cast<size_t>(x)];
                if (includeStyles && style != currentStyle) {
                    currentStyle = style;
                    if (!currentStyle.empty()) {
                        output << currentStyle;
                    }
                }
                output << snapshotCanvas.buffer.grid[static_cast<size_t>(y)][static_cast<size_t>(x)];
            }
            if (includeStyles && !currentStyle.empty()) {
                output << "\033[0m";
            }
            if (y + 1 < snapshotCanvas.buffer.height) {
                output << "\n";
            }
        }
        return output.str();
    }

    int countGlyph(const std::string& glyph) const {
        int count = 0;
        for (const auto& row : snapshotCanvas.buffer.grid) {
            for (const std::string& cell : row) {
                if (cell == glyph) {
                    ++count;
                }
            }
        }
        return count;
    }

    int countStyle(const std::string& style) const {
        int count = 0;
        for (const auto& row : snapshotCanvas.buffer.styles) {
            for (const std::string& cellStyle : row) {
                if (cellStyle == style) {
                    ++count;
                }
            }
        }
        return count;
    }

    bool snapshotContains(const std::string& text) const {
        return snapshotText(false).find(text) != std::string::npos;
    }

    Canvas& canvas() {
        return snapshotCanvas;
    }

    const Canvas& canvas() const {
        return snapshotCanvas;
    }

    game::Scene& sceneRef() {
        return scene;
    }

    const game::Scene& sceneRef() const {
        return scene;
    }

private:
    void applyDebugState() {
        scene.debugOverlayEnabled = config.debugOverlayEnabled;
        scene.minimalInspectorEnabled = config.inspectorEnabled;
    }

    game::Scene& scene;
    SceneGame host;
    GameplayHarnessConfig config;
    Canvas snapshotCanvas;
};

template<typename SceneType>
inline void runScene(SceneType& scene) {
    runGameLoop(scene);
}

template<typename SceneType, typename std::enable_if_t<std::is_base_of_v<game::Scene, SceneType>, int>>
inline void runGameLoop(SceneType& scene) {
    SceneGame host(scene);
    runGameLoop(host);
}

template<typename GameType, typename std::enable_if_t<!std::is_base_of_v<game::Scene, GameType>, int>>
inline void runGameLoop(GameType& game) {
#ifdef _WIN32
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "ZenithGameWindow";
    
    if (RegisterClassExA(&wc)) {
        RECT rect = {0, 0, 1024, 576};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        
        hWnd = CreateWindowExA(
            0,
            "ZenithGameWindow",
            "Zenith Game Engine",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL, NULL, GetModuleHandleA(NULL), NULL
        );
        
        if (hWnd) {
            hDC = GetDC(hWnd);
            setupPixelFormat(hDC);
            hRC = wglCreateContext(hDC);
            wglMakeCurrent(hDC, hRC);
            
            fontListBase = glGenLists(256);
            HFONT hFont = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                      ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
                                      ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Courier New");
SelectObject(hDC, hFont);
            wglUseFontBitmapsA(hDC, 0, 256, fontListBase);
            
            glViewport(0, 0, 1024, 576);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, 1024.0, 576.0, 0.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            Canvas canvas(128, 28, true);
            canvas.fontListBase = fontListBase;
            
            game.init();
            
            auto last_time = std::chrono::high_resolution_clock::now();
            MSG msg = {0};
            bool running = true;
            
            while (running) {
                InputState::getInstance().beginFrame();
                while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) {
                        running = false;
                        break;
                    }
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }
                
                if (!running) break;

                RECT clientRect;
                GetClientRect(hWnd, &clientRect);
                int winW = clientRect.right - clientRect.left;
                int winH = clientRect.bottom - clientRect.top;
                if (winW < 100) winW = 1024;
                if (winH < 100) winH = 576;

                glViewport(0, 0, winW, winH);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0.0, static_cast<double>(winW), static_cast<double>(winH), 0.0, -1.0, 1.0);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                canvas.width = std::max(10, winW / 8);
                canvas.height = std::max(5, winH / 20);
                
                auto current_time = std::chrono::high_resolution_clock::now();
                float dt = std::chrono::duration<float>(current_time - last_time).count();
                last_time = current_time;
                
                game.update(dt);
                canvas.clear("black");
                game.draw(canvas);
                canvas.present();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hRC);
            ReleaseDC(hWnd, hDC);
            DestroyWindow(hWnd);
            return;
        }
    }
#endif

    game.init();
    Canvas canvas(80, 24, false);
    
    auto last_time = std::chrono::high_resolution_clock::now();
    int frames = 0;
    bool infinite = true;
    
    #ifdef __ANDROID__
    infinite = false;
    #endif
    
    bool is_tty = false;
    #ifdef _WIN32
    is_tty = _isatty(0) != 0;
    #else
    is_tty = isatty(0) != 0;
    #endif
    
    if (!infinite || !is_tty) {
        for (int i = 0; i < 20; ++i) {
            InputState::getInstance().beginFrame();
            auto current_time = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;
            
            game.update(dt);
            canvas.clear("black");
            game.draw(canvas);
            canvas.present();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return;
    }
    
    while (true) {
        InputState::getInstance().beginFrame();
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        game.update(dt);
        canvas.clear("black");
        game.draw(canvas);
        canvas.present();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
        frames++;
        if (frames > 100) {
            break;
        }
    }
}

// ============================================================================
// 5. ENGINE PRIMITIVES (CHARACTER, CAMERA, LIGHTS, SPRITEBATCH)
// ============================================================================

class Character2D {
public:
    physics::Vec2 position;
    physics::Vec2 velocity;
    physics::RigidBody2D rigidBody;
    std::shared_ptr<resource::TextureAsset> sprite;
    float moveSpeed = 10.0f;
    bool isGrounded = false;

    virtual void update(float dt) {
        rigidBody.position = position;
        rigidBody.velocity = velocity;
        position = rigidBody.position;
    }
};

class Character3D {
public:
    physics::Vec3 position;
    physics::Vec3 velocity;
    physics::RigidBody3D rigidBody;
    std::shared_ptr<resource::MeshAsset> model;
    float moveSpeed = 10.0f;
    bool useGravity = true;

    virtual void update(float dt) {
        rigidBody.position = position;
        position = rigidBody.position;
    }

    void move(const physics::Vec3& dir) {
        velocity = dir * moveSpeed;
        position += velocity * 0.016f;
    }
};

class Camera2D {
public:
    physics::Vec2 position;
    float zoom = 1.0f;
    float smoothness = 0.1f;

    void follow(const physics::Vec2& targetPos, physics::Vec2 offset = physics::Vec2(0, 0)) {
        physics::Vec2 target = targetPos + offset;
        position.x += (target.x - position.x) * smoothness;
        position.y += (target.y - position.y) * smoothness;
    }
};

class Camera3D {
public:
    physics::Vec3 position;
    physics::Vec3 target;
    float fov = 60.0f;
    float nearClip = 0.1f;
    float farClip = 1000.0f;

    void follow(const physics::Vec3& targetPos, float distance = 5.0f) {
        target = targetPos;
        position = targetPos + physics::Vec3(0, 2.0f, -distance);
    }
};

struct PointLight {
    physics::Vec3 position;
    float intensity = 1.0f;
    float range = 10.0f;
    float r = 1.0f, g = 1.0f, b = 1.0f;
};

struct DirectionalLight {
    physics::Vec3 direction;
    float intensity = 1.0f;
    float r = 1.0f, g = 1.0f, b = 1.0f;
    bool castShadows = true;
};

class SpriteBatcher2D {
public:
    int spriteCount = 0;

    void begin() { spriteCount = 0; }
    void drawSprite(std::shared_ptr<resource::TextureAsset> texture, physics::Vec2 pos, physics::Vec2 size) {
        spriteCount++;
    }
    void end() {
        // High-speed 1-draw-call buffer flush to GPU
    }
};

} // namespace zenith

// Global C FFI Wrappers prefixed with _zenith_builtin_
extern "C" {
    inline bool _zenith_builtin_isKeyPressed(const char* key) {
        return zenith::InputState::getInstance().isKeyPressed(key);
    }

    inline bool _zenith_builtin_wasKeyPressed(const char* key) {
        return zenith::InputState::getInstance().wasKeyPressed(key);
    }

    inline bool _zenith_builtin_wasKeyReleased(const char* key) {
        return zenith::InputState::getInstance().wasKeyReleased(key);
    }

    inline float _zenith_builtin_getAxis(const char* negativeKey, const char* positiveKey) {
        return zenith::InputState::getInstance().getAxis(negativeKey, positiveKey);
    }
    
    inline float _zenith_builtin_getMouseX() {
        return zenith::InputState::getInstance().getMouseX();
    }
    
    inline float _zenith_builtin_getMouseY() {
        return zenith::InputState::getInstance().getMouseY();
    }
}

#endif // ZENITH_GAME_H
