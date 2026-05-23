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

std::string streamAgent(std::string query) {
    // --- AUTO-GENERATED AGENTIC BINDING ---
    std::string prompt = R"(You are a streaming assistant. Query: {query})";
    prompt = std::regex_replace(prompt, std::regex("\\{query\\}"), query);
    zenith::LLMClient client("http://localhost:11434/api/generate");
    std::string response = client.promptStream(prompt, [](const std::string& chunk) { std::cout << chunk << std::flush; });
    return response;
}

std::string visionAgent(std::string query, std::string imagePath) {
    // --- AUTO-GENERATED AGENTIC BINDING ---
    std::string prompt = R"(Look at this image {imagePath} and answer: {query})";
    prompt = std::regex_replace(prompt, std::regex("\\{query\\}"), query);
    prompt = std::regex_replace(prompt, std::regex("\\{imagePath\\}"), imagePath);
    zenith::LLMClient client("http://localhost:11434/api/generate");
    std::string response = client.prompt(prompt, imagePath);
    return response;
}

std::string summaryAgent(std::string text) {
    // --- AUTO-GENERATED AGENTIC BINDING ---
    std::string prompt = R"(Summarize this: {text})";
    prompt = std::regex_replace(prompt, std::regex("\\{text\\}"), text);
    zenith::LLMClient client("http://localhost:11434/api/generate");
    std::string response = client.prompt(prompt);
    return response;
}

std::string sequentialPipe(std::string input) {
    std::string current_val = input;
    current_val = streamAgent(current_val);
    current_val = summaryAgent(current_val);
    return current_val;
}

std::vector<std::string> parallelPipe(std::string input) {
    auto f0 = std::async(std::launch::async, streamAgent, input);
    auto f1 = std::async(std::launch::async, summaryAgent, input);
    std::vector<std::string> results;
    results.push_back(f0.get());
    results.push_back(f1.get());
    return results;
}

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("=== Zenith Agentic Features Test ===");
    println("\n[Test 1] Streaming Agent:");
    std::string resStream = streamAgent("Hello stream agent");
    println(zenith::concat("Final Stream Output: ", resStream));
    println("\n[Test 2] Multimodal Agent:");
    std::string resVision = visionAgent("What is in this image?", "tests/screen.png");
    println(zenith::concat("Vision Agent Output: ", resVision));
    println("\n[Test 3] Sequential Orchestration:");
    std::string resSeq = sequentialPipe("Initial pipeline input");
    println(zenith::concat("Sequential Pipeline Result: ", resSeq));
    println("\n[Test 4] Parallel Orchestration:");
    std::vector<std::string> resPar = parallelPipe("Initial parallel input");
    println("Parallel Pipeline executed successfully.");
    println("=== All Tests Completed ===");

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

