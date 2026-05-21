#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }

class AppCommon {
private:
public:

    AppCommon()  {}

    std::string getCommonMessage() {
        return "Shared Code Block";
    }

    void triggerCallback(std::string name) {
        if (name == "getCommonMessage") { this->getCommonMessage(); return; }
    }

};

class AppDesktop {
private:
public:

    AppDesktop()  {}

    std::string getPlatformName() {
        return "Desktop Native Application";
    }

    void triggerCallback(std::string name) {
        if (name == "getPlatformName") { this->getPlatformName(); return; }
    }

};

class AppAndroid {
private:
public:

    AppAndroid()  {}

    std::string getPlatformName() {
        return "Android Client";
    }

    void triggerCallback(std::string name) {
        if (name == "getPlatformName") { this->getPlatformName(); return; }
    }

};

class AppIos {
private:
public:

    AppIos()  {}

    std::string getPlatformName() {
        return "iOS Client";
    }

    void triggerCallback(std::string name) {
        if (name == "getPlatformName") { this->getPlatformName(); return; }
    }

};

class AppWeb {
private:
public:

    AppWeb()  {}

    std::string getPlatformName() {
        return "Web Application";
    }

    void triggerCallback(std::string name) {
        if (name == "getPlatformName") { this->getPlatformName(); return; }
    }

};

class AppLinux {
private:
public:

    AppLinux()  {}

    std::string getPlatformName() {
        return "Linux Native Application";
    }

    void triggerCallback(std::string name) {
        if (name == "getPlatformName") { this->getPlatformName(); return; }
    }

};

class AppWindows {
private:
public:

    AppWindows()  {}

    std::string getPlatformName() {
        return "Windows Native Application";
    }

    void triggerCallback(std::string name) {
        if (name == "getPlatformName") { this->getPlatformName(); return; }
    }

};

class AppMac {
private:
public:

    AppMac()  {}

    std::string getPlatformName() {
        return "macOS Native Application";
    }

    void triggerCallback(std::string name) {
        if (name == "getPlatformName") { this->getPlatformName(); return; }
    }

};

class MyApp {
private:
public:
    std::string title = "Zenith Cross-Platform App";
    int counter = 0;
    std::string api_result = "No data fetched.";
    AppCommon common = AppCommon();
    AppDesktop desktop = AppDesktop();
    AppAndroid android = AppAndroid();
    AppIos ios = AppIos();
    AppWeb web = AppWeb();
    AppLinux linux = AppLinux();
    AppWindows windows = AppWindows();
    AppMac mac = AppMac();

    MyApp()  {}

    zenith::UIElement build() {
        return zenith::UI::Column({zenith::UI::Container({zenith::UI::Text({zenith::concat(title, zenith::concat(" (", zenith::concat(common.getCommonMessage(), ")")))}, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}})}, {{"padding", zenith::toString(1)}}), zenith::UI::Card({zenith::UI::Text({"Platform Modules Loaded:"}, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("blue")}}), zenith::UI::Text({zenith::concat(" - Common: ", common.getCommonMessage())}, {}), zenith::UI::Text({zenith::concat(" - Desktop module target: ", desktop.getPlatformName())}, {}), zenith::UI::Text({zenith::concat(" - Android module target: ", android.getPlatformName())}, {}), zenith::UI::Text({zenith::concat(" - iOS module target: ", ios.getPlatformName())}, {}), zenith::UI::Text({zenith::concat(" - Web module target: ", web.getPlatformName())}, {}), zenith::UI::Text({zenith::concat(" - Linux module target: ", linux.getPlatformName())}, {}), zenith::UI::Text({zenith::concat(" - Windows module target: ", windows.getPlatformName())}, {}), zenith::UI::Text({zenith::concat(" - macOS module target: ", mac.getPlatformName())}, {})}, {{"padding", zenith::toString(1)}}), zenith::UI::Card({zenith::UI::Text({"Click Counter Sample"}, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("yellow")}}), zenith::UI::Row({zenith::UI::Text({zenith::concat("Value: ", counter)}, {{"color", zenith::toString("green")}}), zenith::UI::Button({"Increment"}, {{"onClick", zenith::toString("handleIncrement")}})}, {})}, {{"padding", zenith::toString(1)}}), zenith::UI::Card({zenith::UI::Text({"REST Network API Verification"}, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("magenta")}}), zenith::UI::Row({zenith::UI::Text({zenith::concat("API Payload: ", api_result)}, {}), zenith::UI::Button({"Fetch Data"}, {{"onClick", zenith::toString("handleFetch")}})}, {})}, {{"padding", zenith::toString(1)}})}, {});
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
    println("Initializing Reorganized Zenith Application...");
    MyApp app = MyApp();
    zenith::runInteractiveLoop(app);
    println("Zenith Application Shutdown.");
}

