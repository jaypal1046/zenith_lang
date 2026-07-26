#ifndef ZENITH_BEHAVIOR_TREE_H
#define ZENITH_BEHAVIOR_TREE_H

#include <vector>
#include <memory>
#include <functional>

namespace zenith {

enum class BehaviorStatus {
    Success,
    Failure,
    Running
};

class BehaviorNode {
public:
    virtual ~BehaviorNode() = default;
    virtual BehaviorStatus tick() = 0;
};

using BehaviorNodePtr = std::shared_ptr<BehaviorNode>;

class ActionNode : public BehaviorNode {
private:
    std::function<BehaviorStatus()> m_action;

public:
    explicit ActionNode(std::function<BehaviorStatus()> action) : m_action(action) {}

    BehaviorStatus tick() override {
        return m_action ? m_action() : BehaviorStatus::Failure;
    }
};

class SequenceNode : public BehaviorNode {
public:
    std::vector<BehaviorNodePtr> children;

    SequenceNode() = default;
    explicit SequenceNode(const std::vector<BehaviorNodePtr>& nodes) : children(nodes) {}

    BehaviorStatus tick() override {
        for (const auto& child : children) {
            if (child) {
                BehaviorStatus status = child->tick();
                if (status != BehaviorStatus::Success) {
                    return status;
                }
            }
        }
        return BehaviorStatus::Success;
    }
};

class SelectorNode : public BehaviorNode {
public:
    std::vector<BehaviorNodePtr> children;

    SelectorNode() = default;
    explicit SelectorNode(const std::vector<BehaviorNodePtr>& nodes) : children(nodes) {}

    BehaviorStatus tick() override {
        for (const auto& child : children) {
            if (child) {
                BehaviorStatus status = child->tick();
                if (status != BehaviorStatus::Failure) {
                    return status;
                }
            }
        }
        return BehaviorStatus::Failure;
    }
};

} // namespace zenith

#endif // ZENITH_BEHAVIOR_TREE_H
