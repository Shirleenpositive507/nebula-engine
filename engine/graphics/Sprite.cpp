#include "Sprite.h"
#include <algorithm>
#include <cmath>

namespace nebula {
    namespace graphics {

        Sprite::Sprite()
            : m_opacity(1.f)
            , m_tint(sf::Color::White)
            , m_size(0.f, 0.f)
            , m_flippedX(false)
            , m_flippedY(false) {}

        Sprite::Sprite(const std::shared_ptr<sf::Texture>& texture)
            : m_texture(texture)
            , m_opacity(1.f)
            , m_tint(sf::Color::White)
            , m_size(0.f, 0.f)
            , m_flippedX(false)
            , m_flippedY(false) {
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
            , m_flippedY(false) {
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

        void Sprite::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
            target.draw(m_sprite, states);
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
