#include "UIPanel.h"
#include <SFML/Graphics/View.hpp>
#include <algorithm>
#include <cmath>

namespace nebula {
    namespace ui {

        UIPanel::UIPanel()
            : UIWidget("Panel")
            , scrollHorizontal(false)
            , scrollVertical(true)
            , autoScroll(false)
            , clipChildren(true)
            , m_scrollPosition(0.f, 0.f)
            , m_scrollSize(0.f, 0.f)
            , m_bgColor(50, 50, 50)
            , m_borderSize(0.f)
            , m_borderColor(80, 80, 80)
            , m_cornerRadius(0.f) {
            m_size.x = 300.f;
            m_size.y = 200.f;
        }

        void UIPanel::setScrollPosition(float x, float y) {
            m_scrollPosition.x = x;
            m_scrollPosition.y = y;
            if (m_scrollPosition.x < 0.f) m_scrollPosition.x = 0.f;
            if (m_scrollPosition.y < 0.f) m_scrollPosition.y = 0.f;
            if (m_scrollSize.x > 0.f && m_scrollPosition.x > m_scrollSize.x - m_size.x)
                m_scrollPosition.x = std::max(0.f, m_scrollSize.x - m_size.x);
            if (m_scrollSize.y > 0.f && m_scrollPosition.y > m_scrollSize.y - m_size.y)
                m_scrollPosition.y = std::max(0.f, m_scrollSize.y - m_size.y);
        }

        sf::Vector2f UIPanel::getScrollPosition() const {
            return m_scrollPosition;
        }

        void UIPanel::setScrollSize(float w, float h) {
            m_scrollSize.x = w;
            m_scrollSize.y = h;
        }

        sf::Vector2f UIPanel::getScrollSize() const {
            return m_scrollSize;
        }

        void UIPanel::scrollTo(std::shared_ptr<UIWidget> child) {
            if (!child) return;
            sf::FloatRect childBounds = child->getBounds();
            setScrollPosition(
                childBounds.left - m_size.x / 2.f + childBounds.width / 2.f,
                childBounds.top - m_size.y / 2.f + childBounds.height / 2.f
            );
        }

        void UIPanel::scrollToBottom() {
            m_scrollPosition.y = std::max(0.f, m_scrollSize.y - m_size.y);
        }

        bool UIPanel::hasScrollBar() const {
            return (scrollVertical && m_scrollSize.y > m_size.y) ||
                   (scrollHorizontal && m_scrollSize.x > m_size.x);
        }

        void UIPanel::setBackgroundColor(const graphics::Color& color) {
            m_bgColor = color;
            m_background.setFillColor(m_bgColor.toSFML());
        }

        void UIPanel::setBorder(float size, const graphics::Color& color) {
            m_borderSize = size;
            m_borderColor = color;
            m_background.setOutlineThickness(size);
            m_background.setOutlineColor(color.toSFML());
        }

        void UIPanel::setCornerRadius(float radius) {
            m_cornerRadius = radius;
        }

        void UIPanel::applyClipping(sf::RenderTarget& target, sf::RenderStates& states) const {
            sf::FloatRect clipRect(
                m_position.x + m_borderSize,
                m_position.y + m_borderSize,
                m_size.x - m_borderSize * 2.f,
                m_size.y - m_borderSize * 2.f
            );

            sf::View originalView = target.getView();
            sf::View clippedView = originalView;

            sf::FloatRect viewport(
                clipRect.left / static_cast<float>(target.getSize().x),
                clipRect.top / static_cast<float>(target.getSize().y),
                clipRect.width / static_cast<float>(target.getSize().x),
                clipRect.height / static_cast<float>(target.getSize().y)
            );

            clippedView.setViewport(viewport);
            target.setView(clippedView);
        }

        void UIPanel::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            states.transform.translate(m_position);

            m_background.setSize(m_size);
            target.draw(m_background, states);

            sf::View originalView;
            if (clipChildren) {
                originalView = target.getView();
                applyClipping(target, states);
            }

            sf::Transform childTransform = states.transform;
            childTransform.translate(-m_scrollPosition.x, -m_scrollPosition.y);

            for (auto& child : m_children) {
                if (child->visible) {
                    child->onRender(target, childTransform);
                }
            }

            if (clipChildren) {
                target.setView(originalView);
            }
        }

        void UIPanel::onUpdate(float dt) {
            UIWidget::onUpdate(dt);

            if (autoScroll && m_children.size() > 0) {
                float maxY = 0.f;
                for (auto& child : m_children) {
                    sf::FloatRect bounds = child->getBounds();
                    float bottom = bounds.top + bounds.height;
                    if (bottom > maxY) maxY = bottom;
                }
                m_scrollSize.y = maxY;
                if (m_scrollPosition.y < maxY - m_size.y) {
                    m_scrollPosition.y = maxY - m_size.y;
                }
            }
        }

        bool UIPanel::onEvent(const sf::Event& event) {
            if (!enabled || !visible) return false;

            if (event.type == sf::Event::MouseWheelScrolled && scrollVertical) {
                sf::Vector2f mousePos(
                    static_cast<float>(event.mouseWheelScroll.x),
                    static_cast<float>(event.mouseWheelScroll.y)
                );
                sf::FloatRect bounds = getGlobalBounds();
                bounds.left += m_borderSize;
                bounds.top += m_borderSize;
                bounds.width -= m_borderSize * 2.f;
                bounds.height -= m_borderSize * 2.f;
                if (bounds.contains(mousePos)) {
                    setScrollPosition(
                        m_scrollPosition.x,
                        m_scrollPosition.y - event.mouseWheelScroll.delta * 30.f
                    );
                    return true;
                }
            }

            sf::Event adjustedEvent = event;
            if (event.type == sf::Event::MouseMoved) {
                adjustedEvent.mouseMove.x += static_cast<int>(m_scrollPosition.x);
                adjustedEvent.mouseMove.y += static_cast<int>(m_scrollPosition.y);
            } else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased) {
                adjustedEvent.mouseButton.x += static_cast<int>(m_scrollPosition.x);
                adjustedEvent.mouseButton.y += static_cast<int>(m_scrollPosition.y);
            }

            return UIWidget::onEvent(adjustedEvent);
        }

    }
}
