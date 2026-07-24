#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <future>
#include <iostream>
#include <functional>
#include "zenith_runtime.h"
#include "zenith/std/concurrency.hpp"


#ifdef __ANDROID__
const bool isAndroid = true;
#else
const bool isAndroid = false;
#endif

#ifdef __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE
    const bool isIos = true;
    const bool isMac = false;
  #else
    const bool isIos = false;
    const bool isMac = true;
  #endif
#else
  const bool isIos = false;
  const bool isMac = false;
#endif

#ifdef __linux__
  #ifndef __ANDROID__
    const bool isLinux = true;
  #else
    const bool isLinux = false;
  #endif
#else
  const bool isLinux = false;
#endif

#ifdef _WIN32
const bool isWindows = true;
#else
const bool isWindows = false;
#endif

const bool isWeb = false;

template <typename T>
inline void print(const T& msg) { std::cout << msg; }
template <typename T>
inline void println(const T& msg) { std::cout << msg << std::endl; }
inline std::string httpGet(std::string url) { return zenith::httpGet(url); }
inline std::string httpPost(std::string url, std::string json_body) { return zenith::httpPost(url, json_body); }
inline std::string gcStats() { return zenith::mem::gcStatsString(); }

extern "C" bool _zenith_builtin_isKeyPressed(const char*);

bool isKeyPressed(std::string key) {
    return ::_zenith_builtin_isKeyPressed(key.c_str());
}

extern "C" bool _zenith_builtin_wasKeyPressed(const char*);

bool wasKeyPressed(std::string key) {
    return ::_zenith_builtin_wasKeyPressed(key.c_str());
}

class NebulaNovelScene : public zenith::game::Scene {
private:
public:
    int currentStep = 0;
    bool isChoiceActive = false;
    float elapsed = 0.0;
    std::string currentSpeaker = "Captain Nova";
    std::string currentText = "Stardate 4092. Deep space sector 7. All primary systems nominal.";
    std::string choiceAText = "";
    std::string choiceBText = "";
    int nextA = 0;
    int nextB = 0;
    std::string statusMessage = "";

    NebulaNovelScene()  {}
    NebulaNovelScene(int currentStep, bool isChoiceActive, float elapsed, std::string currentSpeaker, std::string currentText, std::string choiceAText, std::string choiceBText, int nextA, int nextB, std::string statusMessage) : currentStep(currentStep), isChoiceActive(isChoiceActive), elapsed(elapsed), currentSpeaker(currentSpeaker), currentText(currentText), choiceAText(choiceAText), choiceBText(choiceBText), nextA(nextA), nextB(nextB), statusMessage(statusMessage) {}

    zenith::game::EntityId createEntity(std::string name) {
        return world.createEntity(name);
    }

    void setEntityName(zenith::game::EntityId entity, std::string name) {
        if (!world.isAlive(entity)) {
            return;
        }
        if (zenith::game::NameComponent* component = world.getName(entity)) {
            component->value = name;
        } else {
            world.addName(entity, name);
        }
    }

    std::string entityName(zenith::game::EntityId entity) {
        if (const zenith::game::NameComponent* component = world.getName(entity)) {
            return component->value;
        }
        return "";
    }

    void setEntityTag(zenith::game::EntityId entity, std::string tag) {
        if (!world.isAlive(entity)) {
            return;
        }
        if (zenith::game::TagComponent* component = world.getTag(entity)) {
            component->value = tag;
        } else {
            world.addTag(entity, tag);
        }
    }

    std::string entityTag(zenith::game::EntityId entity) {
        if (const zenith::game::TagComponent* component = world.getTag(entity)) {
            return component->value;
        }
        return "";
    }

    zenith::game::EntityId findEntityByName(std::string name) {
        std::optional<zenith::game::EntityId> entity = world.findByName(name);
        return entity.has_value() ? entity.value() : zenith::game::EntityId::invalid();
    }

    zenith::game::EntityId findEntityByTag(std::string tag) {
        std::optional<zenith::game::EntityId> entity = world.findByTag(tag);
        return entity.has_value() ? entity.value() : zenith::game::EntityId::invalid();
    }

    bool setParent(zenith::game::EntityId child, zenith::game::EntityId parent) {
        return zenith::game::Scene::setParent(child, parent);
    }

    bool clearParent(zenith::game::EntityId child) {
        return zenith::game::Scene::clearParent(child);
    }

    zenith::game::EntityId parentOf(zenith::game::EntityId child) {
        return zenith::game::Scene::parentOf(child);
    }

    int childCount(zenith::game::EntityId parent) {
        return zenith::game::Scene::childCount(parent);
    }

