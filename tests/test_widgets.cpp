#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <future>
#include <iostream>
#include <functional>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

class WidgetDemoApp {
private:
public:
    bool cb_state = false;
    int slider_val = 50;
    bool toggle_state = false;
    std::string selected_option = "Option 1";

    WidgetDemoApp()  {}

    zenith::UIElement build() {
        return zenith::UI::Column(zenith::make_children(zenith::UI::Text("ZENITH WIDGET VERIFICATION DEMO", {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text(zenith::concat("1. Checkbox State: ", cb_state), {{"color", zenith::toString("yellow")}}), zenith::UI::Checkbox("Accept Terms & Conditions", {{"checked", zenith::toString(cb_state)}, {"onChange", zenith::toString("handleCheckbox")}})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text(zenith::concat("2. Slider Value: ", slider_val), {{"color", zenith::toString("green")}}), zenith::UI::Slider({{"min", zenith::toString(0)}, {"max", zenith::toString(100)}, {"value", zenith::toString(slider_val)}, {"onChange", zenith::toString("handleSlider")}})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text(zenith::concat("3. Toggle Switch State: ", toggle_state), {{"color", zenith::toString("magenta")}}), zenith::UI::Toggle("Toggle Feature Alpha", {{"isOn", zenith::toString(toggle_state)}, {"onChange", zenith::toString("handleToggle")}})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text(zenith::concat("4. Dropdown Chosen: ", selected_option), {{"color", zenith::toString("cyan")}}), zenith::UI::Dropdown("Option 1,Option 2,Option 3", {{"value", zenith::toString(selected_option)}, {"onChange", zenith::toString("handleDropdown")}})), {{"padding", zenith::toString(1)}})), {});
    }

    void handleCheckbox(bool checked) {
        {
            cb_state = checked;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleSlider(int val) {
        {
            slider_val = val;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleToggle(bool onState) {
        {
            toggle_state = onState;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleDropdown(std::string choice) {
        {
            selected_option = choice;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "handleCheckbox") { this->handleCheckbox(val == "true"); return; }
        if (name == "handleSlider") { try { this->handleSlider(std::stoi(val)); } catch(...) {} return; }
        if (name == "handleToggle") { this->handleToggle(val == "true"); return; }
        if (name == "handleDropdown") { this->handleDropdown(val); return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("--- Booting Zenith Form Widget Demo App ---");
    WidgetDemoApp app = WidgetDemoApp();
    zenith::runInteractiveLoop(app);
    println("--- Form widgets loaded. Interactive events ready. ---");

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

