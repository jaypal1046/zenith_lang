#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

int main() {
    auto x = 5;
    auto y = 3.14;
    auto name = "Zenith";
    zenith::UI::print(zenith::make_children("Success"), {});
}

