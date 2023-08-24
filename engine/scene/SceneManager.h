#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "engine/scene/Scene.h"
#include "engine/core/math/Vector3.h"

namespace engine {
namespace scene {

struct SceneDependency {
    std::string sceneName;
    bool required;
};

struct StreamingRegion {
    std::string name;
    Vector3 center;
    float radius;
    bool loaded;
    std::vector<std::string> containedScenes;
};

class SceneManager {
public:
    using Ptr = std::shared_ptr<SceneManager>;
    using ProgressCallback = std::function<void(const std::string&, float)>;

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

    void setProgressCallback(ProgressCallback callback) { m_progressCallback = callback; }

    bool loadSceneAsync(const std::string& name, std::function<void(bool)> onComplete = nullptr);
    float getLoadProgress(const std::string& name) const;

    void setSceneDependencies(const std::string& sceneName, const std::vector<SceneDependency>& deps);
    const std::vector<SceneDependency>& getSceneDependencies(const std::string& sceneName) const;

    bool streamSceneRegion(const std::string& regionName);
    bool unloadStreamRegion(const std::string& regionName);
    void updateStreaming(const Vector3& cameraPosition);

    void poolScene(const std::string& name);
    void unpoolScene(const std::string& name);
    bool isScenePooled(const std::string& name) const;
    Scene::Ptr getPooledScene(const std::string& name) const;

private:
    std::unordered_map<std::string, Scene::Ptr> m_scenes;
    Scene::Ptr m_activeScene;
    std::vector<Scene::Ptr> m_sceneStack;
    std::function<void(const std::string&, const std::string&)> m_onSceneChanged;
    ProgressCallback m_progressCallback;

    std::unordered_map<std::string, std::vector<SceneDependency>> m_sceneDependencies;
    std::unordered_map<std::string, float> m_loadProgress;

    std::vector<StreamingRegion> m_streamingRegions;
    std::unordered_map<std::string, Scene::Ptr> m_pooledScenes;
    mutable std::mutex m_asyncMutex;
    std::string m_loadingScene;

    void activateScene(Scene::Ptr scene);
    void deactivateScene(Scene::Ptr scene);
    void loadDependencies(const std::string& sceneName);
};

} // namespace scene
} // namespace engine
