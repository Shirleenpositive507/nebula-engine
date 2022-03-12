#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include "BlendMode.h"
#include "Vertex.h"
#include "PrimitiveType.h"
#include <vector>
#include <unordered_map>
#include <cstddef>

namespace nebula {
    namespace graphics {

        struct SpriteBatchItem {
            const sf::Texture* texture;
            sf::Vertex vertices[4];
            float sortKey;
        };

        class SpriteBatch {
        public:
            static constexpr std::size_t MAX_BATCH_SIZE = 4096;

            SpriteBatch();
            explicit SpriteBatch(sf::RenderTarget& target);
            ~SpriteBatch();

            void setTarget(sf::RenderTarget& target);
            sf::RenderTarget* getTarget() const;

            void begin(BlendMode blendMode = BlendMode::Alpha);
            void begin(const BlendState& blendState);

            void draw(const sf::Sprite& sprite);
            void draw(const sf::Texture& texture, const sf::Vector2f& position,
                      const Color& color = Color::White,
                      float rotation = 0.f,
                      const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f),
                      const sf::Vector2f& origin = sf::Vector2f(0.f, 0.f));
            void draw(const sf::Texture& texture, const sf::IntRect& sourceRect,
                      const sf::FloatRect& destRect,
                      const Color& color = Color::White);
            void draw(const sf::Texture& texture, const sf::Vector2f& position,
                      const sf::IntRect& sourceRect,
                      const Color& color = Color::White,
                      float rotation = 0.f,
                      const sf::Vector2f& scale = sf::Vector2f(1.f, 1.f),
                      const sf::Vector2f& origin = sf::Vector2f(0.f, 0.f));

            void draw(const sf::VertexArray& vertexArray, const sf::Texture* texture = nullptr);
            void draw(const sf::Vertex* vertices, std::size_t count,
                      sf::PrimitiveType type, const sf::Texture* texture = nullptr);

            void flush();
            void end();

            void setTextureSorting(bool enabled);
            bool isTextureSortingEnabled() const;

            std::size_t getBatchCount() const;
            std::size_t getVertexCount() const;
            void resetStats();

            void setPrimitiveType(PrimitiveType type);
            sf::PrimitiveType getPrimitiveType() const;

        private:
            sf::RenderTarget* m_target;
            bool m_began;
            std::size_t m_batchCount;
            std::size_t m_totalVertices;
            bool m_textureSorting;
            sf::PrimitiveType m_primitiveType;
            BlendState m_blendState;

            std::vector<SpriteBatchItem> m_items;

            void flushBatch(const std::vector<SpriteBatchItem>& batch);
            static void generateVertexFromSprite(const sf::Sprite& sprite, sf::Vertex* outVertices);
            void generateVertices(const sf::Texture& texture,
                                  const sf::Vector2f& position,
                                  const sf::IntRect* sourceRect,
                                  const sf::FloatRect* destRect,
                                  const Color& color,
                                  float rotation,
                                  const sf::Vector2f& scale,
                                  const sf::Vector2f& origin,
                                  sf::Vertex* outVertices);
        };

    }
}
