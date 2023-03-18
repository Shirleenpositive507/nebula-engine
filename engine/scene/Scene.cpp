#include "Scene.h"

namespace engine {
namespace scene {

Scene::Scene(const std::string& name)
    : m_name(name)
    , m_rootNode(std::make_shared<SceneNode>("root"))
    , m_gravity(0.0f, -9.81f, 0.0f)
    , m_nextLayerId(0)
    , m_preloading(false)
    , m_preloadComplete(false)
    , m_preloadProgress(0.0f)
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
        auto parent = node->getParent().lock();
        if (parent) {
            parent->removeChild(node);
        }
    }
    m_entityManager.destroy(entityId);
}

void Scene::addDirectionalLight(const Vector3& direction, const Color& color)
{
    m_lighting.directionalLights.push_back(direction);
    m_lighting.lightColors.push_back(color);
}

void Scene::addPointLight(const Vector3& position, const Color& color)
{
    m_lighting.pointLights.push_back(position);
    m_lighting.lightColors.push_back(color);
}

void Scene::addPreloadTask(const std::string& path, std::function<bool()> loader,
                           std::function<void(bool)> onComplete)
{
    PreloadTask task;
    task.resourcePath = path;
    task.loadFunction = loader;
    task.onComplete = onComplete;
    task.completed = false;
    task.success = false;
    m_preloadTasks.push_back(task);
}

void Scene::startPreload(PreloadCallback progressCallback)
{
    if (m_preloading) return;

    m_preloadCallback = progressCallback;
    m_preloading = true;
    m_preloadComplete = false;
    m_preloadProgress = 0.0f;

    m_preloadThread = std::thread([this]() {
        preloadThreadFunc();
    });
    m_preloadThread.detach();
}

float Scene::getPreloadProgress() const
{
    return m_preloadProgress.load();
}

void Scene::onEvent(const std::string& eventName, std::function<void(const void*)> handler)
{
    m_eventHandlers[eventName].push_back(handler);
}

void Scene::dispatchEvent(const std::string& eventName, const void* data)
{
    auto it = m_eventHandlers.find(eventName);
    if (it != m_eventHandlers.end()) {
        for (auto& handler : it->second) {
            handler(data);
        }
    }
}

void Scene::preloadThreadFunc()
{
    size_t total = m_preloadTasks.size();
    if (total == 0) {
        m_preloading = false;
        m_preloadComplete = true;
        m_preloadProgress = 1.0f;
        return;
    }

    for (size_t i = 0; i < total; ++i) {
        {
            std::lock_guard<std::mutex> lock(m_preloadMutex);
            if (m_preloadTasks[i].loadFunction) {
                m_preloadTasks[i].success = m_preloadTasks[i].loadFunction();
            }
            m_preloadTasks[i].completed = true;
        }

        m_preloadProgress = static_cast<float>(i + 1) / static_cast<float>(total);

        if (m_preloadCallback) {
            m_preloadCallback(m_preloadProgress);
        }
    }

    m_preloading = false;
    m_preloadComplete = true;
}

} // namespace scene
} // namespace engine
