#include "UIButton.h"
#include <algorithm>
#include <cmath>

namespace nebula {
    namespace ui {

        UIButton::UIButton()
            : UIWidget("Button")
            , m_state(ButtonState::Normal)
            , m_targetState(ButtonState::Normal)
            , m_cornerRadius(4.f)
            , m_autoSize(true)
            , m_hoverLerp(0.f)
            , m_scalePulse(0.f) {
            m_size.x = 120.f;
            m_size.y = 32.f;
            focusable = true;

            m_label = std::make_shared<UILabel>();
            m_label->setAutoSize(false);
            addChild(m_label);

            for (int i = 0; i < 4; ++i) {
                m_colors[i].bg = graphics::Color(200, 200, 200);
                m_colors[i].text = graphics::Color(0, 0, 0);
                m_colors[i].border = graphics::Color(150, 150, 150);
            }

            m_colors[0].bg = graphics::Color(60, 60, 60);
            m_colors[0].text = graphics::Color(220, 220, 220);
            m_colors[0].border = graphics::Color(80, 80, 80);

            m_colors[1].bg = graphics::Color(80, 80, 80);
            m_colors[1].text = graphics::Color(240, 240, 240);
            m_colors[1].border = graphics::Color(100, 100, 100);

            m_colors[2].bg = graphics::Color(40, 40, 40);
            m_colors[2].text = graphics::Color(200, 200, 200);
            m_colors[2].border = graphics::Color(60, 60, 60);

            m_colors[3].bg = graphics::Color(100, 100, 100);
            m_colors[3].text = graphics::Color(140, 140, 140);
            m_colors[3].border = graphics::Color(100, 100, 100);

            updateAppearance();
        }

        UIButton::UIButton(const std::string& text)
            : UIButton() {
            setText(text);
        }

        void UIButton::setText(const std::string& text) {
            m_label->setText(text);
            if (m_autoSize) {
                onLayout();
            }
        }

        void UIButton::setFont(sf::Font& font) {
            m_label->setFont(font);
        }

        void UIButton::setBackgroundColor(ButtonState state, const graphics::Color& color) {
            m_colors[static_cast<int>(state)].bg = color;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setTextColor(ButtonState state, const graphics::Color& color) {
            m_colors[static_cast<int>(state)].text = color;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setBorderColor(ButtonState state, const graphics::Color& color) {
            m_colors[static_cast<int>(state)].border = color;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setTexture(ButtonState state, const std::shared_ptr<sf::Texture>& texture) {
            m_colors[static_cast<int>(state)].texture = texture;
            if (m_state == state) updateAppearance();
        }

        void UIButton::setCornerRadius(float radius) {
            m_cornerRadius = radius;
        }

        void UIButton::setAutoSize(bool autoSize) {
            m_autoSize = autoSize;
        }

        graphics::Color UIButton::lerpColor(const graphics::Color& a, const graphics::Color& b, float t) const {
            return graphics::Color::lerp(a, b, t);
        }

        void UIButton::updateAppearance() {
            const auto& colors = m_colors[static_cast<int>(m_state)];

            graphics::Color currentBg = colors.bg;
            if (m_hoverLerp > 0.f && m_hoverLerp < 1.f) {
                const auto& fromColors = m_colors[static_cast<int>(ButtonState::Normal)];
                const auto& toColors = m_colors[static_cast<int>(ButtonState::Hovered)];
                currentBg = lerpColor(fromColors.bg, toColors.bg, m_hoverLerp);
            }

            m_background.setFillColor(currentBg.toSFML());
            m_background.setOutlineColor(colors.border.toSFML());
            m_background.setOutlineThickness(style.borderSize);
            m_background.setSize(m_size);

            if (colors.texture) {
                m_background.setTexture(colors.texture.get());
            } else {
                m_background.setTexture(nullptr);
            }

            updateLabelStyle();
        }

        void UIButton::updateLabelStyle() {
            const auto& colors = m_colors[static_cast<int>(m_state)];
            m_label->setColor(colors.text);
            m_label->setCharacterSize(style.fontSize);
            m_label->setAlignment(TextAlignment::Center);
            m_label->setPosition(0.f, 0.f);
            m_label->setSize(m_size.x, m_size.y);
        }

        void UIButton::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            states.transform.translate(m_position);

            float scale = 1.f + m_scalePulse * 0.05f;
            if (scale != 1.f) {
                sf::Vector2f center(m_size.x / 2.f, m_size.y / 2.f);
                states.transform.translate(center);
                states.transform.scale(scale, scale);
                states.transform.translate(-center);
            }

            target.draw(m_background, states);
            UIWidget::onRender(target, states);
        }

        void UIButton::onUpdate(float dt) {
            UIWidget::onUpdate(dt);

            if (m_state != m_targetState) {
                if (m_targetState == ButtonState::Hovered) {
                    m_hoverLerp = std::min(1.f, m_hoverLerp + dt * 8.f);
                    if (m_hoverLerp >= 1.f) {
                        m_state = m_targetState;
                        m_hoverLerp = 0.f;
                    }
                } else {
                    m_hoverLerp = std::max(0.f, m_hoverLerp - dt * 8.f);
                    if (m_hoverLerp <= 0.f) {
                        m_state = m_targetState;
                    }
                }
                updateAppearance();
            }

            if (m_scalePulse > 0.f) {
                m_scalePulse = std::max(0.f, m_scalePulse - dt * 6.f);
            }
        }

        void UIButton::onLayout() {
            if (m_autoSize && m_label) {
                sf::FloatRect textBounds = m_label->getTextBounds();
                m_size.x = textBounds.width + style.padding * 2.f + 16.f;
                m_size.y = textBounds.height + style.padding * 2.f + 8.f;
            }
            m_background.setSize(m_size);
            if (m_label) {
                m_label->setPosition(0.f, 0.f);
                m_label->setSize(m_size.x, m_size.y);
            }
            UIWidget::onLayout();
        }

        bool UIButton::onEvent(const sf::Event& event) {
            if (!enabled || !visible) return false;

            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
                bool hovered = containsPoint(mousePos);
                if (hovered && m_targetState != ButtonState::Pressed) {
                    m_targetState = ButtonState::Hovered;
                    m_hovered = true;
                    if (onHover) onHover();
                    if (onHovered) onHovered();
                } else if (!hovered && m_targetState != ButtonState::Pressed) {
                    m_targetState = ButtonState::Normal;
                    m_hovered = false;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                if (containsPoint(mousePos)) {
                    m_targetState = ButtonState::Pressed;
                    m_pressed = true;
                    m_scalePulse = 1.f;
                    if (onPressed) onPressed();
                    return true;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (m_pressed) {
                    m_pressed = false;
                    sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                    if (containsPoint(mousePos)) {
                        m_targetState = ButtonState::Hovered;
                        if (onClick) onClick();
                        if (onClicked) onClicked();
                    } else {
                        m_targetState = ButtonState::Normal;
                    }
                    if (onRelease) onRelease();
                    if (onReleased) onReleased();
                    return true;
                }
            }

            return UIWidget::onEvent(event);
        }

    }
}
