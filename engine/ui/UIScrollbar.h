#pragma once

#include "UIWidget.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <functional>

namespace nebula {
namespace ui {

enum class ScrollbarOrientation {
    Vertical,
    Horizontal
};

class UIScrollbar : public UIWidget {
public:
    UIScrollbar();
    explicit UIScrollbar(const std::string& name);

    void setOrientation(ScrollbarOrientation orientation);
    ScrollbarOrientation getOrientation() const;

    void setMin(float min);
    float getMin() const;

    void setMax(float max);
    float getMax() const;

    void setValue(float value);
    float getValue() const;

    void setViewportSize(float size);
    float getViewportSize() const;

    void setThumbSize(float size);
    float getThumbSize() const;

    void setStep(float step);
    float getStep() const;

    void setAutoHide(bool autoHide);
    bool isAutoHide() const;

    void setSmoothScrolling(bool smooth);
    bool isSmoothScrolling() const;

    float getScrollProgress() const;
    void scrollTo(float position);
    void scrollBy(float delta);

    std::function<void(float)> onValueChanged;

    void onRender(sf::RenderTarget& target, const sf::RenderStates& states) const override;
    void onEvent(const sf::Event& event) override;
    void onUpdate(float dt) override;

private:
    void updateThumb();
    float thumbPositionFromValue(float value) const;
    float valueFromThumbPosition(float position) const;

    ScrollbarOrientation m_orientation;
    float m_min;
    float m_max;
    float m_value;
    float m_displayValue;
    float m_viewportSize;
    float m_thumbSize;
    float m_step;
    bool m_autoHide;
    bool m_smoothScrolling;
    bool m_dragging;
    float m_dragOffset;

    sf::RectangleShape m_track;
    sf::RectangleShape m_thumb;
    sf::Color m_trackColor;
    sf::Color m_thumbColor;
    sf::Color m_thumbHoverColor;
};

}
}

