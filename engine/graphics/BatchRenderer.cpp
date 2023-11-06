#include "BatchRenderer.h"
#include <cmath>
#include <algorithm>

namespace nebula {
namespace graphics {

BatchRenderer::BatchRenderer()
    : m_inFrame(false)
    , m_batchSize(DEFAULT_BATCH_SIZE)
{
    m_vertices.reserve(DEFAULT_BATCH_SIZE);
}

BatchRenderer::~BatchRenderer() {}

void BatchRenderer::beginFrame() {
    m_inFrame = true;
    m_vertices.clear();
    m_batches.clear();
    m_currentBatchStart = 0;
    m_currentLayer = 0;
    m_currentType = sf::PrimitiveType::Triangles;
}

void BatchRenderer::endFrame() {
    if (!m_inFrame) return;
    if (m_vertices.size() > m_currentBatchStart) {
        BatchGroup group;
        group.primitiveType = m_currentType;
        group.startIndex = m_currentBatchStart;
        group.vertexCount = m_vertices.size() - m_currentBatchStart;
        group.layer = m_currentLayer;
        m_batches.push_back(group);
    }
    m_inFrame = false;
}

void BatchRenderer::flush() {
    if (m_vertices.size() > m_currentBatchStart) {
        BatchGroup group;
        group.primitiveType = m_currentType;
        group.startIndex = m_currentBatchStart;
        group.vertexCount = m_vertices.size() - m_currentBatchStart;
        group.layer = m_currentLayer;
        m_batches.push_back(group);
    }
    m_currentBatchStart = m_vertices.size();
}

void BatchRenderer::render(sf::RenderTarget& target) {
    if (m_inFrame) endFrame();

    std::sort(m_batches.begin(), m_batches.end());

    for (const auto& batch : m_batches) {
        if (batch.vertexCount == 0) continue;
        target.draw(&m_vertices[batch.startIndex], batch.vertexCount, batch.primitiveType);
    }
}

void BatchRenderer::submitQuad(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Color& color) {
    ensureCapacity(6);

    sf::Vector2f p1 = pos;
    sf::Vector2f p2 = pos + sf::Vector2f(size.x, 0);
    sf::Vector2f p3 = pos + sf::Vector2f(size.x, size.y);
    sf::Vector2f p4 = pos + sf::Vector2f(0, size.y);

    addVertex(p1, color);
    addVertex(p2, color);
    addVertex(p3, color);
    addVertex(p1, color);
    addVertex(p3, color);
    addVertex(p4, color);
}

void BatchRenderer::submitQuad(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Color& color, float rotation) {
    ensureCapacity(6);

    sf::Vector2f center = pos + sf::Vector2f(size.x * 0.5f, size.y * 0.5f);
    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);

    auto rot = [&](const sf::Vector2f& p) -> sf::Vector2f {
        sf::Vector2f rel = p - center;
        return center + sf::Vector2f(rel.x * cosR - rel.y * sinR, rel.x * sinR + rel.y * cosR);
    };

    sf::Vector2f p1 = rot(pos);
    sf::Vector2f p2 = rot(pos + sf::Vector2f(size.x, 0));
    sf::Vector2f p3 = rot(pos + sf::Vector2f(size.x, size.y));
    sf::Vector2f p4 = rot(pos + sf::Vector2f(0, size.y));

