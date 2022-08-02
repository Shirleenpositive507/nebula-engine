#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "UIStyle.h"

namespace nebula {
    namespace ui {

        class UIWidget : public std::enable_shared_from_this<UIWidget> {
        public:
            using Ptr = std::shared_ptr<UIWidget>;
            using Callback = std::function<void()>;
            using PointCallback = std::function<void(const sf::Vector2f&)>;

            UIWidget();
            explicit UIWidget(const std::string& name);
            virtual ~UIWidget() = default;

            void setPosition(float x, float y);
            void setPosition(const sf::Vector2f& pos);
            void setSize(float w, float h);
            void setSize(const sf::Vector2f& size);

            sf::Vector2f getPosition() const;
            sf::Vector2f getSize() const;
            sf::FloatRect getBounds() const;
            sf::FloatRect getGlobalBounds() const;
            bool containsPoint(const sf::Vector2f& point) const;

            void show();
            void hide();
            void enable();
            void disable();
            bool isEnabled() const;
            bool isVisible() const;

            void addChild(Ptr child);
            void removeChild(Ptr child);
            Ptr getParent() const;
            void setParent(Ptr parent);

            void bringToFront();
            void sendToBack();

            bool isHovered() const;
            bool isFocused() const;
            bool isPressed() const;
            bool isDisabled() const;

            virtual void onRender(sf::RenderTarget& target, sf::RenderStates states);
            virtual void onUpdate(float dt);
            virtual void onLayout();
            virtual bool onEvent(const sf::Event& event);

            std::string name;
            std::string id;
            bool enabled;
            bool visible;
            bool focusable;
            std::string tooltip;
            UIStyle style;

            Callback onClick;
            Callback onHover;
            PointCallback onDrag;
            Callback onRelease;

            int zOrder;

        protected:
            sf::Vector2f m_position;
            sf::Vector2f m_size;

            Ptr m_parent;
            std::vector<Ptr> m_children;

            bool m_hovered;
            bool m_focused;
            bool m_pressed;

            void sortChildrenByZOrder();
        };

    }
}
