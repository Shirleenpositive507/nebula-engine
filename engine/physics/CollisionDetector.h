#pragma once
#include <vector>
#include <functional>
#include <bitset>
#include <queue>
#include "core/Types.h"
#include "math/Vector2.h"
#include "Broadphase.h"
#include "Narrowphase.h"
#include "RigidBody.h"
#include "Collider.h"

namespace nebula {

static constexpr u32 MAX_COLLISION_LAYERS = 32;

struct CollisionMatrix {
    std::bitset<MAX_COLLISION_LAYERS> layerFilter[MAX_COLLISION_LAYERS];

    CollisionMatrix() {
        for (u32 i = 0; i < MAX_COLLISION_LAYERS; ++i) {
            layerFilter[i].set();
        }
    }

    void setLayerCollision(u32 layerA, u32 layerB, bool canCollide) {
        if (layerA < MAX_COLLISION_LAYERS && layerB < MAX_COLLISION_LAYERS) {
            layerFilter[layerA].set(layerB, canCollide);
            layerFilter[layerB].set(layerA, canCollide);
        }
    }

    bool canCollide(u32 layerA, u32 layerB) const {
        if (layerA >= MAX_COLLISION_LAYERS || layerB >= MAX_COLLISION_LAYERS) return true;
        return layerFilter[layerA].test(layerB);
    }

    void disableAllLayers(u32 layer) {
        if (layer < MAX_COLLISION_LAYERS) {
            for (u32 i = 0; i < MAX_COLLISION_LAYERS; ++i) {
                layerFilter[layer].set(i, false);
                layerFilter[i].set(layer, false);
            }
        }
    }

    void enableAllLayers(u32 layer) {
        if (layer < MAX_COLLISION_LAYERS) {
            for (u32 i = 0; i < MAX_COLLISION_LAYERS; ++i) {
                layerFilter[layer].set(i, true);
                layerFilter[i].set(layer, true);
            }
        }
    }
};

struct BufferedCollisionEvent {
    RigidBody* bodyA;
    RigidBody* bodyB;
    CollisionInfo info;
    bool entered;
    bool exited;
    i32 frameDelay;

    BufferedCollisionEvent()
        : bodyA(nullptr), bodyB(nullptr)
        , entered(false), exited(false)
        , frameDelay(1) {}
};

struct CollisionGroupMask {
    u32 group;
    u32 mask;

    CollisionGroupMask() : group(0x0001), mask(0xFFFF) {}
    explicit CollisionGroupMask(u32 g, u32 m) : group(g), mask(m) {}

    bool canCollideWith(const CollisionGroupMask& other) const {
        return (group & other.mask) != 0 && (other.group & mask) != 0;
    }

    void setGroup(u32 g) { group = g; }
    void setMask(u32 m) { mask = m; }
    u32 getGroup() const { return group; }
    u32 getMask() const { return mask; }

    static const CollisionGroupMask Default;
    static const CollisionGroupMask Static;
    static const CollisionGroupMask Dynamic;
    static const CollisionGroupMask Trigger;
    static const CollisionGroupMask Projectile;
    static const CollisionGroupMask Player;
    static const CollisionGroupMask Enemy;
    static const CollisionGroupMask Environment;
};

struct RaycastHit {
    Vector2f point;
    Vector2f normal;
    f32 distance;
    Collider* collider;
    RigidBody* rigidBody;

    RaycastHit()
        : point(0, 0), normal(0, 0), distance(0)
        , collider(nullptr), rigidBody(nullptr) {}
};

class CollisionDetector {
public:
    Broadphase broadphase;
    Narrowphase narrowphase;
    CollisionMatrix collisionMatrix;

    CollisionDetector() = default;
    explicit CollisionDetector(f32 cellSize) : broadphase(cellSize) {}

    std::vector<CollisionInfo> detectCollisions(const std::vector<ColliderPair>& pairs);

    void setLayerCollision(u32 layerA, u32 layerB, bool canCollide);
    void setBodyCollisionGroup(RigidBody* body, u32 group, u32 mask);
    bool canBodiesCollide(RigidBody* a, RigidBody* b) const;

    void setTriggerOnly(RigidBody* body, bool triggerOnly);
    bool isTriggerOnly(RigidBody* body) const;

    void bufferCollisionEvent(const BufferedCollisionEvent& event);
    std::vector<BufferedCollisionEvent> getBufferedEvents();
    void clearBufferedEvents();
    void updateBufferedEvents();

    RaycastHit raycast(const Vector2f& origin, const Vector2f& direction, f32 maxDistance);
    std::vector<RaycastHit> raycastAll(const Vector2f& origin, const Vector2f& direction, f32 maxDistance);
    RaycastHit raycastFiltered(const Vector2f& origin, const Vector2f& direction,
                                f32 maxDistance, u32 layerMask);

    RaycastHit circleCast(const Vector2f& origin, f32 radius,
                           const Vector2f& direction, f32 maxDistance);
    RaycastHit rectCast(const Rectf& rect, const Vector2f& direction, f32 maxDistance);

    RaycastHit ccdRaycast(RigidBody* body, const Vector2f& origin, const Vector2f& direction,
                           f32 maxDistance);

private:
    bool rayAABBIntersect(const Vector2f& origin, const Vector2f& dir,
                           const Rectf& bounds, f32& tMin, f32& tMax);
    bool rayCircleIntersect(const Vector2f& origin, const Vector2f& dir,
                            const Vector2f& center, f32 radius, f32& t);

    std::unordered_map<RigidBody*, CollisionGroupMask> m_collisionGroups;
    std::unordered_map<RigidBody*, bool> m_triggerOnly;
    std::queue<BufferedCollisionEvent> m_eventBuffer;
    std::vector<BufferedCollisionEvent> m_previousFrameEvents;
};

} // namespace nebula
