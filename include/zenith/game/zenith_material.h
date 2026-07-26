#ifndef ZENITH_MATERIAL_H
#define ZENITH_MATERIAL_H

#include "zenith_window.h"
#include "zenith_texture.h"
#include <string>
#include <unordered_map>

namespace zenith {

class Material2D {
private:
    std::string m_name;
    Color m_colorTint = Color::White();
    float m_roughness = 0.5f;
    float m_emission = 0.0f;
    std::unordered_map<std::string, float> m_floatProperties;

public:
    explicit Material2D(const std::string& name = "DefaultMaterial") : m_name(name) {}

    void setColorTint(const Color& color) { m_colorTint = color; }
    Color getColorTint() const { return m_colorTint; }

    void setEmission(float emission) { m_emission = emission; }
    float getEmission() const { return m_emission; }

    void setFloat(const std::string& propertyName, float val) {
        m_floatProperties[propertyName] = val;
    }

    float getFloat(const std::string& propertyName, float defaultVal = 0.0f) const {
        auto it = m_floatProperties.find(propertyName);
        if (it != m_floatProperties.end()) {
            return it->second;
        }
        return defaultVal;
    }

    void bind() const {
#ifdef _WIN32
        glColor4f(m_colorTint.r, m_colorTint.g, m_colorTint.b, m_colorTint.a);
#endif
    }
};

} // namespace zenith

#endif // ZENITH_MATERIAL_H
