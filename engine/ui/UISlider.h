#pragma once

#include "UIWidget.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <functional>

namespace nebula {
    namespace ui {

        enum class SliderOrientation {
            Horizontal,
            Vertical
        };

        class UISlider : public UIWidget {
        public:
            UISlider();

            void setValue(float value);
            float getValue() const;
            void setRange(float min, float max);
            void setStep(float step);
            void setOrientation(SliderOrientation orientation);
            float getNormalizedValue() const;

            void setTrackTexture(const std::shared_ptr<sf::Texture>& texture);
            void setThumbTexture(const std::shared_ptr<sf::Texture>& texture);
            void setTrackColor(const graphics::Color& color);
            void setThumbColor(const graphics::Color& color);

            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            void onUpdate(float dt) override;
            bool onEvent(const sf::Event& event) override;

            std::function<void(float)> onValueChanged;
            Callback onDragStart;
            Callback onDragEnd;

            bool showTickMarks;
            int tickMarkCount;

        private:
            float m_minValue;
            float m_maxValue;
            float m_value;
            float m_step;
            SliderOrientation m_orientation;

            std::shared_ptr<sf::Texture> m_trackTexture;
            std::shared_ptr<sf::Texture> m_thumbTexture;
            graphics::Color m_trackColor;
            graphics::Color m_thumbColor;

            sf::RectangleShape m_track;
            sf::RectangleShape m_thumbRect;
            sf::CircleShape m_thumbCircle;

            bool m_dragging;
            float m_thumbSize;

            sf::FloatRect getThumbBounds() const;
            void updateThumbPosition();
            float snapToStep(float val) const;
        };

    }
}
