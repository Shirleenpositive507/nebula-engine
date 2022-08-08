#pragma once

#include "UIWidget.h"
#include "UILabel.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>

namespace nebula {
    namespace ui {

        enum class CheckmarkStyle {
            Cross,
            Tick,
            Fill
        };

        class UICheckbox : public UIWidget {
        public:
            UICheckbox();
            explicit UICheckbox(const std::string& text);

            void setChecked(bool checked);
            bool isChecked() const;
            void toggle();
            void setText(const std::string& text);
            void setCheckColor(const graphics::Color& color);
            void setUncheckColor(const graphics::Color& color);
            void setCheckmarkStyle(CheckmarkStyle style);

            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            bool onEvent(const sf::Event& event) override;

            Callback onChecked;
            Callback onUnchecked;
            Callback onToggled;

            CheckmarkStyle checkmarkStyle;

        private:
            bool m_checked;
            std::shared_ptr<UILabel> m_label;
            sf::RectangleShape m_box;
            graphics::Color m_checkColor;
            graphics::Color m_uncheckColor;
            float m_boxSize;

            void drawCheckmark(sf::RenderTarget& target, sf::RenderStates states) const;
        };

    }
}
