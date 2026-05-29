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

class ShowcaseSite {
private:
public:
    std::string title = "ZENITH NATIVE WASM ENGINE";
    std::string subtitle = "Running purely inside the browser sandbox";
    std::string description = "No Javascript VMs. No Garbage Collector Pauses. Pure Systems Performance.";
    int clicks = 0;

    ShowcaseSite()  {}

    zenith::UIElement build() {
        return zenith::UI::Column(zenith::make_children(zenith::UI::Text(title, {}), zenith::UI::Text(description, {}), zenith::UI::Row(zenith::make_children(zenith::UI::Text("Status: ", {}), zenith::UI::Text(subtitle, {})), {}), zenith::UI::Row(zenith::make_children(zenith::UI::Text(zenith::concat("Total Clicks: ", clicks), {}), zenith::UI::Button("Increment Clicks", {{"onClick", zenith::toString("increment")}})), {})), {});
    }

    void increment() {
        {
            clicks = clicks + 1;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "increment") { this->increment(); return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("--- Booting WASM Zenith App ---");
    ShowcaseSite site = ShowcaseSite();
    zenith::runInteractiveLoop(site);
    println("--- UI DOM Rendered Successfully from WASM! ---");

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

