#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>

#include "engine/scene/SceneNode.h"
#include "engine/core/EntityManager.h"
#include "engine/core/EventDispatcher.h"
#include "engine/physics/PhysicsWorld.h"
#include "engine/renderer/Camera.h"
#include "engine/core/math/Color.h"
#include "engine/core/math/Vector3.h"

namespace engine {
namespace scene {

struct FogSettings {
    bool enabled;
    Color color;
    float density;
    float startDistance;
    float endDistance;

    FogSettings()
        : enabled(false)
        , color(0.5f, 0.5f, 0.5f, 1.0f)
        , density(0.01f)
        , startDistance(10.0f)
        , endDistance(100.0f) {}
};

struct RenderSettings {
    Color ambientColor;
    Color backgroundColor;
    FogSettings fog;
    bool enableShadows;
    bool enablePostProcessing;

    RenderSettings()
        : ambientColor(0.2f, 0.2f, 0.2f, 1.0f)
        , backgroundColor(0.0f, 0.0f, 0.0f, 1.0f)
        , enableShadows(true)
        , enablePostProcessing(false) {}
};

struct LightingSetup {
    std::vector<Vector3> directionalLights;
    std::vector<Vector3> pointLights;
    std::vector<Color> lightColors;
    float globalIllumination;
    float ambientIntensity;

    LightingSetup()
        : globalIllumination(0.3f)
        , ambientIntensity(1.0f) {}
};

struct PreloadTask {
    std::string resourcePath;
    std::function<bool()> loadFunction;
    std::function<void(bool)> onComplete;
    bool completed;
    bool success;

    PreloadTask() : completed(false), success(false) {}
};

class Scene {
public:
    using Ptr = std::shared_ptr<Scene>;
    using PreloadCallback = std::function<void(float)>;

    explicit Scene(const std::string& name = "UntitledScene");
    virtual ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    SceneNode::Ptr getRootNode() { return m_rootNode; }
    SceneNode::Ptr findNode(const std::string& name) const;
    SceneNode::Ptr findNodeByEntity(uint32_t entityId) const;

    EntityManager& getEntityManager() { return m_entityManager; }
    EventDispatcher& getEventDispatcher() { return m_eventDispatcher; }

    void setPhysicsWorld(std::shared_ptr<physics::PhysicsWorld> world) { m_physicsWorld = world; }
    std::shared_ptr<physics::PhysicsWorld> getPhysicsWorld() { return m_physicsWorld; }

    void setCamera(std::shared_ptr<renderer::Camera> camera) { m_camera = camera; }
    std::shared_ptr<renderer::Camera> getCamera() { return m_camera; }

    void setAmbientColor(const Color& color) { m_renderSettings.ambientColor = color; }
    const Color& getAmbientColor() const { return m_renderSettings.ambientColor; }
    void setGravity(const Vector3& gravity) { m_gravity = gravity; }
    const Vector3& getGravity() const { return m_gravity; }
    void setBackgroundColor(const Color& color) { m_renderSettings.backgroundColor = color; }
    const Color& getBackgroundColor() const { return m_renderSettings.backgroundColor; }

    RenderSettings& getRenderSettings() { return m_renderSettings; }
    const RenderSettings& getRenderSettings() const { return m_renderSettings; }

    FogSettings& getFogSettings() { return m_renderSettings.fog; }
    const FogSettings& getFogSettings() const { return m_renderSettings.fog; }

    LightingSetup& getLightingSetup() { return m_lighting; }
    const LightingSetup& getLightingSetup() const { return m_lighting; }

    void addDirectionalLight(const Vector3& direction, const Color& color);
    void addPointLight(const Vector3& position, const Color& color);

    struct LayerInfo {
        std::string name;
        bool enabled;
    };

    int addLayer(const std::string& name);
    bool removeLayer(int layer);
    void setLayerEnabled(int layer, bool enabled);
    bool isLayerEnabled(int layer) const;
    const LayerInfo* getLayer(int layer) const;
    size_t getLayerCount() const { return m_layers.size(); }
    const std::unordered_map<int, LayerInfo>& getLayers() const { return m_layers; }

    virtual void onEnter();
    virtual void onExit();
    virtual void onPause();
    virtual void onResume();
    virtual void update(float dt);
    virtual void render(float dt);

    void addPreloadTask(const std::string& path, std::function<bool()> loader,
                        std::function<void(bool)> onComplete = nullptr);
    void startPreload(PreloadCallback progressCallback = nullptr);
    bool isPreloading() const { return m_preloading; }
    float getPreloadProgress() const;
    bool isPreloadComplete() const { return m_preloadComplete; }

    void onEvent(const std::string& eventName, std::function<void(const void*)> handler);
    void dispatchEvent(const std::string& eventName, const void* data = nullptr);

    uint32_t createEntity();
    void destroyEntity(uint32_t entityId);

protected:
    std::string m_name;
    SceneNode::Ptr m_rootNode;
    EntityManager m_entityManager;
    EventDispatcher m_eventDispatcher;
    std::shared_ptr<physics::PhysicsWorld> m_physicsWorld;
    std::shared_ptr<renderer::Camera> m_camera;
    Vector3 m_gravity;
    std::unordered_map<int, LayerInfo> m_layers;
    int m_nextLayerId;

    RenderSettings m_renderSettings;
    LightingSetup m_lighting;

    std::vector<PreloadTask> m_preloadTasks;
    std::atomic<bool> m_preloading;
    std::atomic<bool> m_preloadComplete;
    std::atomic<float> m_preloadProgress;
    std::thread m_preloadThread;
    std::mutex m_preloadMutex;
    PreloadCallback m_preloadCallback;

    std::unordered_map<std::string, std::vector<std::function<void(const void*)>>> m_eventHandlers;

    void findNodeByEntityRecursive(SceneNode::Ptr node, uint32_t entityId, SceneNode::Ptr& result) const;
    void preloadThreadFunc();
};

} // namespace scene
} // namespace engine