    zenith::game::EntityId childAt(zenith::game::EntityId parent, int index) {
        return zenith::game::Scene::childAt(parent, index);
    }

    zenith::game::EntityId spawnSprite(std::string name, float x, float y, float w, float h, std::string color) {
        zenith::game::EntityId entity = world.createEntity(name);
        zenith::game::Transform2D& transform = world.addTransform2D(entity);
        transform.position = zenith::physics::Vec2(x, y);
        zenith::game::SpriteRenderer2D& sprite = world.addSpriteRenderer2D(entity);
        sprite.size = zenith::physics::Vec2(w, h);
        sprite.tintColor = color;
        return entity;
    }

    zenith::game::EntityId spawnTexturedSprite(std::string name, std::string texturePath, float x, float y, float w, float h, std::string color) {
        zenith::game::EntityId entity = spawnSprite(name, x, y, w, h, color);
        if (zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            sprite->textureHandle = zenith::resource::ResourceManager::getInstance().loadTextureHandle(texturePath);
            sprite->texture = zenith::resource::ResourceManager::getInstance().loadTexture(sprite->textureHandle);
        }
        return entity;
    }

    zenith::game::EntityId spawnCamera2D(std::string name, float x, float y, float zoom, bool primary) {
        zenith::game::EntityId entity = world.createEntity(name);
        zenith::game::Transform2D& transform = world.addTransform2D(entity);
        transform.position = zenith::physics::Vec2(x, y);
        if (primary) {
            std::optional<zenith::game::EntityId> current = world.primaryCamera2D();
            if (current.has_value()) {
                if (zenith::game::Camera2DComponent* existing = world.getCamera2D(current.value())) {
                    existing->primary = false;
                }
            }
        }
        zenith::game::Camera2DComponent& camera = world.addCamera2D(entity);
        camera.zoom = zoom;
        camera.primary = primary;
        return entity;
    }

    bool destroyEntity(zenith::game::EntityId entity) {
        return world.destroyEntity(entity);
    }

    bool isEntityAlive(zenith::game::EntityId entity) {
        return world.isAlive(entity);
    }

    int entityCount() {
        return static_cast<int>(world.entityCount());
    }

