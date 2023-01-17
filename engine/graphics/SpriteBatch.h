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
#include <functional>
#include <algorithm>

namespace nebula {
    namespace graphics {

        enum class SpriteSortMode {
            None,
            FrontToBack,
            BackToFront,
            Texture
        };

        struct SpriteBatchItem {
            const sf::Texture* texture;
            sf::Vertex vertices[4];
            float sortKey;
        };

        class SpriteBatch {
        public:
            using SortComparator = std::function<bool(const SpriteBatchItem&, const SpriteBatchItem&)>;
            using FlushCallback = std::function<void()>;

            static constexpr std::size_t DEFAULT_MAX_BATCH = 4096;

            SpriteBatch();
            explicit SpriteBatch(sf::RenderTarget& target);
            explicit SpriteBatch(std::size_t maxVertices);
            SpriteBatch(sf::RenderTarget& target, std::size_t maxVertices);
            ~SpriteBatch();

            void setTarget(sf::RenderTarget& target);
            sf::RenderTarget* getTarget() const;

            void begin(BlendMode blendMode = BlendMode::Alpha);
            void begin(const BlendState& blendState);

            void begin(SpriteSortMode sortMode);
            void begin(SpriteSortMode sortMode, const BlendState& blendState);
            void begin(SpriteSortMode sortMode, const BlendState& blendState, const SortComparator& customComparator);

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

            void setSortMode(SpriteSortMode mode);
            SpriteSortMode getSortMode() const;

            void setMaxVertices(std::size_t maxVertices);
            std::size_t getMaxVertices() const;

            void setFlushCallback(FlushCallback callback);
            FlushCallback getFlushCallback() const;

            void setCustomComparator(const SortComparator& comparator);
            SortComparator getCustomComparator() const;

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
            SpriteSortMode m_sortMode;
            std::size_t m_maxVertices;
            FlushCallback m_flushCallback;
            SortComparator m_customComparator;

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
