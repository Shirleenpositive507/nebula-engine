#include "UITextBox.h"
#include <algorithm>
#include <SFML/Window/Clipboard.hpp>

namespace nebula {
    namespace ui {

        UITextBox::UITextBox()
            : UIWidget("TextBox")
            , readOnly(false)
            , multiline(false)
            , isPassword(false)
            , m_maxLength(256)
            , m_cursorPos(0)
            , m_selectionStart(0)
            , m_selectionEnd(0)
            , m_hasSelection(false)
            , m_font(nullptr)
            , m_fontSize(14)
            , m_textColor(220, 220, 220)
            , m_placeholderColor(120, 120, 120)
            , m_cursorColor(220, 220, 220)
            , m_selectionColor(41, 128, 185)
            , m_backgroundColor(30, 30, 30)
            , m_cursorTimer(0.f)
            , m_cursorVisible(true)
            , m_dirty(true) {
            m_size.x = 200.f;
            m_size.y = 30.f;
            focusable = true;

            m_background.setFillColor(m_backgroundColor.toSFML());
            m_background.setOutlineThickness(1.f);
            m_background.setOutlineColor(sf::Color(80, 80, 80));

            m_cursor.setFillColor(m_cursorColor.toSFML());
            m_cursor.setSize(sf::Vector2f(2.f, static_cast<float>(m_fontSize) + 4.f));

            m_selectionHighlight.setFillColor(m_selectionColor.toSFML());
        }

        void UITextBox::setText(const std::string& text) {
            m_text = text;
            if (static_cast<int>(m_text.length()) > m_maxLength) {
                m_text = m_text.substr(0, m_maxLength);
            }
            m_cursorPos = static_cast<int>(m_text.length());
            m_hasSelection = false;
            m_dirty = true;
        }

        std::string UITextBox::getText() const {
            return m_text;
        }

        void UITextBox::append(const std::string& text) {
            m_text += text;
            if (static_cast<int>(m_text.length()) > m_maxLength) {
                m_text = m_text.substr(0, m_maxLength);
            }
            m_cursorPos = static_cast<int>(m_text.length());
            m_dirty = true;
        }

        void UITextBox::clear() {
            m_text.clear();
            m_cursorPos = 0;
            m_hasSelection = false;
            m_dirty = true;
        }

        void UITextBox::setCursorPosition(int pos) {
            m_cursorPos = std::max(0, std::min(static_cast<int>(m_text.length()), pos));
            m_cursorTimer = 0.f;
            m_cursorVisible = true;
        }

        int UITextBox::getCursorPosition() const {
            return m_cursorPos;
        }

        void UITextBox::selectAll() {
            m_selectionStart = 0;
            m_selectionEnd = static_cast<int>(m_text.length());
            m_hasSelection = m_selectionEnd > m_selectionStart;
            m_dirty = true;
        }

        void UITextBox::setSelection(int start, int end) {
            m_selectionStart = std::max(0, start);
            m_selectionEnd = std::min(static_cast<int>(m_text.length()), end);
            m_hasSelection = m_selectionEnd > m_selectionStart;
            m_dirty = true;
        }

        std::pair<int, int> UITextBox::getSelection() const {
            return {m_selectionStart, m_selectionEnd};
        }

        void UITextBox::copy() {
            if (m_hasSelection) {
                sf::Clipboard::setString(
                    m_text.substr(m_selectionStart, m_selectionEnd - m_selectionStart)
                );
            }
        }

        void UITextBox::cut() {
            if (!readOnly && m_hasSelection) {
                copy();
                m_text.erase(m_selectionStart, m_selectionEnd - m_selectionStart);
                m_cursorPos = m_selectionStart;
                m_hasSelection = false;
                m_dirty = true;
            }
        }

