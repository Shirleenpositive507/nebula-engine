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
    Vertex,
    Mesh,
    Terrain
};

struct Collider {
    Vector2f offset;
    f32 offsetRotation;
    Vector2f scale;
    f32 margin;
    bool isTrigger;
    std::string materialId;

    Collider() : offset(0.0f, 0.0f), offsetRotation(0.0f), scale(1.0f, 1.0f), margin(0.01f), isTrigger(false) {}
    virtual ~Collider() = default;

    virtual ColliderType getType() const = 0;
    virtual Rectf getBounds() const = 0;

    void setOffset(const Vector2f& off) { offset = off; }
    Vector2f getOffset() const { return offset; }
    void setOffsetRotation(f32 rot) { offsetRotation = rot; }
    f32 getOffsetRotation() const { return offsetRotation; }
    void setScale(const Vector2f& s) { scale = s; }
    Vector2f getScale() const { return scale; }
    void setMargin(f32 m) { margin = std::max(0.0f, m); }
    f32 getMargin() const { return margin; }
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

    static CompoundCollider* createFromColliders(const std::vector<Collider*>& colliders);
    void transformSubColliders(const Vector2f& parentPos, f32 parentRot);
    void removeOverlappingSubColliders();
};

struct MeshCollider : public Collider {
    std::vector<Vector2f> vertices;
    std::vector<std::pair<size_t, size_t>> edges;

    MeshCollider() = default;
    explicit MeshCollider(const std::vector<Vector2f>& verts,
                          const std::vector<std::pair<size_t, size_t>>& edgeIndices)
        : vertices(verts), edges(edgeIndices) {}

    ColliderType getType() const override { return ColliderType::Mesh; }

    Rectf getBounds() const override {
        if (vertices.empty()) return Rectf(0, 0, 0, 0);
        f32 minX = vertices[0].x, minY = vertices[0].y;
        f32 maxX = vertices[0].x, maxY = vertices[0].y;
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

    bool rayCast(const Vector2f& origin, const Vector2f& direction, f32& outT, Vector2f& outNormal) const;
    void addVertex(const Vector2f& v) { vertices.push_back(v); }
    void addEdge(size_t i, size_t j) { edges.emplace_back(i, j); }
    size_t getVertexCount() const { return vertices.size(); }
    size_t getEdgeCount() const { return edges.size(); }
};

struct TerrainCollider : public Collider {
    std::vector<f32> heightmap;
    i32 width;
    i32 height;
    f32 tileSize;

    TerrainCollider() : width(0), height(0), tileSize(1.0f) {}
    TerrainCollider(const std::vector<f32>& hm, i32 w, i32 h, f32 ts)
        : heightmap(hm), width(w), height(h), tileSize(ts) {}

    ColliderType getType() const override { return ColliderType::Terrain; }

    Rectf getBounds() const override {
        f32 w = static_cast<f32>(width) * tileSize * scale.x;
        f32 h = static_cast<f32>(height) * tileSize * scale.y;
        return Rectf(offset.x, offset.y, w, h);
    }

    f32 sampleHeight(f32 x, f32 z) const {
        if (heightmap.empty() || width <= 0 || height <= 0) return 0.0f;
        i32 ix = static_cast<i32>(x / tileSize);
        i32 iz = static_cast<i32>(z / tileSize);
        ix = std::clamp(ix, 0, width - 2);
        iz = std::clamp(iz, 0, height - 2);
        f32 fx = (x / tileSize) - static_cast<f32>(ix);
        f32 fz = (z / tileSize) - static_cast<f32>(iz);
        f32 h00 = heightmap[iz * width + ix];
        f32 h10 = heightmap[iz * width + ix + 1];
        f32 h01 = heightmap[(iz + 1) * width + ix];
        f32 h11 = heightmap[(iz + 1) * width + ix + 1];
        f32 h0 = std::lerp(h00, h10, fx);
        f32 h1 = std::lerp(h01, h11, fx);
        return std::lerp(h0, h1, fz);
    }
};

} // namespace nebula
