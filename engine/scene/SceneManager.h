#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "engine/scene/Scene.h"

namespace engine {
namespace scene {

class SceneManager {
public:
    using Ptr = std::shared_ptr<SceneManager>;

    static SceneManager& getInstance();
    static Ptr create();

    SceneManager();
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = default;
    SceneManager& operator=(SceneManager&&) = default;

    void addScene(const std::string& name, Scene::Ptr scene);
    void removeScene(const std::string& name);
    Scene::Ptr getScene(const std::string& name);
    Scene::Ptr getActiveScene() { return m_activeScene; }
    bool hasScene(const std::string& name) const;

    bool loadScene(const std::string& name);
    bool unloadScene(const std::string& name);
    bool setActiveScene(const std::string& name);
    size_t getSceneCount() const { return m_scenes.size(); }

    void pushScene(const std::string& name);
    void popScene();
    size_t getStackDepth() const { return m_sceneStack.size(); }
    Scene::Ptr getTopScene() const;

    void update(float dt);
    void render(float dt);

    void setOnSceneChanged(std::function<void(const std::string&, const std::string&)> callback) {
        m_onSceneChanged = callback;
    }

private:
    std::unordered_map<std::string, Scene::Ptr> m_scenes;
    Scene::Ptr m_activeScene;
    std::vector<Scene::Ptr> m_sceneStack;
    std::function<void(const std::string&, const std::string&)> m_onSceneChanged;

    void activateScene(Scene::Ptr scene);
    void deactivateScene(Scene::Ptr scene);
};

} // namespace scene
} // namespace engine
