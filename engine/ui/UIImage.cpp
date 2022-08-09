#include "UIImage.h"
#include <algorithm>

namespace nebula {
    namespace ui {

        UIImage::UIImage()
            : UIWidget("Image")
            , flipX(false)
            , flipY(false)
            , preserveAspectRatio(true)
            , m_color(255, 255, 255)
            , m_scaleType(ScaleType::Stretch)
            , m_opacity(1.f)
            , m_dirty(true) {
            m_size.x = 100.f;
            m_size.y = 100.f;
        }

        void UIImage::setTexture(const std::shared_ptr<sf::Texture>& texture) {
            m_texture = texture;
            m_dirty = true;
        }

        void UIImage::setSourceRect(const sf::IntRect& rect) {
            m_sourceRect = rect;
            m_dirty = true;
        }

        void UIImage::setColor(const graphics::Color& color) {
            m_color = color;
            m_dirty = true;
        }

        void UIImage::setScaleType(ScaleType type) {
            m_scaleType = type;
            m_dirty = true;
        }

        void UIImage::setOpacity(float opacity) {
            m_opacity = std::max(0.f, std::min(1.f, opacity));
            m_dirty = true;
        }

        void UIImage::rebuildSprite() {
            if (!m_texture) return;

            m_sprite.setTexture(*m_texture);

            if (m_sourceRect.width > 0 && m_sourceRect.height > 0) {
                m_sprite.setTextureRect(m_sourceRect);
            } else {
                sf::Vector2u texSize = m_texture->getSize();
                m_sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(texSize.x), static_cast<int>(texSize.y)));
            }

            sf::Color sfColor = m_color.toSFML();
            sfColor.a = static_cast<uint8_t>(m_opacity * 255.f);
            m_sprite.setColor(sfColor);

            sf::FloatRect spriteBounds = m_sprite.getLocalBounds();
            float srcW = spriteBounds.width;
            float srcH = spriteBounds.height;

            if (srcW <= 0.f || srcH <= 0.f) return;

            switch (m_scaleType) {
                case ScaleType::Stretch:
                    m_sprite.setScale(m_size.x / srcW, m_size.y / srcH);
                    break;

                case ScaleType::Fill: {
                    float scale = std::max(m_size.x / srcW, m_size.y / srcH);
                    m_sprite.setScale(scale, scale);
                    break;
                }

                case ScaleType::Fit: {
                    float scale = std::min(m_size.x / srcW, m_size.y / srcH);
                    m_sprite.setScale(scale, scale);
                    break;
                }

                case ScaleType::Tile:
                    m_sprite.setScale(1.f, 1.f);
                    m_sprite.setTextureRect(sf::IntRect(
                        0, 0,
                        static_cast<int>(m_size.x),
                        static_cast<int>(m_size.y)
                    ));
                    m_texture->setRepeated(true);
                    break;

                case ScaleType::Center:
                    m_sprite.setScale(1.f, 1.f);
                    break;
            }

            sf::FloatRect scaledBounds = m_sprite.getGlobalBounds();
            m_sprite.setOrigin(0.f, 0.f);

            if (m_scaleType == ScaleType::Center) {
                m_sprite.setPosition(
                    (m_size.x - scaledBounds.width) / 2.f,
                    (m_size.y - scaledBounds.height) / 2.f
                );
            }

            if (flipX) {
                m_sprite.setScale(-std::abs(m_sprite.getScale().x), m_sprite.getScale().y);
            }
            if (flipY) {
                m_sprite.setScale(m_sprite.getScale().x, -std::abs(m_sprite.getScale().y));
            }

            m_dirty = false;
        }

        void UIImage::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            if (m_dirty) {
                rebuildSprite();
            }

            states.transform.translate(m_position);

            if (m_texture) {
                target.draw(m_sprite, states);
            }

            UIWidget::onRender(target, states);
        }

        void UIImage::onLayout() {
            if (m_dirty) {
                rebuildSprite();
            }
            UIWidget::onLayout();
        }

    }
}
