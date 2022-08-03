#pragma once

#include "UIWidget.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <memory>

namespace nebula {
    namespace ui {

        enum class TextAlignment {
            Left,
            Center,
            Right
        };

        class UILabel : public UIWidget {
        public:
            UILabel();
            explicit UILabel(const std::string& text);

            void setText(const std::string& text);
            std::string getText() const;

            void setFont(sf::Font& font);
            void setCharacterSize(unsigned int size);
            void setColor(const graphics::Color& color);
            void setAlignment(TextAlignment alignment);
            void setWordWrap(float wrapWidth);
            void setAutoSize(bool autoSize);

            sf::FloatRect getTextBounds() const;

            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            void onLayout() override;

        private:
            sf::Text m_text;
            std::string m_textString;
            sf::Font* m_font;
            unsigned int m_characterSize;
            graphics::Color m_color;
            TextAlignment m_alignment;
            float m_wordWrapWidth;
            bool m_autoSize;
            bool m_dirty;

            void rebuildText();
        };

    }
}
