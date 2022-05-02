#pragma once
#include <vector>
#include <memory>
#include "core/Types.h"
#include "math/Vector2.h"
#include "math/Rect.h"

namespace nebula {

enum class ColliderType {
    Box,
    Circle,
    Polygon
};

struct Collider {
    Vector2f offset;
    bool isTrigger;

    Collider() : offset(0.0f, 0.0f), isTrigger(false) {}
    virtual ~Collider() = default;

    virtual ColliderType getType() const = 0;
    virtual Rectf getBounds() const = 0;

    void setOffset(const Vector2f& off) { offset = off; }
    Vector2f getOffset() const { return offset; }
    void setTrigger(bool trigger) { isTrigger = trigger; }
    bool getTrigger() const { return isTrigger; }
};

struct BoxCollider : public Collider {
    f32 width;
    f32 height;

    BoxCollider() : width(1.0f), height(1.0f) {}
    BoxCollider(f32 w, f32 h) : width(w), height(h) {}

    ColliderType getType() const override { return ColliderType::Box; }

    Rectf getBounds() const override {
        return Rectf(-width / 2.0f + offset.x, -height / 2.0f + offset.y,
                      width, height);
    }
};

struct CircleCollider : public Collider {
    f32 radius;

    CircleCollider() : radius(0.5f) {}
    explicit CircleCollider(f32 r) : radius(r) {}

    ColliderType getType() const override { return ColliderType::Circle; }

    Rectf getBounds() const override {
        return Rectf(-radius + offset.x, -radius + offset.y,
                      radius * 2.0f, radius * 2.0f);
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
        f32 minX = vertices[0].x, minY = vertices[0].y;
        f32 maxX = vertices[0].x, maxY = vertices[0].y;
        for (const auto& v : vertices) {
            minX = std::min(minX, v.x + offset.x);
            minY = std::min(minY, v.y + offset.y);
            maxX = std::max(maxX, v.x + offset.x);
            maxY = std::max(maxY, v.y + offset.y);
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
};

} // namespace nebula
