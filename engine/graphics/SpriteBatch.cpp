#include "SpriteBatch.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace nebula {
    namespace graphics {

        SpriteBatch::SpriteBatch()
            : m_target(nullptr)
            , m_began(false)
            , m_batchCount(0)
            , m_totalVertices(0)
            , m_textureSorting(true)
            , m_primitiveType(sf::PrimitiveType::Triangles) {}

        SpriteBatch::SpriteBatch(sf::RenderTarget& target)
            : m_target(&target)
            , m_began(false)
            , m_batchCount(0)
            , m_totalVertices(0)
            , m_textureSorting(true)
            , m_primitiveType(sf::PrimitiveType::Triangles) {}

        SpriteBatch::~SpriteBatch() {
            if (m_began) {
                end();
            }
        }

        void SpriteBatch::setTarget(sf::RenderTarget& target) {
            m_target = &target;
        }

        sf::RenderTarget* SpriteBatch::getTarget() const {
            return m_target;
        }

        void SpriteBatch::begin(BlendMode blendMode) {
            begin(BlendState(blendMode));
        }

        void SpriteBatch::begin(const BlendState& blendState) {
            if (m_began) return;

            m_began = true;
            m_batchCount = 0;
            m_totalVertices = 0;
            m_blendState = blendState;
            m_items.clear();
            m_items.reserve(MAX_BATCH_SIZE);
        }

        void SpriteBatch::draw(const sf::Sprite& sprite) {
            if (!m_began || m_items.size() >= MAX_BATCH_SIZE) {
                if (m_items.size() >= MAX_BATCH_SIZE) flush();
                else return;
            }

            SpriteBatchItem item;
            item.texture = sprite.getTexture();
            generateVertexFromSprite(sprite, item.vertices);

            item.sortKey = item.texture ? reinterpret_cast<std::uintptr_t>(item.texture) : 0;
            m_items.push_back(item);
        }

        void SpriteBatch::draw(const sf::Texture& texture, const sf::Vector2f& position,
                               const Color& color, float rotation,
                               const sf::Vector2f& scale, const sf::Vector2f& origin)
        {
            if (!m_began || m_items.size() >= MAX_BATCH_SIZE) {
                if (m_items.size() >= MAX_BATCH_SIZE) flush();
                else return;
            }

            SpriteBatchItem item;
            item.texture = &texture;
            generateVertices(texture, position, nullptr, nullptr, color,
                             rotation, scale, origin, item.vertices);
            item.sortKey = reinterpret_cast<std::uintptr_t>(&texture);
            m_items.push_back(item);
        }

        void SpriteBatch::draw(const sf::Texture& texture, const sf::IntRect& sourceRect,
                               const sf::FloatRect& destRect, const Color& color)
        {
            if (!m_began || m_items.size() >= MAX_BATCH_SIZE) {
                if (m_items.size() >= MAX_BATCH_SIZE) flush();
                else return;
            }

            SpriteBatchItem item;
            item.texture = &texture;
            generateVertices(texture, sf::Vector2f(0.f, 0.f), &sourceRect, &destRect,
                             color, 0.f, sf::Vector2f(1.f, 1.f), sf::Vector2f(0.f, 0.f), item.vertices);
            item.sortKey = reinterpret_cast<std::uintptr_t>(&texture);
            m_items.push_back(item);
        }

        void SpriteBatch::draw(const sf::Texture& texture, const sf::Vector2f& position,
                               const sf::IntRect& sourceRect, const Color& color,
                               float rotation, const sf::Vector2f& scale,
                               const sf::Vector2f& origin)
        {
            if (!m_began || m_items.size() >= MAX_BATCH_SIZE) {
                if (m_items.size() >= MAX_BATCH_SIZE) flush();
                else return;
            }

            SpriteBatchItem item;
            item.texture = &texture;
            generateVertices(texture, position, &sourceRect, nullptr, color,
                             rotation, scale, origin, item.vertices);
            item.sortKey = reinterpret_cast<std::uintptr_t>(&texture);
            m_items.push_back(item);
        }

        void SpriteBatch::draw(const sf::VertexArray& vertexArray, const sf::Texture* texture) {
            if (vertexArray.getVertexCount() == 0) return;
            draw(vertexArray.getVertices().data(), vertexArray.getVertexCount(),
                 vertexArray.getPrimitiveType(), texture);
        }

        void SpriteBatch::draw(const sf::Vertex* vertices, std::size_t count,
                               sf::PrimitiveType type, const sf::Texture* texture)
        {
            if (!m_began || count == 0) return;

            SpriteBatchItem item;
            item.texture = texture;

            std::size_t vertsPerPrim = 4;
            switch (type) {
                case sf::PrimitiveType::Points: vertsPerPrim = 1; break;
                case sf::PrimitiveType::Lines: vertsPerPrim = 2; break;
                case sf::PrimitiveType::LineStrip: vertsPerPrim = count; break;
                case sf::PrimitiveType::Triangles: vertsPerPrim = 3; break;
                case sf::PrimitiveType::TriangleStrip:
                case sf::PrimitiveType::TriangleFan:
                case sf::PrimitiveType::Quads:
                default: vertsPerPrim = 4; break;
            }

            std::size_t itemCount = (type == sf::PrimitiveType::Quads) ? count / 4 : count / vertsPerPrim;
            for (std::size_t i = 0; i < itemCount && m_items.size() < MAX_BATCH_SIZE; ++i) {
                SpriteBatchItem vi;
                vi.texture = texture;
                vi.sortKey = texture ? reinterpret_cast<std::uintptr_t>(texture) : 0;

                std::size_t offset = i * vertsPerPrim;
                std::size_t copyCount = std::min(static_cast<std::size_t>(4), count - offset);
                for (std::size_t v = 0; v < copyCount && v < 4; ++v) {
                    vi.vertices[v] = vertices[offset + v];
                }
                m_items.push_back(vi);
            }
        }

        void SpriteBatch::flush() {
            if (!m_began || m_items.empty()) return;

            if (m_textureSorting && m_items.size() > 1) {
                std::stable_sort(m_items.begin(), m_items.end(),
                    [](const SpriteBatchItem& a, const SpriteBatchItem& b) {
                        return a.sortKey < b.sortKey;
                    });
            }

            std::vector<SpriteBatchItem> currentBatch;
            currentBatch.reserve(MAX_BATCH_SIZE);

            const sf::Texture* currentTexture = nullptr;

            for (auto& item : m_items) {
                if (currentTexture != item.texture && !currentBatch.empty()) {
                    flushBatch(currentBatch);
                    currentBatch.clear();
                }
                currentTexture = item.texture;
                currentBatch.push_back(item);
            }

            if (!currentBatch.empty()) {
                flushBatch(currentBatch);
            }

            m_items.clear();
        }

        void SpriteBatch::end() {
            if (!m_began) return;
            flush();
            m_began = false;
        }

        void SpriteBatch::setTextureSorting(bool enabled) {
            m_textureSorting = enabled;
        }

        bool SpriteBatch::isTextureSortingEnabled() const {
            return m_textureSorting;
        }

        std::size_t SpriteBatch::getBatchCount() const {
            return m_batchCount;
        }

        std::size_t SpriteBatch::getVertexCount() const {
            return m_totalVertices;
        }

        void SpriteBatch::resetStats() {
            m_batchCount = 0;
            m_totalVertices = 0;
        }

        void SpriteBatch::setPrimitiveType(PrimitiveType type) {
            m_primitiveType = toSFML(type);
        }

        sf::PrimitiveType SpriteBatch::getPrimitiveType() const {
            return m_primitiveType;
        }

        void SpriteBatch::flushBatch(const std::vector<SpriteBatchItem>& batch) {
            if (batch.empty() || !m_target) return;

            sf::RenderStates states;
            states.blendMode = m_blendState.toSFML();
            if (batch[0].texture) {
                states.texture = batch[0].texture;
            }

            sf::VertexArray vertexArray(m_primitiveType);
            std::size_t totalVerts = batch.size() * 4;
            vertexArray.resize(totalVerts);

            std::size_t idx = 0;
            for (const auto& item : batch) {
                for (int v = 0; v < 4; ++v) {
                    vertexArray[idx++] = item.vertices[v];
                }
            }

            m_target->draw(vertexArray, states);
            ++m_batchCount;
            m_totalVertices += totalVerts;
        }

        void SpriteBatch::generateVertexFromSprite(const sf::Sprite& sprite, sf::Vertex* outVertices) {
            sf::FloatRect bounds = sprite.getGlobalBounds();
            sf::IntRect texRect = sprite.getTextureRect();
            sf::Vector2u texSize = sprite.getTexture() ? sprite.getTexture()->getSize() : sf::Vector2u(1, 1);

            float left = bounds.left;
            float top = bounds.top;
            float right = bounds.left + bounds.width;
            float bottom = bounds.top + bounds.height;

            float texLeft = static_cast<float>(texRect.left) / static_cast<float>(texSize.x);
            float texTop = static_cast<float>(texRect.top) / static_cast<float>(texSize.y);
            float texRight = static_cast<float>(texRect.left + texRect.width) / static_cast<float>(texSize.x);
            float texBottom = static_cast<float>(texRect.top + texRect.height) / static_cast<float>(texSize.y);

            sf::Color sfmlColor = sprite.getColor();

            outVertices[0] = sf::Vertex(sf::Vector2f(left, top), sfmlColor, sf::Vector2f(texLeft, texTop));
            outVertices[1] = sf::Vertex(sf::Vector2f(right, top), sfmlColor, sf::Vector2f(texRight, texTop));
            outVertices[2] = sf::Vertex(sf::Vector2f(right, bottom), sfmlColor, sf::Vector2f(texRight, texBottom));
            outVertices[3] = sf::Vertex(sf::Vector2f(left, bottom), sfmlColor, sf::Vector2f(texLeft, texBottom));
        }

        void SpriteBatch::generateVertices(const sf::Texture& texture,
                                           const sf::Vector2f& position,
                                           const sf::IntRect* sourceRect,
                                           const sf::FloatRect* destRect,
                                           const Color& color,
                                           float rotation,
                                           const sf::Vector2f& scale,
                                           const sf::Vector2f& origin,
                                           sf::Vertex* outVertices)
        {
            sf::Vector2u texSize = texture.getSize();
            sf::Color sfmlColor = color.toSFML();

            float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;
            float texLeft = 0.f, texTop = 0.f, texRight = 1.f, texBottom = 1.f;

            if (sourceRect) {
                texLeft = static_cast<float>(sourceRect->left) / static_cast<float>(texSize.x);
                texTop = static_cast<float>(sourceRect->top) / static_cast<float>(texSize.y);
                texRight = static_cast<float>(sourceRect->left + sourceRect->width) / static_cast<float>(texSize.x);
                texBottom = static_cast<float>(sourceRect->top + sourceRect->height) / static_cast<float>(texSize.y);

                if (destRect) {
                    left = destRect->left;
                    top = destRect->top;
                    right = destRect->left + destRect->width;
                    bottom = destRect->top + destRect->height;
                } else {
                    left = position.x;
                    top = position.y;
                    right = position.x + static_cast<float>(sourceRect->width) * scale.x;
                    bottom = position.y + static_cast<float>(sourceRect->height) * scale.y;
                }
            } else {
                if (destRect) {
                    left = destRect->left;
                    top = destRect->top;
                    right = destRect->left + destRect->width;
                    bottom = destRect->top + destRect->height;
                } else {
                    left = position.x;
                    top = position.y;
                    right = position.x + static_cast<float>(texSize.x) * scale.x;
                    bottom = position.y + static_cast<float>(texSize.y) * scale.y;
                }
                texRight = 1.f;
                texBottom = 1.f;
            }

            if (rotation != 0.f) {
                float cosR = std::cos(rotation * 3.14159265f / 180.f);
                float sinR = std::sin(rotation * 3.14159265f / 180.f);
                float cx = origin.x * scale.x;
                float cy = origin.y * scale.y;

                sf::Vector2f corners[4] = {
                    sf::Vector2f(left - position.x - cx, top - position.y - cy),
                    sf::Vector2f(right - position.x - cx, top - position.y - cy),
                    sf::Vector2f(right - position.x - cx, bottom - position.y - cy),
                    sf::Vector2f(left - position.x - cx, bottom - position.y - cy)
                };

                for (int i = 0; i < 4; ++i) {
                    float rx = corners[i].x * cosR - corners[i].y * sinR;
                    float ry = corners[i].x * sinR + corners[i].y * cosR;
                    corners[i] = sf::Vector2f(rx + position.x, ry + position.y);
                }

                outVertices[0] = sf::Vertex(corners[0], sfmlColor, sf::Vector2f(texLeft, texTop));
                outVertices[1] = sf::Vertex(corners[1], sfmlColor, sf::Vector2f(texRight, texTop));
                outVertices[2] = sf::Vertex(corners[2], sfmlColor, sf::Vector2f(texRight, texBottom));
                outVertices[3] = sf::Vertex(corners[3], sfmlColor, sf::Vector2f(texLeft, texBottom));
            } else {
                outVertices[0] = sf::Vertex(sf::Vector2f(left, top), sfmlColor, sf::Vector2f(texLeft, texTop));
                outVertices[1] = sf::Vertex(sf::Vector2f(right, top), sfmlColor, sf::Vector2f(texRight, texTop));
                outVertices[2] = sf::Vertex(sf::Vector2f(right, bottom), sfmlColor, sf::Vector2f(texRight, texBottom));
                outVertices[3] = sf::Vertex(sf::Vector2f(left, bottom), sfmlColor, sf::Vector2f(texLeft, texBottom));
            }
        }

    }
}
