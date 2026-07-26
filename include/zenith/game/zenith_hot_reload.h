#ifndef ZENITH_HOT_RELOAD_H
#define ZENITH_HOT_RELOAD_H

#include "zenith_window.h"
#include <string>
#include <unordered_map>

namespace zenith {

class LiveTuner {
private:
    static inline std::unordered_map<std::string, float> s_floatRegistry;
    static inline std::unordered_map<std::string, int> s_intRegistry;

public:
    static float tuneFloat(const std::string& key, float defaultVal) {
        auto it = s_floatRegistry.find(key);
        if (it != s_floatRegistry.end()) {
            return it->second;
        }
        s_floatRegistry[key] = defaultVal;
        return defaultVal;
    }

    static void setFloat(const std::string& key, float val) {
        s_floatRegistry[key] = val;
    }

    static int tuneInt(const std::string& key, int defaultVal) {
        auto it = s_intRegistry.find(key);
        if (it != s_intRegistry.end()) {
            return it->second;
        }
        s_intRegistry[key] = defaultVal;
        return defaultVal;
    }

    static void setInt(const std::string& key, int val) {
        s_intRegistry[key] = val;
    }
};

} // namespace zenith

#endif // ZENITH_HOT_RELOAD_H
