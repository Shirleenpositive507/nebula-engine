#pragma once

#include "Font.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>
#include <memory>
#include <stack>

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

        struct RichTextSpan {
            std::string text;
            sf::Color color;
            TextStyle style;
            unsigned int fontSize;
            bool isBold;
            bool isItalic;

            RichTextSpan() : color(255, 255, 255), style(TextStyle::Regular), fontSize(14),
                isBold(false), isItalic(false) {}
        };

        struct MarkupTag {
            std::string name;
            std::unordered_map<std::string, std::string> attributes;
            bool closing;

            MarkupTag() : closing(false) {}
        };

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

            void setRichText(const std::string& markupText);
            bool isRichText() const;
            void setRichTextEnabled(bool enabled);

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

            void setDropShadow(const sf::Vector2f& offset, const sf::Color& color, float blurRadius = 0.f);
            void disableDropShadow();
            bool hasDropShadow() const;

            void setLetterSpacing(float spacingFactor);
            float getLetterSpacing() const;

            void setLineSpacing(float spacingFactor);
            float getLineSpacing() const;

            void setEmojiFont(const std::shared_ptr<Font>& emojiFont);
            std::shared_ptr<Font> getEmojiFont() const;
            void setEmojiSupportEnabled(bool enabled);
            bool isEmojiSupportEnabled() const;

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

            bool m_richTextEnabled;
            std::string m_markupSource;
            std::vector<RichTextSpan> m_richSpans;

            bool m_dropShadowEnabled;
            sf::Vector2f m_shadowOffset;
            sf::Color m_shadowColor;
            float m_shadowBlur;

            bool m_emojiEnabled;
            std::shared_ptr<Font> m_emojiFont;

            void rebuild();
            std::vector<std::string> wrapText(const std::string& text, float maxWidth) const;
            void applyAlignment();
            void parseMarkup(const std::string& markup);
            std::vector<RichTextSpan> tokenizeMarkup(const std::string& markup) const;
            void buildRichTextGeometry();
            bool hasEmoji(const std::string& text) const;
            std::string replaceEmojiWithPlaceholder(const std::string& text) const;
        };

    }
}
