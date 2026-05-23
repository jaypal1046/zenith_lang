#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <future>
#include <iostream>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }

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
}

