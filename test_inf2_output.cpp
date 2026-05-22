#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

int main() {
    auto x = 5;
    auto y = 3.14;
    auto name = "Zenith";
    auto isActive = true;
    zenith::UI::print(zenith::make_children(zenith::concat("x: ", x)), {});
    zenith::UI::print(zenith::make_children(zenith::concat("y: ", y)), {});
    zenith::UI::print(zenith::make_children(zenith::concat("name: ", name)), {});
    zenith::UI::print(zenith::make_children(zenith::concat("isActive: ", isActive)), {});
}

