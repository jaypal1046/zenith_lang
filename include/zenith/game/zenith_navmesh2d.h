#ifndef ZENITH_NAVMESH2D_H
#define ZENITH_NAVMESH2D_H

#include <vector>
#include <cmath>

namespace zenith {

struct NavPoint2D {
    float x = 0.0f;
    float y = 0.0f;
};

struct NavPolygon2D {
    std::vector<NavPoint2D> vertices;
    NavPoint2D centroid;
};

class NavMesh2D {
private:
    std::vector<NavPolygon2D> m_polygons;

public:
    NavMesh2D() = default;

    void addPolygon(const std::vector<NavPoint2D>& verts) {
        NavPolygon2D poly;
        poly.vertices = verts;
        float sumX = 0.0f, sumY = 0.0f;
        for (const auto& v : verts) {
            sumX += v.x;
            sumY += v.y;
        }
        if (!verts.empty()) {
            poly.centroid = {sumX / verts.size(), sumY / verts.size()};
        }
        m_polygons.push_back(poly);
    }

    std::vector<NavPoint2D> findPath(NavPoint2D start, NavPoint2D end) const {
        // Direct waypoint path across convex polygon centroids
        std::vector<NavPoint2D> path;
        path.push_back(start);
        for (const auto& poly : m_polygons) {
            path.push_back(poly.centroid);
        }
        path.push_back(end);
        return path;
    }

    std::size_t getPolygonCount() const { return m_polygons.size(); }
};

} // namespace zenith

#endif // ZENITH_NAVMESH2D_H
