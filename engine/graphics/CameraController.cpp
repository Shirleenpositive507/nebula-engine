#include "CameraController.h"
#include <algorithm>
#include <cmath>

namespace nebula {
    namespace graphics {

        CameraController::CameraController(Camera& camera)
            : m_camera(&camera)
            , m_movementSpeed(500.f)
            , m_zoomSpeed(0.5f)
            , m_rotationSpeed(90.f)
            , m_dragSensitivity(1.f)
            , m_edgeScrollingEnabled(false)
            , m_edgeScrollMargin(20)
            , m_boundsClampingEnabled(false)
            , m_smoothInterpolation(false)
            , m_interpolationSpeed(8.f)
            , m_inputEnabled(true)
            , m_dragging(false)
            , m_targetPosition(camera.getPosition())
            , m_targetZoom(camera.getZoom()) {}

        void CameraController::setCamera(Camera& camera) {
            m_camera = &camera;
            m_targetPosition = camera.getPosition();
            m_targetZoom = camera.getZoom();
        }

        Camera& CameraController::getCamera() const {
            return *m_camera;
        }

        void CameraController::setBindings(const CameraControlBindings& bindings) {
            m_bindings = bindings;
        }

        CameraControlBindings CameraController::getBindings() const {
            return m_bindings;
        }

        void CameraController::setMovementSpeed(float speed) {
            m_movementSpeed = speed;
        }

        float CameraController::getMovementSpeed() const {
            return m_movementSpeed;
        }

        void CameraController::setZoomSpeed(float speed) {
            m_zoomSpeed = speed;
        }

        float CameraController::getZoomSpeed() const {
            return m_zoomSpeed;
        }

        void CameraController::setRotationSpeed(float speed) {
            m_rotationSpeed = speed;
        }

        float CameraController::getRotationSpeed() const {
            return m_rotationSpeed;
        }

        void CameraController::setDragSensitivity(float sensitivity) {
            m_dragSensitivity = sensitivity;
        }

        float CameraController::getDragSensitivity() const {
            return m_dragSensitivity;
        }

        void CameraController::setEdgeScrollingEnabled(bool enabled) {
            m_edgeScrollingEnabled = enabled;
        }

        bool CameraController::isEdgeScrollingEnabled() const {
            return m_edgeScrollingEnabled;
        }

        void CameraController::setEdgeScrollMargin(unsigned int margin) {
            m_edgeScrollMargin = margin;
        }

        unsigned int CameraController::getEdgeScrollMargin() const {
            return m_edgeScrollMargin;
        }

        void CameraController::setBoundsClampingEnabled(bool enabled) {
            m_boundsClampingEnabled = enabled;
        }

        bool CameraController::isBoundsClampingEnabled() const {
            return m_boundsClampingEnabled;
        }

        void CameraController::setBounds(const sf::FloatRect& bounds) {
            m_bounds = bounds;
            m_boundsClampingEnabled = true;
        }

        sf::FloatRect CameraController::getBounds() const {
            return m_bounds;
        }

        void CameraController::setSmoothInterpolation(bool enabled) {
            m_smoothInterpolation = enabled;
            if (!enabled) {
                m_camera->setPosition(m_targetPosition);
            }
        }

        bool CameraController::isSmoothInterpolationEnabled() const {
            return m_smoothInterpolation;
        }

        void CameraController::setInterpolationSpeed(float speed) {
            m_interpolationSpeed = speed;
        }

        float CameraController::getInterpolationSpeed() const {
            return m_interpolationSpeed;
        }

        void CameraController::setInputEnabled(bool enabled) {
            m_inputEnabled = enabled;
        }

        bool CameraController::isInputEnabled() const {
            return m_inputEnabled;
        }

        void CameraController::update(float dt, const sf::RenderWindow& window) {
            if (!m_inputEnabled || !m_camera) return;

            handleKeyboardInput(dt);
            handleMouseInput(dt, window);
            handleEdgeScrolling(dt, window);
            handleDrag(dt, window);

            for (auto& pair : m_customActions) {
                pair.second(*m_camera, dt);
            }

            clampPosition();
            interpolate(dt);

            m_camera->setPosition(m_targetPosition);
            m_camera->setZoom(m_targetZoom);
        }

        void CameraController::setCustomAction(const std::string& name, std::function<void(Camera&, float)> action) {
            m_customActions[name] = action;
        }

        void CameraController::removeCustomAction(const std::string& name) {
            m_customActions.erase(name);
        }

        void CameraController::clearCustomActions() {
            m_customActions.clear();
        }

