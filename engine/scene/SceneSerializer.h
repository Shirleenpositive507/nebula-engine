#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "engine/scene/Scene.h"
#include "engine/scene/SceneNode.h"
#include "engine/core/math/Transform.h"
#include "engine/core/math/Vector3.h"
#include "engine/core/math/Quaternion.h"
#include "engine/core/math/Color.h"

namespace engine {
namespace scene {

class SceneSerializer {
public:
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

private:
    struct PrefabTemplate {
        std::string name;
        std::string jsonData;
    };

    std::unordered_map<std::string, PrefabTemplate> m_prefabCache;

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
};

} // namespace scene
} // namespace engine
