#ifndef ZENITH_WEATHER_H
#define ZENITH_WEATHER_H

#include "zenith_window.h"
#include <vector>
#include <cstdlib>

namespace zenith {

enum class WeatherType {
    Clear,
    Rain,
    Snow,
    Storm
};

struct WeatherDrop {
    float x = 0.0f;
    float y = 0.0f;
    float speedY = 200.0f;
    float length = 10.0f;
};

class WeatherSystem2D {
private:
    WeatherType m_type = WeatherType::Clear;
    std::vector<WeatherDrop> m_drops;
    float m_windX = 20.0f;

public:
    WeatherSystem2D() = default;

    void setWeather(WeatherType type, std::size_t intensity = 100) {
        m_type = type;
        m_drops.resize(intensity);
        for (auto& drop : m_drops) {
            drop.x = static_cast<float>(std::rand() % 1280);
            drop.y = static_cast<float>(std::rand() % 720);
            drop.speedY = (type == WeatherType::Rain) ? 500.0f : 100.0f; // Rain drops fall faster than snow
            drop.length = (type == WeatherType::Rain) ? 12.0f : 4.0f;
        }
    }

    void setWind(float windX) { m_windX = windX; }

    void update(float dt, float screenWidth = 1280.0f, float screenHeight = 720.0f) {
        if (m_type == WeatherType::Clear) return;

        for (auto& drop : m_drops) {
            drop.x += m_windX * dt;
            drop.y += drop.speedY * dt;

            if (drop.y > screenHeight) {
                drop.y = -10.0f;
                drop.x = static_cast<float>(std::rand() % static_cast<int>(screenWidth));
            }
        }
    }

    std::size_t getDropCount() const { return m_drops.size(); }
    WeatherType getType() const { return m_type; }
};

} // namespace zenith

#endif // ZENITH_WEATHER_H
