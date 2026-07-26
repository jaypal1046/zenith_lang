#ifndef ZENITH_DIALOGUE_H
#define ZENITH_DIALOGUE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace zenith {

struct DialogueChoice {
    std::string text;
    int targetNodeId = -1;
    std::function<void()> onSelect;
};

struct DialogueNode {
    int id = 0;
    std::string speaker;
    std::string text;
    std::vector<DialogueChoice> choices;
};

class DialogueTree {
private:
    std::unordered_map<int, DialogueNode> m_nodes;
    int m_currentNodeId = -1;
    bool m_active = false;

public:
    DialogueTree() = default;

    void addNode(const DialogueNode& node) {
        m_nodes[node.id] = node;
    }

    void start(int startNodeId = 0) {
        if (m_nodes.find(startNodeId) != m_nodes.end()) {
            m_currentNodeId = startNodeId;
            m_active = true;
        }
    }

    void chooseOption(std::size_t choiceIndex) {
        if (!m_active) return;
        auto it = m_nodes.find(m_currentNodeId);
        if (it != m_nodes.end() && choiceIndex < it->second.choices.size()) {
            const auto& choice = it->second.choices[choiceIndex];
            if (choice.onSelect) choice.onSelect();

            if (choice.targetNodeId >= 0 && m_nodes.find(choice.targetNodeId) != m_nodes.end()) {
                m_currentNodeId = choice.targetNodeId;
            } else {
                m_active = false;
            }
        }
    }

    bool isActive() const { return m_active; }
    std::string getCurrentSpeaker() const {
        auto it = m_nodes.find(m_currentNodeId);
        return (it != m_nodes.end()) ? it->second.speaker : "";
    }

    std::string getCurrentText() const {
        auto it = m_nodes.find(m_currentNodeId);
        return (it != m_nodes.end()) ? it->second.text : "";
    }
};

} // namespace zenith

#endif // ZENITH_DIALOGUE_H
