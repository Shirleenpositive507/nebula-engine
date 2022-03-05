#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>

namespace nebula {
    namespace graphics {

        class Viewport {
        public:
            Viewport();
            explicit Viewport(const sf::View& view);
            Viewport(const sf::FloatRect& viewport);
            Viewport(const sf::Vector2f& center, const sf::Vector2f& size);

            void setCenter(const sf::Vector2f& center);
            void setCenter(float x, float y);
            void setSize(const sf::Vector2f& size);
            void setSize(float width, float height);
            void setRotation(float angle);
            void setViewport(const sf::FloatRect& viewport);

            void move(const sf::Vector2f& offset);
            void move(float offsetX, float offsetY);
            void zoom(float factor);
            void rotate(float angle);
            void reset(const sf::FloatRect& rectangle);

            sf::Vector2f getCenter() const;
            sf::Vector2f getSize() const;
            float getRotation() const;
            sf::FloatRect getViewport() const;

            sf::Transform getTransform() const;
            sf::Transform getInverseTransform() const;

            sf::View toSFML() const;
            static Viewport fromSFML(const sf::View& view);

            void setSplitScreen(const Viewport& other, float splitRatio, bool horizontal = true);
            void fitToAspectRatio(float aspectRatio);
            void setBounds(float left, float top, float right, float bottom);
            sf::FloatRect getBounds() const;

            bool contains(const sf::Vector2f& point) const;
            sf::Vector2f screenToWorld(const sf::Vector2f& screenPoint, const sf::Vector2u& windowSize) const;
            sf::Vector2f worldToScreen(const sf::Vector2f& worldPoint, const sf::Vector2u& windowSize) const;

        private:
            sf::View m_view;
        };

    }
}
