#pragma once

#include "UIWidget.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <memory>

namespace nebula {
    namespace ui {

        class UIPanel : public UIWidget {
        public:
            UIPanel();

            void setScrollPosition(float x, float y);
            sf::Vector2f getScrollPosition() const;
            void setScrollSize(float w, float h);
            sf::Vector2f getScrollSize() const;
            void scrollTo(std::shared_ptr<UIWidget> child);
            void scrollToBottom();
            bool hasScrollBar() const;

            void setBackgroundColor(const graphics::Color& color);
            void setBorder(float size, const graphics::Color& color);
            void setCornerRadius(float radius);

            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            void onUpdate(float dt) override;
            bool onEvent(const sf::Event& event) override;

            bool scrollHorizontal;
            bool scrollVertical;
            bool autoScroll;
            bool clipChildren;

        private:
            sf::Vector2f m_scrollPosition;
            sf::Vector2f m_scrollSize;
            sf::RectangleShape m_background;
            graphics::Color m_bgColor;
            float m_borderSize;
            graphics::Color m_borderColor;
            float m_cornerRadius;

            void applyClipping(sf::RenderTarget& target, sf::RenderStates& states) const;
        };

    }
}