        void UITextBox::paste() {
            if (readOnly) return;
            std::string clipboard = sf::Clipboard::getString();
            if (m_hasSelection) {
                m_text.erase(m_selectionStart, m_selectionEnd - m_selectionStart);
                m_cursorPos = m_selectionStart;
                m_hasSelection = false;
            }
            m_text.insert(m_cursorPos, clipboard);
            m_cursorPos += static_cast<int>(clipboard.length());
            if (static_cast<int>(m_text.length()) > m_maxLength) {
                m_text = m_text.substr(0, m_maxLength);
                m_cursorPos = std::min(m_cursorPos, m_maxLength);
            }
            m_dirty = true;
        }

        void UITextBox::setPlaceholder(const std::string& text) {
            m_placeholder = text;
            m_dirty = true;
        }

        void UITextBox::setMaxLength(int max) {
            m_maxLength = std::max(1, max);
            if (static_cast<int>(m_text.length()) > m_maxLength) {
                m_text = m_text.substr(0, m_maxLength);
                m_cursorPos = std::min(m_cursorPos, m_maxLength);
                m_dirty = true;
            }
        }

        std::string UITextBox::getPasswordString() const {
            return std::string(m_text.length(), '*');
        }

        void UITextBox::rebuildDisplay() {
            std::string displayStr = isPassword ? getPasswordString() : m_text;

            if (m_font) {
                m_displayText.setFont(*m_font);
                m_placeholderText.setFont(*m_font);
            }
            m_displayText.setString(displayStr);
            m_displayText.setCharacterSize(m_fontSize);
            m_displayText.setFillColor(m_textColor.toSFML());

            m_placeholderText.setString(m_placeholder);
            m_placeholderText.setCharacterSize(m_fontSize);
            m_placeholderText.setFillColor(m_placeholderColor.toSFML());
            m_placeholderText.setPosition(4.f, (m_size.y - m_fontSize) / 2.f);

            m_background.setSize(m_size);
            m_displayText.setPosition(4.f, (m_size.y - m_fontSize) / 2.f);

            updateCursorPosition();

            m_dirty = false;
        }

        void UITextBox::updateCursorPosition() {
            std::string str = isPassword ? getPasswordString() : m_text;
            std::string beforeCursor = str.substr(0, m_cursorPos);
            sf::Text temp;
            if (m_font) temp.setFont(*m_font);
            temp.setString(beforeCursor);
            temp.setCharacterSize(m_fontSize);
            float cursorX = 4.f + temp.getLocalBounds().width;
            float cursorY = (m_size.y - static_cast<float>(m_fontSize) - 4.f) / 2.f;
            m_cursor.setPosition(cursorX, cursorY);
            m_cursor.setSize(sf::Vector2f(2.f, static_cast<float>(m_fontSize) + 4.f));
        }

        void UITextBox::insertChar(char c) {
            if (readOnly) return;
            if (m_hasSelection) {
                m_text.erase(m_selectionStart, m_selectionEnd - m_selectionStart);
                m_cursorPos = m_selectionStart;
                m_hasSelection = false;
            }
            if (static_cast<int>(m_text.length()) >= m_maxLength) return;
            m_text.insert(m_cursorPos, 1, c);
            m_cursorPos++;
            m_dirty = true;
        }

        void UITextBox::deleteChar(bool backward) {
            if (readOnly) return;
            if (m_hasSelection) {
                m_text.erase(m_selectionStart, m_selectionEnd - m_selectionStart);
                m_cursorPos = m_selectionStart;
                m_hasSelection = false;
                m_dirty = true;
                return;
            }
            if (backward && m_cursorPos > 0) {
                m_text.erase(m_cursorPos - 1, 1);
                m_cursorPos--;
                m_dirty = true;
            } else if (!backward && m_cursorPos < static_cast<int>(m_text.length())) {
                m_text.erase(m_cursorPos, 1);
                m_dirty = true;
            }
        }

