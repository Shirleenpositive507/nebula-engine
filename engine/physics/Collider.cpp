#include "Collider.h"
#include <algorithm>

namespace nebula {

CompoundCollider* CompoundCollider::createFromColliders(
    const std::vector<Collider*>& colliders) {
    auto* compound = new CompoundCollider();
    for (auto* col : colliders) {
        switch (col->getType()) {
            case ColliderType::Box: {
                auto* box = static_cast<BoxCollider*>(col);
                auto sub = std::make_unique<BoxCollider>(box->width, box->height);
                sub->offset = box->offset;
                sub->offsetRotation = box->offsetRotation;
                sub->scale = box->scale;
                sub->isTrigger = box->isTrigger;
                compound->addSubCollider(std::move(sub));
                break;
            }
            case ColliderType::Circle: {
                auto* circle = static_cast<CircleCollider*>(col);
                auto sub = std::make_unique<CircleCollider>(circle->radius);
                sub->offset = circle->offset;
                sub->offsetRotation = circle->offsetRotation;
                sub->scale = circle->scale;
                sub->isTrigger = circle->isTrigger;
                compound->addSubCollider(std::move(sub));
                break;
            }
            case ColliderType::Polygon: {
                auto* poly = static_cast<PolygonCollider*>(col);
                auto sub = std::make_unique<PolygonCollider>(poly->vertices);
                sub->offset = poly->offset;
                sub->offsetRotation = poly->offsetRotation;
                sub->scale = poly->scale;
                sub->isTrigger = poly->isTrigger;
                compound->addSubCollider(std::move(sub));
                break;
            }
            default:
                break;
        }
    }
    return compound;
}

void CompoundCollider::transformSubColliders(const Vector2f& parentPos, f32 parentRot) {
    for (auto& sub : subColliders) {
        Vector2f worldOffset = sub->offset.rotated(parentRot + sub->offsetRotation);
        sub->offset = worldOffset;
        sub->offsetRotation = 0.0f;
    }
}

void CompoundCollider::removeOverlappingSubColliders() {
    for (auto it = subColliders.begin(); it != subColliders.end();) {
        bool overlapping = false;
        for (auto other = subColliders.begin(); other != subColliders.end(); ++other) {
            if (it == other) continue;
            Rectf boundsA = (*it)->getBounds();
            Rectf boundsB = (*other)->getBounds();
            if (boundsA.intersects(boundsB)) {
                overlapping = true;
                break;
            }
        }
        if (overlapping) {
            it = subColliders.erase(it);
        } else {
            ++it;
        }
    }
}

void PolygonCollider::generateEdgeColliders(std::vector<BoxCollider>& edgeColliders,
                                            f32 edgeThickness) const {
    if (vertices.size() < 2) return;

    for (size_t i = 0; i < vertices.size(); ++i) {
        const Vector2f& a = vertices[i];
        const Vector2f& b = vertices[(i + 1) % vertices.size()];

        Vector2f mid = (a + b) * 0.5f;
        Vector2f edgeDir = b - a;
        f32 edgeLength = edgeDir.length();

        BoxCollider edgeCol(edgeLength, edgeThickness);
        edgeCol.offset = mid;
        edgeCol.offsetRotation = std::atan2(edgeDir.y, edgeDir.x);
        edgeColliders.push_back(edgeCol);
    }
}

void PolygonCollider::generateVertexColliders(std::vector<CircleCollider>& vertexColliders,
                                              f32 vertexRadius) const {
    for (const auto& v : vertices) {
        CircleCollider vertCol(vertexRadius);
        vertCol.offset = v;
        vertexColliders.push_back(vertCol);
    }
}

} // namespace nebula
