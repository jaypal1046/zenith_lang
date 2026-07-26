#ifndef ZENITH_STATE_H
#define ZENITH_STATE_H

#include <vector>
#include <memory>
#include <iostream>
#include <typeindex>

namespace zenith {

class IGameState {
public:
    virtual ~IGameState() = default;
    virtual void onEnter() {}
    virtual void onUpdate(float dt) = 0;
    virtual void onRender() = 0;
    virtual void onExit() {}
};

class GameStateMachine {
private:
    std::vector<std::shared_ptr<IGameState>> m_stateStack;

public:
    GameStateMachine() = default;

    template <typename StateT, typename... Args>
    void changeState(Args&&... args) {
        if (!m_stateStack.empty()) {
            m_stateStack.back()->onExit();
            m_stateStack.pop_back();
        }
        auto newState = std::make_shared<StateT>(std::forward<Args>(args)...);
        m_stateStack.push_back(newState);
        newState->onEnter();
    }

    template <typename StateT, typename... Args>
    void pushState(Args&&... args) {
        auto newState = std::make_shared<StateT>(std::forward<Args>(args)...);
        m_stateStack.push_back(newState);
        newState->onEnter();
    }

    void popState() {
        if (!m_stateStack.empty()) {
            m_stateStack.back()->onExit();
            m_stateStack.pop_back();
        }
    }

    void update(float dt) {
        if (!m_stateStack.empty()) {
            m_stateStack.back()->onUpdate(dt);
        }
    }

    void render() {
        for (const auto& state : m_stateStack) {
            state->onRender();
        }
    }

    bool empty() const { return m_stateStack.empty(); }
    std::size_t size() const { return m_stateStack.size(); }
};

} // namespace zenith

#endif // ZENITH_STATE_H
