#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

namespace nebula {
    class ParallaxLayer {
    public:
        void setTexture(const std::string& path);
        void setScrollFactor(float factor);
        float getScrollFactor() const;
        void setSpeed(const sf::Vector2f& speed);
        void setTiling(bool tileX, bool tileY);
        void update(float dt);
        void render(sf::RenderTarget& target, const sf::Vector2f& cameraPos);
    private:
        std::shared_ptr<sf::Texture> m_texture;
        sf::Sprite m_sprite;
        sf::Vector2f m_offset;
        sf::Vector2f m_speed;
        sf::Vector2f m_scrollFactor{1.0f, 1.0f};
        bool m_tileX = true, m_tileY = false;
    };
}
