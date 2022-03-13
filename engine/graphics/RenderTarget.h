#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include "Color.h"
#include "Viewport.h"
#include "BlendMode.h"
#include <stack>

namespace nebula {
    namespace graphics {

        class RenderTarget {
        public:
            RenderTarget();
            virtual ~RenderTarget();

            virtual void clear(const Color& color = Color::Black);
            virtual void clear(const sf::Color& color);

            virtual void draw(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default);
            virtual void draw(const sf::VertexArray& vertexArray, const sf::RenderStates& states = sf::RenderStates::Default);
            virtual void draw(const sf::Vertex* vertices, std::size_t count,
                              sf::PrimitiveType type, const sf::RenderStates& states = sf::RenderStates::Default);

            virtual sf::Vector2u getSize() const = 0;

            virtual void setView(const sf::View& view);
            virtual void setView(const Viewport& viewport);
            virtual sf::View getView() const;
            virtual sf::View getDefaultView() const;

            virtual void setActive(bool active = true);

            virtual void pushGLStates();
            virtual void popGLStates();
            virtual void resetGLStates();

            virtual sf::RenderTarget& getSFMLTarget() = 0;
            virtual const sf::RenderTarget& getSFMLTarget() const = 0;

            void pushView(const sf::View& view);
            void popView();
            std::size_t getViewStackSize() const;

            void setBlendMode(const BlendState& state);
            BlendState getBlendMode() const;

            void setViewport(const sf::FloatRect& viewport);
            sf::FloatRect getViewport() const;

        protected:
            void applyBlendMode();

            std::stack<sf::View> m_viewStack;
            BlendState m_blendState;
            sf::FloatRect m_viewport;
        };

    }
}
