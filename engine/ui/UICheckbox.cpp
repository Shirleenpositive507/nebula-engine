#include "UICheckbox.h"
#include <SFML/Graphics/ConvexShape.hpp>

namespace nebula {
    namespace ui {

        UICheckbox::UICheckbox()
            : UIWidget("Checkbox")
            , checkmarkStyle(CheckmarkStyle::Tick)
            , m_checked(false)
            , m_checkColor(41, 128, 185)
            , m_uncheckColor(60, 60, 60)
            , m_boxSize(18.f) {
            m_size.x = 120.f;
            m_size.y = 24.f;

            m_label = std::make_shared<UILabel>();
            m_label->setAutoSize(false);
            addChild(m_label);

            m_box.setSize(sf::Vector2f(m_boxSize, m_boxSize));
            m_box.setOutlineThickness(1.f);
            m_box.setOutlineColor(sf::Color(120, 120, 120));
        }

        UICheckbox::UICheckbox(const std::string& text)
            : UICheckbox() {
            setText(text);
        }

        void UICheckbox::setChecked(bool checked) {
            if (m_checked != checked) {
                m_checked = checked;
                if (m_checked) {
                    if (onChecked) onChecked();
                } else {
                    if (onUnchecked) onUnchecked();
                }
                if (onToggled) onToggled();
            }
        }

        bool UICheckbox::isChecked() const {
            return m_checked;
        }

        void UICheckbox::toggle() {
            setChecked(!m_checked);
        }

        void UICheckbox::setText(const std::string& text) {
            m_label->setText(text);
        }

        void UICheckbox::setCheckColor(const graphics::Color& color) {
            m_checkColor = color;
        }

        void UICheckbox::setUncheckColor(const graphics::Color& color) {
            m_uncheckColor = color;
        }

        void UICheckbox::setCheckmarkStyle(CheckmarkStyle style) {
            checkmarkStyle = style;
        }

        void UICheckbox::drawCheckmark(sf::RenderTarget& target, sf::RenderStates states) const {
            sf::Color checkColor = m_checkColor.toSFML();
            float half = m_boxSize / 2.f;
            float quarter = m_boxSize / 4.f;
            float inset = 3.f;

            switch (checkmarkStyle) {
                case CheckmarkStyle::Tick: {
                    sf::ConvexShape tick;
                    tick.setPointCount(3);
                    tick.setPoint(0, sf::Vector2f(inset, half));
                    tick.setPoint(1, sf::Vector2f(half, m_boxSize - inset));
                    tick.setPoint(2, sf::Vector2f(m_boxSize - inset, inset));
                    tick.setFillColor(sf::Color::Transparent);
                    tick.setOutlineThickness(2.f);
                    tick.setOutlineColor(checkColor);
                    target.draw(tick, states);
                    break;
                }
                case CheckmarkStyle::Cross: {
                    sf::VertexArray cross(sf::Lines, 4);
                    cross[0].position = sf::Vector2f(inset, inset);
                    cross[0].color = checkColor;
                    cross[1].position = sf::Vector2f(m_boxSize - inset, m_boxSize - inset);
                    cross[1].color = checkColor;
                    cross[2].position = sf::Vector2f(m_boxSize - inset, inset);
                    cross[2].color = checkColor;
                    cross[3].position = sf::Vector2f(inset, m_boxSize - inset);
                    cross[3].color = checkColor;
                    target.draw(cross, states);
                    break;
                }
                case CheckmarkStyle::Fill: {
                    sf::ConvexShape fill;
                    fill.setPointCount(4);
                    fill.setPoint(0, sf::Vector2f(inset, inset));
                    fill.setPoint(1, sf::Vector2f(m_boxSize - inset, inset));
                    fill.setPoint(2, sf::Vector2f(m_boxSize - inset, m_boxSize - inset));
                    fill.setPoint(3, sf::Vector2f(inset, m_boxSize - inset));
                    fill.setFillColor(checkColor);
                    target.draw(fill, states);
                    break;
                }
            }
        }

        void UICheckbox::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            states.transform.translate(m_position);

            m_box.setFillColor(m_uncheckColor.toSFML());
            if (!enabled) {
                m_box.setFillColor(sf::Color(80, 80, 80));
                m_box.setOutlineColor(sf::Color(80, 80, 80));
            } else if (m_hovered) {
                m_box.setOutlineColor(sf::Color(180, 180, 180));
            } else {
                m_box.setOutlineColor(sf::Color(120, 120, 120));
            }

            target.draw(m_box, states);

            if (m_checked) {
                drawCheckmark(target, states);
            }

            UIWidget::onRender(target, states);
        }

        bool UICheckbox::onEvent(const sf::Event& event) {
            if (!enabled || !visible) return false;

            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
                m_hovered = containsPoint(mousePos);
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                if (containsPoint(mousePos)) {
                    toggle();
                    if (onClick) onClick();
                    return true;
                }
            }

            return UIWidget::onEvent(event);
        }

    }
}
