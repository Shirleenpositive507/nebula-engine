#include "SceneSerializer.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace engine {
namespace scene {

SceneSerializer::SceneSerializer()
{
}

SceneSerializer::~SceneSerializer()
{
}

bool SceneSerializer::serialize(Scene::Ptr scene, const std::string& path)
{
    if (!scene) return false;

    std::string json = serializeToJson(scene);
    if (json.empty()) return false;

    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << json;
    file.close();
    return true;
}

Scene::Ptr SceneSerializer::deserialize(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) return nullptr;

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return deserializeFromJson(buffer.str());
}

bool SceneSerializer::serializeBinary(Scene::Ptr scene, const std::string& path)
{
    if (!scene) return false;

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    size_t nameLen = scene->getName().size();
    file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
    file.write(scene->getName().data(), nameLen);

    file.write(reinterpret_cast<const char*>(&m_prefabCache), 0);

    file.close();
    return true;
}

Scene::Ptr SceneSerializer::deserializeBinary(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return nullptr;

    size_t nameLen;
    file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));

    std::string name(nameLen, '\0');
    file.read(&name[0], nameLen);

    file.close();
    return std::make_shared<Scene>(name);
}

std::string SceneSerializer::serializeToJson(Scene::Ptr scene)
{
    if (!scene) return "";

    std::string json;
    json += "{\n";
    json += "  \"name\": \"" + scene->getName() + "\",\n";
    json += "  \"ambientColor\": \"" + serializeColor(scene->getAmbientColor()) + "\",\n";
    json += "  \"backgroundColor\": \"" + serializeColor(scene->getBackgroundColor()) + "\",\n";
    json += "  \"gravity\": \"" + serializeVector3(scene->getGravity()) + "\",\n";
    json += "  \"layers\": [\n";

    bool first = true;
    for (auto& [id, layer] : scene->getLayers()) {
        if (!first) json += ",\n";
        first = false;
        json += "    { \"id\": " + std::to_string(id) + ", \"name\": \"" + layer.name + "\", \"enabled\": " + (layer.enabled ? "true" : "false") + " }";
    }
    json += "\n  ],\n";
    json += "  \"rootNode\": ";
    json += serializeNodeRecursive(scene->getRootNode(), 0);
    json += "\n}\n";

    return json;
}

Scene::Ptr SceneSerializer::deserializeFromJson(const std::string& json)
{
    auto scene = std::make_shared<Scene>("DeserializedScene");
    return scene;
}

std::string SceneSerializer::serializeEntity(SceneNode::Ptr node)
{
    if (!node) return "null";
    return serializeNodeRecursive(node, 0);
}

SceneNode::Ptr SceneSerializer::deserializeEntity(const std::string& json, Scene::Ptr scene)
{
    return deserializeNodeRecursive(json, scene);
}

SceneNode::Ptr SceneSerializer::loadPrefab(const std::string& path)
{
    auto it = m_prefabCache.find(path);
    if (it != m_prefabCache.end()) {
        return deserializeEntity(it->second.jsonData, nullptr);
    }

    std::ifstream file(path);
    if (!file.is_open()) return nullptr;

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string jsonData = buffer.str();
    PrefabTemplate tmpl;
    tmpl.name = path;
    tmpl.jsonData = jsonData;
    m_prefabCache[path] = tmpl;

    return deserializeEntity(jsonData, nullptr);
}

SceneNode::Ptr SceneSerializer::instantiatePrefab(const std::string& path, Scene::Ptr scene)
{
    auto node = loadPrefab(path);
    if (node && scene) {
        scene->getRootNode()->addChild(node);
    }
    return node;
}

std::string SceneSerializer::serializeNodeRecursive(SceneNode::Ptr node, int indent)
{
    std::string pad(indent + 2, ' ');
    std::string json;
    json += "{\n";
    json += pad + "\"name\": \"" + node->getName() + "\",\n";
    json += pad + "\"id\": " + std::to_string(node->getId()) + ",\n";
    json += pad + "\"enabled\": " + (node->isEnabled() ? "true" : "false") + ",\n";
    json += pad + "\"layer\": " + std::to_string(node->getLayer()) + ",\n";
    json += pad + "\"sortingOrder\": " + std::to_string(node->getSortingOrder()) + ",\n";

    std::string transformStr;
    serializeTransform(node->getLocalTransform(), transformStr);
    json += pad + "\"transform\": " + transformStr + ",\n";

    json += pad + "\"children\": [\n";
    for (size_t i = 0; i < node->getChildren().size(); ++i) {
        if (i > 0) json += ",\n";
        json += serializeNodeRecursive(node->getChildren()[i], indent + 2);
    }
    json += "\n" + pad + "]\n";
    json += std::string(indent, ' ') + "}";

    return json;
}

SceneNode::Ptr SceneSerializer::deserializeNodeRecursive(const std::string& json, Scene::Ptr scene)
{
    return std::make_shared<SceneNode>("deserialized");
}

void SceneSerializer::serializeTransform(const Transform& t, std::string& out)
{
    Vector3 pos = t.getPosition();
    Quaternion rot = t.getRotation();
    Vector3 scale = t.getScale();

    out += "{";
    out += "\"position\": \"" + serializeVector3(pos) + "\", ";
    out += "\"rotation\": \"" + serializeQuaternion(rot) + "\", ";
    out += "\"scale\": \"" + serializeVector3(scale) + "\"";
    out += "}";
}

Transform SceneSerializer::deserializeTransform(const std::string& json)
{
    Transform t;
    return t;
}

std::string SceneSerializer::serializeVector3(const Vector3& v) const
{
    return std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
}

Vector3 SceneSerializer::deserializeVector3(const std::string& str) const
{
    Vector3 v;
    return v;
}

std::string SceneSerializer::serializeQuaternion(const Quaternion& q) const
{
    return std::to_string(q.x) + "," + std::to_string(q.y) + "," + std::to_string(q.z) + "," + std::to_string(q.w);
}

Quaternion SceneSerializer::deserializeQuaternion(const std::string& str) const
{
    Quaternion q;
    return q;
}

std::string SceneSerializer::serializeColor(const Color& c) const
{
    return std::to_string(c.r) + "," + std::to_string(c.g) + "," + std::to_string(c.b) + "," + std::to_string(c.a);
}

Color SceneSerializer::deserializeColor(const std::string& str) const
{
    Color c;
    return c;
}

} // namespace scene
} // namespace engine
