#include "Scene.h"

namespace engine {
namespace scene {

Scene::Scene(const std::string& name)
    : m_name(name)
    , m_rootNode(std::make_shared<SceneNode>("root"))
    , m_ambientColor(0.2f, 0.2f, 0.2f, 1.0f)
    , m_gravity(0.0f, -9.81f, 0.0f)
    , m_backgroundColor(0.0f, 0.0f, 0.0f, 1.0f)
    , m_nextLayerId(0)
{
    addLayer("default");
}

Scene::~Scene()
{
    onExit();
}

SceneNode::Ptr Scene::findNode(const std::string& name) const
{
    return m_rootNode->findChild(name);
}

SceneNode::Ptr Scene::findNodeByEntity(uint32_t entityId) const
{
    SceneNode::Ptr result = nullptr;
    findNodeByEntityRecursive(m_rootNode, entityId, result);
    return result;
}

void Scene::findNodeByEntityRecursive(SceneNode::Ptr node, uint32_t entityId, SceneNode::Ptr& result) const
{
    if (result) return;
    if (node->getId() == entityId) {
        result = node;
        return;
    }
    for (auto& child : node->getChildren()) {
        findNodeByEntityRecursive(child, entityId, result);
        if (result) return;
    }
}

int Scene::addLayer(const std::string& name)
{
    int id = m_nextLayerId++;
    m_layers[id] = { name, true };
    return id;
}

bool Scene::removeLayer(int layer)
{
    auto it = m_layers.find(layer);
    if (it != m_layers.end()) {
        m_layers.erase(it);
        return true;
    }
    return false;
}

void Scene::setLayerEnabled(int layer, bool enabled)
{
    auto it = m_layers.find(layer);
    if (it != m_layers.end()) {
        it->second.enabled = enabled;
    }
}

bool Scene::isLayerEnabled(int layer) const
{
    auto it = m_layers.find(layer);
    if (it != m_layers.end()) {
        return it->second.enabled;
    }
    return false;
}

const Scene::LayerInfo* Scene::getLayer(int layer) const
{
    auto it = m_layers.find(layer);
    if (it != m_layers.end()) {
        return &it->second;
    }
    return nullptr;
}

void Scene::onEnter()
{
}

void Scene::onExit()
{
}

void Scene::onPause()
{
}

void Scene::onResume()
{
}

void Scene::update(float dt)
{
    if (m_physicsWorld) {
        m_physicsWorld->update(dt);
    }
    m_rootNode->updateWorldTransform();
}

void Scene::render(float dt)
{
    m_eventDispatcher.dispatch("render", dt);
}

uint32_t Scene::createEntity()
{
    uint32_t id = m_entityManager.create();
    return id;
}

void Scene::destroyEntity(uint32_t entityId)
{
    auto node = findNodeByEntity(entityId);
    if (node) {
        node->getParent().lock()->removeChild(node);
    }
    m_entityManager.destroy(entityId);
}

} // namespace scene
} // namespace engine
