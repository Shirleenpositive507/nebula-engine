#include "TextRenderer.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace nebula {
    namespace graphics {

        TextRenderer::TextRenderer()
            : m_alignment(TextAlignment::Left)
            , m_maxWidth(0.f)
            , m_wordWrap(false)
            , m_autoSize(false)
            , m_dirty(true) {}

        TextRenderer::TextRenderer(const std::string& text)
            : m_string(text)
            , m_alignment(TextAlignment::Left)
            , m_maxWidth(0.f)
            , m_wordWrap(false)
            , m_autoSize(false)
            , m_dirty(true) {
            m_text.setString(text);
        }

        TextRenderer::TextRenderer(const std::string& text, const std::shared_ptr<Font>& font)
            : m_font(font)
            , m_string(text)
            , m_alignment(TextAlignment::Left)
            , m_maxWidth(0.f)
            , m_wordWrap(false)
            , m_autoSize(false)
            , m_dirty(true) {
            if (font) {
                m_text.setFont(font->getFont());
            }
            m_text.setString(text);
        }

        void TextRenderer::setFont(const std::shared_ptr<Font>& font) {
            m_font = font;
            if (font) {
                m_text.setFont(font->getFont());
            }
            m_dirty = true;
        }

        std::shared_ptr<Font> TextRenderer::getFont() const {
            return m_font;
        }

        void TextRenderer::setString(const std::string& text) {
            m_string = text;
            m_dirty = true;
        }

        const std::string& TextRenderer::getString() const {
            return m_string;
        }

        void TextRenderer::setCharacterSize(unsigned int size) {
            m_text.setCharacterSize(size);
            m_dirty = true;
        }

        unsigned int TextRenderer::getCharacterSize() const {
            return m_text.getCharacterSize();
        }

        void TextRenderer::setStyle(int style) {
            m_text.setStyle(style);
            m_dirty = true;
        }

        void TextRenderer::setStyle(TextStyle style) {
            m_text.setStyle(static_cast<int>(style));
            m_dirty = true;
        }

        int TextRenderer::getStyle() const {
            return m_text.getStyle();
        }

        void TextRenderer::setColor(const sf::Color& color) {
            m_text.setFillColor(color);
        }

        sf::Color TextRenderer::getColor() const {
            return m_text.getFillColor();
        }

        void TextRenderer::setOutlineColor(const sf::Color& color) {
            m_text.setOutlineColor(color);
        }

        sf::Color TextRenderer::getOutlineColor() const {
            return m_text.getOutlineColor();
        }

        void TextRenderer::setOutlineThickness(float thickness) {
            m_text.setOutlineThickness(thickness);
        }

        float TextRenderer::getOutlineThickness() const {
            return m_text.getOutlineThickness();
        }

        void TextRenderer::setLetterSpacing(float spacingFactor) {
            m_text.setLetterSpacing(spacingFactor);
            m_dirty = true;
        }

        float TextRenderer::getLetterSpacing() const {
            return m_text.getLetterSpacing();
        }

        void TextRenderer::setLineSpacing(float spacingFactor) {
            m_text.setLineSpacing(spacingFactor);
            m_dirty = true;
        }

        float TextRenderer::getLineSpacing() const {
            return m_text.getLineSpacing();
        }

        sf::FloatRect TextRenderer::getLocalBounds() const {
            if (m_dirty) {
                const_cast<TextRenderer*>(this)->rebuild();
            }
            return m_text.getLocalBounds();
        }

        sf::FloatRect TextRenderer::getGlobalBounds() const {
            if (m_dirty) {
                const_cast<TextRenderer*>(this)->rebuild();
            }
            return m_text.getGlobalBounds();
        }

        sf::Vector2f TextRenderer::getCharacterPos(std::size_t index) const {
            if (m_dirty) {
                const_cast<TextRenderer*>(this)->rebuild();
            }
            return m_text.findCharacterPos(static_cast<std::size_t>(index));
        }

        std::size_t TextRenderer::findCharacterPos(const sf::Vector2f& point) const {
            if (m_dirty) {
                const_cast<TextRenderer*>(this)->rebuild();
            }

            std::size_t bestIndex = 0;
            float bestDist = std::numeric_limits<float>::max();

            for (std::size_t i = 0; i <= m_string.length(); ++i) {
                sf::Vector2f charPos = m_text.findCharacterPos(i);
                float dist = std::sqrt(
                    (point.x - charPos.x) * (point.x - charPos.x) +
                    (point.y - charPos.y) * (point.y - charPos.y));
                if (dist < bestDist) {
                    bestDist = dist;
                    bestIndex = i;
                }
            }

            return bestIndex;
        }

        void TextRenderer::setAlignment(TextAlignment alignment) {
            m_alignment = alignment;
            m_dirty = true;
        }

        TextAlignment TextRenderer::getAlignment() const {
            return m_alignment;
        }

        void TextRenderer::setMaxWidth(float maxWidth) {
            m_maxWidth = maxWidth;
            m_dirty = true;
        }

        float TextRenderer::getMaxWidth() const {
            return m_maxWidth;
        }

        bool TextRenderer::isWordWrapping() const {
            return m_wordWrap;
        }

        void TextRenderer::setWordWrap(bool enabled) {
            m_wordWrap = enabled;
            m_dirty = true;
        }

        void TextRenderer::setAutoSize(bool enabled) {
            m_autoSize = enabled;
            m_dirty = true;
        }

        void TextRenderer::update(float maxWidth) {
            if (maxWidth > 0.f) {
                m_maxWidth = maxWidth;
            }
            rebuild();
        }

        void TextRenderer::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
            if (m_dirty) {
                const_cast<TextRenderer*>(this)->rebuild();
            }
            target.draw(m_text, states);
        }

        sf::Text& TextRenderer::getSFMLText() {
            return m_text;
        }

        const sf::Text& TextRenderer::getSFMLText() const {
            return m_text;
        }

        void TextRenderer::rebuild() {
            if (!m_dirty) return;

            float effectiveMaxWidth = m_maxWidth;

            if (m_wordWrap && effectiveMaxWidth > 0.f) {
                m_lines = wrapText(m_string, effectiveMaxWidth);

                std::string wrapped;
                for (std::size_t i = 0; i < m_lines.size(); ++i) {
                    if (i > 0) wrapped += '\n';
                    wrapped += m_lines[i];
                }
                m_text.setString(wrapped);
            } else {
                m_lines.clear();
                m_lines.push_back(m_string);
                m_text.setString(m_string);
            }

            applyAlignment();

            if (m_autoSize && m_font) {
                sf::FloatRect bounds = m_text.getLocalBounds();
                float scaleX = effectiveMaxWidth > 0.f ? effectiveMaxWidth / bounds.width : 1.f;
                float scaleY = scaleX;

                if (scaleX < 1.f || scaleY < 1.f) {
                    float scale = std::min(scaleX, scaleY);
                    m_text.setScale(scale, scale);
                }
            }

            m_dirty = false;
        }

        std::vector<std::string> TextRenderer::wrapText(const std::string& text, float maxWidth) const {
            std::vector<std::string> lines;
            if (!m_font) {
                lines.push_back(text);
                return lines;
            }

            std::istringstream stream(text);
            std::string word;
            std::string currentLine;
            float charSize = static_cast<float>(m_text.getCharacterSize());

            while (stream >> word) {
                std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                sf::Text testText(testLine, m_font->getFont(), m_text.getCharacterSize());
                testText.setStyle(m_text.getStyle());
                testText.setLetterSpacing(m_text.getLetterSpacing());

                if (testText.getLocalBounds().width > maxWidth && !currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = word;
                } else {
                    currentLine = testLine;
                }
            }

            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }

            if (lines.empty()) {
                lines.push_back("");
            }

            return lines;
        }

        void TextRenderer::applyAlignment() {
            if (m_alignment == TextAlignment::Left || !m_font || m_lines.size() <= 1) {
                return;
            }

            float charSize = static_cast<float>(m_text.getCharacterSize());
            float totalHeight = m_lines.size() * m_font->getLineSpacing(m_text.getCharacterSize());
            float maxLineWidth = 0.f;

            std::vector<float> lineWidths;
            for (const auto& line : m_lines) {
                sf::Text measure(line, m_font->getFont(), m_text.getCharacterSize());
                measure.setStyle(m_text.getStyle());
                lineWidths.push_back(measure.getLocalBounds().width);
                maxLineWidth = std::max(maxLineWidth, lineWidths.back());
            }

            if (m_alignment == TextAlignment::Center) {
                float totalOffset = 0.f;
                std::string aligned;

                for (std::size_t i = 0; i < m_lines.size(); ++i) {
                    float lineWidth = lineWidths[i];
                    float offset = (maxLineWidth - lineWidth) * 0.5f;

                    if (offset > 0.f) {
                        int spaceCount = static_cast<int>(offset / (m_font->getGlyph(' ', m_text.getCharacterSize()).advance * m_text.getLetterSpacing()));
                        if (spaceCount > 0) {
                            if (i > 0) aligned += '\n';
                            aligned += std::string(static_cast<std::size_t>(spaceCount), ' ') + m_lines[i];
                            continue;
                        }
                    }

                    if (i > 0) aligned += '\n';
                    aligned += m_lines[i];
                }

                m_text.setString(aligned);
            }
        }

    }
}
