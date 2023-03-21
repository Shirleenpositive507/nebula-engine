#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

#include "engine/core/math/Transform.h"
#include "engine/core/math/Vector3.h"
#include "engine/core/math/Quaternion.h"
#include "engine/core/math/BoundingBox.h"

namespace engine {
namespace scene {

enum class SceneNodeFlag {
    Static = 1 << 0,
    Dynamic = 1 << 1,
    Occluder = 1 << 2,
    Occludee = 1 << 3,
    CastShadows = 1 << 4,
    ReceiveShadows = 1 << 5
};

struct NodeSortKey {
    int layer;
    uint64_t materialId;
    float distance;

    bool operator<(const NodeSortKey& other) const {
        if (layer != other.layer) return layer < other.layer;
        if (materialId != other.materialId) return materialId < other.materialId;
        return distance < other.distance;
    }
};

class SceneNode : public std::enable_shared_from_this<SceneNode> {
public:
    using Ptr = std::shared_ptr<SceneNode>;
    using WeakPtr = std::weak_ptr<SceneNode>;
    using Visitor = std::function<void(Ptr)>;

    SceneNode();
    explicit SceneNode(const std::string& name);
    explicit SceneNode(uint32_t id);
    SceneNode(const std::string& name, uint32_t id);
    ~SceneNode();

    SceneNode(const SceneNode&) = delete;
    SceneNode& operator=(const SceneNode&) = delete;
    SceneNode(SceneNode&&) = default;
    SceneNode& operator=(SceneNode&&) = default;

    WeakPtr getParent() const { return m_parent; }
    void setParent(Ptr parent);
    const std::vector<Ptr>& getChildren() const { return m_children; }
    std::vector<Ptr>& getChildren() { return m_children; }

    Ptr addChild(Ptr child);
    Ptr addChild(const std::string& name);
    bool removeChild(Ptr child);
    bool removeChild(const std::string& name);
    Ptr getChild(size_t index) const;
    Ptr findChild(const std::string& name) const;
    Ptr findChild(uint32_t id) const;
    std::string getPath() const;

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    uint32_t getId() const { return m_id; }
    void setId(uint32_t id) { m_id = id; }
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    int getLayer() const { return m_layer; }
    void setLayer(int layer) { m_layer = layer; }
    int getSortingOrder() const { return m_sortingOrder; }
    void setSortingOrder(int order) { m_sortingOrder = order; }

    void setNodeFlags(uint32_t flags) { m_nodeFlags = flags; }
    uint32_t getNodeFlags() const { return m_nodeFlags; }
    void setNodeFlag(SceneNodeFlag flag) { m_nodeFlags |= static_cast<uint32_t>(flag); }
    void clearNodeFlag(SceneNodeFlag flag) { m_nodeFlags &= ~static_cast<uint32_t>(flag); }
    bool hasNodeFlag(SceneNodeFlag flag) const { return (m_nodeFlags & static_cast<uint32_t>(flag)) != 0; }

    void setMaterialId(uint64_t id) { m_materialId = id; }
    uint64_t getMaterialId() const { return m_materialId; }

    void setPosition(const Vector3& position);
    void setRotation(const Quaternion& rotation);
    void setScale(const Vector3& scale);
    Vector3 getPosition() const;
    Quaternion getRotation() const;
    Vector3 getScale() const;

    Transform& getLocalTransform() { return m_localTransform; }
    const Transform& getLocalTransform() const { return m_localTransform; }
    void setLocalTransform(const Transform& transform);
    const Transform& getWorldTransform() const { return m_worldTransform; }
    void updateWorldTransform(bool force = false);

    BoundingBox getBoundingBox() const { return m_boundingBox; }
    void setBoundingBox(const BoundingBox& box) { m_boundingBox = box; }
    void calculateBoundingBox();
    BoundingBox getWorldBoundingBox() const;

    bool isVisible(const BoundingBox& frustum) const;
    bool isVisible(const Vector3& cameraPosition, float viewDistance) const;

    NodeSortKey getSortKey() const;
    static void sortByLayer(std::vector<Ptr>& nodes);
    static void sortByMaterial(std::vector<Ptr>& nodes);

    template<typename T>
    std::shared_ptr<T> attachComponent(std::shared_ptr<T> component) {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        component->setOwner(shared_from_this());
        m_components.push_back(component);
        return component;
    }

    template<typename T>
    bool detachComponent() {
        for (auto it = m_components.begin(); it != m_components.end(); ++it) {
            if (std::dynamic_pointer_cast<T>(*it)) {
                m_components.erase(it);
                return true;
            }
        }
        return false;
    }

    template<typename T>
    std::shared_ptr<T> getComponent() const {
        for (auto& comp : m_components) {
            if (auto typed = std::dynamic_pointer_cast<T>(comp)) {
                return typed;
            }
        }
        return nullptr;
    }

    const std::vector<std::shared_ptr<Component>>& getComponents() const { return m_components; }

    void traversePreOrder(const Visitor& visitor);
    void traversePostOrder(const Visitor& visitor);
    void traverseUp(const Visitor& visitor);

    void markDirty();

private:
    WeakPtr m_parent;
    std::vector<Ptr> m_children;
    Transform m_localTransform;
    Transform m_worldTransform;
    BoundingBox m_boundingBox;
    std::string m_name;
    uint32_t m_id;
    uint32_t m_nodeFlags;
    uint64_t m_materialId;
    bool m_enabled;
    bool m_dirty;
    int m_layer;
    int m_sortingOrder;
    std::vector<std::shared_ptr<Component>> m_components;

    static uint32_t s_nextId;
};

} // namespace scene
} // namespace engine
