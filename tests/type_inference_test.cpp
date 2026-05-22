#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include "zenith_runtime.h"

int main() {
    auto count = 42;
    auto name = "Zenith";
    auto pi = 3.14159;
    auto is_active = true;
    zenith::UI::println(zenith::make_children("=== Type Inference Tests ==="), {});
    zenith::UI::println(zenith::make_children(zenith::concat("count (inferred Int): ", count)), {});
    zenith::UI::println(zenith::make_children(zenith::concat("name (inferred String): ", name)), {});
    zenith::UI::println(zenith::make_children(zenith::concat("pi (inferred Float): ", pi)), {});
    zenith::UI::println(zenith::make_children(zenith::concat("is_active (inferred Bool): ", is_active)), {});
    int explicit_int = 100;
    std::string explicit_str = "Explicit";
    zenith::UI::println(zenith::make_children(zenith::concat("explicit_int: ", explicit_int)), {});
    zenith::UI::println(zenith::make_children(zenith::concat("explicit_str: ", explicit_str)), {});
    void greet;
    name = "World";
    zenith::UI::println(zenith::make_children(zenith::concat("Hello, ", zenith::concat(name, "!"))), {});
}

