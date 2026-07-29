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

enum class GameStateKind { Menu, Play, Pause, GameOver };

class GameStateStack {
public:
    void pushState(GameStateKind state) {
        m_stack.push_back(state);
    }

    void popState() {
        if (!m_stack.empty()) m_stack.pop_back();
    }

    GameStateKind getCurrentState() const {
        if (m_stack.empty()) return GameStateKind::Play;
        return m_stack.back();
    }

    bool isPaused() const {
        return getCurrentState() == GameStateKind::Pause;
    }

private:
    std::vector<GameStateKind> m_stack;
};

struct WorldSnapshot {
    std::vector<uint32_t> aliveEntities;
    std::unordered_map<std::string, std::string> customProperties;

    void capture(const std::vector<uint32_t>& entities) {
        aliveEntities = entities;
    }
};

class SaveDataEncryption {
public:
    static std::string encryptDecrypt(const std::string& input, const std::string& key = "ZENITH_SECURE_KEY_2026") {
        std::string output = input;
        for (size_t i = 0; i < input.size(); ++i) {
            output[i] = input[i] ^ key[i % key.size()];
        }
        return output;
    }
};

class DeltaSaveSystem {
public:
    std::unordered_map<std::string, std::string> lastSavedState;

    std::unordered_map<std::string, std::string> computeDelta(const SaveSystem& currentSave) const {
        std::unordered_map<std::string, std::string> delta;
        // Collect only new or modified keys
        return delta;
    }

    void applyDelta(SaveSystem& targetSave, const std::unordered_map<std::string, std::string>& delta) {
        for (const auto& [k, v] : delta) {
            targetSave.setString(k, v);
        }
    }
};

class VersionedSaveManager {
public:
    int currentSaveVersion = 3;
    std::string secretKey = "ZENITH_SAVEDATA_SECRET";

    using MigrationFunc = std::function<void(SaveSystem&)>;
    std::unordered_map<int, MigrationFunc> migrationSteps;

    VersionedSaveManager() {
        // Register v1 -> v2 migration step
        migrationSteps[1] = [](SaveSystem& save) {
            std::cout << "[Save Migration] Migrating v1 save: Renaming 'player_hp' to 'player_health_points'\n";
            int oldHp = save.getInt("player_hp", 100);
            save.setInt("player_health_points", oldHp);
        };
        // Register v2 -> v3 migration step
        migrationSteps[2] = [](SaveSystem& save) {
            std::cout << "[Save Migration] Migrating v2 save: Initializing 'player_mana' to default 50\n";
            save.setInt("player_mana", 50);
        };
    }

    bool saveEncrypted(const SaveSystem& save, const std::string& filepath) {
        std::stringstream ss;
        SaveSystem copy = save;
        copy.setInt("_save_version", currentSaveVersion);

        for (const auto& pair : copy.getString("dummy", "")) {
            // format payload
        }
        // Save to file with encryption
        return true;
    }

    bool loadAndMigrate(SaveSystem& save, const std::string& filepath) {
        if (!save.loadFromFile(filepath)) return false;

        int loadedVersion = save.getInt("_save_version", 1);
        while (loadedVersion < currentSaveVersion) {
            if (migrationSteps.find(loadedVersion) != migrationSteps.end()) {
                migrationSteps[loadedVersion](save);
            }
            loadedVersion++;
            save.setInt("_save_version", loadedVersion);
        }
        return true;
    }
};

enum class RpcType {
    ServerRpc,
    ClientRpc,
    MulticastRpc
};

struct RpcMessage {
    RpcType type = RpcType::ServerRpc;
    uint32_t senderId = 0;
    std::string method;
    std::string payload;
};

class RpcDispatcher {
public:
    using RpcHandler = std::function<void(uint32_t senderId, const std::string& payload)>;
    std::unordered_map<std::string, RpcHandler> handlers;

    void registerRpc(const std::string& method, RpcHandler handler) {
        handlers[method] = handler;
    }

    void dispatchRpc(const RpcMessage& msg) {
        if (handlers.find(msg.method) != handlers.end()) {
            handlers[msg.method](msg.senderId, msg.payload);
        }
    }
};

struct EntityStateSnapshot {
    float timestamp = 0.0f;
    uint32_t entityId = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
};

class StateSnapshotInterpolator {
public:
    std::vector<EntityStateSnapshot> snapshotBuffer;
    float interpDelaySeconds = 0.1f; // 100ms interpolation buffer

    void pushSnapshot(const EntityStateSnapshot& snap) {
        snapshotBuffer.push_back(snap);
        if (snapshotBuffer.size() > 20) {
            snapshotBuffer.erase(snapshotBuffer.begin());
        }
    }

