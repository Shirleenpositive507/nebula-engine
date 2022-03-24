#pragma once

#include "Font.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

namespace nebula {
    namespace graphics {

        enum class TextAlignment {
            Left,
            Center,
            Right
        };

        enum class TextStyle {
            Regular = sf::Text::Regular,
            Bold = sf::Text::Bold,
            Italic = sf::Text::Italic,
            Underlined = sf::Text::Underlined,
            StrikeThrough = sf::Text::StrikeThrough
        };

        inline TextStyle operator|(TextStyle a, TextStyle b) {
            return static_cast<TextStyle>(static_cast<int>(a) | static_cast<int>(b));
        }

        inline TextStyle operator&(TextStyle a, TextStyle b) {
            return static_cast<TextStyle>(static_cast<int>(a) & static_cast<int>(b));
        }

        class TextRenderer {
        public:
            TextRenderer();
            explicit TextRenderer(const std::string& text);
            TextRenderer(const std::string& text, const std::shared_ptr<Font>& font);
            ~TextRenderer() = default;

            void setFont(const std::shared_ptr<Font>& font);
            std::shared_ptr<Font> getFont() const;

            void setString(const std::string& text);
            const std::string& getString() const;

            void setCharacterSize(unsigned int size);
            unsigned int getCharacterSize() const;

            void setStyle(int style);
            void setStyle(TextStyle style);
            int getStyle() const;

            void setColor(const sf::Color& color);
            sf::Color getColor() const;

            void setOutlineColor(const sf::Color& color);
            sf::Color getOutlineColor() const;

            void setOutlineThickness(float thickness);
            float getOutlineThickness() const;

            void setLetterSpacing(float spacingFactor);
            float getLetterSpacing() const;

            void setLineSpacing(float spacingFactor);
            float getLineSpacing() const;

            sf::FloatRect getLocalBounds() const;
            sf::FloatRect getGlobalBounds() const;

            sf::Vector2f getCharacterPos(std::size_t index) const;
            std::size_t findCharacterPos(const sf::Vector2f& point) const;

            void setAlignment(TextAlignment alignment);
            TextAlignment getAlignment() const;

            void setMaxWidth(float maxWidth);
            float getMaxWidth() const;
            bool isWordWrapping() const;

            void setWordWrap(bool enabled);
            void setAutoSize(bool enabled);

            void update(float maxWidth = 0.f);

            void draw(sf::RenderTarget& target, const sf::RenderStates& states = sf::RenderStates::Default) const;

            sf::Text& getSFMLText();
            const sf::Text& getSFMLText() const;

        private:
            std::shared_ptr<Font> m_font;
            sf::Text m_text;
            std::string m_string;
            TextAlignment m_alignment;
            float m_maxWidth;
            bool m_wordWrap;
            bool m_autoSize;
            bool m_dirty;
            std::vector<std::string> m_lines;

            void rebuild();
            std::vector<std::string> wrapText(const std::string& text, float maxWidth) const;
            void applyAlignment();
        };

    }
}
