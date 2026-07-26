#ifndef ZENITH_CRAFTING2D_H
#define ZENITH_CRAFTING2D_H

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cstdint>

namespace zenith {

struct Recipe {
    std::string resultItem;
    uint32_t resultColor;
    std::map<std::string, int> ingredients; // Ingredient name -> required count
};

class Crafting2D {
private:
    std::vector<Recipe> m_recipes;

public:
    void registerRecipe(const std::string& resultItem, uint32_t resultColor, const std::map<std::string, int>& ingredients) {
        m_recipes.push_back({ resultItem, resultColor, ingredients });
    }

    bool canCraft(size_t recipeIndex, const std::map<std::string, int>& inventory) const {
        if (recipeIndex >= m_recipes.size()) return false;

        const auto& recipe = m_recipes[recipeIndex];
        for (const auto& kv : recipe.ingredients) {
            auto it = inventory.find(kv.first);
            if (it == inventory.end() || it->second < kv.second) {
                return false; // Missing ingredient or insufficient count
            }
        }
        return true;
    }

    void drawCraftingGrid(
        std::vector<uint32_t>& buffer, int width, int height,
        const std::map<std::string, int>& inventory,
        int posX = 20, int posY = 20, int slotSize = 30, int spacing = 8) const
    {
        if (buffer.empty() || width <= 0 || height <= 0 || m_recipes.empty()) return;

        auto drawPixel = [&](int px, int py, uint32_t c) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = c;
            }
        };

        for (size_t i = 0; i < m_recipes.size(); ++i) {
            int sX = posX;
            int sY = posY + static_cast<int>(i) * (slotSize + spacing);

            bool craftable = canCraft(i, inventory);
            uint32_t borderCol = craftable ? 0xFF00FF00 : 0xFFFF0000;

            for (int y = sY; y < sY + slotSize; ++y) {
                for (int x = sX; x < sX + slotSize; ++x) {
                    if (x == sX || x == sX + slotSize - 1 || y == sY || y == sY + slotSize - 1) {
                        drawPixel(x, y, borderCol);
                    } else {
                        drawPixel(x, y, m_recipes[i].resultColor);
                    }
                }
            }
        }
    }
};

} // namespace zenith

#endif // ZENITH_CRAFTING2D_H
