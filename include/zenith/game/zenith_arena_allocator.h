#ifndef ZENITH_ARENA_ALLOCATOR_H
#define ZENITH_ARENA_ALLOCATOR_H

#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>

namespace zenith {

class LinearArenaAllocator {
private:
    std::vector<uint8_t> m_buffer;
    std::size_t m_offset = 0;

public:
    explicit LinearArenaAllocator(std::size_t capacityBytes = 1024 * 1024)
        : m_buffer(capacityBytes), m_offset(0) {}

    void* allocate(std::size_t size, std::size_t alignment = 8) {
        std::size_t currentAddr = reinterpret_cast<std::size_t>(m_buffer.data() + m_offset);
        std::size_t padding = (alignment - (currentAddr % alignment)) % alignment;

        if (m_offset + padding + size > m_buffer.size()) {
            return nullptr; // Out of memory in arena
        }

        m_offset += padding;
        void* ptr = m_buffer.data() + m_offset;
        m_offset += size;
        return ptr;
    }

    template <typename T, typename... Args>
    T* create(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        if (!mem) return nullptr;
        return new (mem) T(std::forward<Args>(args)...);
    }

    void reset() {
        m_offset = 0;
    }

    std::size_t getUsedBytes() const { return m_offset; }
    std::size_t getCapacity() const { return m_buffer.size(); }
};

} // namespace zenith

#endif // ZENITH_ARENA_ALLOCATOR_H
