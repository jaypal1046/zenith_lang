#ifndef ZENITH_NETWORK_H
#define ZENITH_NETWORK_H

#include <vector>
#include <string>
#include <cstring>
#include <cstdint>

namespace zenith {

class NetPacket {
private:
    std::vector<uint8_t> m_buffer;
    std::size_t m_readPos = 0;

public:
    NetPacket() = default;

    void writeInt(int32_t val) {
        uint8_t bytes[4];
        std::memcpy(bytes, &val, 4);
        m_buffer.insert(m_buffer.end(), bytes, bytes + 4);
    }

    int32_t readInt() {
        if (m_readPos + 4 > m_buffer.size()) return 0;
        int32_t val = 0;
        std::memcpy(&val, &m_buffer[m_readPos], 4);
        m_readPos += 4;
        return val;
    }

    void writeFloat(float val) {
        uint8_t bytes[4];
        std::memcpy(bytes, &val, 4);
        m_buffer.insert(m_buffer.end(), bytes, bytes + 4);
    }

    float readFloat() {
        if (m_readPos + 4 > m_buffer.size()) return 0.0f;
        float val = 0.0f;
        std::memcpy(&val, &m_buffer[m_readPos], 4);
        m_readPos += 4;
        return val;
    }

    std::size_t getSize() const { return m_buffer.size(); }
    void resetRead() { m_readPos = 0; }
};

} // namespace zenith

#endif // ZENITH_NETWORK_H
