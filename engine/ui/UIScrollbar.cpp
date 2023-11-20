#include "UIScrollbar.h"
#include <algorithm>
#include <cmath>

namespace nebula {
namespace ui {

UIScrollbar::UIScrollbar()
    : UIWidget("scrollbar")
    , m_orientation(ScrollbarOrientation::Vertical)
    , m_min(0.f)
    , m_max(100.f)
    , m_value(0.f)
    , m_displayValue(0.f)
    , m_viewportSize(50.f)
    , m_thumbSize(20.f)
    , m_step(1.f)
    , m_autoHide(true)
    , m_smoothScrolling(true)
    , m_dragging(false)
    , m_dragOffset(0.f)
    , m_trackColor(50, 50, 50)
    , m_thumbColor(120, 120, 120)
    , m_thumbHoverColor(150, 150, 150)
{
    setSize(16.f, 200.f);
}

UIScrollbar::UIScrollbar(const std::string& name)
    : UIWidget(name)
    , m_orientation(ScrollbarOrientation::Vertical)
    , m_min(0.f)
    , m_max(100.f)
    , m_value(0.f)
    , m_displayValue(0.f)
    , m_viewportSize(50.f)
    , m_thumbSize(20.f)
    , m_step(1.f)
    , m_autoHide(true)
    , m_smoothScrolling(true)
    , m_dragging(false)
    , m_dragOffset(0.f)
    , m_trackColor(50, 50, 50)
    , m_thumbColor(120, 120, 120)
    , m_thumbHoverColor(150, 150, 150)
{
    setSize(16.f, 200.f);
}

void UIScrollbar::setOrientation(ScrollbarOrientation orientation) {
    m_orientation = orientation;
    if (orientation == ScrollbarOrientation::Vertical) {
        setSize(16.f, 200.f);
    } else {
        setSize(200.f, 16.f);
    }
}

ScrollbarOrientation UIScrollbar::getOrientation() const { return m_orientation; }
void UIScrollbar::setMin(float min) { m_min = min; }
float UIScrollbar::getMin() const { return m_min; }
void UIScrollbar::setMax(float max) { m_max = max; }
float UIScrollbar::getMax() const { return m_max; }
void UIScrollbar::setViewportSize(float size) { m_viewportSize = std::max(1.f, size); }
float UIScrollbar::getViewportSize() const { return m_viewportSize; }
void UIScrollbar::setStep(float step) { m_step = std::max(0.01f, step); }
float UIScrollbar::getStep() const { return m_step; }
void UIScrollbar::setAutoHide(bool autoHide) { m_autoHide = autoHide; }
bool UIScrollbar::isAutoHide() const { return m_autoHide; }
void UIScrollbar::setSmoothScrolling(bool smooth) { m_smoothScrolling = smooth; }
bool UIScrollbar::isSmoothScrolling() const { return m_smoothScrolling; }

void UIScrollbar::setValue(float value) {
    float clamped = std::clamp(value, m_min, std::max(m_min, m_max - m_viewportSize));
    if (m_value != clamped) {
        m_value = clamped;
        if (onValueChanged) onValueChanged(m_value);
    }
}

float UIScrollbar::getValue() const { return m_value; }

void UIScrollbar::setThumbSize(float size) { m_thumbSize = std::max(10.f, size); }
float UIScrollbar::getThumbSize() const { return m_thumbSize; }

float UIScrollbar::getScrollProgress() const {
    float range = m_max - m_min;
    if (range <= 0.f) return 0.f;
    return (m_value - m_min) / range;
}

void UIScrollbar::scrollTo(float position) { setValue(position); }

void UIScrollbar::scrollBy(float delta) { setValue(m_value + delta * m_step); }

void UIScrollbar::updateThumb() {
    sf::FloatRect bounds = getGlobalBounds();
    if (m_orientation == ScrollbarOrientation::Vertical) {
        float trackHeight = bounds.height;
        float available = trackHeight - m_thumbSize;
        float progress = getScrollProgress();
        float thumbPos = bounds.top + progress * available;
        m_thumb.setPosition(bounds.left, thumbPos);
        m_thumb.setSize(sf::Vector2f(bounds.width, m_thumbSize));
    } else {
        float trackWidth = bounds.width;
        float available = trackWidth - m_thumbSize;
        float progress = getScrollProgress();
        float thumbPos = bounds.left + progress * available;
        m_thumb.setPosition(thumbPos, bounds.top);
        m_thumb.setSize(sf::Vector2f(m_thumbSize, bounds.height));
    }
}

float UIScrollbar::thumbPositionFromValue(float value) const {
    sf::FloatRect bounds = getGlobalBounds();
    float range = m_max - m_min;
    if (range <= 0.f) return 0.f;
    float progress = (value - m_min) / range;
    if (m_orientation == ScrollbarOrientation::Vertical) {
        return bounds.top + progress * (bounds.height - m_thumbSize);
    }
    return bounds.left + progress * (bounds.width - m_thumbSize);
}

float UIScrollbar::valueFromThumbPosition(float position) const {
    sf::FloatRect bounds = getGlobalBounds();
    float range = m_max - m_min;
    if (range <= 0.f) return m_min;
    float progress;
    if (m_orientation == ScrollbarOrientation::Vertical) {
        progress = (position - bounds.top) / (bounds.height - m_thumbSize);
    } else {
        progress = (position - bounds.left) / (bounds.width - m_thumbSize);
    }
    return m_min + std::clamp(progress, 0.f, 1.f) * range;
}

void UIScrollbar::onRender(sf::RenderTarget& target, const sf::RenderStates& states) const {
    if (m_autoHide && m_viewportSize >= (m_max - m_min)) return;

    UIWidget::onRender(target, states);
    sf::FloatRect bounds = getGlobalBounds();

    m_track.setPosition(bounds.left, bounds.top);
    m_track.setSize(sf::Vector2f(bounds.width, bounds.height));
    m_track.setFillColor(m_trackColor);
    target.draw(m_track, states);

    m_thumb.setFillColor(m_thumbColor);
    target.draw(m_thumb, states);
}

void UIScrollbar::onEvent(const sf::Event& event) {
    if (m_autoHide && m_viewportSize >= (m_max - m_min)) return;

    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        if (m_thumb.getGlobalBounds().contains(mousePos)) {
            m_dragging = true;
            if (m_orientation == ScrollbarOrientation::Vertical) {
                m_dragOffset = mousePos.y - m_thumb.getPosition().y;
            } else {
                m_dragOffset = mousePos.x - m_thumb.getPosition().x;
            }
        } else if (getGlobalBounds().contains(mousePos)) {
            if (m_orientation == ScrollbarOrientation::Vertical) {
                scrollBy((mousePos.y < m_thumb.getPosition().y) ? -m_viewportSize : m_viewportSize);
            } else {
                scrollBy((mousePos.x < m_thumb.getPosition().x) ? -m_viewportSize : m_viewportSize);
            }
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        m_dragging = false;
    }

    if (event.type == sf::Event::MouseMoved && m_dragging) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
        float thumbPos = (m_orientation == ScrollbarOrientation::Vertical)
            ? mousePos.y - m_dragOffset : mousePos.x - m_dragOffset;
        setValue(valueFromThumbPosition(thumbPos));
    }
}

void UIScrollbar::onUpdate(float dt) {
    UIWidget::onUpdate(dt);
    if (m_smoothScrolling) {
        float diff = m_value - m_displayValue;
        if (std::abs(diff) > 0.01f) {
            m_displayValue += diff * std::min(1.f, 8.f * dt);
        } else {
            m_displayValue = m_value;
        }
    } else {
        m_displayValue = m_value;
    }
    updateThumb();
}

}
}

