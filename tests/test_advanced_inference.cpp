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

int multiBranchReturn(bool cond) {
    if (cond) {
        return 42;
    } else {
        return 0;
    }
}

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("=== Zenith Advanced Type Inference Test ===");
    std::function<int(int)> doubler = [=](int x) {
        return x * 2;
    };
    auto d_res = doubler(10);
    println(zenith::concat("Lambda parameter inference (expected 20): ", d_res));
    auto multi_result = multiBranchReturn(true);
    println(zenith::concat("Function return type inference (expected 42): ", multi_result));
    std::vector<int> numbers = {};
    numbers.push_back(10);
    numbers.push_back(20);
    println(zenith::concat("List size (expected 2): ", numbers.size()));
    println("=== Test Complete ===");

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

