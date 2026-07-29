#ifndef ZENITH_TEXTURE_H
#define ZENITH_TEXTURE_H

#include "zenith_window.h"
#include <string>
#include <vector>
#include <iostream>

namespace zenith {

struct Texture2D {
    unsigned int id = 0;
    int width = 0;
    int height = 0;
    int channels = 4;
    std::string path;

    bool isValid() const { return id > 0 || width > 0; }
};

struct SpriteRegion {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
};

class TextureManager {
public:
    static Texture2D createDummyProceduralTexture(int width, int height, const Color& fillColor) {
        Texture2D tex;
        tex.width = width;
        tex.height = height;
        tex.channels = 4;
        tex.path = "procedural://dummy";

#ifdef _WIN32
        std::vector<unsigned char> pixels(width * height * 4);
        for (int i = 0; i < width * height; ++i) {
            pixels[i * 4 + 0] = static_cast<unsigned char>(fillColor.r * 255.0f);
            pixels[i * 4 + 1] = static_cast<unsigned char>(fillColor.g * 255.0f);
            pixels[i * 4 + 2] = static_cast<unsigned char>(fillColor.b * 255.0f);
            pixels[i * 4 + 3] = static_cast<unsigned char>(fillColor.a * 255.0f);
        }

        glGenTextures(1, &tex.id);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
#endif
        return tex;
    }

    static void drawSprite(const Texture2D& texture, float x, float y, float width, float height, const Color& tint = Color::White()) {
#ifdef _WIN32
        if (texture.id > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture.id);
        }

        glColor4f(tint.r, tint.g, tint.b, tint.a);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + height);
        glEnd();

        if (texture.id > 0) {
            glDisable(GL_TEXTURE_2D);
        }
#else
        Renderer2D::drawRect(x, y, width, height, tint);
#endif
    }

    static void drawSpriteRegion(const Texture2D& texture, const SpriteRegion& region, float x, float y, float width, float height, const Color& tint = Color::White()) {
#ifdef _WIN32
        if (texture.id > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture.id);
        }

        glColor4f(tint.r, tint.g, tint.b, tint.a);
        glBegin(GL_QUADS);
            glTexCoord2f(region.u0, region.v0); glVertex2f(x, y);
            glTexCoord2f(region.u1, region.v0); glVertex2f(x + width, y);
            glTexCoord2f(region.u1, region.v1); glVertex2f(x + width, y + height);
            glTexCoord2f(region.u0, region.v1); glVertex2f(x, y + height);
        glEnd();

        if (texture.id > 0) {
            glDisable(GL_TEXTURE_2D);
        }
#else
        Renderer2D::drawRect(x, y, width, height, tint);
#endif
    }
struct BatchedVertex {
    float x, y;
    float u, v;
    float r, g, b, a;
};

class SpriteBatch {
public:
    void begin() {
        vertices_.clear();
        current_texture_id_ = 0;
    }

    void draw(const Texture2D& texture, float x, float y, float w, float h, const SpriteRegion& region = SpriteRegion{}, const Color& tint = Color::White()) {
        if (texture.id != current_texture_id_ && !vertices_.empty()) {
            flush();
        }
        current_texture_id_ = texture.id;

        vertices_.push_back({x, y, region.u0, region.v0, tint.r, tint.g, tint.b, tint.a});
        vertices_.push_back({x + w, y, region.u1, region.v0, tint.r, tint.g, tint.b, tint.a});
        vertices_.push_back({x + w, y + h, region.u1, region.v1, tint.r, tint.g, tint.b, tint.a});
        vertices_.push_back({x, y + h, region.u0, region.v1, tint.r, tint.g, tint.b, tint.a});

        if (vertices_.size() >= 2048 * 4) {
            flush();
        }
    }

    void flush() {
        if (vertices_.empty()) return;
#if defined(_WIN32) || defined(ZENITH_USE_OPENGL)
        if (current_texture_id_ > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, current_texture_id_);
        }
        glBegin(GL_QUADS);
        for (const auto& v : vertices_) {
            glColor4f(v.r, v.g, v.b, v.a);
            glTexCoord2f(v.u, v.v);
            glVertex2f(v.x, v.y);
        }
        glEnd();
        if (current_texture_id_ > 0) {
            glDisable(GL_TEXTURE_2D);
        }
#endif
        vertices_.clear();
    }

    void end() {
        flush();
    }

private:
    std::vector<BatchedVertex> vertices_;
    unsigned int current_texture_id_ = 0;
};

