#ifndef ZENITH_SAVE_SYSTEM_H
#define ZENITH_SAVE_SYSTEM_H

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

namespace zenith {

class SaveSystem {
private:
    std::unordered_map<std::string, std::string> m_data;

public:
    SaveSystem() = default;

    void setString(const std::string& key, const std::string& value) {
        m_data[key] = value;
    }

    void setInt(const std::string& key, int value) {
        m_data[key] = std::to_string(value);
    }

    void setFloat(const std::string& key, float value) {
        m_data[key] = std::to_string(value);
    }

    void setBool(const std::string& key, bool value) {
        m_data[key] = value ? "1" : "0";
    }

    std::string getString(const std::string& key, const std::string& defaultVal = "") const {
        auto it = m_data.find(key);
        return (it != m_data.end()) ? it->second : defaultVal;
    }

    int getInt(const std::string& key, int defaultVal = 0) const {
        auto it = m_data.find(key);
        if (it != m_data.end()) {
            try { return std::stoi(it->second); } catch (...) {}
        }
        return defaultVal;
    }

    float getFloat(const std::string& key, float defaultVal = 0.0f) const {
        auto it = m_data.find(key);
        if (it != m_data.end()) {
            try { return std::stof(it->second); } catch (...) {}
        }
        return defaultVal;
    }

    bool getBool(const std::string& key, bool defaultVal = false) const {
        auto it = m_data.find(key);
        if (it != m_data.end()) {
            return (it->second == "1" || it->second == "true");
        }
        return defaultVal;
    }

    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) return false;

        for (const auto& pair : m_data) {
            file << pair.first << "=" << pair.second << "\n";
        }
        return true;
    }

    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        m_data.clear();
        std::string line;
        while (std::getline(file, line)) {
            std::size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                m_data[key] = value;
            }
        }
        return true;
    }

    void clear() { m_data.clear(); }
};

} // namespace zenith

#endif // ZENITH_SAVE_SYSTEM_H
