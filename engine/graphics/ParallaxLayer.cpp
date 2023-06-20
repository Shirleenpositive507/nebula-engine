#include "ParallaxLayer.h"

namespace nebula {

    void ParallaxLayer::setTexture(const std::string& path) {
        auto tex = std::make_shared<sf::Texture>();
        if (tex->loadFromFile(path)) {
            m_texture = tex;
            m_sprite.setTexture(*m_texture);
        }
    }

    void ParallaxLayer::setScrollFactor(float factor) {
        m_scrollFactor.x = factor;
        m_scrollFactor.y = factor;
    }

    float ParallaxLayer::getScrollFactor() const {
        return m_scrollFactor.x;
    }

    void ParallaxLayer::setSpeed(const sf::Vector2f& speed) {
        m_speed = speed;
    }

    void ParallaxLayer::setTiling(bool tileX, bool tileY) {
        m_tileX = tileX;
        m_tileY = tileY;
        if (m_texture) {
            m_texture->setRepeated(tileX || tileY);
        }
    }

    void ParallaxLayer::update(float dt) {
        m_offset += m_speed * dt;
    }

    void ParallaxLayer::render(sf::RenderTarget& target, const sf::Vector2f& cameraPos) {
        if (!m_texture) return;

        sf::Vector2f pos = cameraPos * m_scrollFactor + m_offset;
        sf::Vector2u texSize = m_texture->getSize();
        sf::Vector2u winSize = target.getSize();

        if (m_tileX || m_tileY) {
            sf::Vector2f scale(
                m_tileX ? (float)winSize.x / texSize.x : 1.0f,
                m_tileY ? (float)winSize.y / texSize.y : 1.0f
            );
            m_sprite.setScale(scale);

            sf::Vector2f finalPos(
                m_tileX ? std::fmod(pos.x, texSize.x * scale.x) - texSize.x * scale.x : pos.x,
                m_tileY ? std::fmod(pos.y, texSize.y * scale.y) - texSize.y * scale.y : pos.y
            );

            if (m_tileX) {
                int tilesNeeded = std::ceil((float)winSize.x / (texSize.x * scale.x)) + 2;
                for (int i = 0; i < tilesNeeded; i++) {
                    m_sprite.setPosition(finalPos.x + i * texSize.x * scale.x, finalPos.y);
                    target.draw(m_sprite);
                }
            } else {
                m_sprite.setPosition(finalPos);
                target.draw(m_sprite);
            }
        } else {
            m_sprite.setPosition(pos);
            target.draw(m_sprite);
        }
    }
}