        void CameraController::handleKeyboardInput(float dt) {
            sf::Vector2f movement(0.f, 0.f);

            if (sf::Keyboard::isKeyPressed(m_bindings.moveLeft)) movement.x -= 1.f;
            if (sf::Keyboard::isKeyPressed(m_bindings.moveRight)) movement.x += 1.f;
            if (sf::Keyboard::isKeyPressed(m_bindings.moveUp)) movement.y -= 1.f;
            if (sf::Keyboard::isKeyPressed(m_bindings.moveDown)) movement.y += 1.f;

            float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            if (length > 0.f) {
                movement /= length;
                m_targetPosition += movement * m_movementSpeed * dt * m_camera->getZoom();
            }

            if (sf::Keyboard::isKeyPressed(m_bindings.zoomIn)) {
                m_targetZoom = std::max(0.01f, m_targetZoom - m_zoomSpeed * dt);
            }
            if (sf::Keyboard::isKeyPressed(m_bindings.zoomOut)) {
                m_targetZoom = m_targetZoom + m_zoomSpeed * dt;
            }

            if (sf::Keyboard::isKeyPressed(m_bindings.rotateLeft)) {
                m_camera->setRotation(m_camera->getRotation() - m_rotationSpeed * dt);
            }
            if (sf::Keyboard::isKeyPressed(m_bindings.rotateRight)) {
                m_camera->setRotation(m_camera->getRotation() + m_rotationSpeed * dt);
            }

            if (sf::Keyboard::isKeyPressed(m_bindings.resetView)) {
                m_targetPosition = sf::Vector2f(0.f, 0.f);
                m_targetZoom = 1.f;
                m_camera->setRotation(0.f);
            }
        }

        void CameraController::handleMouseInput(float dt, const sf::RenderWindow& window) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2u windowSize = window.getSize();

            int scrollDelta = 0;
            (void)scrollDelta;

            float scrollWheel = 0.f;
            (void)scrollWheel;
        }

        void CameraController::handleEdgeScrolling(float dt, const sf::RenderWindow& window) {
            if (!m_edgeScrollingEnabled) return;

            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2u windowSize = window.getSize();
            sf::Vector2f movement(0.f, 0.f);

            if (static_cast<unsigned int>(mousePos.x) < m_edgeScrollMargin) movement.x -= 1.f;
            if (static_cast<unsigned int>(mousePos.x) > windowSize.x - m_edgeScrollMargin) movement.x += 1.f;
            if (static_cast<unsigned int>(mousePos.y) < m_edgeScrollMargin) movement.y -= 1.f;
            if (static_cast<unsigned int>(mousePos.y) > windowSize.y - m_edgeScrollMargin) movement.y += 1.f;

            float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            if (length > 0.f) {
                movement /= length;
                m_targetPosition += movement * m_movementSpeed * dt * m_camera->getZoom();
            }
        }

        void CameraController::handleDrag(float dt, const sf::RenderWindow& window) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
                if (!m_dragging) {
                    m_dragging = true;
                    m_dragStart = sf::Mouse::getPosition(window);
                    m_cameraPosAtDragStart = m_targetPosition;
                } else {
                    sf::Vector2i currentMouse = sf::Mouse::getPosition(window);
                    sf::Vector2i delta = currentMouse - m_dragStart;
                    m_targetPosition = m_cameraPosAtDragStart -
                        sf::Vector2f(static_cast<float>(delta.x), static_cast<float>(delta.y)) *
                        m_dragSensitivity * m_camera->getZoom();
                }
            } else {
                m_dragging = false;
            }
        }

        void CameraController::clampPosition() {
            if (!m_boundsClampingEnabled) return;

            sf::Vector2f viewSize = m_camera->getSize();
            float halfW = viewSize.x / 2.f;
            float halfH = viewSize.y / 2.f;

            m_targetPosition.x = std::max(m_bounds.left + halfW,
                std::min(m_bounds.left + m_bounds.width - halfW, m_targetPosition.x));
            m_targetPosition.y = std::max(m_bounds.top + halfH,
                std::min(m_bounds.top + m_bounds.height - halfH, m_targetPosition.y));
        }

        void CameraController::interpolate(float dt) {
            if (!m_smoothInterpolation) {
                m_camera->setPosition(m_targetPosition);
                m_camera->setZoom(m_targetZoom);
                return;
            }

            sf::Vector2f currentPos = m_camera->getPosition();
            float currentZoom = m_camera->getZoom();

            float t = 1.0f - std::exp(-m_interpolationSpeed * dt);
            sf::Vector2f newPos = currentPos + (m_targetPosition - currentPos) * t;
            float newZoom = currentZoom + (m_targetZoom - currentZoom) * t;

            m_camera->setPosition(newPos);
            m_camera->setZoom(newZoom);
        }

    }
}
