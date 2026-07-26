#ifndef ZENITH_RAGDOLL2D_H
#define ZENITH_RAGDOLL2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct RagdollJoint2D {
    float x = 0.0f;
    float y = 0.0f;
    float prevX = 0.0f;
    float prevY = 0.0f;
    bool pinned = false;
};

struct RagdollBone2D {
    std::size_t j1 = 0;
    std::size_t j2 = 0;
    float restLength = 1.0f;
};

class Ragdoll2D {
private:
    std::vector<RagdollJoint2D> m_joints;
    std::vector<RagdollBone2D> m_bones;

public:
    Ragdoll2D() = default;

    void createHumanoid(float rootX, float rootY) {
        m_joints.clear();
        m_bones.clear();

        // 0: Head, 1: Neck, 2: Pelvis, 3: Left Hand, 4: Right Hand, 5: Left Foot, 6: Right Foot
        m_joints.push_back({rootX, rootY - 40.0f, rootX, rootY - 40.0f, false});
        m_joints.push_back({rootX, rootY - 20.0f, rootX, rootY - 20.0f, false});
        m_joints.push_back({rootX, rootY, rootX, rootY, false});
        m_joints.push_back({rootX - 25.0f, rootY - 10.0f, rootX - 25.0f, rootY - 10.0f, false});
        m_joints.push_back({rootX + 25.0f, rootY - 10.0f, rootX + 25.0f, rootY - 10.0f, false});
        m_joints.push_back({rootX - 15.0f, rootY + 30.0f, rootX - 15.0f, rootY + 30.0f, false});
        m_joints.push_back({rootX + 15.0f, rootY + 30.0f, rootX + 15.0f, rootY + 30.0f, false});

        auto addBone = [this](std::size_t a, std::size_t b) {
            float dx = m_joints[b].x - m_joints[a].x;
            float dy = m_joints[b].y - m_joints[a].y;
            float len = std::sqrt(dx * dx + dy * dy);
            m_bones.push_back({a, b, len});
        };

        addBone(0, 1); // Head to Neck
        addBone(1, 2); // Torso Spine
        addBone(1, 3); // Left Arm
        addBone(1, 4); // Right Arm
        addBone(2, 5); // Left Leg
        addBone(2, 6); // Right Leg
    }

    void update(float dt, float gravity = 980.0f) {
        for (auto& j : m_joints) {
            if (j.pinned) continue;
            float vx = j.x - j.prevX;
            float vy = j.y - j.prevY;
            j.prevX = j.x;
            j.prevY = j.y;
            j.x += vx;
            j.y += vy + gravity * dt * dt;
        }

        // Satisfy bone length constraints
        for (const auto& b : m_bones) {
            auto& j1 = m_joints[b.j1];
            auto& j2 = m_joints[b.j2];
            float dx = j2.x - j1.x;
            float dy = j2.y - j1.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > 0.0001f) {
                float diff = (dist - b.restLength) / dist;
                float offX = dx * 0.5f * diff;
                float offY = dy * 0.5f * diff;
                if (!j1.pinned) { j1.x += offX; j1.y += offY; }
                if (!j2.pinned) { j2.x -= offX; j2.y -= offY; }
            }
        }
    }

    std::size_t getJointCount() const { return m_joints.size(); }
    const RagdollJoint2D& getJoint(std::size_t index) const { return m_joints[index]; }
};

} // namespace zenith

#endif // ZENITH_RAGDOLL2D_H
