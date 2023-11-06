#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <vector>
#include <array>
#include <cstddef>

namespace nebula {
namespace graphics {

class Color;

struct BatchVertex {
    sf::Vector2f position;
    sf::Color color;
};

struct BatchGroup {
    sf::PrimitiveType primitiveType;
    std::size_t startIndex;
    std::size_t vertexCount;
    int layer;

    bool operator<(const BatchGroup& other) const {
        return layer < other.layer;
    }
};

class BatchRenderer {
public:
    BatchRenderer();
    ~BatchRenderer();

    void beginFrame();
    void endFrame();
    void flush();
    void render(sf::RenderTarget& target);

    void submitQuad(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Color& color);
    void submitQuad(const sf::Vector2f& pos, const sf::Vector2f& size, const sf::Color& color, float rotation);
    void submitTriangle(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Color& color);
    void submitCircle(const sf::Vector2f& center, float radius, const sf::Color& color, int segments = 32);
    void submitLine(const sf::Vector2f& from, const sf::Vector2f& to, const sf::Color& color, float thickness = 1.0f);
    void submitRect(const sf::FloatRect& rect, const sf::Color& color, float thickness = 1.0f);
    void submitRect(const sf::FloatRect& rect, const sf::Color& fillColor, const sf::Color& outlineColor, float thickness = 1.0f);

    void submitVertices(const std::vector<sf::Vertex>& vertices, sf::PrimitiveType type, int layer = 0);

    void setBatchSize(std::size_t size) { m_batchSize = size; }
    std::size_t getBatchSize() const { return m_batchSize; }

    std::size_t getVertexCount() const { return m_vertices.size(); }
    std::size_t getBatchCount() const { return m_batches.size(); }
    void clear();

    bool isInFrame() const { return m_inFrame; }

private:
    void ensureCapacity(std::size_t needed);
    void addVertex(const sf::Vector2f& pos, const sf::Color& color);

    std::vector<sf::Vertex> m_vertices;
    std::vector<BatchGroup> m_batches;
    sf::PrimitiveType m_currentType;
    std::size_t m_currentBatchStart;
    int m_currentLayer;
    bool m_inFrame;
    std::size_t m_batchSize;

    static constexpr std::size_t DEFAULT_BATCH_SIZE = 4096;
};

}
}

