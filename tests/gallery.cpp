#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <future>
#include <iostream>
#include <functional>
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"

using json = nlohmann::json;

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

template<typename T>
void print_t(const T& value) {
    std::cout << value << std::endl;
}

template<typename T>
std::string to_string_generic(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

class LLMClient {
public:
    static std::string getEndpoint() {
        const char* env_endpoint = std::getenv("ZENITH_LLM_ENDPOINT");
        if (env_endpoint) {
            return std::string(env_endpoint);
        }
        return "http://localhost:11434"; // Default to Ollama
    }

    static std::string getModel() {
        const char* env_model = std::getenv("ZENITH_LLM_MODEL");
        if (env_model) {
            return std::string(env_model);
        }
        return "llama3"; // Default model
    }

    static std::string generate(const std::string& prompt, const std::string& model = "", float temperature = 0.7f, int max_tokens = 512) {
        CURL* curl = curl_easy_init();
        if (!curl) throw std::runtime_error("Failed to initialize CURL");

        json payload = {
            {"model", model.empty() ? getModel() : model},
            {"prompt", prompt},
            {"stream", false},
            {"options", {{"temperature", temperature}, {"num_predict", max_tokens}}}
        };

        std::string response_data;
        std::string url = getEndpoint() + "/api/generate";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.dump().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, void* userp) -> size_t {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
        }

        try {
            json result = json::parse(response_data);
            return result.value("response", "");
        } catch (...) {
            return response_data;
        }
    }
};

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
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("--- Booting Zenith Multi-Platform Gallery App ---");
    GalleryApp app = GalleryApp();
    zenith::runInteractiveLoop(app);
    println("--- Gallery Loaded Successfully. Interactive loop started! ---");

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

