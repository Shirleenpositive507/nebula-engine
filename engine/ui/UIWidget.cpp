#include "UIWidget.h"
#include <algorithm>

namespace nebula {
    namespace ui {

        UIWidget::UIWidget()
            : enabled(true)
            , visible(true)
            , focusable(false)
            , zOrder(0)
            , m_position(0.f, 0.f)
            , m_size(100.f, 30.f)
            , m_parent(nullptr)
            , m_hovered(false)
            , m_focused(false)
            , m_pressed(false) {}

        UIWidget::UIWidget(const std::string& name)
            : name(name)
            , enabled(true)
            , visible(true)
            , focusable(false)
            , zOrder(0)
            , m_position(0.f, 0.f)
            , m_size(100.f, 30.f)
            , m_parent(nullptr)
            , m_hovered(false)
            , m_focused(false)
            , m_pressed(false) {}

        void UIWidget::setPosition(float x, float y) {
            m_position.x = x;
            m_position.y = y;
        }

        void UIWidget::setPosition(const sf::Vector2f& pos) {
            m_position = pos;
        }

        void UIWidget::setSize(float w, float h) {
            m_size.x = w;
            m_size.y = h;
        }

        void UIWidget::setSize(const sf::Vector2f& size) {
            m_size = size;
        }

        sf::Vector2f UIWidget::getPosition() const {
            return m_position;
        }

        sf::Vector2f UIWidget::getSize() const {
            return m_size;
        }

        sf::FloatRect UIWidget::getBounds() const {
            return sf::FloatRect(0.f, 0.f, m_size.x, m_size.y);
        }

        sf::FloatRect UIWidget::getGlobalBounds() const {
            sf::FloatRect bounds = getBounds();
            bounds.left = m_position.x;
            bounds.top = m_position.y;
            return bounds;
        }

        bool UIWidget::containsPoint(const sf::Vector2f& point) const {
            return getGlobalBounds().contains(point);
        }

        void UIWidget::show() { visible = true; }
        void UIWidget::hide() { visible = false; }
        void UIWidget::enable() { enabled = true; }
        void UIWidget::disable() { enabled = false; }
        bool UIWidget::isEnabled() const { return enabled; }
        bool UIWidget::isVisible() const { return visible; }

        void UIWidget::addChild(Ptr child) {
            if (child && child->m_parent.lock().get() != this) {
                child->setParent(shared_from_this());
                m_children.push_back(child);
                sortChildrenByZOrder();
            }
        }

        void UIWidget::removeChild(Ptr child) {
            auto it = std::find(m_children.begin(), m_children.end(), child);
            if (it != m_children.end()) {
                (*it)->m_parent.reset();
                m_children.erase(it);
            }
        }

        UIWidget::Ptr UIWidget::getParent() const {
            return m_parent.lock();
        }

        void UIWidget::setParent(Ptr parent) {
            m_parent = parent;
        }

        void UIWidget::bringToFront() {
            if (auto p = m_parent.lock()) {
                auto it = std::find(p->m_children.begin(), p->m_children.end(), shared_from_this());
                if (it != p->m_children.end()) {
                    p->m_children.erase(it);
                    p->m_children.push_back(shared_from_this());
                }
            }
        }

        void UIWidget::sendToBack() {
            if (auto p = m_parent.lock()) {
                auto it = std::find(p->m_children.begin(), p->m_children.end(), shared_from_this());
                if (it != p->m_children.end()) {
                    p->m_children.erase(it);
                    p->m_children.insert(p->m_children.begin(), shared_from_this());
                }
            }
        }

        bool UIWidget::isHovered() const { return m_hovered; }
        bool UIWidget::isFocused() const { return m_focused; }
        bool UIWidget::isPressed() const { return m_pressed; }
        bool UIWidget::isDisabled() const { return !enabled; }

        void UIWidget::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            for (auto& child : m_children) {
                if (child->visible) {
                    child->onRender(target, states);
                }
            }
        }

        void UIWidget::onUpdate(float dt) {
            for (auto& child : m_children) {
                child->onUpdate(dt);
            }
        }

        void UIWidget::onLayout() {
            for (auto& child : m_children) {
                child->onLayout();
            }
        }

        bool UIWidget::onEvent(const sf::Event& event) {
            for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
                if ((*it)->visible && (*it)->onEvent(event))
                    return true;
            }
            return false;
        }

        void UIWidget::sortChildrenByZOrder() {
            std::sort(m_children.begin(), m_children.end(),
                [](const Ptr& a, const Ptr& b) { return a->zOrder < b->zOrder; });
        }

    }
}
