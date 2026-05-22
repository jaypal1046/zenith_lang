#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }

class MyApp {
private:
public:
    std::string title = "Zenith Cross-Platform App";
    int counter = 0;
    std::string api_result = "No data fetched.";

    MyApp()  {}

    zenith::UIElement build() {
        return zenith::UI::Column({zenith::UI::Container({zenith::UI::Text({title}, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}})}, {{"padding", zenith::toString(1)}}), zenith::UI::Card({zenith::UI::Text({"Click Counter Sample"}, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("yellow")}}), zenith::UI::Row({zenith::UI::Text({zenith::concat("Value: ", counter)}, {{"color", zenith::toString("green")}}), zenith::UI::Button({"Increment"}, {{"onClick", zenith::toString("handleIncrement")}})}, {})}, {{"padding", zenith::toString(1)}}), zenith::UI::Card({zenith::UI::Text({"REST Network API Verification"}, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("magenta")}}), zenith::UI::Row({zenith::UI::Text({zenith::concat("API Payload: ", api_result)}, {}), zenith::UI::Button({"Fetch Data"}, {{"onClick", zenith::toString("handleFetch")}})}, {})}, {{"padding", zenith::toString(1)}})}, {});
    }

    void handleIncrement() {
        {
            counter = counter + 1;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleFetch() {
        std::string res = httpGet("https://jsonplaceholder.typicode.com/todos/1");
        {
            api_result = res;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void triggerCallback(std::string name) {
        if (name == "handleIncrement") { this->handleIncrement(); return; }
        if (name == "handleFetch") { this->handleFetch(); return; }
    }

};

int main() {
    println("Initializing Zenith Application...");
    MyApp app = MyApp();
    zenith::runInteractiveLoop(app);
    println("Zenith Application Shutdown.");
}

