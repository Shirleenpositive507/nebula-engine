#include "PhysicsDebug.h"

namespace nebula {

const sf::Color PhysicsDebug::COLOR_STATIC(100, 100, 255, 128);
const sf::Color PhysicsDebug::COLOR_DYNAMIC(100, 255, 100, 128);
const sf::Color PhysicsDebug::COLOR_KINEMATIC(255, 255, 100, 128);
const sf::Color PhysicsDebug::COLOR_CONTACT(255, 50, 50, 255);
const sf::Color PhysicsDebug::COLOR_JOINT(255, 128, 255, 255);
const sf::Color PhysicsDebug::COLOR_BROADPHASE(128, 128, 128, 64);
const sf::Color PhysicsDebug::COLOR_AABB(200, 200, 50, 128);

PhysicsDebug::PhysicsDebug()
    : m_batchRenderer(nullptr)
    , m_enabled(false)
    , m_drawColliders(true)
    , m_drawContacts(true)
    , m_drawJoints(true)
    , m_drawBroadphase(false)
    , m_drawAABBs(false)
{
}

PhysicsDebug::~PhysicsDebug() {}

sf::Color PhysicsDebug::getColorForBodyType(RigidBody* body) {
    if (!body) return COLOR_STATIC;
    if (body->isStatic()) return COLOR_STATIC;
    if (body->isKinematic()) return COLOR_KINEMATIC;
    return COLOR_DYNAMIC;
}

void PhysicsDebug::drawWorld(PhysicsWorld& world) {
    if (!m_enabled || !m_batchRenderer) return;
    if (!m_batchRenderer->isInFrame()) m_batchRenderer->beginFrame();

    if (m_drawColliders) {
        for (auto* body : world.bodies) {
            if (body && body->collider) {
                sf::Color color = getColorForBodyType(body);
                drawCollider(body, color);
            }
        }
    }

    if (m_drawContacts) {
        drawContactPoints({});
    }

    if (m_drawJoints) {
        drawJoints(world.joints);
    }

    if (m_drawBroadphase) {
        drawBroadphaseCells(world);
    }

    if (m_drawAABBs) {
        for (auto* body : world.bodies) {
            if (body && body->collider) {
                drawAABB(body, COLOR_AABB);
            }
        }
    }
}

void PhysicsDebug::drawCollider(RigidBody* body, const sf::Color& color) {
    if (!body || !body->collider || !m_batchRenderer) return;

    Collider* collider = body->collider;
    Vector2f pos = body->position;

    switch (collider->getType()) {
        case ColliderType::Box: {
            auto* box = static_cast<BoxCollider*>(collider);
            sf::FloatRect bounds(
                pos.x + box->offset.x - box->width * 0.5f,
                pos.y + box->offset.y - box->height * 0.5f,
                box->width,
                box->height
            );
            m_batchRenderer->submitRect(bounds, sf::Color::Transparent, color, 1.5f);
            break;
        }
        case ColliderType::Circle: {
            auto* circle = static_cast<CircleCollider*>(collider);
            sf::Vector2f center(pos.x + circle->offset.x, pos.y + circle->offset.y);
            m_batchRenderer->submitCircle(center, circle->radius, color, 24);
            break;
        }
        default:
            break;
    }
}

void PhysicsDebug::drawContactPoints(const std::vector<CollisionInfo>& collisions) {
    (void)collisions;
    if (!m_batchRenderer) return;
}

void PhysicsDebug::drawJoints(const std::vector<Joint*>& joints) {
    if (!m_batchRenderer) return;
    for (auto* joint : joints) {
        if (!joint) continue;
        RigidBody* bodyA = nullptr;
        RigidBody* bodyB = nullptr;
        if (bodyA && bodyB) {
            sf::Vector2f from(bodyA->position.x, bodyA->position.y);
            sf::Vector2f to(bodyB->position.x, bodyB->position.y);
            m_batchRenderer->submitLine(from, to, COLOR_JOINT, 1.0f);
        }
    }
}

void PhysicsDebug::drawBroadphaseCells(PhysicsWorld& world) {
    if (!m_batchRenderer) return;
    (void)world;
}

void PhysicsDebug::drawAABB(RigidBody* body, const sf::Color& color) {
    if (!body || !body->collider || !m_batchRenderer) return;
    Rectf bounds = body->collider->getBounds();
    bounds.offset(body->position);
    sf::FloatRect sfBounds(bounds.x, bounds.y, bounds.width, bounds.height);
    m_batchRenderer->submitRect(sfBounds, sf::Color::Transparent, color, 1.0f);
}

}

