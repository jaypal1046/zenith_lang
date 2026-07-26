#ifndef ZENITH_RENDER_LAYER_H
#define ZENITH_RENDER_LAYER_H

#include <vector>
#include <functional>
#include <algorithm>

namespace zenith {

struct Renderable2D {
    int zIndex = 0;
    float ySortPos = 0.0f;
    std::function<void()> renderFunc;
};

class RenderLayerManager2D {
private:
    std::vector<Renderable2D> m_renderQueue;

public:
    RenderLayerManager2D() = default;

    void addRenderable(int zIndex, float ySortPos, std::function<void()> renderFunc) {
        m_renderQueue.push_back({zIndex, ySortPos, renderFunc});
    }

    void sortAndRender() {
        std::stable_sort(m_renderQueue.begin(), m_renderQueue.end(), [](const Renderable2D& a, const Renderable2D& b) {
            if (a.zIndex != b.zIndex) {
                return a.zIndex < b.zIndex;
            }
            return a.ySortPos < b.ySortPos;
        });

        for (const auto& item : m_renderQueue) {
            if (item.renderFunc) item.renderFunc();
        }

        m_renderQueue.clear();
    }

    std::size_t getQueueSize() const { return m_renderQueue.size(); }
};

} // namespace zenith

#endif // ZENITH_RENDER_LAYER_H
