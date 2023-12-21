#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

namespace nebula {
    namespace graphics {

        struct NinePatchSlice {
            int left;
            int right;
            int top;
            int bottom;

            NinePatchSlice() : left(0), right(0), top(0), bottom(0) {}
            NinePatchSlice(int l, int r, int t, int b) : left(l), right(r), top(t), bottom(b) {}
        };

        struct FlipBookFrame {
            sf::IntRect rect;
            float duration;

            FlipBookFrame() : rect(0, 0, 0, 0), duration(0.1f) {}
            FlipBookFrame(const sf::IntRect& r, float d) : rect(r), duration(d) {}
        };

        class Sprite {
        public:
            Sprite();
            explicit Sprite(const std::shared_ptr<sf::Texture>& texture);
            Sprite(const std::shared_ptr<sf::Texture>& texture, const sf::IntRect& textureRect);
            ~Sprite() = default;

            void setTexture(const std::shared_ptr<sf::Texture>& texture, bool resetRect = false);
            void setTextureRect(const sf::IntRect& rect);
            void unsetTexture();

            void setPosition(const sf::Vector2f& position);
            void setPosition(float x, float y);
            void setRotation(float angle);
            void setScale(const sf::Vector2f& scale);
            void setScale(float x, float y);
            void setOrigin(const sf::Vector2f& origin);
            void setOrigin(float x, float y);
            void setColor(const sf::Color& color);

            void move(const sf::Vector2f& offset);
            void move(float x, float y);
            void rotate(float angle);
            void scale(const sf::Vector2f& factor);
            void scale(float x, float y);

            void setOpacity(float opacity);
            void setTint(const sf::Color& tint);

            std::shared_ptr<sf::Texture> getTexture() const;
            const sf::IntRect& getTextureRect() const;
            sf::Vector2f getPosition() const;
            float getRotation() const;
            sf::Vector2f getScale() const;
            sf::Vector2f getOrigin() const;
            sf::Color getColor() const;
            float getOpacity() const;
            sf::Color getTint() const;

            sf::FloatRect getGlobalBounds() const;
            sf::FloatRect getLocalBounds() const;

            void flipX(bool flip);
            void flipY(bool flip);
            bool isFlippedX() const;
            bool isFlippedY() const;

            bool containsPoint(const sf::Vector2f& point) const;

            sf::Transform getTransform() const;
            sf::Transform getInverseTransform() const;

            void setSize(const sf::Vector2f& size);
            sf::Vector2f getSize() const;

            void fitToRect(const sf::FloatRect& rect, bool keepAspectRatio = true);

            void enableNinePatch(const NinePatchSlice& slice);
            void disableNinePatch();
            bool isNinePatchEnabled() const;
            NinePatchSlice getNinePatchSlice() const;

            void setTiling(bool enabled, const sf::Vector2f& tileSize = sf::Vector2f(0.f, 0.f));
            bool isTilingEnabled() const;

            void setFlipBook(const std::vector<FlipBookFrame>& frames);
            void clearFlipBook();
            void updateFlipBook(float dt);
            void setFlipBookFrame(size_t index);
            size_t getFlipBookFrame() const;
            size_t getFlipBookFrameCount() const;

            void draw(sf::RenderTarget& target, const sf::RenderStates& states = sf::RenderStates::Default) const;

            sf::Sprite& getSFMLSprite();
            const sf::Sprite& getSFMLSprite() const;

            operator sf::Sprite() const;

        private:
            sf::Sprite m_sprite;
            std::shared_ptr<sf::Texture> m_texture;
            sf::IntRect m_textureRect;
            float m_opacity;
            sf::Color m_tint;
            sf::Vector2f m_size;
            bool m_flippedX;
            bool m_flippedY;

            bool m_ninePatchEnabled;
            NinePatchSlice m_ninePatchSlice;

            bool m_tilingEnabled;
            sf::Vector2f m_tileSize;

            std::vector<FlipBookFrame> m_flipBookFrames;
            size_t m_flipBookCurrent;
            float m_flipBookElapsed;
            bool m_flipBookPlaying;

            void applyColor();
            void drawNinePatch(sf::RenderTarget& target, const sf::RenderStates& states) const;
            void drawTiled(sf::RenderTarget& target, const sf::RenderStates& states) const;
        };

    }
}
