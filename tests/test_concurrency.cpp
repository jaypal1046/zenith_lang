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

zenith::stdlib::Future<int> calculateSquare(int n) {
    auto _promise = std::make_shared<zenith::stdlib::Promise<int>>();
    std::thread([_promise, n]() mutable {
        try {
            auto i = 0;
            while (i < 5000) {
                i = i + 1;
            }
            _promise->set_value(n * n);
            return;
        } catch (...) {
            _promise->set_exception(std::current_exception());
        }
    }).detach();
    return _promise->get_future();
}

zenith::stdlib::Future<int> runConcurrencyDemo() {
    auto _promise = std::make_shared<zenith::stdlib::Promise<int>>();
    std::thread([_promise]() mutable {
        try {
            println("[Test] Spawning concurrent tasks...");
            auto f1 = calculateSquare(5);
            auto f2 = calculateSquare(10);
            auto f3 = calculateSquare(12);
            println("[Test] Awaiting concurrent tasks...");
            auto r1 = (f1).get();
            auto r2 = (f2).get();
            auto r3 = (f3).get();
            println(zenith::concat("[Test] Completed. Results: ", zenith::concat(r1, zenith::concat(", ", zenith::concat(r2, zenith::concat(", ", r3))))));
            _promise->set_value(r1 + r2 + r3);
            return;
        } catch (...) {
            _promise->set_exception(std::current_exception());
        }
    }).detach();
    return _promise->get_future();
}

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("=== Zenith Multithreading / Concurrency Verification ===");
    auto future_res = runConcurrencyDemo();
    auto total = (future_res).get();
    println(zenith::concat("[Result] Combined total sum: ", total));
    if (total == 269) {
        println("[Status] SUCCESS: Concurrency check passed!");
    } else {
        println("[Status] FAILURE: Concurrency check failed!");
    }

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

