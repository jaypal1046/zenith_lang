#ifndef ZENITH_PROFILER_H
#define ZENITH_PROFILER_H

#include <string>
#include <unordered_map>
#include <chrono>

namespace zenith {

struct ProfileMetric {
    double totalTimeMs = 0.0;
    int count = 0;
};

class Profiler {
private:
    static inline std::unordered_map<std::string, ProfileMetric> s_metrics;
    static inline std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> s_startTimes;

public:
    static void beginSection(const std::string& name) {
        s_startTimes[name] = std::chrono::high_resolution_clock::now();
    }

    static void endSection(const std::string& name) {
        auto it = s_startTimes.find(name);
        if (it != s_startTimes.end()) {
            auto now = std::chrono::high_resolution_clock::now();
            double durationMs = std::chrono::duration<double, std::milli>(now - it->second).count();
            s_metrics[name].totalTimeMs += durationMs;
            s_metrics[name].count++;
        }
    }

    static double getAverageTimeMs(const std::string& name) {
        auto it = s_metrics.find(name);
        if (it != s_metrics.end() && it->second.count > 0) {
            return it->second.totalTimeMs / it->second.count;
        }
        return 0.0;
    }

    static void reset() {
        s_metrics.clear();
        s_startTimes.clear();
    }
};

class ProfileScope {
private:
    std::string m_name;

public:
    ProfileScope(const std::string& name) : m_name(name) {
        Profiler::beginSection(m_name);
    }
    ~ProfileScope() {
        Profiler::endSection(m_name);
    }
};

} // namespace zenith

#endif // ZENITH_PROFILER_H