        void UITextBox::moveCursor(int delta) {
            m_cursorPos = std::max(0, std::min(static_cast<int>(m_text.length()), m_cursorPos + delta));
            m_cursorTimer = 0.f;
            m_cursorVisible = true;
            m_dirty = true;
        }

        bool UITextBox::onEvent(const sf::Event& event) {
            if (!enabled || !visible) return false;

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                if (containsPoint(mousePos)) {
                    m_focused = true;
                    float localX = mousePos.x - m_position.x - 4.f;
                    std::string str = isPassword ? getPasswordString() : m_text;
                    int closestPos = 0;
                    for (int i = 0; i <= static_cast<int>(str.length()); ++i) {
                        sf::Text temp;
                        if (m_font) temp.setFont(*m_font);
                        temp.setString(str.substr(0, i));
                        temp.setCharacterSize(m_fontSize);
                        float charX = temp.getLocalBounds().width;
                        if (std::abs(charX - localX) < std::abs(
                            [&]() {
                                sf::Text t;
                                if (m_font) t.setFont(*m_font);
                                t.setString(str.substr(0, closestPos));
                                t.setCharacterSize(m_fontSize);
                                return t.getLocalBounds().width;
                            }() - localX)) {
                            closestPos = i;
                        }
                    }
                    m_cursorPos = closestPos;
                    m_hasSelection = false;
                    m_cursorTimer = 0.f;
                    m_cursorVisible = true;
                    m_dirty = true;
                    return true;
                } else {
                    m_focused = false;
                }
            }

            if (event.type == sf::Event::TextEntered && m_focused && !readOnly) {
                if (event.text.unicode >= 32 && event.text.unicode < 127) {
                    insertChar(static_cast<char>(event.text.unicode));
                    return true;
                }
            }

            if (event.type == sf::Event::KeyPressed && m_focused) {
                switch (event.key.code) {
                    case sf::Keyboard::Backspace:
                        deleteChar(true);
                        return true;
                    case sf::Keyboard::Delete:
                        deleteChar(false);
                        return true;
                    case sf::Keyboard::Enter:
                        if (multiline) {
                            insertChar('\n');
                        }
                        return true;
                    case sf::Keyboard::Left:
                        moveCursor(-1);
                        return true;
                    case sf::Keyboard::Right:
                        moveCursor(1);
                        return true;
                    case sf::Keyboard::Home:
                        setCursorPosition(0);
                        return true;
                    case sf::Keyboard::End:
                        setCursorPosition(static_cast<int>(m_text.length()));
                        return true;
                    case sf::Keyboard::C:
                        if (event.key.control) { copy(); return true; }
                        break;
                    case sf::Keyboard::X:
                        if (event.key.control) { cut(); return true; }
                        break;
                    case sf::Keyboard::V:
                        if (event.key.control) { paste(); return true; }
                        break;
                    case sf::Keyboard::A:
                        if (event.key.control) { selectAll(); return true; }
                        break;
                    default:
                        break;
                }
            }

            return UIWidget::onEvent(event);
        }

        void UITextBox::onRender(sf::RenderTarget& target, sf::RenderStates states) {
            if (!visible) return;

            if (m_dirty) rebuildDisplay();

            states.transform.translate(m_position);

            target.draw(m_background, states);

            if (m_text.empty() && !m_focused && !m_placeholder.empty()) {
                target.draw(m_placeholderText, states);
            } else {
                target.draw(m_displayText, states);
            }

            if (m_focused && m_cursorVisible) {
                target.draw(m_cursor, states);
            }

            UIWidget::onRender(target, states);
        }

        void UITextBox::onUpdate(float dt) {
            UIWidget::onUpdate(dt);

            if (m_focused) {
                m_cursorTimer += dt;
                if (m_cursorTimer >= 0.5f) {
                    m_cursorVisible = !m_cursorVisible;
                    m_cursorTimer = 0.f;
                }
            } else {
                m_cursorVisible = false;
            }
        }

    }
}