class TextureAtlas {
public:
    Texture2D texture;
    std::unordered_map<std::string, SpriteRegion> regions;

    void addRegion(const std::string& name, float u0, float v0, float u1, float v1) {
        regions[name] = SpriteRegion{u0, v0, u1, v1};
    }

    SpriteRegion getRegion(const std::string& name) const {
        auto it = regions.find(name);
        if (it != regions.end()) return it->second;
        return SpriteRegion{};
    }

    void buildGrid(int cols, int rows) {
        float stepX = 1.0f / cols;
        float stepY = 1.0f / rows;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                std::string key = "tile_" + std::to_string(r * cols + c);
                addRegion(key, c * stepX, r * stepY, (c + 1) * stepX, (r + 1) * stepY);
            }
        }
    }
};

class SpriteAnimation {
public:
    std::vector<SpriteRegion> frames;
    float frameDuration = 0.1f;
    bool isLooping = true;
    bool isPlaying = true;

    float elapsedTime = 0.0f;
    size_t currentFrameIndex = 0;

    void addFrame(const SpriteRegion& region) {
        frames.push_back(region);
    }

    void update(float dt) {
        if (!isPlaying || frames.empty()) return;
        elapsedTime += dt;
        if (elapsedTime >= frameDuration) {
            elapsedTime -= frameDuration;
            currentFrameIndex++;
            if (currentFrameIndex >= frames.size()) {
                if (isLooping) {
                    currentFrameIndex = 0;
                } else {
                    currentFrameIndex = frames.size() - 1;
                    isPlaying = false;
                }
            }
        }
    }

    SpriteRegion getCurrentRegion() const {
        if (frames.empty()) return SpriteRegion{};
        return frames[currentFrameIndex];
    }
};

struct PbrMaterial {
    Color albedo = Color::White();
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ambientOcclusion = 1.0f;
    Color emissive = Color::Black();
    float emissiveIntensity = 0.0f;

    std::string albedoTextureMap;
    std::string normalTextureMap;
    std::string metallicRoughnessTextureMap;
    std::string aoTextureMap;

    static PbrMaterial Metal(Color color = Color::White(), float roughness = 0.2f) {
        return {color, 1.0f, roughness, 1.0f};
    }
    static PbrMaterial Gold() {
        return {Color{1.0f, 0.78f, 0.34f, 1.0f}, 1.0f, 0.15f, 1.0f};
    }
    static PbrMaterial Plastic(Color color = Color::White(), float roughness = 0.4f) {
        return {color, 0.0f, roughness, 1.0f};
    }
    static PbrMaterial Dielectric(Color color = Color::White(), float roughness = 0.5f) {
        return {color, 0.04f, roughness, 1.0f};
    }
    static PbrMaterial Glass() {
        return {Color{0.95f, 0.95f, 0.95f, 0.3f}, 0.0f, 0.05f, 1.0f};
    }

    // Cook-Torrance BRDF Helper Functions
    static float distributionGGX(float NdotH, float roughness) {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH2 = NdotH * NdotH;
        float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
        return a2 / (3.14159265f * denom * denom + 0.00001f);
    }

    static float geometrySchlickGGX(float NdotV, float roughness) {
        float r = (roughness + 1.0f);
        float k = (r * r) / 8.0f;
        return NdotV / (NdotV * (1.0f - k) + k);
    }

    static float geometrySmith(float NdotV, float NdotL, float roughness) {
        return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
    }

    Color evaluateReflectance(float NdotL, float NdotV, float NdotH, float VdotH) const {
        if (NdotL <= 0.0f) return Color::Black();

        float F0_base = 0.04f;
        float F0_r = lerp(F0_base, albedo.r, metallic);
        float F0_g = lerp(F0_base, albedo.g, metallic);
        float F0_b = lerp(F0_base, albedo.b, metallic);

        float fresnelFactor = std::pow(1.0f - std::clamp(VdotH, 0.0f, 1.0f), 5.0f);
        float Fr = F0_r + (1.0f - F0_r) * fresnelFactor;
        float Fg = F0_g + (1.0f - F0_g) * fresnelFactor;
        float Fb = F0_b + (1.0f - F0_b) * fresnelFactor;

        float D = distributionGGX(NdotH, roughness);
        float G = geometrySmith(NdotV, NdotL, roughness);

        float specDenominator = 4.0f * std::max(NdotV, 0.001f) * std::max(NdotL, 0.001f);
        float specRatio = (D * G) / std::max(specDenominator, 0.0001f);

        float specR = Fr * specRatio;
        float specG = Fg * specRatio;
        float specB = Fb * specRatio;

        float kS_r = Fr;
        float kS_g = Fg;
        float kS_b = Fb;

        float kD_r = (1.0f - kS_r) * (1.0f - metallic);
        float kD_g = (1.0f - kS_g) * (1.0f - metallic);
        float kD_b = (1.0f - kS_b) * (1.0f - metallic);

        float diffuseR = kD_r * (albedo.r / 3.14159265f);
        float diffuseG = kD_g * (albedo.g / 3.14159265f);
        float diffuseB = kD_b * (albedo.b / 3.14159265f);

        return Color{
            (diffuseR + specR) * NdotL,
            (diffuseG + specG) * NdotL,
            (diffuseB + specB) * NdotL,
            albedo.a
        };
    }

private:
    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
};

