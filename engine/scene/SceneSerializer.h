#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <map>
#include <set>

#include "engine/scene/Scene.h"
#include "engine/scene/SceneNode.h"
#include "engine/core/math/Transform.h"
#include "engine/core/math/Vector3.h"
#include "engine/core/math/Quaternion.h"
#include "engine/core/math/Color.h"

namespace engine {
namespace scene {

struct BinarySceneHeader {
    uint32_t magic;
    uint32_t versionMajor;
    uint32_t versionMinor;
    uint32_t sceneNameLength;
    uint64_t nodeCount;
    uint64_t dataSize;
    uint64_t thumbnailOffset;
    uint64_t thumbnailSize;
};

struct SceneDependencyInfo {
    std::string sceneName;
    std::string filePath;
    bool required;
};

struct ThumbnailData {
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    std::vector<uint8_t> pixels;
};

struct PrefabOverride {
    std::string propertyPath;
    std::string originalValue;
    std::string overrideValue;

    PrefabOverride() = default;
    PrefabOverride(const std::string& path, const std::string& orig, const std::string& over)
        : propertyPath(path), originalValue(orig), overrideValue(over) {}
};

struct PrefabVariant {
    std::string prefabPath;
    std::vector<PrefabOverride> overrides;

    PrefabVariant() = default;
    explicit PrefabVariant(const std::string& path) : prefabPath(path) {}
};

struct SceneDiffEntry {
    enum class ChangeType { Added, Removed, Modified };

    ChangeType type;
    std::string nodePath;
    std::string propertyName;
    std::string oldValue;
    std::string newValue;

    SceneDiffEntry() = default;
    SceneDiffEntry(ChangeType t, const std::string& path)
        : type(t), nodePath(path) {}
    SceneDiffEntry(ChangeType t, const std::string& path, const std::string& prop,
                   const std::string& oldVal, const std::string& newVal)
        : type(t), nodePath(path), propertyName(prop), oldValue(oldVal), newValue(newVal) {}
};

class SceneSerializer {
public:
    static constexpr uint32_t BINARY_MAGIC = 0x4E42494E;
    static constexpr uint32_t CURRENT_VERSION_MAJOR = 1;
    static constexpr uint32_t CURRENT_VERSION_MINOR = 0;

    SceneSerializer();
    ~SceneSerializer();

    SceneSerializer(const SceneSerializer&) = delete;
    SceneSerializer& operator=(const SceneSerializer&) = delete;
    SceneSerializer(SceneSerializer&&) = default;
    SceneSerializer& operator=(SceneSerializer&&) = default;

    bool serialize(Scene::Ptr scene, const std::string& path);
    Scene::Ptr deserialize(const std::string& path);

    bool serializeBinary(Scene::Ptr scene, const std::string& path);
    Scene::Ptr deserializeBinary(const std::string& path);

    std::string serializeToJson(Scene::Ptr scene);
    Scene::Ptr deserializeFromJson(const std::string& json);

    std::string serializeEntity(SceneNode::Ptr node);
    SceneNode::Ptr deserializeEntity(const std::string& json, Scene::Ptr scene);

    SceneNode::Ptr loadPrefab(const std::string& path);
    SceneNode::Ptr instantiatePrefab(const std::string& path, Scene::Ptr scene);

    SceneNode::Ptr instantiatePrefabVariant(const PrefabVariant& variant, Scene::Ptr scene);
    PrefabVariant createPrefabVariant(const std::string& prefabPath,
                                       const std::vector<PrefabOverride>& overrides);

    bool mergeScenes(const std::string& basePath, const std::string& overlayPath,
                     const std::string& outputPath);
    Scene::Ptr mergeScenes(Scene::Ptr baseScene, Scene::Ptr overlayScene);

    std::vector<SceneDiffEntry> diffScenes(Scene::Ptr oldScene, Scene::Ptr newScene);
    std::vector<SceneDiffEntry> diffScenes(const std::string& oldPath, const std::string& newPath);

    std::vector<SceneDependencyInfo> getSceneDependencies(const std::string& path) const;
    void setSceneDependencies(const std::string& path, const std::vector<SceneDependencyInfo>& deps);

    bool generateThumbnail(const std::string& scenePath, const std::string& outputPath,
                           uint32_t width, uint32_t height);
    ThumbnailData loadThumbnail(const std::string& scenePath) const;

    bool migrateSceneFormat(const std::string& path, uint32_t fromMajor, uint32_t fromMinor);
    uint32_t getSerializedVersion(const std::string& path) const;

private:
    struct PrefabTemplate {
        std::string name;
        std::string jsonData;
    };

    std::unordered_map<std::string, PrefabTemplate> m_prefabCache;
    std::unordered_map<std::string, std::vector<SceneDependencyInfo>> m_dependencyCache;

    std::string serializeNodeRecursive(SceneNode::Ptr node, int indent);
    SceneNode::Ptr deserializeNodeRecursive(const std::string& json, Scene::Ptr scene);
    void serializeTransform(const Transform& t, std::string& out);
    Transform deserializeTransform(const std::string& json);

    std::string serializeVector3(const Vector3& v) const;
    Vector3 deserializeVector3(const std::string& str) const;
    std::string serializeQuaternion(const Quaternion& q) const;
    Quaternion deserializeQuaternion(const std::string& str) const;
    std::string serializeColor(const Color& c) const;
    Color deserializeColor(const std::string& str) const;

    bool writeBinaryHeader(std::ofstream& file, const BinarySceneHeader& header) const;
    BinarySceneHeader readBinaryHeader(std::ifstream& file) const;

    std::string applyPrefabOverride(const std::string& json, const PrefabOverride& override) const;
};

} // namespace scene
} // namespace engine
