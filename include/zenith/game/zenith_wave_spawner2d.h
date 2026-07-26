#ifndef ZENITH_WAVE_SPAWNER2D_H
#define ZENITH_WAVE_SPAWNER2D_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct WaveConfig {
    int waveNumber;
    std::string enemyType;
    int count;
    float spawnInterval;
};

class WaveSpawner2D {
private:
    std::vector<WaveConfig> m_waves;
    size_t m_currentWaveIndex = 0;
    int m_spawnedCount = 0;
    float m_timer = 0.0f;
    bool m_isSpawning = false;

public:
    void addWave(int waveNum, const std::string& enemyType, int count, float interval = 0.5f) {
        m_waves.push_back({ waveNum, enemyType, count, interval });
    }

    void startNextWave() {
        if (m_currentWaveIndex < m_waves.size()) {
            m_spawnedCount = 0;
            m_timer = 0.0f;
            m_isSpawning = true;
        }
    }

    bool update(float deltaTime) {
        if (!m_isSpawning || m_currentWaveIndex >= m_waves.size()) return false;

        const auto& wave = m_waves[m_currentWaveIndex];
        m_timer += deltaTime;

        if (m_timer >= wave.spawnInterval) {
            m_timer = 0.0f;
            m_spawnedCount++;

            if (m_spawnedCount >= wave.count) {
                m_isSpawning = false;
                m_currentWaveIndex++;
            }
            return true; // Single enemy spawned signal
        }
        return false;
    }

    bool isSpawning() const { return m_isSpawning; }
    int getCurrentWaveNumber() const { return static_cast<int>(m_currentWaveIndex) + 1; }
    int getTotalWaves() const { return static_cast<int>(m_waves.size()); }

    void drawWaveInfo(
        std::vector<uint32_t>& buffer, int width, int height,
        int posX = 20, int posY = 20, int barWidth = 100, int barHeight = 12) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_waves.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Draw dark background bar
        for (int y = posY; y < posY + barHeight; ++y) {
            for (int x = posX; x < posX + barWidth; ++x) {
                if (x == posX || x == posX + barWidth - 1 || y == posY || y == posY + barHeight - 1) {
                    drawPixel(x, y, 0xFF808080);
                } else {
                    drawPixel(x, y, 0xFF10101A);
                }
            }
        }

        // Draw wave progress fill (Gold fill)
        float progress = static_cast<float>(m_currentWaveIndex) / m_waves.size();
        int fillW = static_cast<int>((barWidth - 4) * progress);

        for (int y = posY + 2; y < posY + barHeight - 2; ++y) {
            for (int x = posX + 2; x < posX + 2 + fillW; ++x) {
                drawPixel(x, y, 0xFFFFD700);
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_WAVE_SPAWNER2D_H
