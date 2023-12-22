#include "Sprite.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace nebula {
    namespace graphics {

        Sprite::Sprite()
            : m_opacity(1.f)
            , m_tint(sf::Color::White)
            , m_size(0.f, 0.f)
            , m_flippedX(false)
            , m_flippedY(false)
            , m_ninePatchEnabled(false)
            , m_tilingEnabled(false)
            , m_tileSize(0.f, 0.f)
            , m_flipBookCurrent(0)
            , m_flipBookElapsed(0.f)
            , m_flipBookPlaying(false) {}

        Sprite::Sprite(const std::shared_ptr<sf::Texture>& texture)
            : m_texture(texture)
            , m_opacity(1.f)
            , m_tint(sf::Color::White)
            , m_size(0.f, 0.f)
            , m_flippedX(false)
            , m_flippedY(false)
            , m_ninePatchEnabled(false)
            , m_tilingEnabled(false)
            , m_tileSize(0.f, 0.f)
            , m_flipBookCurrent(0)
            , m_flipBookElapsed(0.f)
            , m_flipBookPlaying(false) {
            if (texture) {
                m_sprite.setTexture(*texture);
                m_textureRect = sf::IntRect(0, 0,
                    static_cast<int>(texture->getSize().x),
                    static_cast<int>(texture->getSize().y));
                m_size = sf::Vector2f(texture->getSize());
            }
        }

        Sprite::Sprite(const std::shared_ptr<sf::Texture>& texture, const sf::IntRect& textureRect)
            : m_texture(texture)
            , m_textureRect(textureRect)
            , m_opacity(1.f)
            , m_tint(sf::Color::White)
            , m_size(static_cast<float>(textureRect.width), static_cast<float>(textureRect.height))
            , m_flippedX(false)
            , m_flippedY(false)
            , m_ninePatchEnabled(false)
            , m_tilingEnabled(false)
            , m_tileSize(0.f, 0.f)
            , m_flipBookCurrent(0)
            , m_flipBookElapsed(0.f)
            , m_flipBookPlaying(false) {
            if (texture) {
                m_sprite.setTexture(*texture);
                m_sprite.setTextureRect(textureRect);
            }
        }

        void Sprite::setTexture(const std::shared_ptr<sf::Texture>& texture, bool resetRect) {
            m_texture = texture;
            if (texture) {
                m_sprite.setTexture(*texture, resetRect);
                if (resetRect) {
                    m_textureRect = sf::IntRect(0, 0,
                        static_cast<int>(texture->getSize().x),
                        static_cast<int>(texture->getSize().y));
                    m_size = sf::Vector2f(texture->getSize());
                }
            }
        }

        void Sprite::setTextureRect(const sf::IntRect& rect) {
            m_textureRect = rect;
            m_size = sf::Vector2f(static_cast<float>(rect.width), static_cast<float>(rect.height));
            m_sprite.setTextureRect(rect);
        }

        void Sprite::unsetTexture() {
            m_texture.reset();
        }

        void Sprite::setPosition(const sf::Vector2f& position) {
            m_sprite.setPosition(position);
        }

        void Sprite::setPosition(float x, float y) {
            m_sprite.setPosition(x, y);
        }

        void Sprite::setRotation(float angle) {
            m_sprite.setRotation(angle);
        }

        void Sprite::setScale(const sf::Vector2f& scale) {
            m_sprite.setScale(scale);
        }

        void Sprite::setScale(float x, float y) {
            m_sprite.setScale(x, y);
        }

        void Sprite::setOrigin(const sf::Vector2f& origin) {
            m_sprite.setOrigin(origin);
        }

        void Sprite::setOrigin(float x, float y) {
            m_sprite.setOrigin(x, y);
        }

        void Sprite::setColor(const sf::Color& color) {
            m_sprite.setColor(color);
        }

        void Sprite::move(const sf::Vector2f& offset) {
            m_sprite.move(offset);
        }

        void Sprite::move(float x, float y) {
            m_sprite.move(x, y);
        }

        void Sprite::rotate(float angle) {
            m_sprite.rotate(angle);
        }

        void Sprite::scale(const sf::Vector2f& factor) {
            m_sprite.scale(factor);
        }

        void Sprite::scale(float x, float y) {
            m_sprite.scale(x, y);
        }

        void Sprite::setOpacity(float opacity) {
            m_opacity = std::clamp(opacity, 0.f, 1.f);
            applyColor();
        }

        void Sprite::setTint(const sf::Color& tint) {
            m_tint = tint;
            applyColor();
        }

        std::shared_ptr<sf::Texture> Sprite::getTexture() const {
            return m_texture;
        }

        const sf::IntRect& Sprite::getTextureRect() const {
            return m_textureRect;
        }

        sf::Vector2f Sprite::getPosition() const {
            return m_sprite.getPosition();
        }

        float Sprite::getRotation() const {
            return m_sprite.getRotation();
        }

        sf::Vector2f Sprite::getScale() const {
            return m_sprite.getScale();
        }

        sf::Vector2f Sprite::getOrigin() const {
            return m_sprite.getOrigin();
        }

        sf::Color Sprite::getColor() const {
            return m_sprite.getColor();
        }

        float Sprite::getOpacity() const {
            return m_opacity;
        }

        sf::Color Sprite::getTint() const {
            return m_tint;
        }

        sf::FloatRect Sprite::getGlobalBounds() const {
            return m_sprite.getGlobalBounds();
        }

        sf::FloatRect Sprite::getLocalBounds() const {
            return m_sprite.getLocalBounds();
        }

        void Sprite::flipX(bool flip) {
            m_flippedX = flip;
            sf::Vector2f scale = m_sprite.getScale();
            scale.x = std::abs(scale.x) * (flip ? -1.f : 1.f);
            m_sprite.setScale(scale);
        }

        void Sprite::flipY(bool flip) {
            m_flippedY = flip;
            sf::Vector2f scale = m_sprite.getScale();
            scale.y = std::abs(scale.y) * (flip ? -1.f : 1.f);
            m_sprite.setScale(scale);
        }

        bool Sprite::isFlippedX() const {
            return m_flippedX;
        }

        bool Sprite::isFlippedY() const {
            return m_flippedY;
        }

        bool Sprite::containsPoint(const sf::Vector2f& point) const {
            return getGlobalBounds().contains(point);
        }

        sf::Transform Sprite::getTransform() const {
            return m_sprite.getTransform();
        }

        sf::Transform Sprite::getInverseTransform() const {
            return m_sprite.getInverseTransform();
        }

        void Sprite::setSize(const sf::Vector2f& size) {
            if (m_textureRect.width > 0 && m_textureRect.height > 0) {
                m_sprite.setScale(
                    size.x / static_cast<float>(m_textureRect.width),
                    size.y / static_cast<float>(m_textureRect.height)
                );
            }
            m_size = size;
        }

        sf::Vector2f Sprite::getSize() const {
            if (m_size.x > 0.f && m_size.y > 0.f) {
                return m_size;
            }
            sf::FloatRect bounds = getLocalBounds();
            return sf::Vector2f(bounds.width, bounds.height);
        }

        void Sprite::fitToRect(const sf::FloatRect& rect, bool keepAspectRatio) {
            setPosition(rect.left, rect.top);

            if (keepAspectRatio && m_textureRect.width > 0 && m_textureRect.height > 0) {
                float scaleX = rect.width / static_cast<float>(m_textureRect.width);
                float scaleY = rect.height / static_cast<float>(m_textureRect.height);
                float scale = std::min(scaleX, scaleY);
                setScale(scale, scale);
            } else {
                setSize(sf::Vector2f(rect.width, rect.height));
            }
        }

        sf::Sprite& Sprite::getSFMLSprite() {
            return m_sprite;
        }

        const sf::Sprite& Sprite::getSFMLSprite() const {
            return m_sprite;
        }

        Sprite::operator sf::Sprite() const {
            return m_sprite;
        }

        void Sprite::enableNinePatch(const NinePatchSlice& slice) {
            m_ninePatchEnabled = true;
            m_ninePatchSlice = slice;
        }

        void Sprite::disableNinePatch() {
            m_ninePatchEnabled = false;
        }

        bool Sprite::isNinePatchEnabled() const {
            return m_ninePatchEnabled;
        }

        NinePatchSlice Sprite::getNinePatchSlice() const {
            return m_ninePatchSlice;
        }

        void Sprite::setTiling(bool enabled, const sf::Vector2f& tileSize) {
            m_tilingEnabled = enabled;
            m_tileSize = tileSize;
        }

        bool Sprite::isTilingEnabled() const {
            return m_tilingEnabled;
        }

        void Sprite::setFlipBook(const std::vector<FlipBookFrame>& frames) {
            m_flipBookFrames = frames;
            m_flipBookCurrent = 0;
            m_flipBookElapsed = 0.f;
            m_flipBookPlaying = !frames.empty();
            if (!frames.empty()) {
                setTextureRect(frames[0].rect);
            }
        }

        void Sprite::clearFlipBook() {
            m_flipBookFrames.clear();
            m_flipBookCurrent = 0;
            m_flipBookElapsed = 0.f;
            m_flipBookPlaying = false;
        }

        void Sprite::updateFlipBook(float dt) {
            if (!m_flipBookPlaying || m_flipBookFrames.empty()) return;

            m_flipBookElapsed += dt;
            const auto& frame = m_flipBookFrames[m_flipBookCurrent];

            if (m_flipBookElapsed >= frame.duration) {
                m_flipBookElapsed -= frame.duration;
                m_flipBookCurrent = (m_flipBookCurrent + 1) % m_flipBookFrames.size();
                setTextureRect(m_flipBookFrames[m_flipBookCurrent].rect);
            }
        }

        void Sprite::setFlipBookFrame(size_t index) {
            if (index < m_flipBookFrames.size()) {
                m_flipBookCurrent = index;
                m_flipBookElapsed = 0.f;
                setTextureRect(m_flipBookFrames[index].rect);
            }
        }

        size_t Sprite::getFlipBookFrame() const {
            return m_flipBookCurrent;
        }

        size_t Sprite::getFlipBookFrameCount() const {
            return m_flipBookFrames.size();
        }

        void Sprite::drawNinePatch(sf::RenderTarget& target, const sf::RenderStates& states) const {
            if (!m_texture) return;

            float w = m_size.x;
            float h = m_size.y;
            const sf::IntRect& texRect = m_textureRect;

            int left = m_ninePatchSlice.left;
            int right = m_ninePatchSlice.right;
            int top = m_ninePatchSlice.top;
            int bottom = m_ninePatchSlice.bottom;

            float srcLeft = static_cast<float>(left);
            float srcRight = static_cast<float>(texRect.width - right);
            float srcTop = static_cast<float>(top);
            float srcBottom = static_cast<float>(texRect.height - bottom);

            float dstLeft = srcLeft;
            float dstRight = w - static_cast<float>(right);
            float dstTop = srcTop;
            float dstBottom = h - static_cast<float>(bottom);

            sf::VertexArray vertices(sf::PrimitiveType::Triangles, 54);

            float srcX[3] = { 0.f, srcLeft, srcRight };
            float srcY[3] = { 0.f, srcTop, srcBottom };
            float dstX[3] = { 0.f, dstLeft, dstRight };
            float dstY[3] = { 0.f, dstTop, dstBottom };

            float texLeft = static_cast<float>(texRect.left);
            float texTop = static_cast<float>(texRect.top);

            int vi = 0;
            for (int gy = 0; gy < 3; ++gy) {
                for (int gx = 0; gx < 3; ++gx) {
                    float x0 = dstX[gx];
                    float y0 = dstY[gy];
                    float x1 = dstX[gx + 1];
                    float y1 = dstY[gy + 1];
                    float u0 = (texLeft + srcX[gx]) / static_cast<float>(m_texture->getSize().x);
                    float v0 = (texTop + srcY[gy]) / static_cast<float>(m_texture->getSize().y);
                    float u1 = (texLeft + srcX[gx + 1]) / static_cast<float>(m_texture->getSize().x);
                    float v1 = (texTop + srcY[gy + 1]) / static_cast<float>(m_texture->getSize().y);

                    sf::Vertex v0v(sf::Vector2f(x0, y0), sf::Vector2f(u0, v0));
                    sf::Vertex v1v(sf::Vector2f(x1, y0), sf::Vector2f(u1, v0));
                    sf::Vertex v2v(sf::Vector2f(x0, y1), sf::Vector2f(u0, v1));
                    sf::Vertex v3v(sf::Vector2f(x1, y1), sf::Vector2f(u1, v1));

                    vertices[vi++] = v0v;
                    vertices[vi++] = v1v;
                    vertices[vi++] = v2v;
                    vertices[vi++] = v1v;
                    vertices[vi++] = v3v;
                    vertices[vi++] = v2v;
                }
            }

            sf::RenderStates rs = states;
            rs.texture = m_texture.get();
            target.draw(vertices, rs);
        }

        void Sprite::drawTiled(sf::RenderTarget& target, const sf::RenderStates& states) const {
            if (!m_texture) return;

            float tileW = m_tileSize.x > 0.f ? m_tileSize.x : static_cast<float>(m_textureRect.width);
            float tileH = m_tileSize.y > 0.f ? m_tileSize.y : static_cast<float>(m_textureRect.height);
            float totalW = m_size.x > 0.f ? m_size.x : tileW;
            float totalH = m_size.y > 0.f ? m_size.y : tileH;

            int tilesX = static_cast<int>(std::ceil(totalW / tileW));
            int tilesY = static_cast<int>(std::ceil(totalH / tileH));

            float texLeft = static_cast<float>(m_textureRect.left) / static_cast<float>(m_texture->getSize().x);
            float texTop = static_cast<float>(m_textureRect.top) / static_cast<float>(m_texture->getSize().y);
            float texW = static_cast<float>(m_textureRect.width) / static_cast<float>(m_texture->getSize().x);
            float texH = static_cast<float>(m_textureRect.height) / static_cast<float>(m_texture->getSize().y);

            sf::RenderStates rs = states;
            rs.texture = m_texture.get();

            for (int ty = 0; ty < tilesY; ++ty) {
                for (int tx = 0; tx < tilesX; ++tx) {
                    float x = static_cast<float>(tx) * tileW;
                    float y = static_cast<float>(ty) * tileH;
                    float dw = std::min(tileW, totalW - x);
                    float dh = std::min(tileH, totalH - y);
                    float uw = dw / tileW * texW;
                    float vh = dh / tileH * texH;

                    sf::VertexArray quad(sf::PrimitiveType::TriangleStrip, 4);
                    quad[0] = sf::Vertex(sf::Vector2f(x, y), sf::Vector2f(texLeft, texTop));
                    quad[1] = sf::Vertex(sf::Vector2f(x + dw, y), sf::Vector2f(texLeft + uw, texTop));
                    quad[2] = sf::Vertex(sf::Vector2f(x, y + dh), sf::Vector2f(texLeft, texTop + vh));
                    quad[3] = sf::Vertex(sf::Vector2f(x + dw, y + dh), sf::Vector2f(texLeft + uw, texTop + vh));
                    target.draw(quad, rs);
                }
            }
        }

        void Sprite::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
            if (m_ninePatchEnabled) {
                drawNinePatch(target, states);
            } else if (m_tilingEnabled) {
                drawTiled(target, states);
            } else {
                target.draw(m_sprite, states);
            }
        }

        void Sprite::applyColor() {
            sf::Color combined(
                static_cast<sf::Uint8>(static_cast<float>(m_tint.r) * m_opacity),
                static_cast<sf::Uint8>(static_cast<float>(m_tint.g) * m_opacity),
                static_cast<sf::Uint8>(static_cast<float>(m_tint.b) * m_opacity),
                static_cast<sf::Uint8>(static_cast<float>(m_tint.a) * m_opacity)
            );
            m_sprite.setColor(combined);
        }

    }
}
