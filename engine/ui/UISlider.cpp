#include "UISlider.h"
#include <algorithm>
#include <cmath>

namespace nebula {
    namespace ui {

        UISlider::UISlider()
            : UIWidget("Slider")
            , m_minValue(0.f)
            , m_maxValue(100.f)
            , m_value(50.f)
            , m_step(1.f)
            , m_orientation(SliderOrientation::Horizontal)
            , m_trackColor(80, 80, 80)
            , m_thumbColor(180, 180, 180)
            , m_dragging(false)
            , m_thumbSize(16.f)
            , m_fillColor(60, 140, 210)
            , m_showFill(true)
            , m_snapToStep(true)
            , showTickMarks(false)
            , tickMarkCount(10) {
            m_size.x = 200.f;
            m_size.y = 20.f;
        }

        void UISlider::setValue(float value) {
            m_value = std::max(m_minValue, std::min(m_maxValue, value));
            if (m_snapToStep && m_step > 0.f) {
                m_value = snapToStep(m_value);
            }
            updateThumbPosition();
        }

        float UISlider::getValue() const {
            return m_value;
        }

        void UISlider::setRange(float min, float max) {
            m_minValue = min;
            m_maxValue = max;
            setValue(m_value);
        }

        float UISlider::getMinValue() const {
            return m_minValue;
        }

        float UISlider::getMaxValue() const {
            return m_maxValue;
        }

        void UISlider::setStep(float step) {
            m_step = step;
            setValue(m_value);
        }

        float UISlider::getStep() const {
            return m_step;
        }

        void UISlider::setOrientation(SliderOrientation orientation) {
            m_orientation = orientation;
            if (orientation == SliderOrientation::Vertical) {
                m_size.x = 20.f;
                m_size.y = 200.f;
            } else {
                m_size.x = 200.f;
                m_size.y = 20.f;
            }
        }

        SliderOrientation UISlider::getOrientation() const {
            return m_orientation;
        }

        float UISlider::getNormalizedValue() const {
            float range = m_maxValue - m_minValue;
            if (range <= 0.f) return 0.f;
            return (m_value - m_minValue) / range;
        }

        void UISlider::setTrackTexture(const std::shared_ptr<sf::Texture>& texture) {
            m_trackTexture = texture;
        }

        void UISlider::setThumbTexture(const std::shared_ptr<sf::Texture>& texture) {
            m_thumbTexture = texture;
        }

        void UISlider::setTrackColor(const graphics::Color& color) {
            m_trackColor = color;
        }

        void UISlider::setThumbColor(const graphics::Color& color) {
            m_thumbColor = color;
        }

        void UISlider::setFillColor(const graphics::Color& color) {
            m_fillColor = color;
        }

        void UISlider::setShowFill(bool show) {
            m_showFill = show;
        }

        void UISlider::setTickMarks(bool show, int count) {
            showTickMarks = show;
            tickMarkCount = std::max(2, count);
        }

        bool UISlider::hasTickMarks() const {
            return showTickMarks;
        }

        int UISlider::getTickMarkCount() const {
            return tickMarkCount;
        }

        void UISlider::setSnapToStep(bool snap) {
            m_snapToStep = snap;
        }

        bool UISlider::isSnapToStep() const {
            return m_snapToStep;
        }

        float UISlider::snapToStep(float val) const {
            if (m_step <= 0.f) return val;
            float steps = std::round((val - m_minValue) / m_step);
            return m_minValue + steps * m_step;
        }

        sf::FloatRect UISlider::getThumbBounds() const {
            float norm = getNormalizedValue();
            if (m_orientation == SliderOrientation::Horizontal) {
                float trackWidth = m_size.x - m_thumbSize;
                float x = m_position.x + norm * trackWidth;
                float y = m_position.y + (m_size.y - m_thumbSize) / 2.f;
                return sf::FloatRect(x, y, m_thumbSize, m_thumbSize);
            } else {
                float trackHeight = m_size.y - m_thumbSize;
                float x = m_position.x + (m_size.x - m_thumbSize) / 2.f;
                float y = m_position.y + norm * trackHeight;
                return sf::FloatRect(x, y, m_thumbSize, m_thumbSize);
            }
        }

        void UISlider::updateThumbPosition() {
            float norm = getNormalizedValue();
            if (m_orientation == SliderOrientation::Horizontal) {
                float trackWidth = m_size.x - m_thumbSize;
                float x = norm * trackWidth;
                float y = (m_size.y - m_thumbSize) / 2.f;
                m_thumbRect.setPosition(x, y);
                m_thumbCircle.setPosition(x, y);
            } else {
                float trackHeight = m_size.y - m_thumbSize;
                float x = (m_size.x - m_thumbSize) / 2.f;
                float y = norm * trackHeight;
                m_thumbRect.setPosition(x, y);
                m_thumbCircle.setPosition(x, y);
            }
        }

        void UISlider::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            states.transform.translate(m_position);

