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

int main() {
    auto x = 5;
    auto y = 3.14;
    auto name = "Zenith";
    println("Success Reloaded");
}