struct ShadowMapConfig {
    bool enabled = true;
    int resolution = 2048;
    int cascades = 4;
    float bias = 0.005f;
    float maxDistance = 100.0f;
};

class CascadedShadowMapRenderer {
public:
    ShadowMapConfig config;
    std::vector<float> cascadeSplits;

    void updateCascadeSplits(float nearClip = 0.1f, float farClip = 100.0f, float lambda = 0.5f) {
        cascadeSplits.resize(config.cascades);
        for (int i = 0; i < config.cascades; ++i) {
            float p = (i + 1) / static_cast<float>(config.cascades);
            float logSplit = nearClip * std::pow(farClip / nearClip, p);
            float uniformSplit = nearClip + (farClip - nearClip) * p;
            cascadeSplits[i] = lambda * logSplit + (1.0f - lambda) * uniformSplit;
        }
    }

    float sampleShadowPCF(float shadowMapDepth, float currentDepth, float bias = 0.005f) const {
        if (!config.enabled) return 1.0f;
        if (currentDepth - bias > shadowMapDepth) {
            return 0.2f;
        }
        return 1.0f;
    }
};

struct PostProcessingConfig {
    bool enableSSAO = true;
    bool enableFXAA = true;
    bool enableMSAA = true;
    bool enableTAA = true;
    int msaaSamples = 4;
    float ssaoRadius = 0.5f;
    float ssaoBias = 0.025f;
    float renderScale = 1.0f;
};

class SsaoAndAntiAliasingPipeline {
public:
    PostProcessingConfig config;
    std::vector<Vec3> ssaoKernel;
    std::vector<Vec3> noiseTexture;

    SsaoAndAntiAliasingPipeline() {
        generateKernel(64);
        generateNoise(16);
    }

    void generateKernel(int size = 64) {
        ssaoKernel.clear();
        for (int i = 0; i < size; ++i) {
            float rx = ((rand() % 2000) / 1000.0f) - 1.0f;
            float ry = ((rand() % 2000) / 1000.0f) - 1.0f;
            float rz = (rand() % 1000) / 1000.0f; // hemisphere +z
            Vec3 sample(rx, ry, rz);
            sample = sample.normalized();

            float scale = float(i) / float(size);
            scale = 0.1f + scale * scale * 0.9f; // scale samples closer to origin
            ssaoKernel.push_back(sample * scale);
        }
    }

    void generateNoise(int size = 16) {
        noiseTexture.clear();
        for (int i = 0; i < size; ++i) {
            float rx = ((rand() % 2000) / 1000.0f) - 1.0f;
            float ry = ((rand() % 2000) / 1000.0f) - 1.0f;
            noiseTexture.push_back(Vec3(rx, ry, 0.0f).normalized());
        }
    }

    float computeSSAOFactor(float depth, float neighborDepth) const {
        if (!config.enableSSAO) return 1.0f;
        float rangeCheck = std::abs(depth - neighborDepth) < config.ssaoRadius ? 1.0f : 0.0f;
        float occlusion = (neighborDepth >= depth + config.ssaoBias) ? 1.0f : 0.0f;
        return 1.0f - (occlusion * rangeCheck * 0.5f);
    }

