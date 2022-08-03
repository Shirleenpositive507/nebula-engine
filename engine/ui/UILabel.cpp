#include "UILabel.h"
#include <algorithm>

namespace nebula {
    namespace ui {

        UILabel::UILabel()
            : UIWidget("Label")
            , m_font(nullptr)
            , m_characterSize(14)
            , m_color(220, 220, 220)
            , m_alignment(TextAlignment::Left)
            , m_wordWrapWidth(0.f)
            , m_autoSize(true)
            , m_dirty(true) {
            m_size.x = 100.f;
            m_size.y = 20.f;
        }

        UILabel::UILabel(const std::string& text)
            : UIWidget("Label")
            , m_textString(text)
            , m_font(nullptr)
            , m_characterSize(14)
            , m_color(220, 220, 220)
            , m_alignment(TextAlignment::Left)
            , m_wordWrapWidth(0.f)
            , m_autoSize(true)
            , m_dirty(true) {
            m_size.x = 100.f;
            m_size.y = 20.f;
        }

        void UILabel::setText(const std::string& text) {
            m_textString = text;
            m_dirty = true;
        }

        std::string UILabel::getText() const {
            return m_textString;
        }

        void UILabel::setFont(sf::Font& font) {
            m_font = &font;
            m_dirty = true;
        }

        void UILabel::setCharacterSize(unsigned int size) {
            m_characterSize = size;
            m_dirty = true;
        }

        void UILabel::setColor(const graphics::Color& color) {
            m_color = color;
            m_dirty = true;
        }

        void UILabel::setAlignment(TextAlignment alignment) {
            m_alignment = alignment;
            m_dirty = true;
        }

        void UILabel::setWordWrap(float wrapWidth) {
            m_wordWrapWidth = wrapWidth;
            m_dirty = true;
        }

        void UILabel::setAutoSize(bool autoSize) {
            m_autoSize = autoSize;
            m_dirty = true;
        }

        sf::FloatRect UILabel::getTextBounds() const {
            return m_text.getLocalBounds();
        }

        void UILabel::rebuildText() {
            if (!m_font) return;

            m_text.setFont(*m_font);
            m_text.setString(m_textString);
            m_text.setCharacterSize(m_characterSize);
            m_text.setFillColor(m_color.toSFML());

            if (m_wordWrapWidth > 0.f) {
                std::string wrapped;
                std::string currentLine;
                std::string word;

                for (char c : m_textString) {
                    if (c == ' ' || c == '\n') {
                        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                        sf::Text test(m_text);
                        test.setString(testLine);
                        if (test.getLocalBounds().width > m_wordWrapWidth && !currentLine.empty()) {
                            wrapped += currentLine + "\n";
                            currentLine = word;
                        } else {
                            currentLine = currentLine.empty() ? word : currentLine + " " + word;
                        }
                        word.clear();
                        if (c == '\n') {
                            wrapped += currentLine + "\n";
                            currentLine.clear();
                        }
                    } else {
                        word += c;
                    }
                }
                if (!currentLine.empty() || !word.empty()) {
                    std::string last = currentLine.empty() ? word : currentLine + " " + word;
                    wrapped += last;
                }
                m_text.setString(wrapped);
            }

            if (m_autoSize) {
                sf::FloatRect bounds = m_text.getLocalBounds();
                m_size.x = bounds.width + style.padding * 2.f;
                m_size.y = bounds.height + style.padding * 2.f;
            }

            float textX = style.padding;
            if (m_alignment == TextAlignment::Center) {
                sf::FloatRect bounds = m_text.getLocalBounds();
                textX = (m_size.x - bounds.width) / 2.f;
            } else if (m_alignment == TextAlignment::Right) {
                sf::FloatRect bounds = m_text.getLocalBounds();
                textX = m_size.x - bounds.width - style.padding;
            }
            m_text.setPosition(textX, style.padding);

            m_dirty = false;
        }

        void UILabel::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            if (m_dirty) {
                rebuildText();
            }

            states.transform.translate(m_position);
            if (m_font) {
                target.draw(m_text, states);
            }

            UIWidget::onRender(target, states);
        }

        void UILabel::onLayout() {
            if (m_dirty) {
                rebuildText();
            }
            UIWidget::onLayout();
        }

    }
}
