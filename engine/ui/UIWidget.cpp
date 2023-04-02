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
            , m_pressed(false)
            , m_anchor(WidgetAnchor::TopLeft)
            , m_margins(0.f, 0.f, 0.f, 0.f)
            , m_padding(0.f, 0.f, 0.f, 0.f)
            , m_tooltipDelay(0.5f)
            , m_tooltipTimer(0.f)
            , m_tooltipVisible(false) {}

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
            , m_pressed(false)
            , m_anchor(WidgetAnchor::TopLeft)
            , m_margins(0.f, 0.f, 0.f, 0.f)
            , m_padding(0.f, 0.f, 0.f, 0.f)
            , m_tooltipDelay(0.5f)
            , m_tooltipTimer(0.f)
            , m_tooltipVisible(false) {}

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
            if (m_hovered && !m_tooltipText.empty()) {
                m_tooltipTimer += dt;
                if (m_tooltipTimer >= m_tooltipDelay) {
                    m_tooltipVisible = true;
                }
            } else {
                m_tooltipTimer = 0.f;
                m_tooltipVisible = false;
            }

            updateTween(dt);

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

        void UIWidget::setAnchor(WidgetAnchor anchor) {
            m_anchor = anchor;
        }

        WidgetAnchor UIWidget::getAnchor() const {
            return m_anchor;
        }

        void UIWidget::setMargins(float left, float top, float right, float bottom) {
            m_margins = sf::FloatRect(left, top, right, bottom);
        }

        void UIWidget::setPadding(float left, float top, float right, float bottom) {
            m_padding = sf::FloatRect(left, top, right, bottom);
        }

        sf::FloatRect UIWidget::getMargins() const {
            return m_margins;
        }

        sf::FloatRect UIWidget::getPadding() const {
            return m_padding;
        }

        void UIWidget::setTooltip(const std::string& tip) {
            m_tooltipText = tip;
        }

        std::string UIWidget::getTooltip() const {
            return m_tooltipText;
        }

        void UIWidget::setTooltipDelay(float seconds) {
            m_tooltipDelay = std::max(0.f, seconds);
        }

        float UIWidget::getTooltipDelay() const {
            return m_tooltipDelay;
        }

        void UIWidget::startTween(const TweenTarget& from, const TweenTarget& to, float duration, bool loop) {
            m_tween.from = from;
            m_tween.to = to;
            m_tween.current = from;
            m_tween.duration = std::max(0.016f, duration);
            m_tween.elapsed = 0.f;
            m_tween.active = true;
            m_tween.loop = loop;
        }

        void UIWidget::stopTween() {
            m_tween.active = false;
            m_tween.elapsed = 0.f;
        }

        bool UIWidget::isTweening() const {
            return m_tween.active;
        }

        sf::Vector2f UIWidget::resolveAnchorOffset(const sf::Vector2f& containerSize) const {
            sf::Vector2f offset(0.f, 0.f);
            switch (m_anchor) {
                case WidgetAnchor::TopLeft:
                    break;
                case WidgetAnchor::TopCenter:
                    offset.x = (containerSize.x - m_size.x) / 2.f;
                    break;
                case WidgetAnchor::TopRight:
                    offset.x = containerSize.x - m_size.x;
                    break;
                case WidgetAnchor::MiddleLeft:
                    offset.y = (containerSize.y - m_size.y) / 2.f;
                    break;
                case WidgetAnchor::MiddleCenter:
                    offset.x = (containerSize.x - m_size.x) / 2.f;
                    offset.y = (containerSize.y - m_size.y) / 2.f;
                    break;
                case WidgetAnchor::MiddleRight:
                    offset.x = containerSize.x - m_size.x;
                    offset.y = (containerSize.y - m_size.y) / 2.f;
                    break;
                case WidgetAnchor::BottomLeft:
                    offset.y = containerSize.y - m_size.y;
                    break;
                case WidgetAnchor::BottomCenter:
                    offset.x = (containerSize.x - m_size.x) / 2.f;
                    offset.y = containerSize.y - m_size.y;
                    break;
                case WidgetAnchor::BottomRight:
                    offset.x = containerSize.x - m_size.x;
                    offset.y = containerSize.y - m_size.y;
                    break;
                case WidgetAnchor::Fill:
                    break;
            }
            offset.x += m_margins.left - m_margins.width;
            offset.y += m_margins.top - m_margins.height;
            return offset;
        }

        void UIWidget::updateTween(float dt) {
            if (!m_tween.active) return;

            m_tween.elapsed += dt;
            float t = std::min(1.f, m_tween.elapsed / m_tween.duration);

            float smoothT = t * t * (3.f - 2.f * t);

            m_tween.current.position.x = m_tween.from.position.x + (m_tween.to.position.x - m_tween.from.position.x) * smoothT;
            m_tween.current.position.y = m_tween.from.position.y + (m_tween.to.position.y - m_tween.from.position.y) * smoothT;
            m_tween.current.size.x = m_tween.from.size.x + (m_tween.to.size.x - m_tween.from.size.x) * smoothT;
            m_tween.current.size.y = m_tween.from.size.y + (m_tween.to.size.y - m_tween.from.size.y) * smoothT;

            m_position = m_tween.current.position;
            m_size = m_tween.current.size;

            if (t >= 1.f) {
                if (m_tween.loop) {
                    m_tween.elapsed = 0.f;
                    m_tween.current = m_tween.from;
                } else {
                    m_tween.active = false;
                    if (m_tween.onComplete) {
                        m_tween.onComplete();
                    }
                }
            }
        }

    }
}
