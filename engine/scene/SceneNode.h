#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

#include "engine/core/math/Transform.h"
#include "engine/core/math/Vector3.h"
#include "engine/core/math/Quaternion.h"

namespace engine {
namespace scene {

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
    std::string m_name;
    uint32_t m_id;
    bool m_enabled;
    bool m_dirty;
    int m_layer;
    int m_sortingOrder;
    std::vector<std::shared_ptr<Component>> m_components;

    static uint32_t s_nextId;
};

} // namespace scene
} // namespace engine
