#pragma once

#include "UIWidget.h"
#include "UILabel.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>

namespace nebula {
    namespace ui {

        enum class ButtonState {
            Normal,
            Hovered,
            Pressed,
            Disabled
        };

        class UIButton : public UIWidget {
        public:
            UIButton();
            explicit UIButton(const std::string& text);

            void setText(const std::string& text);
            void setFont(sf::Font& font);

            void setBackgroundColor(ButtonState state, const graphics::Color& color);
            void setTextColor(ButtonState state, const graphics::Color& color);
            void setBorderColor(ButtonState state, const graphics::Color& color);
            void setTexture(ButtonState state, const std::shared_ptr<sf::Texture>& texture);
            void setCornerRadius(float radius);
            void setAutoSize(bool autoSize);

            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            void onUpdate(float dt) override;
            void onLayout() override;
            bool onEvent(const sf::Event& event) override;

            Callback onClicked;
            Callback onPressed;
            Callback onReleased;
            Callback onHovered;

        private:
            ButtonState m_state;
            ButtonState m_targetState;
            std::shared_ptr<UILabel> m_label;

            sf::RectangleShape m_background;

            struct StateColors {
                graphics::Color bg;
                graphics::Color text;
                graphics::Color border;
                std::shared_ptr<sf::Texture> texture;
            };

            StateColors m_colors[4];
            float m_cornerRadius;
            bool m_autoSize;
            float m_hoverLerp;
            float m_scalePulse;

            void updateAppearance();
            void updateLabelStyle();
            graphics::Color lerpColor(const graphics::Color& a, const graphics::Color& b, float t) const;
        };

    }
}
