#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"

inline void print(std::string msg) { std::cout << msg; }
inline void println(std::string msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }

zenith::stdlib::Future<int> delayedValue(int val) {
    auto _promise = std::make_shared<zenith::stdlib::Promise<int>>();
    std::thread([_promise, val]() mutable {
        try {
            auto i = 0;
            while (i < 1000) {
                i = i + 1;
            }
            _promise->set_value(val * 2);
            return;
        } catch (...) {
            _promise->set_exception(std::current_exception());
        }
    }).detach();
    return _promise->get_future();
}

zenith::stdlib::Future<int> runAsyncTest() {
    auto _promise = std::make_shared<zenith::stdlib::Promise<int>>();
    std::thread([_promise]() mutable {
        try {
            auto f1 = delayedValue(10);
            auto f2 = delayedValue(20);
            auto v1 = (f1).get();
            auto v2 = (f2).get();
            _promise->set_value(v1 + v2);
            return;
        } catch (...) {
            _promise->set_exception(std::current_exception());
        }
    }).detach();
    return _promise->get_future();
}

int main() {
    println("--- E2E Async/Await Test ---");
    auto test_fut = runAsyncTest();
    auto result = (test_fut).get();
    println(zenith::concat("Async result is: ", result));
    if (result == 60) {
        println("SUCCESS");
    } else {
        println("FAILED");
    }
}

