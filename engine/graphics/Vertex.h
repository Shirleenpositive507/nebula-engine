#pragma once

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/System/Vector2.hpp>
#include "Color.h"
#include <vector>
#include <cstddef>

namespace nebula {
    namespace graphics {

        struct Vertex {
            sf::Vector2f position;
            Color color;
            sf::Vector2f texCoords;

            Vertex();
            Vertex(const sf::Vector2f& pos, const Color& col = Color::White, const sf::Vector2f& tex = sf::Vector2f(0.f, 0.f));
            explicit Vertex(const sf::Vertex& sfmlVertex);

            sf::Vertex toSFML() const;
            static Vertex fromSFML(const sf::Vertex& sfmlVertex);
        };

        class VertexBuffer {
        public:
            VertexBuffer();
            explicit VertexBuffer(sf::PrimitiveType type);
            ~VertexBuffer();

            bool create(std::size_t vertexCount);
            void update(const Vertex* vertices, std::size_t count, std::size_t offset = 0);
            void update(const std::vector<Vertex>& vertices, std::size_t offset = 0);
            bool bind() const;
            static void unbind();
            void setPrimitiveType(sf::PrimitiveType type);
            sf::PrimitiveType getPrimitiveType() const;
            std::size_t getVertexCount() const;

            void resize(std::size_t newSize);
            void addVertex(const Vertex& vertex);
            void clear();

            sf::VertexBuffer& getHandle();
            const sf::VertexBuffer& getHandle() const;

        private:
            sf::VertexBuffer m_buffer;
            std::vector<Vertex> m_vertices;
            bool m_dirty;
        };

    }
}
