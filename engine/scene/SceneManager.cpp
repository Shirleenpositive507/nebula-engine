#include "SceneManager.h"
#include <thread>
#include <future>

namespace engine {
namespace scene {

SceneManager& SceneManager::getInstance()
{
    static SceneManager instance;
    return instance;
}

SceneManager::Ptr SceneManager::create()
{
    return std::make_shared<SceneManager>();
}

SceneManager::SceneManager()
    : m_activeScene(nullptr)
{
}

SceneManager::~SceneManager()
{
    m_sceneStack.clear();
    m_scenes.clear();
    m_pooledScenes.clear();
    m_activeScene = nullptr;
}

void SceneManager::addScene(const std::string& name, Scene::Ptr scene)
{
    if (!scene) return;
    m_scenes[name] = scene;
    m_loadProgress[name] = 1.0f;
}

void SceneManager::removeScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        if (it->second == m_activeScene) {
            deactivateScene(it->second);
        }
        for (auto stackIt = m_sceneStack.begin(); stackIt != m_sceneStack.end(); ++stackIt) {
            if (*stackIt == it->second) {
                m_sceneStack.erase(stackIt);
                break;
            }
        }
        m_scenes.erase(it);
    }
    m_loadProgress.erase(name);
}

Scene::Ptr SceneManager::getScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        return it->second;
    }
    return nullptr;
}

bool SceneManager::hasScene(const std::string& name) const
{
    return m_scenes.find(name) != m_scenes.end();
}

bool SceneManager::loadScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) return false;

    loadDependencies(name);
    return setActiveScene(name);
}

bool SceneManager::loadSceneAsync(const std::string& name, std::function<void(bool)> onComplete)
{
    std::lock_guard<std::mutex> lock(m_asyncMutex);
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        if (onComplete) onComplete(false);
        return false;
    }

    if (m_loadingScene == name) return false;
    m_loadingScene = name;
    m_loadProgress[name] = 0.0f;

    std::thread([this, name, onComplete]() {
        loadDependencies(name);

        for (int i = 0; i <= 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            {
                std::lock_guard<std::mutex> lock(m_asyncMutex);
                m_loadProgress[name] = static_cast<float>(i) / 10.0f;
            }
            if (m_progressCallback) {
                m_progressCallback(name, m_loadProgress[name]);
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_asyncMutex);
            m_loadingScene.clear();
        }

        if (onComplete) onComplete(true);
    }).detach();

    return true;
}

float SceneManager::getLoadProgress(const std::string& name) const
{
    auto it = m_loadProgress.find(name);
    if (it != m_loadProgress.end()) {
        return it->second;
    }
    return 0.0f;
}

void SceneManager::setSceneDependencies(const std::string& sceneName, const std::vector<SceneDependency>& deps)
{
    m_sceneDependencies[sceneName] = deps;
}

const std::vector<SceneDependency>& SceneManager::getSceneDependencies(const std::string& sceneName) const
{
    static std::vector<SceneDependency> empty;
    auto it = m_sceneDependencies.find(sceneName);
    if (it != m_sceneDependencies.end()) {
        return it->second;
    }
    return empty;
}

void SceneManager::loadDependencies(const std::string& sceneName)
{
    auto depIt = m_sceneDependencies.find(sceneName);
    if (depIt == m_sceneDependencies.end()) return;

    for (const auto& dep : depIt->second) {
        if (dep.required && !hasScene(dep.sceneName)) {
            auto it = m_pooledScenes.find(dep.sceneName);
            if (it != m_pooledScenes.end()) {
                m_scenes[dep.sceneName] = it->second;
            }
        }
    }
}

bool SceneManager::streamSceneRegion(const std::string& regionName)
{
    for (auto& region : m_streamingRegions) {
        if (region.name == regionName && !region.loaded) {
            region.loaded = true;
            for (const auto& sceneName : region.containedScenes) {
                loadDependencies(sceneName);
            }
            return true;
        }
    }
    return false;
}

bool SceneManager::unloadStreamRegion(const std::string& regionName)
{
    for (auto& region : m_streamingRegions) {
        if (region.name == regionName && region.loaded) {
            region.loaded = false;
            for (const auto& sceneName : region.containedScenes) {
                m_scenes.erase(sceneName);
            }
            return true;
        }
    }
    return false;
}

void SceneManager::updateStreaming(const Vector3& cameraPosition)
{
    for (auto& region : m_streamingRegions) {
        Vector3 diff = cameraPosition - region.center;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        if (dist <= region.radius && !region.loaded) {
            streamSceneRegion(region.name);
        } else if (dist > region.radius * 1.5f && region.loaded) {
            unloadStreamRegion(region.name);
        }
    }
}

void SceneManager::poolScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        m_pooledScenes[name] = it->second;
        m_scenes.erase(it);
    }
}

void SceneManager::unpoolScene(const std::string& name)
{
    auto it = m_pooledScenes.find(name);
    if (it != m_pooledScenes.end()) {
        m_scenes[name] = it->second;
        m_pooledScenes.erase(it);
    }
}

bool SceneManager::isScenePooled(const std::string& name) const
{
    return m_pooledScenes.find(name) != m_pooledScenes.end();
}

Scene::Ptr SceneManager::getPooledScene(const std::string& name) const
{
    auto it = m_pooledScenes.find(name);
    if (it != m_pooledScenes.end()) {
        return it->second;
    }
    return nullptr;
}

bool SceneManager::unloadScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) return false;

    if (it->second == m_activeScene) {
        deactivateScene(it->second);
        m_activeScene = nullptr;
    }

    for (auto stackIt = m_sceneStack.begin(); stackIt != m_sceneStack.end(); ++stackIt) {
        if (*stackIt == it->second) {
            m_sceneStack.erase(stackIt);
            break;
        }
    }

    m_scenes.erase(it);
    m_loadProgress.erase(name);
    return true;
}

bool SceneManager::setActiveScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) return false;

    std::string previousName = m_activeScene ? m_activeScene->getName() : "";

    if (m_activeScene) {
        deactivateScene(m_activeScene);
    }

    m_activeScene = it->second;
    activateScene(m_activeScene);

    if (m_onSceneChanged) {
        m_onSceneChanged(previousName, name);
    }

    return true;
}

void SceneManager::pushScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) return;

    if (m_activeScene) {
        m_activeScene->onPause();
        m_sceneStack.push_back(m_activeScene);
    }

    m_activeScene = it->second;
    activateScene(m_activeScene);
}

void SceneManager::popScene()
{
    if (m_sceneStack.empty()) return;

    if (m_activeScene) {
        deactivateScene(m_activeScene);
    }

    m_activeScene = m_sceneStack.back();
    m_sceneStack.pop_back();
    m_activeScene->onResume();
}

Scene::Ptr SceneManager::getTopScene() const
{
    if (!m_sceneStack.empty()) {
        return m_sceneStack.back();
    }
    return m_activeScene;
}

void SceneManager::update(float dt)
{
    if (m_activeScene) {
        m_activeScene->update(dt);
    }
}

void SceneManager::render(float dt)
{
    for (auto& scene : m_sceneStack) {
        scene->render(dt);
    }
    if (m_activeScene) {
        m_activeScene->render(dt);
    }
}

void SceneManager::activateScene(Scene::Ptr scene)
{
    if (scene) {
        scene->onEnter();
    }
}

void SceneManager::deactivateScene(Scene::Ptr scene)
{
    if (scene) {
        scene->onExit();
    }
}

} // namespace scene
} // namespace engine
