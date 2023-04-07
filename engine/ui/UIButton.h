#pragma once

#include "UIWidget.h"
#include "UILabel.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <memory>
#include <set>
#include <unordered_map>

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

            void setToggleMode(bool toggle);
            bool isToggleMode() const;
            void setToggled(bool toggled);
            bool isToggled() const;

            void setRadioGroup(const std::string& group);
            std::string getRadioGroup() const;
            void setRadioGroupExclusive(bool exclusive);

            void setIcon(const std::string& iconPath);
            void setIcon(const std::shared_ptr<sf::Texture>& iconTexture);
            void setIconSize(float width, float height);

            void setShortcutKey(sf::Keyboard::Key key, bool ctrl = false, bool alt = false, bool shift = false);
            sf::Keyboard::Key getShortcutKey() const;
            bool matchesShortcut(const sf::Event& event) const;

            void setAutoRepeat(bool repeat);
            bool isAutoRepeat() const;
            void setAutoRepeatDelay(float delay);
            void setAutoRepeatRate(float rate);

            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            void onUpdate(float dt) override;
            void onLayout() override;
            bool onEvent(const sf::Event& event) override;

            Callback onClicked;
            Callback onPressed;
            Callback onReleased;
            Callback onHovered;
            Callback onToggled;

            static std::shared_ptr<UIButton> getSelectedInGroup(const std::string& group);
            static void setGroupSelection(const std::string& group, std::shared_ptr<UIButton> button);

        private:
            ButtonState m_state;
            ButtonState m_targetState;
            std::shared_ptr<UILabel> m_label;

            sf::RectangleShape m_background;
            sf::Sprite m_iconSprite;
            std::shared_ptr<sf::Texture> m_iconTexture;
            sf::Vector2f m_iconSize;

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

            bool m_toggleMode;
            bool m_toggled;
            std::string m_radioGroup;
            bool m_radioExclusive;

            sf::Keyboard::Key m_shortcutKey;
            bool m_shortcutCtrl;
            bool m_shortcutAlt;
            bool m_shortcutShift;

            bool m_autoRepeat;
            float m_autoRepeatDelay;
            float m_autoRepeatRate;
            float m_repeatTimer;
            bool m_repeatFired;

            static std::unordered_map<std::string, std::weak_ptr<UIButton>> s_groupSelection;

            void updateAppearance();
            void updateLabelStyle();
            graphics::Color lerpColor(const graphics::Color& a, const graphics::Color& b, float t) const;
            void handleAutoRepeat(float dt);
        };

    }
}
