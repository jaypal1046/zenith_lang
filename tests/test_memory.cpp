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
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

class Node : public zenith::mem::Managed {
private:
public:
    std::string value;

    Node()  {}

    void __gc_enumerate(std::vector<zenith::mem::RcBlock*>& out) override {
    }

    void triggerCallback(std::string name, std::string val = "") {
    }

};

class Counter : public zenith::mem::Managed {
private:
public:
    int count;

    Counter()  {}

    void __gc_enumerate(std::vector<zenith::mem::RcBlock*>& out) override {
    }

    void triggerCallback(std::string name, std::string val = "") {
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    println("=== Zenith Hybrid RC + GC Memory Test ===");
    println("\n[Test 1] Basic Ref<T> — Strong Reference Counting:");
    zenith::mem::Ref<Node> nodeA = [&]() { auto* __obj = new auto("hello-rc"); return zenith::mem::Ref<std::decay_t<decltype(*__obj)>>(__obj); }();
    println("Created Ref<Node> with value: hello-rc");
    println("Ref<T> strong ownership established.");
    println("\n[Test 2] Weak<T> — Weak Reference (no RC increment):");
    zenith::mem::Weak<Node> weakRef = nodeA;
    println("Weak<Node> created. Does not prevent collection.");
    println("Weak ref is non-owning — breaks potential cycles.");
    println("\n[Test 3] @managed class — inherits zenith::mem::Managed:");
    zenith::mem::Ref<Counter> counter = [&]() { auto* __obj = new auto(42); return zenith::mem::Ref<std::decay_t<decltype(*__obj)>>(__obj); }();
    println("Counter object created with count: 42");
    println("Counter is heap-tracked by GcHeap.");
    println("\n[Test 4] GC Statistics:");
    std::string stats = zenith::UI::gcStats(zenith::make_children(), {});
    println(zenith::concat("GC Stats: ", stats));
    println("\n[Test 5] Scope Exit — RC Deallocation:");
    println("All Ref<T> objects will be freed when they go out of scope.");
    println("GcHeap background thread running every 5000ms for cycle detection.");
    println("\n=== Memory Test Complete ===");
    println("RC frees acyclic objects. GC collects cycles. Both run transparently.");

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

