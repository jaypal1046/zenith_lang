#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }

class GalleryApp {
private:
public:
    std::string title = "ZENITH PLATFORM GALLERY";
    std::string loaded_data = "No API data fetched yet.";
    int clicks = 0;

    GalleryApp()  {}

    zenith::UIElement build() {
        return zenith::UI::Column(zenith::make_children(zenith::UI::Text(title, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}}), zenith::UI::Container(zenith::make_children(zenith::UI::Text("1. Widget Grid & Flow Layouts", {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("yellow")}}), zenith::UI::Row(zenith::make_children(zenith::UI::Text("Flow item 1", {{"color", zenith::toString("green")}}), zenith::UI::Text("Flow item 2", {{"color", zenith::toString("magenta")}})), {})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("2. Reactive Component States", {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}}), zenith::UI::Row(zenith::make_children(zenith::UI::Text(zenith::concat("Interactive clicks: ", clicks), {{"color", zenith::toString("green")}}), zenith::UI::Button("Click Me!", {{"onClick", zenith::toString("handlePress")}})), {})), {{"padding", zenith::toString(1)}}), zenith::UI::Row(zenith::make_children(zenith::UI::Image("https://images.unsplash.com/photo-1618005182384-a83a8bd57fbe", {{"width", zenith::toString(35)}, {"height", zenith::toString(4)}}), zenith::UI::Video("https://assets.mixkit.co/videos/preview/mixkit-stars-in-space-background-1611-large.mp4", {{"width", zenith::toString(35)}, {"height", zenith::toString(4)}})), {}), zenith::UI::Scrolling(zenith::make_children(zenith::UI::Text("Line 1: Infinite Scrolling Row", {}), zenith::UI::Text("Line 2: GPU Accelerated Layouts", {}), zenith::UI::Text("Line 3: Reactive State Binding", {}), zenith::UI::Text("Line 4: Native Platform Primitives", {}), zenith::UI::Text("Line 5: Dynamic CSS Variables", {}), zenith::UI::Text("Line 6: Single-Thread WASM Event Loop", {})), {{"height", zenith::toString(4)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("3. REST API / Network Client Integration", {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("yellow")}}), zenith::UI::Row(zenith::make_children(zenith::UI::Text(zenith::concat("Response: ", loaded_data), {}), zenith::UI::Button("Fetch JSON", {{"onClick", zenith::toString("fetchData")}})), {})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Text("4. CSS Flexbox Layout Demonstration", {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("magenta")}}), zenith::UI::Row(zenith::make_children(zenith::UI::Text("[Grow 1]", {{"color", zenith::toString("red")}, {"flexGrow", zenith::toString(1)}}), zenith::UI::Text("[Grow 2]", {{"color", zenith::toString("green")}, {"flexGrow", zenith::toString(2)}}), zenith::UI::Text("[Grow 1]", {{"color", zenith::toString("blue")}, {"flexGrow", zenith::toString(1)}})), {{"justifyContent", zenith::toString("space-around")}, {"gap", zenith::toString(1)}, {"width", zenith::toString(35)}})), {{"padding", zenith::toString(1)}})), {});
    }

    void handlePress() {
        {
            clicks = clicks + 1;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void fetchData() {
        std::string res = httpGet("https://jsonplaceholder.typicode.com/users");
        {
            loaded_data = res;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "handlePress") { this->handlePress(); return; }
        if (name == "fetchData") { this->fetchData(); return; }
    }

};

int main() {
    println("--- Booting Zenith Multi-Platform Gallery App ---");
    GalleryApp app = GalleryApp();
    zenith::runInteractiveLoop(app);
    println("--- Gallery Loaded Successfully. Interactive loop started! ---");
}

