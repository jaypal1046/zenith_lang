#ifndef ZENITH_QUEST_H
#define ZENITH_QUEST_H

#include <string>
#include <vector>
#include <unordered_map>

namespace zenith {

struct QuestObjective {
    std::string description;
    int requiredCount = 1;
    int currentCount = 0;

    bool isCompleted() const { return currentCount >= requiredCount; }
};

class Quest {
private:
    std::string m_id;
    std::string m_title;
    std::string m_description;
    std::vector<QuestObjective> m_objectives;
    bool m_completed = false;

public:
    Quest() = default;
    Quest(const std::string& id, const std::string& title, const std::string& description)
        : m_id(id), m_title(title), m_description(description) {}

    void addObjective(const std::string& desc, int required = 1) {
        m_objectives.push_back({desc, required, 0});
    }

    void advanceObjective(std::size_t index, int count = 1) {
        if (index < m_objectives.size()) {
            m_objectives[index].currentCount += count;
            checkCompletion();
        }
    }

    void checkCompletion() {
        for (const auto& obj : m_objectives) {
            if (!obj.isCompleted()) {
                m_completed = false;
                return;
            }
        }
        m_completed = !m_objectives.empty();
    }

    bool isCompleted() const { return m_completed; }
    const std::string& getId() const { return m_id; }
    const std::string& getTitle() const { return m_title; }
};

class QuestManager {
private:
    std::unordered_map<std::string, Quest> m_quests;

public:
    QuestManager() = default;

    void addQuest(const Quest& quest) {
        m_quests[quest.getId()] = quest;
    }

    void advanceObjective(const std::string& questId, std::size_t objectiveIndex, int count = 1) {
        auto it = m_quests.find(questId);
        if (it != m_quests.end()) {
            it->second.advanceObjective(objectiveIndex, count);
        }
    }

    bool isQuestCompleted(const std::string& questId) const {
        auto it = m_quests.find(questId);
        return (it != m_quests.end()) ? it->second.isCompleted() : false;
    }
};

} // namespace zenith

#endif // ZENITH_QUEST_H