    void setEntityPosition2D(zenith::game::EntityId entity, float x, float y) {
        if (!world.isAlive(entity)) {
            return;
        }
        zenith::game::Transform2D* transform = world.getTransform2D(entity);
        if (transform == nullptr) {
            transform = &world.addTransform2D(entity);
        }
        transform->position = zenith::physics::Vec2(x, y);
        if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            body->position = transform->position;
        }
    }

    void moveEntity2D(zenith::game::EntityId entity, float dx, float dy) {
        setEntityPosition2D(entity, entityPositionX(entity) + dx, entityPositionY(entity) + dy);
    }

    float entityPositionX(zenith::game::EntityId entity) {
        if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {
            return transform->position.x;
        }
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->position.x;
        }
        return 0.0f;
    }

    float entityPositionY(zenith::game::EntityId entity) {
        if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {
            return transform->position.y;
        }
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->position.y;
        }
        return 0.0f;
    }

    void setSpriteColor(zenith::game::EntityId entity, std::string color) {
        if (zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            sprite->tintColor = color;
        }
    }

    void setSpriteTexture(zenith::game::EntityId entity, std::string texturePath) {
        if (!world.isAlive(entity)) {
            return;
        }
        zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity);
        if (sprite == nullptr) {
            sprite = &world.addSpriteRenderer2D(entity);
        }
        sprite->textureHandle = zenith::resource::ResourceManager::getInstance().loadTextureHandle(texturePath);
        sprite->texture = zenith::resource::ResourceManager::getInstance().loadTexture(sprite->textureHandle);
    }

    std::string spriteTexturePath(zenith::game::EntityId entity) {
        if (const zenith::game::SpriteRenderer2D* sprite = world.getSpriteRenderer2D(entity)) {
            if (!sprite->textureHandle.path.empty()) {
                return sprite->textureHandle.path;
            }
            if (sprite->texture) {
                return sprite->texture->path;
            }
        }
        return "";
    }

    void attachBody2D(zenith::game::EntityId entity, float mass, float gravityScale, float friction, float restitution) {
        if (!world.isAlive(entity)) {
            return;
        }
        zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity);
        if (body == nullptr) {
            body = &world.addRigidBody2D(entity);
        }
        body->mass = (mass <= 0.0f) ? 1.0f : mass;
        body->gravityScale = gravityScale;
        body->friction = friction;
        body->restitution = restitution;
        if (const zenith::game::Transform2D* transform = world.getTransform2D(entity)) {
            body->position = transform->position;
        }
    }

    zenith::game::BoxCollider2DView boxCollider2D(zenith::game::EntityId entity) {
        return zenith::game::Scene::boxCollider2D(entity);
    }

    zenith::game::CircleCollider2DView circleCollider2D(zenith::game::EntityId entity) {
        return zenith::game::Scene::circleCollider2D(entity);
    }

    zenith::game::CapsuleCollider2DView capsuleCollider2D(zenith::game::EntityId entity) {
        return zenith::game::Scene::capsuleCollider2D(entity);
    }

    void attachBoxCollider2D(zenith::game::EntityId entity, float width, float height, bool isTrigger) {
        zenith::game::Scene::attachBoxCollider2D(entity, width, height, isTrigger);
    }

    void attachCircleCollider2D(zenith::game::EntityId entity, float radius, bool isTrigger) {
        zenith::game::Scene::attachCircleCollider2D(entity, radius, isTrigger);
    }

    void attachCapsuleCollider2D(zenith::game::EntityId entity, float height, float radius, bool isTrigger) {
        zenith::game::Scene::attachCapsuleCollider2D(entity, height, radius, isTrigger);
    }

    void setBodyVelocity2D(zenith::game::EntityId entity, float vx, float vy) {
        if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            body->velocity = zenith::physics::Vec2(vx, vy);
        }
    }

    void applyBodyImpulse2D(zenith::game::EntityId entity, float ix, float iy) {
        if (zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            body->applyImpulse(zenith::physics::Vec2(ix, iy));
        }
    }

    float bodyVelocityX(zenith::game::EntityId entity) {
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->velocity.x;
        }
        return 0.0f;
    }

    float bodyVelocityY(zenith::game::EntityId entity) {
        if (const zenith::physics::RigidBody2D* body = world.getRigidBody2D(entity)) {
            return body->velocity.y;
        }
        return 0.0f;
    }

    bool overlaps2D(zenith::game::EntityId first, zenith::game::EntityId second) {
        return zenith::game::Scene::overlaps2D(first, second);
    }

    bool containsPoint2D(zenith::game::EntityId entity, float x, float y) {
        return zenith::game::Scene::containsPoint2D(entity, x, y);
    }

    zenith::game::RaycastHit2DResult raycast2D(float originX, float originY, float directionX, float directionY, float maxDistance) {
        return zenith::game::Scene::raycast2D(originX, originY, directionX, directionY, maxDistance);
    }

    zenith::game::BoxCollider3DView boxCollider3D(zenith::game::EntityId entity) {
        return zenith::game::Scene::boxCollider3D(entity);
    }

    zenith::game::SphereCollider3DView sphereCollider3D(zenith::game::EntityId entity) {
        return zenith::game::Scene::sphereCollider3D(entity);
    }

    void attachBoxCollider3D(zenith::game::EntityId entity, float width, float height, float depth, bool isTrigger) {
        zenith::game::Scene::attachBoxCollider3D(entity, width, height, depth, isTrigger);
    }

    void attachSphereCollider3D(zenith::game::EntityId entity, float radius, bool isTrigger) {
        zenith::game::Scene::attachSphereCollider3D(entity, radius, isTrigger);
    }

    bool overlaps3D(zenith::game::EntityId first, zenith::game::EntityId second) {
        return zenith::game::Scene::overlaps3D(first, second);
    }

    bool containsPoint3D(zenith::game::EntityId entity, float x, float y, float z) {
        return zenith::game::Scene::containsPoint3D(entity, x, y, z);
    }

    zenith::game::RaycastHit3DResult raycast3D(float originX, float originY, float originZ, float directionX, float directionY, float directionZ, float maxDistance) {
        return zenith::game::Scene::raycast3D(originX, originY, originZ, directionX, directionY, directionZ, maxDistance);
    }

    bool followPrimaryCamera2D(zenith::game::EntityId target, float offsetX, float offsetY, float smoothing) {
        return zenith::followPrimaryCamera2D(world, target, zenith::physics::Vec2(offsetX, offsetY), smoothing);
    }

    bool followPrimaryCamera3D(zenith::game::EntityId target, float offsetX, float offsetY, float offsetZ, float smoothing) {
        return zenith::followPrimaryCamera3D(world, target, zenith::physics::Vec3(offsetX, offsetY, offsetZ), smoothing);
    }

    void setStepData(int stepId, std::string speaker, std::string text, std::string optA, std::string optB, int nA, int nB, std::string status) {
        currentStep = stepId;
        currentSpeaker = speaker;
        currentText = text;
        choiceAText = optA;
        choiceBText = optB;
        nextA = nA;
        nextB = nB;
        statusMessage = status;
        if (optA != "") {
            isChoiceActive = true;
        } else {
            isChoiceActive = false;
        }
    }

    void loadStoryNode(int nodeId) {
        if (nodeId == 0) {
            setStepData(0, "Captain Nova", "Stardate 4092. We have entered the uncharted outer nebula near Sector 7.", "", "", 1, 1, "NOMINAL");
        } else {
            if (nodeId == 1) {
                setStepData(1, "Lieutenant Ray", "Captain! Long-range sensors are detecting an energy anomaly ahead in the nebula quadrant.", "1. Investigate the Signal", "2. Raise Deflector Shields & Hold Position", 2, 3, "ALERT");
            } else {
                if (nodeId == 2) {
                    setStepData(2, "Ship Computer", "WARNING: Thermal overload in main power core! Emergency venting sequence engaged immediately.", "", "", 4, 4, "OVERHEAT");
                } else {
                    if (nodeId == 3) {
                        setStepData(3, "Captain Nova", "Shields up to maximum! Rerouting auxiliary power to thermal dampeners.", "", "", 4, 4, "SHIELDS 100%");
                    } else {
                        if (nodeId == 4) {
                            setStepData(4, "Lieutenant Ray", "Power levels are back in safe parameters! The ship and crew are secure.", "", "", 5, 5, "STABILIZED");
                        } else {
                            if (nodeId == 5) {
                                setStepData(5, "Captain Nova", "Great work crew! Sector 7 survey complete. Setting course for home station.", "", "", 0, 0, "COMPLETE");
                            }
                        }
                    }
                }
            }
        }
    }

    void advanceStory() {
        if (isChoiceActive == false) {
            loadStoryNode(nextA);
        }
    }

    void chooseOption(int optionNum) {
        if (isChoiceActive) {
            if (optionNum == 1) {
                loadStoryNode(nextA);
            } else {
                if (optionNum == 2) {
                    loadStoryNode(nextB);
                }
            }
        }
    }

    void onLoad() {
        clearColor = "dark_slate";
        autoRenderWorld2D = false;
        debugOverlayEnabled = false;
        loadStoryNode(0);
    }

    void onUpdate(float dt) {
        elapsed = elapsed + dt;
        if (wasKeyPressed("Space")) {
            advanceStory();
        } else {
            if (wasKeyPressed("Enter")) {
                advanceStory();
            } else {
                if (wasKeyPressed("Digit1")) {
                    chooseOption(1);
                } else {
                    if (wasKeyPressed("Digit2")) {
                        chooseOption(2);
                    }
                }
            }
        }
    }

    void onDraw(zenith::Canvas& canvas, float alpha) {
        float screenW = canvas.mediaWidth();
        float screenH = canvas.mediaHeight();
        if (screenW < 80.0) {
            screenW = 80.0;
        }
        if (screenH < 24.0) {
            screenH = 24.0;
        }
        canvas.drawRect(0.0, 0.0, screenW, screenH, "dark_slate");
        canvas.drawPoint(canvas.pctX(15.0), canvas.pctY(10.0), "sky_blue");
        canvas.drawPoint(canvas.pctX(30.0), canvas.pctY(25.0), "white");
        canvas.drawPoint(canvas.pctX(70.0), canvas.pctY(15.0), "amber");
        canvas.drawPoint(canvas.pctX(85.0), canvas.pctY(35.0), "sky_blue");
        canvas.drawPoint(canvas.pctX(45.0), canvas.pctY(50.0), "emerald");
        canvas.drawPoint(canvas.pctX(60.0), canvas.pctY(30.0), "white");
        canvas.drawRect(0.0, 0.0, screenW, 2.0, "dark_navy");
        canvas.drawFrameRect(0.0, 0.0, screenW, 2.0, "panel_border");
        canvas.drawText("ZENITH GAME ENGINE * NEBULA CRISIS", 2.0, 0.5, "sky_blue");
        float statusX = screenW - 28.0;
        if (statusX < 35.0) {
            statusX = 35.0;
        }
        canvas.drawText(zenith::concat("STATUS: ", statusMessage), statusX, 0.5, "emerald");
        float avatarW = 18.0;
        float avatarH = 8.0;
        if (currentSpeaker == "Captain Nova") {
            canvas.drawRect(3.0, 3.0, avatarW, avatarH, "glass_panel");
            canvas.drawFrameRect(3.0, 3.0, avatarW, avatarH, "sky_blue");
            canvas.drawRect(3.0, 3.0, avatarW, 1.5, "slate");
            canvas.drawText("[ CAPTAIN ]", 5.0, 3.2, "amber");
            canvas.drawText("NOVA", 7.0, 6.0, "white");
        } else {
            if (currentSpeaker == "Lieutenant Ray") {
                canvas.drawRect(3.0, 3.0, avatarW, avatarH, "glass_panel");
                canvas.drawFrameRect(3.0, 3.0, avatarW, avatarH, "emerald");
                canvas.drawRect(3.0, 3.0, avatarW, 1.5, "slate");
                canvas.drawText("[ LIEUTENANT ]", 4.0, 3.2, "sky_blue");
                canvas.drawText("RAY", 7.0, 6.0, "white");
            } else {
                canvas.drawRect(3.0, 3.0, avatarW, avatarH, "glass_panel");
                canvas.drawFrameRect(3.0, 3.0, avatarW, avatarH, "red");
                canvas.drawRect(3.0, 3.0, avatarW, 1.5, "slate");
                canvas.drawText("[ COMPUTER ]", 4.0, 3.2, "red");
                canvas.drawText("AI CORE", 6.0, 6.0, "white");
            }
        }
        if (isChoiceActive) {
            float cardX = 24.0;
            float cardW = screenW - 27.0;
            if (cardW < 40.0) {
                cardW = 40.0;
            }
            canvas.drawRect(cardX, 3.0, cardW, 3.0, "glass_panel");
            canvas.drawFrameRect(cardX, 3.0, cardW, 3.0, "sky_blue");
            canvas.drawText(choiceAText, cardX + 2.0, 4.0, "amber");
            canvas.drawRect(cardX, 7.0, cardW, 3.0, "glass_panel");
            canvas.drawFrameRect(cardX, 7.0, cardW, 3.0, "sky_blue");
            canvas.drawText(choiceBText, cardX + 2.0, 8.0, "amber");
            canvas.drawText("--> Press [1] or [2] to select action <--", cardX + 2.0, 11.0, "emerald");
        }
        float dlgX = 2.0;
        float dlgY = screenH - 10.0;
        float dlgW = screenW - 4.0;
        float dlgH = 9.0;
        if (dlgY < 12.0) {
            dlgY = 12.0;
        }
        canvas.drawRect(dlgX, dlgY, dlgW, dlgH, "dark_navy");
        canvas.drawFrameRect(dlgX, dlgY, dlgW, dlgH, "panel_border");
        canvas.drawRect(dlgX + 2.0, dlgY - 1.0, 22.0, 1.8, "slate");
        canvas.drawFrameRect(dlgX + 2.0, dlgY - 1.0, 22.0, 1.8, "sky_blue");
        canvas.drawText(zenith::concat(" ", zenith::concat(currentSpeaker, " ")), dlgX + 3.0, dlgY - 0.6, "amber");
        canvas.drawTextWrapped(currentText, dlgX + 3.0, dlgY + 1.8, dlgW - 6.0, "white");
        if (isChoiceActive == false) {
            float promptX = dlgX + dlgW - 48.0;
            if (promptX < dlgX + 3.0) {
                promptX = dlgX + 3.0;
            }
            canvas.drawText("Press [SPACE] / [ENTER] to continue >>", promptX, dlgY + dlgH - 1.5, "sky_blue");
        }
    }

    void triggerCallback(std::string name, std::string val = "") {
        if (name == "loadStoryNode") { try { this->loadStoryNode(std::stoi(val)); } catch(...) {} return; }
        if (name == "advanceStory") { this->advanceStory(); return; }
        if (name == "chooseOption") { try { this->chooseOption(std::stoi(val)); } catch(...) {} return; }
        if (name == "onLoad") { this->onLoad(); return; }
        if (name == "onUpdate") { try { this->onUpdate(std::stof(val)); } catch(...) {} return; }
    }

    void triggerEntityCallback(std::string name, zenith::game::EntityId entity) override {
    }

};

int main() {
    // --- Zenith RC+GC Memory Manager: Start background cycle collector ---
    zenith::mem::GcHeap::instance().start_background_gc(5000);

    NebulaNovelScene scene = NebulaNovelScene();
    zenith::runGameLoop(scene);

// --- Zenith RC+GC Memory Manager: Shutdown ---
zenith::mem::GcHeap::instance().stop_background_gc();
zenith::mem::GcHeap::instance().collect(); // Final cycle sweep
#ifdef ZENITH_GC_STATS
std::cout << zenith::mem::gcStatsString() << std::endl;
#endif
}

