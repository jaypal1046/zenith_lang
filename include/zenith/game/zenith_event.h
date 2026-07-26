#ifndef ZENITH_EVENT_H
#define ZENITH_EVENT_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <functional>
#include <iostream>

namespace zenith {

class IEvent {
public:
    virtual ~IEvent() = default;
};

class EventBus {
private:
    using EventCallback = std::function<void(const IEvent&)>;
    std::unordered_map<std::type_index, std::vector<EventCallback>> m_listeners;

public:
    EventBus() = default;

    template <typename EventType, typename Func>
    void subscribe(Func callback) {
        std::type_index typeIdx(typeid(EventType));
        m_listeners[typeIdx].push_back([callback](const IEvent& event) {
            callback(static_cast<const EventType&>(event));
        });
    }

    template <typename EventType>
    void publish(const EventType& event) {
        std::type_index typeIdx(typeid(EventType));
        auto it = m_listeners.find(typeIdx);
        if (it != m_listeners.end()) {
            for (const auto& callback : it->second) {
                callback(event);
            }
        }
    }

    void clear() {
        m_listeners.clear();
    }
};

} // namespace zenith

#endif // ZENITH_EVENT_H
