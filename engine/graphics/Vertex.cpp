#include "Vertex.h"

namespace nebula {
    namespace graphics {

        Vertex::Vertex()
            : position(0.f, 0.f), color(Color::White), texCoords(0.f, 0.f) {}

        Vertex::Vertex(const sf::Vector2f& pos, const Color& col, const sf::Vector2f& tex)
            : position(pos), color(col), texCoords(tex) {}

        Vertex::Vertex(const sf::Vertex& sfmlVertex)
            : position(sfmlVertex.position)
            , color(Color::fromSFML(sfmlVertex.color))
            , texCoords(sfmlVertex.texCoords) {}

        sf::Vertex Vertex::toSFML() const {
            sf::Vertex v;
            v.position = position;
            v.color = color.toSFML();
            v.texCoords = texCoords;
            return v;
        }

        Vertex Vertex::fromSFML(const sf::Vertex& sfmlVertex) {
            return Vertex(sfmlVertex);
        }

        VertexBuffer::VertexBuffer()
            : m_buffer(sf::PrimitiveType::Triangles)
            , m_dirty(false) {}

        VertexBuffer::VertexBuffer(sf::PrimitiveType type)
            : m_buffer(type)
            , m_dirty(false) {}

        VertexBuffer::~VertexBuffer() {}

        bool VertexBuffer::create(std::size_t vertexCount) {
            m_vertices.resize(vertexCount);
            m_dirty = true;
            return m_buffer.create(vertexCount);
        }

        void VertexBuffer::update(const Vertex* vertices, std::size_t count, std::size_t offset) {
            if (offset + count > m_vertices.size()) {
                m_vertices.resize(offset + count);
            }
            for (std::size_t i = 0; i < count; ++i) {
                m_vertices[offset + i] = vertices[i];
            }
            m_dirty = true;

            std::vector<sf::Vertex> sfmlVerts(count);
            for (std::size_t i = 0; i < count; ++i) {
                sfmlVerts[i] = vertices[i].toSFML();
            }
            m_buffer.update(sfmlVerts.data(), count, offset);
        }

        void VertexBuffer::update(const std::vector<Vertex>& vertices, std::size_t offset) {
            update(vertices.data(), vertices.size(), offset);
        }

        bool VertexBuffer::bind() const {
            return m_buffer.bind();
        }

        void VertexBuffer::unbind() {
            sf::VertexBuffer::unbind();
        }

        void VertexBuffer::setPrimitiveType(sf::PrimitiveType type) {
            m_buffer.setPrimitiveType(type);
        }

        sf::PrimitiveType VertexBuffer::getPrimitiveType() const {
            return m_buffer.getPrimitiveType();
        }

        std::size_t VertexBuffer::getVertexCount() const {
            return m_vertices.size();
        }

        void VertexBuffer::resize(std::size_t newSize) {
            m_vertices.resize(newSize);
            m_dirty = true;
        }

        void VertexBuffer::addVertex(const Vertex& vertex) {
            m_vertices.push_back(vertex);
            m_dirty = true;
        }

        void VertexBuffer::clear() {
            m_vertices.clear();
            m_dirty = true;
        }

        sf::VertexBuffer& VertexBuffer::getHandle() {
            return m_buffer;
        }

        const sf::VertexBuffer& VertexBuffer::getHandle() const {
            return m_buffer;
        }

    }
}
