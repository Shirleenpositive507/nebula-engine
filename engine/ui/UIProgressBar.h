#pragma once

#include "UIWidget.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <functional>
#include <string>

namespace nebula {
namespace ui {

enum class ProgressBarOrientation {
    Horizontal,
    Vertical
};

class UIProgressBar : public UIWidget {
public:
    UIProgressBar();
    explicit UIProgressBar(const std::string& name);

    void setMin(float min);
    float getMin() const;

    void setMax(float max);
    float getMax() const;

    void setValue(float value);
    float getValue() const;

    void setProgress(float progress);
    float getProgress() const;

    void setOrientation(ProgressBarOrientation orientation);
    ProgressBarOrientation getOrientation() const;

    void setFillColor(const sf::Color& color);
    sf::Color getFillColor() const;

    void setBackgroundColor(const sf::Color& color);
    sf::Color getBackgroundColor() const;

    void setLabel(const std::string& label);
    std::string getLabel() const;

    void setShowLabel(bool show);
    bool isShowLabel() const;

    void setSmoothTransition(bool smooth);
    bool isSmoothTransition() const;

    void setAnimationSpeed(float speed);
    float getAnimationSpeed() const;

    std::function<void(float)> onValueChanged;

    void onRender(sf::RenderTarget& target, const sf::RenderStates& states) const override;
    void onUpdate(float dt) override;

private:
    float m_min;
    float m_max;
    float m_value;
    float m_displayValue;
    float m_animationSpeed;
    ProgressBarOrientation m_orientation;
    sf::Color m_fillColor;
    sf::Color m_backgroundColor;
    std::string m_labelText;
    bool m_showLabel;
    bool m_smoothTransition;
    sf::RectangleShape m_background;
    sf::RectangleShape m_fill;
};

}
}

