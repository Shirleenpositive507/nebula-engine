#include "SceneManager.h"

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
    m_activeScene = nullptr;
}

void SceneManager::addScene(const std::string& name, Scene::Ptr scene)
{
    if (!scene) return;
    m_scenes[name] = scene;
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

    return setActiveScene(name);
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
