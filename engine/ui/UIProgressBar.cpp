#include "UIProgressBar.h"
#include <algorithm>
#include <cmath>

namespace nebula {
namespace ui {

UIProgressBar::UIProgressBar()
    : UIWidget("progressbar")
    , m_min(0.f)
    , m_max(100.f)
    , m_value(0.f)
    , m_displayValue(0.f)
    , m_animationSpeed(5.f)
    , m_orientation(ProgressBarOrientation::Horizontal)
    , m_fillColor(50, 150, 255)
    , m_backgroundColor(60, 60, 60)
    , m_showLabel(true)
    , m_smoothTransition(true)
{
    setSize(200.f, 24.f);
}

UIProgressBar::UIProgressBar(const std::string& name)
    : UIWidget(name)
    , m_min(0.f)
    , m_max(100.f)
    , m_value(0.f)
    , m_displayValue(0.f)
    , m_animationSpeed(5.f)
    , m_orientation(ProgressBarOrientation::Horizontal)
    , m_fillColor(50, 150, 255)
    , m_backgroundColor(60, 60, 60)
    , m_showLabel(true)
    , m_smoothTransition(true)
{
    setSize(200.f, 24.f);
}

void UIProgressBar::setMin(float min) {
    m_min = min;
    if (m_value < m_min) m_value = m_min;
}

float UIProgressBar::getMin() const { return m_min; }

void UIProgressBar::setMax(float max) {
    m_max = max;
    if (m_value > m_max) m_value = m_max;
}

float UIProgressBar::getMax() const { return m_max; }

void UIProgressBar::setValue(float value) {
    float oldValue = m_value;
    m_value = std::clamp(value, m_min, m_max);
    if (oldValue != m_value && onValueChanged) {
        onValueChanged(m_value);
    }
}

float UIProgressBar::getValue() const { return m_value; }

void UIProgressBar::setProgress(float progress) {
    setValue(m_min + (m_max - m_min) * std::clamp(progress, 0.f, 1.f));
}

float UIProgressBar::getProgress() const {
    if (m_max <= m_min) return 0.f;
    return (m_value - m_min) / (m_max - m_min);
}

void UIProgressBar::setOrientation(ProgressBarOrientation orientation) {
    m_orientation = orientation;
    if (orientation == ProgressBarOrientation::Vertical) {
        setSize(24.f, 200.f);
    } else {
        setSize(200.f, 24.f);
    }
}

ProgressBarOrientation UIProgressBar::getOrientation() const { return m_orientation; }

void UIProgressBar::setFillColor(const sf::Color& color) { m_fillColor = color; }
sf::Color UIProgressBar::getFillColor() const { return m_fillColor; }

void UIProgressBar::setBackgroundColor(const sf::Color& color) { m_backgroundColor = color; }
sf::Color UIProgressBar::getBackgroundColor() const { return m_backgroundColor; }

void UIProgressBar::setLabel(const std::string& label) { m_labelText = label; }
std::string UIProgressBar::getLabel() const { return m_labelText; }

void UIProgressBar::setShowLabel(bool show) { m_showLabel = show; }
bool UIProgressBar::isShowLabel() const { return m_showLabel; }

void UIProgressBar::setSmoothTransition(bool smooth) { m_smoothTransition = smooth; }
bool UIProgressBar::isSmoothTransition() const { return m_smoothTransition; }

void UIProgressBar::setAnimationSpeed(float speed) { m_animationSpeed = std::max(0.1f, speed); }
float UIProgressBar::getAnimationSpeed() const { return m_animationSpeed; }

void UIProgressBar::onRender(sf::RenderTarget& target, const sf::RenderStates& states) const {
    UIWidget::onRender(target, states);

    sf::FloatRect bounds = getGlobalBounds();
    m_background.setPosition(bounds.left, bounds.top);
    m_background.setSize(sf::Vector2f(bounds.width, bounds.height));
    m_background.setFillColor(m_backgroundColor);
    target.draw(m_background, states);

    float fillRatio = getProgress();
    sf::Vector2f fillSize;
    if (m_orientation == ProgressBarOrientation::Horizontal) {
        fillSize = sf::Vector2f(bounds.width * fillRatio, bounds.height);
    } else {
        fillSize = sf::Vector2f(bounds.width, bounds.height * fillRatio);
    }
    m_fill.setPosition(bounds.left, bounds.top);
    m_fill.setSize(fillSize);
    m_fill.setFillColor(m_fillColor);
    target.draw(m_fill, states);
}

void UIProgressBar::onUpdate(float dt) {
    UIWidget::onUpdate(dt);
    if (m_smoothTransition) {
        float diff = m_value - m_displayValue;
        if (std::abs(diff) > 0.01f) {
            m_displayValue += diff * std::min(1.f, m_animationSpeed * dt);
        } else {
            m_displayValue = m_value;
        }
    } else {
        m_displayValue = m_value;
    }
}

}
}

