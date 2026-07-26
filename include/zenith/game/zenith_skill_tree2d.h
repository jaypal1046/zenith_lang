#ifndef ZENITH_SKILL_TREE2D_H
#define ZENITH_SKILL_TREE2D_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct SkillNode {
    int id;
    std::string name;
    float x, y;
    int parentId; // -1 if root node
    bool isUnlocked;
};

class SkillTree2D {
private:
    std::vector<SkillNode> m_nodes;

public:
    void addNode(int id, const std::string& name, float x, float y, int parentId = -1, bool isUnlocked = false) {
        m_nodes.push_back({ id, name, x, y, parentId, isUnlocked });
    }

    bool unlockNode(int id) {
        for (auto& node : m_nodes) {
            if (node.id == id) {
                // Check if parent is unlocked (or root)
                if (node.parentId == -1) {
                    node.isUnlocked = true;
                    return true;
                } else {
                    for (const auto& p : m_nodes) {
                        if (p.id == node.parentId && p.isUnlocked) {
                            node.isUnlocked = true;
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    void drawSkillTree(
        std::vector<uint32_t>& buffer, int width, int height,
        float nodeRadius = 14.0f) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_nodes.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        // Render connecting dependency lines between parent and child nodes
        for (const auto& node : m_nodes) {
            if (node.parentId != -1) {
                for (const auto& parent : m_nodes) {
                    if (parent.id == node.parentId) {
                        uint32_t lineCol = (node.isUnlocked && parent.isUnlocked) ? 0xFF00FF00 : 0xFF505050;

                        int x0 = static_cast<int>(parent.x);
                        int y0 = static_cast<int>(parent.y);
                        int x1 = static_cast<int>(node.x);
                        int y1 = static_cast<int>(node.y);

                        // Simple line segment
                        int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0));
                        if (steps > 0) {
                            float dx = static_cast<float>(x1 - x0) / steps;
                            float dy = static_cast<float>(y1 - y0) / steps;
                            for (int i = 0; i <= steps; ++i) {
                                drawPixel(static_cast<int>(x0 + i * dx), static_cast<int>(y0 + i * dy), lineCol);
                            }
                        }
                    }
                }
            }
        }

        // Render skill node circles
        for (const auto& node : m_nodes) {
            uint32_t nodeCol = node.isUnlocked ? 0xFFFFD700 : 0xFF2A2A3D; // Unlocked Gold / Locked Dark Slate
            int nX = static_cast<int>(node.x);
            int nY = static_cast<int>(node.y);
            int r = static_cast<int>(nodeRadius);

            for (int dy = -r; dy <= r; ++dy) {
                for (int dx = -r; dx <= r; ++dx) {
                    if (dx * dx + dy * dy <= nodeRadius * nodeRadius) {
                        drawPixel(nX + dx, nY + dy, nodeCol);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_SKILL_TREE2D_H