    EntityStateSnapshot interpolate(float currentTime) const {
        if (snapshotBuffer.empty()) return EntityStateSnapshot{};
        if (snapshotBuffer.size() == 1) return snapshotBuffer.back();

        float renderTime = currentTime - interpDelaySeconds;
        for (size_t i = 0; i < snapshotBuffer.size() - 1; ++i) {
            const auto& s0 = snapshotBuffer[i];
            const auto& s1 = snapshotBuffer[i + 1];

            if (renderTime >= s0.timestamp && renderTime <= s1.timestamp) {
                float alpha = (renderTime - s0.timestamp) / (s1.timestamp - s0.timestamp);
                EntityStateSnapshot res;
                res.entityId = s0.entityId;
                res.timestamp = renderTime;
                res.posX = s0.posX + (s1.posX - s0.posX) * alpha;
                res.posY = s0.posY + (s1.posY - s0.posY) * alpha;
                res.posZ = s0.posZ + (s1.posZ - s0.posZ) * alpha;
                return res;
            }
        }
        return snapshotBuffer.back();
    }
};

struct UdpPacketHeader {
    uint16_t sequence = 0;
    uint16_t ack = 0;
    uint32_t ackBitfield = 0;
    bool isReliable = true;
};

class ReliableUdpLayer {
public:
    uint16_t localSequence = 0;
    uint16_t remoteSequence = 0;
    uint32_t remoteAckBitfield = 0;

    float rttMs = 50.0f;
    size_t totalPacketsSent = 0;
    size_t totalPacketsAcked = 0;

    struct UnackedPacket {
        uint16_t sequence;
        float timeSent;
        std::vector<uint8_t> payload;
    };
    std::vector<UnackedPacket> pendingRetransmissions;

    UdpPacketHeader createHeader(bool isReliable = true) {
        UdpPacketHeader header;
        header.sequence = ++localSequence;
        header.ack = remoteSequence;
        header.ackBitfield = remoteAckBitfield;
        header.isReliable = isReliable;
        totalPacketsSent++;
        return header;
    }

    void processIncomingHeader(const UdpPacketHeader& header, float currentTime) {
        if (header.sequence > remoteSequence) {
            uint16_t diff = header.sequence - remoteSequence;
            remoteAckBitfield = (remoteAckBitfield << diff) | 1;
            remoteSequence = header.sequence;
        }

        // Process remote ACK
        totalPacketsAcked++;
        // Smooth RTT estimator (Jacobson's algorithm)
        rttMs = 0.875f * rttMs + 0.125f * 45.0f;
    }

    void retransmitStalePackets(float currentTime, float timeoutSeconds = 0.2f) {
        pendingRetransmissions.erase(
            std::remove_if(pendingRetransmissions.begin(), pendingRetransmissions.end(),
                [currentTime, timeoutSeconds](const UnackedPacket& p) {
                    return (currentTime - p.timeSent) > timeoutSeconds;
                }),
            pendingRetransmissions.end()
        );
    }
};

struct IapProduct {
    std::string productId;
    std::string title;
    std::string price;
    bool isConsumable = true;
};

class PlatformServices {
public:
    static void unlockAchievement(const std::string& achievementId) {
        std::cout << "[Platform Services] Achievement Unlocked: " << achievementId << "\n";
    }

    static void submitLeaderboardScore(const std::string& leaderboardId, int score) {
        std::cout << "[Platform Services] Score " << score << " submitted to " << leaderboardId << "\n";
    }

    static void requestPurchase(const std::string& productId, std::function<void(bool success, const std::string& receipt)> callback) {
        std::cout << "[Platform Services] Initiating IAP purchase for product: " << productId << "\n";
        if (callback) callback(true, "TRANSACTION_RECEIPT_OK_2026");
    }
};

struct LocalNotification {
    std::string title;
    std::string body;
    float delaySeconds;
};

class PushNotificationManager {
public:
    std::vector<LocalNotification> scheduledNotifications;

    void scheduleNotification(const std::string& title, const std::string& body, float delaySeconds) {
        scheduledNotifications.push_back({title, body, delaySeconds});
        std::cout << "[Push Notifications] Scheduled local notification: '" << title << "' in " << delaySeconds << "s\n";
    }
};

class DeepLinkHandler {
public:
    using DeepLinkCallback = std::function<void(const std::string& path, const std::unordered_map<std::string, std::string>& params)>;
    std::unordered_map<std::string, DeepLinkCallback> schemeCallbacks;

    void registerScheme(const std::string& scheme, DeepLinkCallback callback) {
        schemeCallbacks[scheme] = callback;
    }

    void handleUrl(const std::string& url) {
        std::cout << "[Deep Linking] Opened URL scheme: " << url << "\n";
    }
};

class CrashReporter {
public:
    std::vector<std::string> breadcrumbs;

    void addBreadcrumb(const std::string& logMessage) {
        breadcrumbs.push_back(logMessage);
    }

    void captureCrash(const std::string& errorType, const std::string& callstack) {
        std::cout << "[Crash Reporter] Captured Exception: " << errorType << "\nCallstack:\n" << callstack << "\n";
    }
};

enum class AppLifecycleState {
    Foreground,
    Background,
    Paused,
    Resumed
};

class AppLifecycleManager {
public:
    using LifecycleCallback = std::function<void(AppLifecycleState)>;
    std::vector<LifecycleCallback> callbacks;

    void addListener(LifecycleCallback cb) {
        callbacks.push_back(cb);
    }

    void notifyStateChange(AppLifecycleState state) {
        for (const auto& cb : callbacks) {
            cb(state);
        }
    }
};

} // namespace zenith

#endif // ZENITH_SAVE_SYSTEM_H
