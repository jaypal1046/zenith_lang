#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

int main() {
    auto intVar = 42;
    auto floatVar = 3.14;
    auto stringVar = "Hello Zenith";
    auto boolVar = true;
    zenith::UI::print(zenith::make_children("Int: "), {});
    zenith::UI::print(zenith::make_children(intVar), {});
    zenith::UI::print(zenith::make_children("Float: "), {});
    zenith::UI::print(zenith::make_children(floatVar), {});
}