    addVertex(p1, color);
    addVertex(p2, color);
    addVertex(p3, color);
    addVertex(p1, color);
    addVertex(p3, color);
    addVertex(p4, color);
}

void BatchRenderer::submitTriangle(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Color& color) {
    ensureCapacity(3);
    addVertex(p1, color);
    addVertex(p2, color);
    addVertex(p3, color);
}

void BatchRenderer::submitCircle(const sf::Vector2f& center, float radius, const sf::Color& color, int segments) {
    if (segments < 3) segments = 3;
    ensureCapacity(static_cast<std::size_t>(segments) * 3);

    for (int i = 0; i < segments; ++i) {
        float a1 = 2.0f * 3.14159265f * static_cast<float>(i) / static_cast<float>(segments);
        float a2 = 2.0f * 3.14159265f * static_cast<float>(i + 1) / static_cast<float>(segments);
        sf::Vector2f p1 = center + sf::Vector2f(std::cos(a1) * radius, std::sin(a1) * radius);
        sf::Vector2f p2 = center + sf::Vector2f(std::cos(a2) * radius, std::sin(a2) * radius);
        addVertex(center, color);
        addVertex(p1, color);
        addVertex(p2, color);
    }
}

void BatchRenderer::submitLine(const sf::Vector2f& from, const sf::Vector2f& to, const sf::Color& color, float thickness) {
    ensureCapacity(6);

    sf::Vector2f dir = to - from;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f) return;
    sf::Vector2f perp(-dir.y / len * thickness * 0.5f, dir.x / len * thickness * 0.5f);

    sf::Vector2f p1 = from - perp;
    sf::Vector2f p2 = from + perp;
    sf::Vector2f p3 = to + perp;
    sf::Vector2f p4 = to - perp;

    addVertex(p1, color);
    addVertex(p2, color);
    addVertex(p3, color);
    addVertex(p1, color);
    addVertex(p3, color);
    addVertex(p4, color);
}

void BatchRenderer::submitRect(const sf::FloatRect& rect, const sf::Color& color, float thickness) {
    submitRect(rect, sf::Color::Transparent, color, thickness);
}

void BatchRenderer::submitRect(const sf::FloatRect& rect, const sf::Color& fillColor, const sf::Color& outlineColor, float thickness) {
    if (fillColor.a > 0) {
        submitQuad(sf::Vector2f(rect.left, rect.top), sf::Vector2f(rect.width, rect.height), fillColor);
    }
    if (outlineColor.a > 0 && thickness > 0) {
        float t = thickness;
        submitQuad(sf::Vector2f(rect.left, rect.top), sf::Vector2f(rect.width, t), outlineColor);
        submitQuad(sf::Vector2f(rect.left, rect.top + rect.height - t), sf::Vector2f(rect.width, t), outlineColor);
        submitQuad(sf::Vector2f(rect.left, rect.top + t), sf::Vector2f(t, rect.height - t * 2.0f), outlineColor);
        submitQuad(sf::Vector2f(rect.left + rect.width - t, rect.top + t), sf::Vector2f(t, rect.height - t * 2.0f), outlineColor);
    }
}

void BatchRenderer::submitVertices(const std::vector<sf::Vertex>& vertices, sf::PrimitiveType type, int layer) {
    if (vertices.empty()) return;

    if (m_currentType != type || m_currentLayer != layer) {
        if (m_vertices.size() > m_currentBatchStart) {
            BatchGroup group;
            group.primitiveType = m_currentType;
            group.startIndex = m_currentBatchStart;
            group.vertexCount = m_vertices.size() - m_currentBatchStart;
            group.layer = m_currentLayer;
            m_batches.push_back(group);
        }
        m_currentType = type;
        m_currentLayer = layer;
        m_currentBatchStart = m_vertices.size();
    }

    ensureCapacity(vertices.size());
    for (const auto& v : vertices) {
        m_vertices.push_back(v);
    }
}

void BatchRenderer::clear() {
    m_vertices.clear();
    m_batches.clear();
    m_currentBatchStart = 0;
    m_currentLayer = 0;
}

void BatchRenderer::ensureCapacity(std::size_t needed) {
    if (m_vertices.capacity() - m_vertices.size() < needed) {
        m_vertices.reserve(m_vertices.capacity() + std::max(m_batchSize, needed));
    }
}

void BatchRenderer::addVertex(const sf::Vector2f& pos, const sf::Color& color) {
    m_vertices.push_back(sf::Vertex(pos, color));
}

}
}

