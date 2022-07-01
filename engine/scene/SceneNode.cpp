#include "SceneNode.h"

namespace engine {
namespace scene {

uint32_t SceneNode::s_nextId = 1;

SceneNode::SceneNode()
    : m_id(s_nextId++)
    , m_enabled(true)
    , m_dirty(true)
    , m_layer(0)
    , m_sortingOrder(0)
{
}

SceneNode::SceneNode(const std::string& name)
    : m_name(name)
    , m_id(s_nextId++)
    , m_enabled(true)
    , m_dirty(true)
    , m_layer(0)
    , m_sortingOrder(0)
{
}

SceneNode::SceneNode(uint32_t id)
    : m_id(id)
    , m_enabled(true)
    , m_dirty(true)
    , m_layer(0)
    , m_sortingOrder(0)
{
    if (id >= s_nextId) {
        s_nextId = id + 1;
    }
}

SceneNode::SceneNode(const std::string& name, uint32_t id)
    : m_name(name)
    , m_id(id)
    , m_enabled(true)
    , m_dirty(true)
    , m_layer(0)
    , m_sortingOrder(0)
{
    if (id >= s_nextId) {
        s_nextId = id + 1;
    }
}

SceneNode::~SceneNode()
{
}

SceneNode::Ptr SceneNode::addChild(Ptr child)
{
    if (!child) return nullptr;

    if (auto parent = child->m_parent.lock()) {
        parent->removeChild(child);
    }

    child->m_parent = weak_from_this();
    m_children.push_back(child);
    child->markDirty();
    return child;
}

SceneNode::Ptr SceneNode::addChild(const std::string& name)
{
    auto child = std::make_shared<SceneNode>(name);
    return addChild(child);
}

bool SceneNode::removeChild(Ptr child)
{
    if (!child) return false;

    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (*it == child) {
            child->m_parent.reset();
            m_children.erase(it);
            return true;
        }
    }
    return false;
}

bool SceneNode::removeChild(const std::string& name)
{
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if ((*it)->getName() == name) {
            (*it)->m_parent.reset();
            m_children.erase(it);
            return true;
        }
    }
    return false;
}

SceneNode::Ptr SceneNode::getChild(size_t index) const
{
    if (index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

SceneNode::Ptr SceneNode::findChild(const std::string& name) const
{
    for (auto& child : m_children) {
        if (child->getName() == name) {
            return child;
        }
        auto found = child->findChild(name);
        if (found) return found;
    }
    return nullptr;
}

SceneNode::Ptr SceneNode::findChild(uint32_t id) const
{
    for (auto& child : m_children) {
        if (child->getId() == id) {
            return child;
        }
        auto found = child->findChild(id);
        if (found) return found;
    }
    return nullptr;
}

std::string SceneNode::getPath() const
{
    std::string path = m_name.empty() ? std::to_string(m_id) : m_name;
    auto parent = m_parent.lock();
    if (parent) {
        return parent->getPath() + "/" + path;
    }
    return "/" + path;
}

void SceneNode::setParent(Ptr parent)
{
    if (parent) {
        parent->addChild(shared_from_this());
    } else {
        if (auto p = m_parent.lock()) {
            p->removeChild(shared_from_this());
        }
        m_parent.reset();
    }
}

void SceneNode::setPosition(const Vector3& position)
{
    m_localTransform.setPosition(position);
    markDirty();
}

void SceneNode::setRotation(const Quaternion& rotation)
{
    m_localTransform.setRotation(rotation);
    markDirty();
}

void SceneNode::setScale(const Vector3& scale)
{
    m_localTransform.setScale(scale);
    markDirty();
}

Vector3 SceneNode::getPosition() const
{
    return m_localTransform.getPosition();
}

Quaternion SceneNode::getRotation() const
{
    return m_localTransform.getRotation();
}

Vector3 SceneNode::getScale() const
{
    return m_localTransform.getScale();
}

void SceneNode::setLocalTransform(const Transform& transform)
{
    m_localTransform = transform;
    markDirty();
}

void SceneNode::updateWorldTransform(bool force)
{
    if (!m_dirty && !force) return;

    auto parent = m_parent.lock();
    if (parent) {
        m_worldTransform = parent->getWorldTransform() * m_localTransform;
    } else {
        m_worldTransform = m_localTransform;
    }

    m_dirty = false;

    for (auto& child : m_children) {
        child->updateWorldTransform(true);
    }
}

void SceneNode::traversePreOrder(const Visitor& visitor)
{
    visitor(shared_from_this());
    for (auto& child : m_children) {
        child->traversePreOrder(visitor);
    }
}

void SceneNode::traversePostOrder(const Visitor& visitor)
{
    for (auto& child : m_children) {
        child->traversePostOrder(visitor);
    }
    visitor(shared_from_this());
}

void SceneNode::traverseUp(const Visitor& visitor)
{
    visitor(shared_from_this());
    auto parent = m_parent.lock();
    if (parent) {
        parent->traverseUp(visitor);
    }
}

void SceneNode::markDirty()
{
    m_dirty = true;
    for (auto& child : m_children) {
        child->markDirty();
    }
}

} // namespace scene
} // namespace engine
