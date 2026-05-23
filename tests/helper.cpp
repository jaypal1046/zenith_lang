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

std::string getVerificationStatus() {
    return " [Status: Verified (Custom import)]";
}

class CounterApp {
private:
public:
    int count = 0;
    std::string label = "Counter Value: ";

    CounterApp()  {}

    zenith::UIElement build() {
        return zenith::UI::Column(zenith::make_children(zenith::UI::Text(zenith::concat(label, count), {})), {});
    }

    void increment() {
        {
            count = count + 1;
            std::cout << "\n[Runtime] setState: Re-rendering UI Layout...\n";
            this->build().render();
        }
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "increment") { this->increment(); return; }
    }

};

class Shape {
public:
    virtual ~Shape() = default;
    virtual float getArea() = 0;
};

class Circle : public Shape {
private:
    float radius;
public:

    Circle(float radius) : radius(radius) {}

    float getArea() {
        return 3.14159 * radius * radius;
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "getArea") { this->getArea(); return; }
    }

};

