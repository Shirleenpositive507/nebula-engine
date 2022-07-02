#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "engine/scene/SceneNode.h"
#include "engine/core/EntityManager.h"
#include "engine/core/EventDispatcher.h"
#include "engine/physics/PhysicsWorld.h"
#include "engine/renderer/Camera.h"
#include "engine/core/math/Color.h"
#include "engine/core/math/Vector3.h"

namespace engine {
namespace scene {

class Scene {
public:
    using Ptr = std::shared_ptr<Scene>;

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

    void setAmbientColor(const Color& color) { m_ambientColor = color; }
    const Color& getAmbientColor() const { return m_ambientColor; }
    void setGravity(const Vector3& gravity) { m_gravity = gravity; }
    const Vector3& getGravity() const { return m_gravity; }
    void setBackgroundColor(const Color& color) { m_backgroundColor = color; }
    const Color& getBackgroundColor() const { return m_backgroundColor; }

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

    uint32_t createEntity();
    void destroyEntity(uint32_t entityId);

protected:
    std::string m_name;
    SceneNode::Ptr m_rootNode;
    EntityManager m_entityManager;
    EventDispatcher m_eventDispatcher;
    std::shared_ptr<physics::PhysicsWorld> m_physicsWorld;
    std::shared_ptr<renderer::Camera> m_camera;
    Color m_ambientColor;
    Vector3 m_gravity;
    Color m_backgroundColor;
    std::unordered_map<int, LayerInfo> m_layers;
    int m_nextLayerId;

    void findNodeByEntityRecursive(SceneNode::Ptr node, uint32_t entityId, SceneNode::Ptr& result) const;
};

} // namespace scene
} // namespace engine
