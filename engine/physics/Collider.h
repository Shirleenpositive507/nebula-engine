#pragma once
#include <vector>
#include <memory>
#include <string>
#include "core/Types.h"
#include "math/Vector2.h"
#include "math/Rect.h"

namespace nebula {

enum class ColliderType {
    Box,
    Circle,
    Polygon,
    Edge,
    Vertex
};

struct Collider {
    Vector2f offset;
    f32 offsetRotation;
    Vector2f scale;
    bool isTrigger;
    std::string materialId;

    Collider() : offset(0.0f, 0.0f), offsetRotation(0.0f), scale(1.0f, 1.0f), isTrigger(false) {}
    virtual ~Collider() = default;

    virtual ColliderType getType() const = 0;
    virtual Rectf getBounds() const = 0;

    void setOffset(const Vector2f& off) { offset = off; }
    Vector2f getOffset() const { return offset; }
    void setOffsetRotation(f32 rot) { offsetRotation = rot; }
    f32 getOffsetRotation() const { return offsetRotation; }
    void setScale(const Vector2f& s) { scale = s; }
    Vector2f getScale() const { return scale; }
    void setTrigger(bool trigger) { isTrigger = trigger; }
    bool getTrigger() const { return isTrigger; }
    void setMaterialId(const std::string& id) { materialId = id; }
    std::string getMaterialId() const { return materialId; }
};

struct BoxCollider : public Collider {
    f32 width;
    f32 height;

    BoxCollider() : width(1.0f), height(1.0f) {}
    BoxCollider(f32 w, f32 h) : width(w), height(h) {}

    ColliderType getType() const override { return ColliderType::Box; }

    Rectf getBounds() const override {
        f32 w = width * scale.x;
        f32 h = height * scale.y;
        return Rectf(-w / 2.0f + offset.x, -h / 2.0f + offset.y, w, h);
    }
};

struct CircleCollider : public Collider {
    f32 radius;

    CircleCollider() : radius(0.5f) {}
    explicit CircleCollider(f32 r) : radius(r) {}

    ColliderType getType() const override { return ColliderType::Circle; }

    Rectf getBounds() const override {
        f32 r = radius * std::max(scale.x, scale.y);
        return Rectf(-r + offset.x, -r + offset.y, r * 2.0f, r * 2.0f);
    }
};

struct PolygonCollider : public Collider {
    std::vector<Vector2f> vertices;

    PolygonCollider() = default;
    explicit PolygonCollider(const std::vector<Vector2f>& verts)
        : vertices(verts) {}

    ColliderType getType() const override { return ColliderType::Polygon; }

    Rectf getBounds() const override {
        if (vertices.empty()) return Rectf(0, 0, 0, 0);
        f32 minX = vertices[0].x * scale.x, minY = vertices[0].y * scale.y;
        f32 maxX = vertices[0].x * scale.x, maxY = vertices[0].y * scale.y;
        for (const auto& v : vertices) {
            Vector2f sv(v.x * scale.x, v.y * scale.y);
            Vector2f r = sv.rotated(offsetRotation);
            minX = std::min(minX, r.x + offset.x);
            minY = std::min(minY, r.y + offset.y);
            maxX = std::max(maxX, r.x + offset.x);
            maxY = std::max(maxY, r.y + offset.y);
        }
        return Rectf(minX, minY, maxX - minX, maxY - minY);
    }

    bool isConvex() const {
        if (vertices.size() < 3) return false;
        i32 sign = 0;
        for (size_t i = 0; i < vertices.size(); ++i) {
            const Vector2f& a = vertices[i];
            const Vector2f& b = vertices[(i + 1) % vertices.size()];
            const Vector2f& c = vertices[(i + 2) % vertices.size()];
            Vector2f ab = b - a;
            Vector2f bc = c - b;
            f32 cross = ab.cross(bc);
            if (std::abs(cross) < 1e-8f) continue;
            i32 currentSign = (cross > 0) ? 1 : -1;
            if (sign == 0) sign = currentSign;
            else if (sign != currentSign) return false;
        }
        return true;
    }

    void addVertex(const Vector2f& v) { vertices.push_back(v); }
    size_t getVertexCount() const { return vertices.size(); }
    Vector2f getVertex(size_t index) const { return vertices[index]; }
    Vector2f getTransformedVertex(size_t index, const Vector2f& pos, f32 rot) const {
        Vector2f sv = vertices[index] * scale;
        Vector2f r = sv.rotated(rot + offsetRotation);
        return r + pos + offset;
    }

    std::vector<Vector2f> getEdges() const {
        std::vector<Vector2f> edges;
        for (size_t i = 0; i < vertices.size(); ++i) {
            const Vector2f& a = vertices[i];
            const Vector2f& b = vertices[(i + 1) % vertices.size()];
            edges.push_back(b - a);
        }
        return edges;
    }
};

struct CompoundCollider : public Collider {
    std::vector<std::unique_ptr<Collider>> subColliders;

    CompoundCollider() = default;

    ColliderType getType() const override { return ColliderType::Polygon; }

    void addSubCollider(std::unique_ptr<Collider> collider) {
        subColliders.push_back(std::move(collider));
    }

    size_t getSubColliderCount() const { return subColliders.size(); }
    Collider* getSubCollider(size_t index) const {
        if (index < subColliders.size()) return subColliders[index].get();
        return nullptr;
    }

    Rectf getBounds() const override {
        if (subColliders.empty()) return Rectf(0, 0, 0, 0);
        Rectf total = subColliders[0]->getBounds();
        for (size_t i = 1; i < subColliders.size(); ++i) {
            Rectf subBounds = subColliders[i]->getBounds();
            f32 minX = std::min(total.getMinX(), subBounds.getMinX());
            f32 minY = std::min(total.getMinY(), subBounds.getMinY());
            f32 maxX = std::max(total.getMaxX(), subBounds.getMaxX());
            f32 maxY = std::max(total.getMaxY(), subBounds.getMaxY());
            total = Rectf(minX, minY, maxX - minX, maxY - minY);
        }
        return total;
    }

    void removeSubCollider(size_t index) {
        if (index < subColliders.size()) {
            subColliders.erase(subColliders.begin() + static_cast<std::ptrdiff_t>(index));
        }
    }

    void clearSubColliders() {
        subColliders.clear();
    }
};

} // namespace nebula
