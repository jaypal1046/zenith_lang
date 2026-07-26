#ifndef ZENITH_NODE2D_H
#define ZENITH_NODE2D_H

#include "zenith_window.h"
#include <vector>
#include <memory>
#include <cmath>

namespace zenith {

class Node2D : public std::enable_shared_from_this<Node2D> {
public:
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    std::weak_ptr<Node2D> parent;
    std::vector<std::shared_ptr<Node2D>> children;

    Node2D() = default;

    void addChild(std::shared_ptr<Node2D> child) {
        if (!child) return;
        child->parent = shared_from_this();
        children.push_back(child);
    }

    Point2D getWorldPosition() const {
        Point2D pos{x, y};
        auto parentPtr = parent.lock();
        if (parentPtr) {
            Point2D parentPos = parentPtr->getWorldPosition();
            pos.x += parentPos.x;
            pos.y += parentPos.y;
        }
        return pos;
    }

    virtual void update(float dt) {
        for (auto& child : children) {
            if (child) child->update(dt);
        }
    }

    virtual void render() {
        for (auto& child : children) {
            if (child) child->render();
        }
    }
};

} // namespace zenith

#endif // ZENITH_NODE2D_H
