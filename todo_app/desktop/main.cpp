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

class MyApp {
private:
public:
    std::string title = "Zenith Todo App - Reactive Tasks";
    std::string newTaskText = "";
    std::string filter = "All";
    std::string task1 = "Design Zenith Architecture";
    bool done1 = true;
    std::string task2 = "Transpile to Web Target";
    bool done2 = true;
    std::string task3 = "Enable Live Reactivity";
    bool done3 = false;
    std::string task4 = "Deploy to Production";
    bool done4 = false;

    MyApp()  {}
    MyApp(std::string title, std::string newTaskText, std::string filter, std::string task1, bool done1, std::string task2, bool done2, std::string task3, bool done3, std::string task4, bool done4) : title(title), newTaskText(newTaskText), filter(filter), task1(task1), done1(done1), task2(task2), done2(done2), task3(task3), done3(done3), task4(task4), done4(done4) {}

    zenith::UIElement build() {
        auto total = 0;
        auto doneCount = 0;
        if (task1 != "") {
            total = total + 1;
            if (done1) {
                doneCount = doneCount + 1;
            }
        }
        if (task2 != "") {
            total = total + 1;
            if (done2) {
                doneCount = doneCount + 1;
            }
        }
        if (task3 != "") {
            total = total + 1;
            if (done3) {
                doneCount = doneCount + 1;
            }
        }
        if (task4 != "") {
            total = total + 1;
            if (done4) {
                doneCount = doneCount + 1;
            }
        }
        auto progress = 0;
        if (total > 0) {
            progress = doneCount * 100 / total;
        }
        auto statsText = zenith::concat("Completion: ", zenith::concat(progress, zenith::concat("% Done (", zenith::concat(doneCount, zenith::concat("/", zenith::concat(total, ")"))))));
        std::vector<zenith::UIElement> items = {};
        if (task1 != "") {
            auto show = false;
            if (filter == "All") {
                show = true;
            }
            if (filter == "Active") {
                if (done1 == false) {
                    show = true;
                }
            }
            if (filter == "Completed") {
                if (done1) {
                    show = true;
                }
            }
            if (show) {
                items.push_back(zenith::UI::Row(zenith::make_children(zenith::UI::Checkbox(task1, {{"checked", zenith::toString(done1)}, {"onChange", zenith::toString("handleToggle1")}})), {}));
            }
        }
        if (task2 != "") {
            auto show = false;
            if (filter == "All") {
                show = true;
            }
            if (filter == "Active") {
                if (done2 == false) {
                    show = true;
                }
            }
            if (filter == "Completed") {
                if (done2) {
                    show = true;
                }
            }
            if (show) {
                items.push_back(zenith::UI::Row(zenith::make_children(zenith::UI::Checkbox(task2, {{"checked", zenith::toString(done2)}, {"onChange", zenith::toString("handleToggle2")}})), {}));
            }
        }
        if (task3 != "") {
            auto show = false;
            if (filter == "All") {
                show = true;
            }
            if (filter == "Active") {
                if (done3 == false) {
                    show = true;
                }
            }
            if (filter == "Completed") {
                if (done3) {
                    show = true;
                }
            }
            if (show) {
                items.push_back(zenith::UI::Row(zenith::make_children(zenith::UI::Checkbox(task3, {{"checked", zenith::toString(done3)}, {"onChange", zenith::toString("handleToggle3")}})), {}));
            }
        }
        if (task4 != "") {
            auto show = false;
            if (filter == "All") {
                show = true;
            }
            if (filter == "Active") {
                if (done4 == false) {
                    show = true;
                }
            }
            if (filter == "Completed") {
                if (done4) {
                    show = true;
                }
            }
            if (show) {
                items.push_back(zenith::UI::Row(zenith::make_children(zenith::UI::Checkbox(task4, {{"checked", zenith::toString(done4)}, {"onChange", zenith::toString("handleToggle4")}})), {}));
            }
        }
        return zenith::UI::Column(zenith::make_children(zenith::UI::Card(zenith::make_children(zenith::UI::Text(title, {{"fontWeight", zenith::toString("bold")}, {"color", zenith::toString("cyan")}}), zenith::UI::Text(statsText, {{"color", zenith::toString("green")}})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Row(zenith::make_children(zenith::UI::TextField("Enter new task...", {{"value", zenith::toString(newTaskText)}, {"onChange", zenith::toString("handleNewTaskChange")}}), zenith::UI::Button("Add Task", {{"onClick", zenith::toString("addTask")}})), {})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(zenith::UI::Row(zenith::make_children(zenith::UI::Button("All", {{"onClick", zenith::toString("setFilterAll")}}), zenith::UI::Button("Active", {{"onClick", zenith::toString("setFilterActive")}}), zenith::UI::Button("Completed", {{"onClick", zenith::toString("setFilterCompleted")}}), zenith::UI::Button("Clear Completed", {{"onClick", zenith::toString("clearCompleted")}})), {})), {{"padding", zenith::toString(1)}}), zenith::UI::Card(zenith::make_children(items), {{"padding", zenith::toString(1)}})), {});
    }

    void handleToggle1(bool checked) {
        {
            done1 = checked;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleToggle2(bool checked) {
        {
            done2 = checked;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleToggle3(bool checked) {
        {
            done3 = checked;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleToggle4(bool checked) {
        {
            done4 = checked;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void handleNewTaskChange(std::string val) {
        {
            newTaskText = val;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void addTask() {
        if (newTaskText != "") {
            {
                if (task1 == "") {
                    task1 = newTaskText;
                    done1 = false;
                    newTaskText = "";
                } else {
                    if (task2 == "") {
                        task2 = newTaskText;
                        done2 = false;
                        newTaskText = "";
                    } else {
                        if (task3 == "") {
                            task3 = newTaskText;
                            done3 = false;
                            newTaskText = "";
                        } else {
                            if (task4 == "") {
                                task4 = newTaskText;
                                done4 = false;
                                newTaskText = "";
                            }
                        }
                    }
                }
                std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
                this->build().render();
            }
        }
    }

    void clearCompleted() {
        {
            if (done1) {
                task1 = "";
                done1 = false;
            }
            if (done2) {
                task2 = "";
                done2 = false;
            }
            if (done3) {
                task3 = "";
                done3 = false;
            }
            if (done4) {
                task4 = "";
                done4 = false;
            }
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void setFilterAll() {
        {
            filter = "All";
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void setFilterActive() {
        {
            filter = "Active";
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void setFilterCompleted() {
        {
            filter = "Completed";
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "handleToggle1") { this->handleToggle1(val == "true"); return; }
        if (name == "handleToggle2") { this->handleToggle2(val == "true"); return; }
        if (name == "handleToggle3") { this->handleToggle3(val == "true"); return; }
        if (name == "handleToggle4") { this->handleToggle4(val == "true"); return; }
        if (name == "handleNewTaskChange") { this->handleNewTaskChange(val); return; }
        if (name == "addTask") { this->addTask(); return; }
        if (name == "clearCompleted") { this->clearCompleted(); return; }
        if (name == "setFilterAll") { this->setFilterAll(); return; }
        if (name == "setFilterActive") { this->setFilterActive(); return; }
        if (name == "setFilterCompleted") { this->setFilterCompleted(); return; }
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("Initializing Reorganized Zenith Application...");
    MyApp app = MyApp();
    zenith::runInteractiveLoop(app);
    println("Zenith Application Shutdown.");

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

