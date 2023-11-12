#pragma once

#include "RigidBody.h"
#include "Collider.h"
#include "Joint.h"
#include "PhysicsWorld.h"
#include "../graphics/BatchRenderer.h"
#include <SFML/Graphics/Color.hpp>
#include <vector>

namespace nebula {

class PhysicsDebug {
public:
    PhysicsDebug();
    ~PhysicsDebug();

    void setBatchRenderer(graphics::BatchRenderer* renderer) { m_batchRenderer = renderer; }
    graphics::BatchRenderer* getBatchRenderer() const { return m_batchRenderer; }

    void drawWorld(PhysicsWorld& world);
    void drawCollider(RigidBody* body, const sf::Color& color);
    void drawContactPoints(const std::vector<CollisionInfo>& collisions);
    void drawJoints(const std::vector<Joint*>& joints);
    void drawBroadphaseCells(PhysicsWorld& world);
    void drawAABB(RigidBody* body, const sf::Color& color);

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    void setDrawColliders(bool draw) { m_drawColliders = draw; }
    void setDrawContacts(bool draw) { m_drawContacts = draw; }
    void setDrawJoints(bool draw) { m_drawJoints = draw; }
    void setDrawBroadphase(bool draw) { m_drawBroadphase = draw; }
    void setDrawAABBs(bool draw) { m_drawAABBs = draw; }

    static sf::Color getColorForBodyType(RigidBody* body);

private:
    graphics::BatchRenderer* m_batchRenderer;
    bool m_enabled;
    bool m_drawColliders;
    bool m_drawContacts;
    bool m_drawJoints;
    bool m_drawBroadphase;
    bool m_drawAABBs;

    static const sf::Color COLOR_STATIC;
    static const sf::Color COLOR_DYNAMIC;
    static const sf::Color COLOR_KINEMATIC;
    static const sf::Color COLOR_CONTACT;
    static const sf::Color COLOR_JOINT;
    static const sf::Color COLOR_BROADPHASE;
    static const sf::Color COLOR_AABB;
};

}

