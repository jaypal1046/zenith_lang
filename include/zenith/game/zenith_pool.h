#ifndef ZENITH_POOL_H
#define ZENITH_POOL_H

#include <vector>
#include <memory>
#include <functional>
#include <iostream>

namespace zenith {

template <typename T>
class ObjectPool {
private:
    std::vector<std::shared_ptr<T>> m_pool;
    std::vector<std::shared_ptr<T>> m_available;

public:
    explicit ObjectPool(std::size_t initialCapacity = 64) {
        m_pool.reserve(initialCapacity);
        m_available.reserve(initialCapacity);
        for (std::size_t i = 0; i < initialCapacity; ++i) {
            auto obj = std::make_shared<T>();
            m_pool.push_back(obj);
            m_available.push_back(obj);
        }
    }

    std::shared_ptr<T> acquire() {
        if (m_available.empty()) {
            auto obj = std::make_shared<T>();
            m_pool.push_back(obj);
            return obj;
        }
        auto obj = m_available.back();
        m_available.pop_back();
        return obj;
    }

    void release(std::shared_ptr<T> obj) {
        if (obj) {
            m_available.push_back(obj);
        }
    }

    std::size_t getAvailableCount() const { return m_available.size(); }
    std::size_t getTotalCapacity() const { return m_pool.size(); }
};

} // namespace zenith

#endif // ZENITH_POOL_H
