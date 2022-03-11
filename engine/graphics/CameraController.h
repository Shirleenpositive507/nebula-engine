#pragma once

#include "Camera.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <unordered_map>
#include <functional>

namespace nebula {
    namespace graphics {

        struct CameraControlBindings {
            sf::Keyboard::Key moveUp;
            sf::Keyboard::Key moveDown;
            sf::Keyboard::Key moveLeft;
            sf::Keyboard::Key moveRight;
            sf::Keyboard::Key zoomIn;
            sf::Keyboard::Key zoomOut;
            sf::Keyboard::Key rotateLeft;
            sf::Keyboard::Key rotateRight;
            sf::Keyboard::Key resetView;

            CameraControlBindings()
                : moveUp(sf::Keyboard::W)
                , moveDown(sf::Keyboard::S)
                , moveLeft(sf::Keyboard::A)
                , moveRight(sf::Keyboard::D)
                , zoomIn(sf::Keyboard::E)
                , zoomOut(sf::Keyboard::Q)
                , rotateLeft(sf::Keyboard::Num1)
                , rotateRight(sf::Keyboard::Num3)
                , resetView(sf::Keyboard::Num5) {}
        };

        class CameraController {
        public:
            explicit CameraController(Camera& camera);

            void setCamera(Camera& camera);
            Camera& getCamera() const;

            void setBindings(const CameraControlBindings& bindings);
            CameraControlBindings getBindings() const;

            void setMovementSpeed(float speed);
            float getMovementSpeed() const;

            void setZoomSpeed(float speed);
            float getZoomSpeed() const;

            void setRotationSpeed(float speed);
            float getRotationSpeed() const;

            void setDragSensitivity(float sensitivity);
            float getDragSensitivity() const;

            void setEdgeScrollingEnabled(bool enabled);
            bool isEdgeScrollingEnabled() const;

            void setEdgeScrollMargin(unsigned int margin);
            unsigned int getEdgeScrollMargin() const;

            void setBoundsClampingEnabled(bool enabled);
            bool isBoundsClampingEnabled() const;

            void setBounds(const sf::FloatRect& bounds);
            sf::FloatRect getBounds() const;

            void setSmoothInterpolation(bool enabled);
            bool isSmoothInterpolationEnabled() const;

            void setInterpolationSpeed(float speed);
            float getInterpolationSpeed() const;

            void setInputEnabled(bool enabled);
            bool isInputEnabled() const;

            void update(float dt, const sf::RenderWindow& window);

            void setCustomAction(const std::string& name, std::function<void(Camera&, float)> action);
            void removeCustomAction(const std::string& name);
            void clearCustomActions();

        private:
            Camera* m_camera;
            CameraControlBindings m_bindings;

            float m_movementSpeed;
            float m_zoomSpeed;
            float m_rotationSpeed;
            float m_dragSensitivity;

            bool m_edgeScrollingEnabled;
            unsigned int m_edgeScrollMargin;

            bool m_boundsClampingEnabled;
            sf::FloatRect m_bounds;

            bool m_smoothInterpolation;
            float m_interpolationSpeed;
            sf::Vector2f m_targetPosition;
            float m_targetZoom;

            bool m_inputEnabled;

            bool m_dragging;
            sf::Vector2i m_dragStart;
            sf::Vector2f m_cameraPosAtDragStart;

            std::unordered_map<std::string, std::function<void(Camera&, float)>> m_customActions;

            void handleKeyboardInput(float dt);
            void handleMouseInput(float dt, const sf::RenderWindow& window);
            void handleEdgeScrolling(float dt, const sf::RenderWindow& window);
            void handleDrag(float dt, const sf::RenderWindow& window);
            void clampPosition();
            void interpolate(float dt);
        };

    }
}
