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

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

extern "C" int abs(int x);

void alert(std::string msg) {
    std::cerr << "[Warning] JS interop function 'alert' is not supported on desktop platform." << std::endl;
    return;
}

int compute_square(int n) {
    return zenith::ffi::PythonFFIBridge::callInt("bridge", "compute_square", n);
}

#ifdef _WIN32
extern "C" __declspec(dllexport)
#else
extern "C"
#endif
int zenith_add(int a, int b) {
    return a + b;
}

class InteropApp {
private:
public:

    InteropApp()  {}

    zenith::UIElement build() {
        return zenith::UI::Column(zenith::make_children(zenith::UI::Text("ZENITH NATIVE INTEROP VERIFICATION", {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("1. C standard FFI: abs(0 - 42)", {{"color", zenith::toString("yellow")}}), zenith::UI::Button("Execute Call", {{"onClick", zenith::toString("handleCallC")}})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("2. Python bridge: compute_square(5)", {{"color", zenith::toString("green")}}), zenith::UI::Button("Execute Call", {{"onClick", zenith::toString("handleCallPython")}})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("3. JavaScript bridge: alert('Hello')", {{"color", zenith::toString("magenta")}}), zenith::UI::Button("Execute Call", {{"onClick", zenith::toString("handleCallJS")}})), {{"padding", zenith::toString(1)}})), {});
    }

    void handleCallC() {
        int res = abs(0 - 42);
        println(zenith::concat("[Interop] C abs(0 - 42) returned: ", res));
    }

    void handleCallPython() {
        int res = compute_square(5);
        println(zenith::concat("[Interop] Python compute_square(5) returned: ", res));
    }

    void handleCallJS() {
        alert("Hello from Zenith Web/WASM Interop!");
        println("[Interop] JS alert function called!");
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "handleCallC") { this->handleCallC(); return; }
        if (name == "handleCallPython") { this->handleCallPython(); return; }
        if (name == "handleCallJS") { this->handleCallJS(); return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("--- Direct Interop Testing ---");
    InteropApp app = InteropApp();
    app.handleCallC();
    app.handleCallPython();
    app.handleCallJS();
    println("--- Testing exported function zenith_add(10, 20) ---");
    int sum = zenith_add(10, 20);
    println(zenith::concat("[Export] zenith_add(10, 20) returned: ", sum));

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

