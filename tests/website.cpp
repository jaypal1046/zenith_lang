#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }

class ShowcaseSite {
private:
public:
    std::string title = "ZENITH NATIVE WASM ENGINE";
    std::string subtitle = "Running purely inside the browser sandbox";
    std::string description = "No Javascript VMs. No Garbage Collector Pauses. Pure Systems Performance.";
    int clicks = 0;

    ShowcaseSite()  {}

    zenith::UIElement build() {
        return zenith::UI::Column({zenith::UI::Text({title}, {}), zenith::UI::Text({description}, {}), zenith::UI::Row({zenith::UI::Text({"Status: "}, {}), zenith::UI::Text({subtitle}, {})}, {}), zenith::UI::Row({zenith::UI::Text({zenith::concat("Total Clicks: ", clicks)}, {}), zenith::UI::Button({"Increment Clicks"}, {{"onClick", "increment"}})}, {})}, {});
    }

    void increment() {
        {
            clicks = clicks + 1;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void triggerCallback(std::string name) {
        if (name == "increment") { this->increment(); return; }
    }

};

int main() {
    println("--- Booting WASM Zenith App ---");
    ShowcaseSite site = ShowcaseSite();
    zenith::runInteractiveLoop(site);
    println("--- UI DOM Rendered Successfully from WASM! ---");
}

