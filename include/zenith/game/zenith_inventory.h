#ifndef ZENITH_INVENTORY_H
#define ZENITH_INVENTORY_H

#include <vector>
#include <string>
#include <algorithm>

namespace zenith {

struct ItemSlot {
    std::string itemId;
    int quantity = 0;
    int maxStack = 64;

    bool isEmpty() const { return quantity <= 0 || itemId.empty(); }
};

class Inventory2D {
private:
    std::vector<ItemSlot> m_slots;

public:
    Inventory2D(std::size_t capacity = 20) : m_slots(capacity) {}

    bool addItem(const std::string& itemId, int count = 1, int maxStack = 64) {
        int remaining = count;

        // Try stacking into existing non-full slots
        for (auto& slot : m_slots) {
            if (slot.itemId == itemId && slot.quantity < slot.maxStack) {
                int space = slot.maxStack - slot.quantity;
                int toAdd = std::min(remaining, space);
                slot.quantity += toAdd;
                remaining -= toAdd;
                if (remaining <= 0) return true;
            }
        }

        // Try placing into empty slots
        for (auto& slot : m_slots) {
            if (slot.isEmpty()) {
                slot.itemId = itemId;
                slot.maxStack = maxStack;
                int toAdd = std::min(remaining, maxStack);
                slot.quantity = toAdd;
                remaining -= toAdd;
                if (remaining <= 0) return true;
            }
        }

        return remaining < count;
    }

    int getItemCount(const std::string& itemId) const {
        int total = 0;
        for (const auto& slot : m_slots) {
            if (slot.itemId == itemId) {
                total += slot.quantity;
            }
        }
        return total;
    }

    std::size_t getCapacity() const { return m_slots.size(); }
    const ItemSlot& getSlot(std::size_t index) const { return m_slots[index]; }
};

} // namespace zenith

#endif // ZENITH_INVENTORY_H