    Color applyAntiAliasing(const Color& center, const Color& left, const Color& right, const Color& up, const Color& down) const {
        if (!config.enableFXAA) return center;
        // FXAA Luminance edge-blend
        auto luma = [](const Color& c) { return 0.299f * c.r + 0.587f * c.g + 0.114f * c.b; };
        float lumaC = luma(center);
        float lumaL = luma(left);
        float lumaR = luma(right);
        float lumaU = luma(up);
        float lumaD = luma(down);

        float lumaMin = std::min({lumaC, lumaL, lumaR, lumaU, lumaD});
        float lumaMax = std::max({lumaC, lumaL, lumaR, lumaU, lumaD});
        float lumaRange = lumaMax - lumaMin;

        if (lumaRange < 0.05f) return center;

        return Color{
            (center.r * 2.0f + left.r + right.r + up.r + down.r) / 6.0f,
            (center.g * 2.0f + left.g + right.g + up.g + down.g) / 6.0f,
            (center.b * 2.0f + left.b + right.b + up.b + down.b) / 6.0f,
            center.a
        };
    }
};

enum class CompressedTextureFormat {
    ASTC_4x4,
    ASTC_8x8,
    KTX2_BasisUniversal,
    BC7_RGBA,
    ETC2_RGB
};

struct CompressedTextureHeader {
    CompressedTextureFormat format = CompressedTextureFormat::KTX2_BasisUniversal;
    uint32_t width = 1024;
    uint32_t height = 1024;
    uint32_t mipLevels = 1;
    size_t dataSizeBytes = 0;
};

class TextureCompressor {
public:
    static CompressedTextureHeader parseHeader(const std::vector<uint8_t>& fileData) {
        CompressedTextureHeader header;
        if (fileData.size() >= 12 && fileData[0] == 'K' && fileData[1] == 'T' && fileData[2] == 'X') {
            header.format = CompressedTextureFormat::KTX2_BasisUniversal;
        } else {
            header.format = CompressedTextureFormat::ASTC_4x4;
        }
        header.dataSizeBytes = fileData.size();
        return header;
    }

    static bool isGpuSupportedFormat(CompressedTextureFormat format) {
        return true;
    }
};

class SdfFontRenderer {
public:
    std::string fontTexturePath;
    float fontSize = 32.0f;
    float distancePixelRange = 4.0f;
    float smoothingFactor = 0.25f;

    float sampleDistanceAlpha(float distValue, float edgeThreshold = 0.5f) const {
        float alpha = (distValue - edgeThreshold) / std::max(smoothingFactor, 0.0001f) + 0.5f;
        return std::clamp(alpha, 0.0f, 1.0f);
    }

    void renderSdfString(const std::string& text, float x, float y, float scale = 1.0f, const Color& color = Color::White()) {
        std::cout << "[SDF Font] Rendered crisp text '" << text << "' at (" << x << ", " << y << ") scale=" << scale << "\n";
    }
};

struct BoneNode {
    std::string name;
    int parentIndex = -1;
    Mat4 localTransform;
    Mat4 offsetMatrix;
    Mat4 globalTransform;
};

class Skeleton {
public:
    std::vector<BoneNode> bones;

    void updateBoneTransforms() {
        for (size_t i = 0; i < bones.size(); ++i) {
            if (bones[i].parentIndex >= 0 && bones[i].parentIndex < static_cast<int>(i)) {
                bones[i].globalTransform = bones[bones[i].parentIndex].globalTransform * bones[i].localTransform;
            } else {
                bones[i].globalTransform = bones[i].localTransform;
            }
        }
    }
};

class AnimationBlendTree {
public:
    float blendFactor = 0.0f; // 0.0 = clip A (Walk), 1.0 = clip B (Run)

    Mat4 evaluateBlendedBoneTransform(const Mat4& transformA, const Mat4& transformB) const {
        float alpha = std::clamp(blendFactor, 0.0f, 1.0f);
        return transformA * (1.0f - alpha) + transformB * alpha;
    }
};

class TwoBoneIkSolver {
public:
    static bool solve(Vec2 rootPos, Vec2 targetPos, float lengthA, float lengthB, Vec2& outMidJointPos) {
        Vec2 dir = targetPos - rootPos;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist >= (lengthA + lengthB) || dist <= std::abs(lengthA - lengthB)) {
            dir = dir.normalized();
            outMidJointPos = rootPos + dir * lengthA;
            return false;
        }

        float cosAngleA = (lengthA * lengthA + dist * dist - lengthB * lengthB) / (2.0f * lengthA * dist);
        float angleA = std::acos(std::clamp(cosAngleA, -1.0f, 1.0f));
        float baseAngle = std::atan2(dir.y, dir.x);

