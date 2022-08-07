#pragma once

#include "UIWidget.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <string>

namespace nebula {
    namespace ui {

        class UITextBox : public UIWidget {
        public:
            UITextBox();

            void setText(const std::string& text);
            std::string getText() const;
            void append(const std::string& text);
            void clear();

            void setCursorPosition(int pos);
            int getCursorPosition() const;

            void selectAll();
            void setSelection(int start, int end);
            std::pair<int, int> getSelection() const;

            void copy();
            void cut();
            void paste();

            void setPlaceholder(const std::string& text);
            void setMaxLength(int max);

            bool onEvent(const sf::Event& event) override;
            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            void onUpdate(float dt) override;

            bool readOnly;
            bool multiline;
            bool isPassword;

        private:
            std::string m_text;
            std::string m_placeholder;
            int m_maxLength;
            int m_cursorPos;
            int m_selectionStart;
            int m_selectionEnd;
            bool m_hasSelection;

            sf::Text m_displayText;
            sf::Text m_placeholderText;
            sf::RectangleShape m_background;
            sf::RectangleShape m_cursor;
            sf::RectangleShape m_selectionHighlight;
            sf::Font* m_font;
            unsigned int m_fontSize;
            graphics::Color m_textColor;
            graphics::Color m_placeholderColor;
            graphics::Color m_cursorColor;
            graphics::Color m_selectionColor;
            graphics::Color m_backgroundColor;

            float m_cursorTimer;
            bool m_cursorVisible;
            bool m_dirty;

            void rebuildDisplay();
            void updateCursorPosition();
            void insertChar(char c);
            void deleteChar(bool backward);
            void moveCursor(int delta);
            std::string getPasswordString() const;
        };

    }
}