            if (m_orientation == SliderOrientation::Horizontal) {
                float trackHeight = 6.f;
                float trackY = (m_size.y - trackHeight) / 2.f;
                m_track.setSize(sf::Vector2f(m_size.x, trackHeight));
                m_track.setPosition(0.f, trackY);

                if (m_showFill) {
                    float norm = getNormalizedValue();
                    float fillWidth = norm * m_size.x;
                    m_fillBar.setSize(sf::Vector2f(fillWidth, trackHeight));
                    m_fillBar.setPosition(0.f, trackY);
                    m_fillBar.setFillColor(m_fillColor.toSFML());
                    target.draw(m_fillBar, states);
                }
            } else {
                float trackWidth = 6.f;
                float trackX = (m_size.x - trackWidth) / 2.f;
                m_track.setSize(sf::Vector2f(trackWidth, m_size.y));
                m_track.setPosition(trackX, 0.f);

                if (m_showFill) {
                    float norm = getNormalizedValue();
                    float fillHeight = norm * m_size.y;
                    m_fillBar.setSize(sf::Vector2f(trackWidth, fillHeight));
                    m_fillBar.setPosition(trackX, m_size.y - fillHeight);
                    m_fillBar.setFillColor(m_fillColor.toSFML());
                    target.draw(m_fillBar, states);
                }
            }

            m_track.setFillColor(m_trackColor.toSFML());
            if (m_trackTexture) {
                m_track.setTexture(m_trackTexture.get());
            }
            target.draw(m_track, states);

            if (m_thumbTexture) {
                m_thumbRect.setSize(sf::Vector2f(m_thumbSize, m_thumbSize));
                m_thumbRect.setTexture(m_thumbTexture.get());
                target.draw(m_thumbRect, states);
            } else {
                m_thumbCircle.setRadius(m_thumbSize / 2.f);
                m_thumbCircle.setFillColor(m_thumbColor.toSFML());
                m_thumbCircle.setOutlineThickness(1.f);
                m_thumbCircle.setOutlineColor(sf::Color(200, 200, 200));
                target.draw(m_thumbCircle, states);
            }

            if (showTickMarks && tickMarkCount > 0) {
                sf::VertexArray ticks(sf::Lines, tickMarkCount * 2);
                for (int i = 0; i < tickMarkCount; ++i) {
                    float t = static_cast<float>(i) / static_cast<float>(tickMarkCount - 1);
                    float x, y1, y2;
                    if (m_orientation == SliderOrientation::Horizontal) {
                        x = t * m_size.x;
                        y1 = m_size.y / 2.f + 8.f;
                        y2 = y1 + 6.f;
                    } else {
                        y1 = t * m_size.y;
                        x = m_size.x / 2.f + 8.f;
                        float x2 = x + 6.f;
                        ticks[i * 2].position = sf::Vector2f(x, y1);
                        ticks[i * 2 + 1].position = sf::Vector2f(x2, y1);
                        continue;
                    }
                    ticks[i * 2].position = sf::Vector2f(x, y1);
                    ticks[i * 2 + 1].position = sf::Vector2f(x, y2);
                    ticks[i * 2].color = sf::Color(150, 150, 150);
                    ticks[i * 2 + 1].color = sf::Color(150, 150, 150);
                }
                target.draw(ticks, states);
            }

            UIWidget::onRender(target, states);
        }

        void UISlider::onUpdate(float dt) {
            UIWidget::onUpdate(dt);
        }

        bool UISlider::onEvent(const sf::Event& event) {
            if (!enabled || !visible) return false;

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                sf::FloatRect thumbBounds = getThumbBounds();
                if (thumbBounds.contains(mousePos)) {
                    m_dragging = true;
                    if (onDragStart) onDragStart();
                    return true;
                }
                if (containsPoint(mousePos)) {
                    float norm;
                    if (m_orientation == SliderOrientation::Horizontal) {
                        float localX = mousePos.x - m_position.x;
                        norm = localX / m_size.x;
                    } else {
                        float localY = mousePos.y - m_position.y;
                        norm = localY / m_size.y;
                    }
                    float newVal = m_minValue + norm * (m_maxValue - m_minValue);
                    setValue(newVal);
                    if (onValueChanged) onValueChanged(m_value);
                    return true;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (m_dragging) {
                    m_dragging = false;
                    if (onDragEnd) onDragEnd();
                    return true;
                }
            }

            if (event.type == sf::Event::MouseMoved && m_dragging) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
                float norm;
                if (m_orientation == SliderOrientation::Horizontal) {
                    float localX = mousePos.x - m_position.x;
                    norm = std::max(0.f, std::min(1.f, localX / m_size.x));
                } else {
                    float localY = mousePos.y - m_position.y;
                    norm = std::max(0.f, std::min(1.f, localY / m_size.y));
                }
                float newVal = m_minValue + norm * (m_maxValue - m_minValue);
                setValue(newVal);
                if (onValueChanged) onValueChanged(m_value);
                return true;
            }

            if (event.type == sf::Event::KeyPressed && isFocused()) {
                float delta = m_step > 0.f ? m_step : 1.f;
                if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Down) {
                    setValue(m_value - delta);
                    if (onValueChanged) onValueChanged(m_value);
                    return true;
                }
                if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Up) {
                    setValue(m_value + delta);
                    if (onValueChanged) onValueChanged(m_value);
                    return true;
                }
            }

            return UIWidget::onEvent(event);
        }

    }
}