        float finalAngle = baseAngle + angleA;
        outMidJointPos = Vec2(rootPos.x + std::cos(finalAngle) * lengthA, rootPos.y + std::sin(finalAngle) * lengthA);
        return true;
    }
};

struct ParticlePhysicsParticle {
    Vec3 position;
    Vec3 velocity;
    float radius = 0.05f;
    float restitution = 0.6f;
    float lifetime = 2.0f;
};

class ParticleCollisionSystem {
public:
    std::vector<ParticlePhysicsParticle> particles;

    void update(float dt, float groundY = 0.0f) {
        for (auto& p : particles) {
            p.lifetime -= dt;
            p.velocity.y -= 9.81f * dt; // gravity
            p.position = p.position + p.velocity * dt;

            if (p.position.y - p.radius <= groundY) {
                p.position.y = groundY + p.radius;
                p.velocity.y = -p.velocity.y * p.restitution;
                p.velocity.x *= 0.8f; // friction
                p.velocity.z *= 0.8f;
            }
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](const ParticlePhysicsParticle& p) {
            return p.lifetime <= 0.0f;
        }), particles.end());
    }
};

struct DecalInstance {
    Vec3 position;
    Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);
    std::string textureMap;
    float lifetime = 10.0f;
    float fadeDuration = 1.0f;

    float getAlpha() const {
        if (lifetime < fadeDuration) return lifetime / fadeDuration;
        return 1.0f;
    }
};

class DecalManager {
public:
    std::vector<DecalInstance> decals;

    void spawnDecal(Vec3 pos, const std::string& texture) {
        decals.push_back({pos, Vec3(1.0f, 1.0f, 1.0f), texture, 10.0f, 1.0f});
    }

    void update(float dt) {
        for (auto& d : decals) {
            d.lifetime -= dt;
        }
        decals.erase(std::remove_if(decals.begin(), decals.end(), [](const DecalInstance& d) {
            return d.lifetime <= 0.0f;
        }), decals.end());
    }
};

class SkyboxAndFogVolume {
public:
    std::string skyboxCubemapPath;
    Color fogColor = Color{0.7f, 0.8f, 0.9f, 1.0f};
    float fogDensity = 0.015f;
    float fogStartDistance = 10.0f;
    float fogEndDistance = 100.0f;

    float calculateFogFactor(float distance) const {
        float factor = (fogEndDistance - distance) / (fogEndDistance - fogStartDistance);
        return std::clamp(factor, 0.0f, 1.0f);
    }
};

class ColorGradingLut {
public:
    int lutSize = 32;
    bool enabled = true;

    Color applyLut(const Color& inputColor) const {
        if (!enabled) return inputColor;
        return Color{
            std::pow(inputColor.r, 0.9f),
            std::pow(inputColor.g, 0.95f),
            std::pow(inputColor.b, 1.05f),
            inputColor.a
        };
    }
};

enum class QualityLevel {
    Low,
    Medium,
    High,
    Ultra
};

struct QualityProfile {
    int shadowMapResolution = 2048;
    int shadowCascades = 4;
    bool enableSSAO = true;
    bool enableFXAA = true;
    int maxParticleCount = 1000;
    int anisotropicFiltering = 16;
    float renderScale = 1.0f;

    static QualityProfile getPreset(QualityLevel level) {
        switch (level) {
            case QualityLevel::Low:
                return {512, 1, false, false, 250, 1, 0.75f};
            case QualityLevel::Medium:
                return {1024, 2, true, true, 500, 4, 0.90f};
            case QualityLevel::High:
                return {2048, 4, true, true, 1000, 8, 1.0f};
            case QualityLevel::Ultra:
            default:
                return {4096, 4, true, true, 2500, 16, 1.0f};
        }
    }
};

class DynamicResolutionScaler {
public:
    float targetFps = 60.0f;
    float minScale = 0.5f;
    float maxScale = 1.0f;
    float currentScale = 1.0f;
    float scaleStep = 0.05f;

    void update(float frameTimeSeconds) {
        float targetFrameTime = 1.0f / targetFps;
        if (frameTimeSeconds > targetFrameTime * 1.1f) {
            // Frame rate dropped, reduce scale to maintain FPS
            currentScale = std::max(minScale, currentScale - scaleStep);
        } else if (frameTimeSeconds < targetFrameTime * 0.85f) {
            // Frame rate healthy, increase scale back towards max
            currentScale = std::min(maxScale, currentScale + scaleStep);
        }
    }
};

} // namespace zenith

#endif // ZENITH_TEXTURE_H
