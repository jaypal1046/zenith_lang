#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <future>
#include <iostream>
#include <functional>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"


#ifdef __ANDROID__
const bool isAndroid = true;
#else
const bool isAndroid = false;
#endif

#ifdef __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE
    const bool isIos = true;
    const bool isMac = false;
  #else
    const bool isIos = false;
    const bool isMac = true;
  #endif
#else
  const bool isIos = false;
  const bool isMac = false;
#endif

#ifdef __linux__
  #ifndef __ANDROID__
    const bool isLinux = true;
  #else
    const bool isLinux = false;
  #endif
#else
  const bool isLinux = false;
#endif

#ifdef _WIN32
const bool isWindows = true;
#else
const bool isWindows = false;
#endif

const bool isWeb = false;

template <typename T>
inline void print(const T& msg) { std::cout << msg; }
template <typename T>
inline void println(const T& msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

extern "C" bool _zenith_builtin_isKeyPressed(const char*);

bool isKeyPressed(std::string key) {
    return ::_zenith_builtin_isKeyPressed(key.c_str());
}

extern "C" bool _zenith_builtin_wasKeyPressed(const char*);

bool wasKeyPressed(std::string key) {
    return ::_zenith_builtin_wasKeyPressed(key.c_str());
}

extern "C" bool _zenith_builtin_wasKeyReleased(const char*);

bool wasKeyReleased(std::string key) {
    return ::_zenith_builtin_wasKeyReleased(key.c_str());
}

extern "C" float _zenith_builtin_getAxis(const char*, const char*);

float getAxis(std::string negativeKey, std::string positiveKey) {
    return ::_zenith_builtin_getAxis(negativeKey.c_str(), positiveKey.c_str());
}

extern "C" float _zenith_builtin_getMouseX();

float getMouseX() {
    return ::_zenith_builtin_getMouseX();
}

extern "C" float _zenith_builtin_getMouseY();

float getMouseY() {
    return ::_zenith_builtin_getMouseY();
}

class GameplayInputTestGame {
private:
public:
    float player_x = 12.0;
    float player_y = 7.0;
    float move_speed = 18.0;
    float axis_x = 0.0;
    float axis_y = 0.0;
    std::string player_color = "green";
    std::string status_text = "Use WASD or arrows. Tap Space to flash yellow.";

    GameplayInputTestGame()  {}
    GameplayInputTestGame(float player_x, float player_y, float move_speed, float axis_x, float axis_y, std::string player_color, std::string status_text) : player_x(player_x), player_y(player_y), move_speed(move_speed), axis_x(axis_x), axis_y(axis_y), player_color(player_color), status_text(status_text) {}

    void init() {
        println("GameplayInputTestGame initialized");
    }

    void update(float dt) {
        axis_x = getAxis("a", "d");
        axis_y = getAxis("w", "s");
        if (isKeyPressed("ArrowLeft")) {
            axis_x = axis_x - 1.0;
        }
        if (isKeyPressed("ArrowRight")) {
            axis_x = axis_x + 1.0;
        }
        if (isKeyPressed("ArrowUp")) {
            axis_y = axis_y - 1.0;
        }
        if (isKeyPressed("ArrowDown")) {
            axis_y = axis_y + 1.0;
        }
        player_x = player_x + axis_x * move_speed * dt;
        player_y = player_y + axis_y * move_speed * dt;
        if (player_x < 0.0) {
            player_x = 0.0;
        }
        if (player_y < 0.0) {
            player_y = 0.0;
        }
        if (player_x > 74.0) {
            player_x = 74.0;
        }
        if (player_y > 20.0) {
            player_y = 20.0;
        }
        if (wasKeyPressed("Space")) {
            player_color = "yellow";
            status_text = "Space pressed this frame";
        }
        if (wasKeyReleased("Space")) {
            player_color = "green";
            status_text = "Space released this frame";
        }
    }

    void draw(zenith::Canvas& canvas) {
        std::string axis_text = "Axis X: ";
        axis_text = zenith::concat(axis_text, axis_x);
        axis_text = zenith::concat(axis_text, ", Axis Y: ");
        axis_text = zenith::concat(axis_text, axis_y);
        std::string mouse_x_text = "Mouse X: ";
        mouse_x_text = zenith::concat(mouse_x_text, getMouseX());
        std::string mouse_y_text = "Mouse Y: ";
        mouse_y_text = zenith::concat(mouse_y_text, getMouseY());
        canvas.clear("black");
        canvas.drawRect(player_x, player_y, 6.0, 3.0, player_color);
        canvas.drawText("Gameplay Input Test", 2.0, 1.0, "yellow");
        canvas.drawText(status_text, 2.0, 3.0, "white");
        canvas.drawText(axis_text, 2.0, 5.0, "cyan");
        canvas.drawText(mouse_x_text, 2.0, 7.0, "magenta");
        canvas.drawText(mouse_y_text, 2.0, 8.0, "magenta");
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "init") { this->init(); return; }
        if (name == "update") { try { this->update(std::stof(val)); } catch(...) {} return; }
            }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    GameplayInputTestGame game = GameplayInputTestGame();
    zenith::runGameLoop(game);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

