#include "Camera.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace nebula {
    namespace graphics {

        Camera::Camera()
            : m_position(0.f, 0.f)
            , m_size(800.f, 600.f)
            , m_rotation(0.f)
            , m_zoom(1.f)
            , m_projectionMode(ProjectionMode::Orthographic)
            , m_orthographicSize(5.f)
            , m_aspectRatioLock(false)
            , m_aspectRatio(1.f)
            , m_pixelPerfect(false)
            , m_renderOrder(RenderOrder::Default)
            , m_shakeTime(0.f)
            , m_shakeOffset(0.f, 0.f)
            , m_smoothPosition(0.f, 0.f) {}

        Camera::Camera(const sf::Vector2f& position)
            : m_position(position)
            , m_size(800.f, 600.f)
            , m_rotation(0.f)
            , m_zoom(1.f)
            , m_projectionMode(ProjectionMode::Orthographic)
            , m_orthographicSize(5.f)
            , m_aspectRatioLock(false)
            , m_aspectRatio(1.f)
            , m_pixelPerfect(false)
            , m_renderOrder(RenderOrder::Default)
            , m_shakeTime(0.f)
            , m_shakeOffset(0.f, 0.f)
            , m_smoothPosition(position) {}

        Camera::Camera(const sf::Vector2f& position, float rotation, float zoom)
            : m_position(position)
            , m_size(800.f, 600.f)
            , m_rotation(rotation)
            , m_zoom(zoom)
            , m_projectionMode(ProjectionMode::Orthographic)
            , m_orthographicSize(5.f)
            , m_aspectRatioLock(false)
            , m_aspectRatio(1.f)
            , m_pixelPerfect(false)
            , m_renderOrder(RenderOrder::Default)
            , m_shakeTime(0.f)
            , m_shakeOffset(0.f, 0.f)
            , m_smoothPosition(position) {}

        void Camera::setPosition(const sf::Vector2f& position) {
            m_position = position;
            m_smoothPosition = position;
        }

        void Camera::setPosition(float x, float y) {
            setPosition(sf::Vector2f(x, y));
        }

        sf::Vector2f Camera::getPosition() const {
            return m_position;
        }

        void Camera::setRotation(float degrees) {
            m_rotation = degrees;
        }

        float Camera::getRotation() const {
            return m_rotation;
        }

        void Camera::setZoom(float zoom) {
            m_zoom = std::max(0.01f, zoom);
        }

        float Camera::getZoom() const {
            return m_zoom;
        }

        void Camera::setViewport(const sf::FloatRect& viewport) {
            m_viewport.setViewport(viewport);
        }

        sf::FloatRect Camera::getViewport() const {
            return m_viewport.getViewport();
        }

        void Camera::setProjectionMode(ProjectionMode mode) {
            m_projectionMode = mode;
        }

        ProjectionMode Camera::getProjectionMode() const {
            return m_projectionMode;
        }

        void Camera::setSize(const sf::Vector2f& size) {
            m_size = size;
            m_viewport.setSize(size);
        }

        sf::Vector2f Camera::getSize() const {
            return m_size * m_zoom;
        }

        sf::Transform Camera::getViewMatrix() const {
            sf::Transform transform;
            sf::Vector2f pos = applyShake(m_position);
            transform.translate(-pos.x, -pos.y);
            transform.rotate(-m_rotation);
            return transform;
        }

        sf::Transform Camera::getProjectionMatrix() const {
            sf::Transform transform;
            if (m_projectionMode == ProjectionMode::Orthographic) {
                float halfW = m_size.x * m_zoom / 2.f;
                float halfH = m_size.y * m_zoom / 2.f;
                transform.scale(2.f / (halfW * 2.f), -2.f / (halfH * 2.f));
                transform.translate(-1.f, 1.f);
            }
            return transform;
        }

        sf::Transform Camera::getViewProjectionMatrix() const {
            return getProjectionMatrix() * getViewMatrix();
        }

        void Camera::lookAt(const sf::Vector2f& target) {
            m_position = target;
        }

        void Camera::orbit(const sf::Vector2f& center, float angleDelta, float radiusDelta) {
            sf::Vector2f offset = m_position - center;
            float radius = std::sqrt(offset.x * offset.x + offset.y * offset.y);
            float angle = std::atan2(offset.y, offset.x);

            angle += angleDelta * 3.14159265f / 180.f;
            radius = std::max(0.f, radius + radiusDelta);

            m_position = center + sf::Vector2f(
                radius * std::cos(angle),
                radius * std::sin(angle)
            );
        }

        sf::Vector2f Camera::screenToWorld(const sf::Vector2f& screenPoint, const sf::Vector2u& windowSize) const {
            return m_viewport.screenToWorld(screenPoint, windowSize);
        }

        sf::Vector2f Camera::worldToScreen(const sf::Vector2f& worldPoint, const sf::Vector2u& windowSize) const {
            return m_viewport.worldToScreen(worldPoint, windowSize);
        }

        void Camera::shake(float intensity, float duration, float frequency) {
            m_shake.intensity = intensity;
            m_shake.duration = duration;
            m_shake.frequency = frequency;
            m_shakeTime = duration;
            m_shakeOffset = sf::Vector2f(0.f, 0.f);
        }

        void Camera::updateShake(float dt) {
            if (m_shakeTime <= 0.f) {
                m_shakeOffset = sf::Vector2f(0.f, 0.f);
                return;
            }

            m_shakeTime -= dt;
            if (m_shakeTime < 0.f) m_shakeTime = 0.f;

            float intensity = m_shake.intensity;
            if (m_shake.fadeOut) {
                intensity *= (m_shakeTime / m_shake.duration);
            }

            static std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> dist(-1.f, 1.f);

            float time = (m_shake.duration - m_shakeTime) * m_shake.frequency;
            m_shakeOffset = sf::Vector2f(
                std::sin(time * 6.28318f) * intensity * dist(rng),
                std::cos(time * 6.28318f + 1.5708f) * intensity * dist(rng)
            );
        }

        bool Camera::isShaking() const {
            return m_shakeTime > 0.f;
        }

        ScreenShakeConfig Camera::getShakeConfig() const {
            return m_shake;
        }

        void Camera::startFollow(const sf::Vector2f& target, float lerpSpeed) {
            m_follow.target = target;
            m_follow.lerpSpeed = lerpSpeed;
            m_follow.deadzone = sf::FloatRect(0.f, 0.f, 0.f, 0.f);
            m_follow.enabled = true;
        }

        void Camera::startFollow(const sf::Vector2f& target, const sf::FloatRect& deadzone, float lerpSpeed) {
            m_follow.target = target;
            m_follow.lerpSpeed = lerpSpeed;
            m_follow.deadzone = deadzone;
            m_follow.enabled = true;
        }

        void Camera::stopFollow() {
            m_follow.enabled = false;
        }

        void Camera::updateFollow(float dt) {
            if (!m_follow.enabled) return;

            sf::Vector2f delta = m_follow.target - m_smoothPosition;

            bool inDeadzone = false;
            if (m_follow.deadzone.width > 0.f && m_follow.deadzone.height > 0.f) {
                inDeadzone = std::abs(delta.x) < m_follow.deadzone.width / 2.f &&
                             std::abs(delta.y) < m_follow.deadzone.height / 2.f;
            }

            if (!inDeadzone) {
                sf::Vector2f targetPos = m_follow.target;
                if (m_follow.deadzone.width > 0.f) {
                    if (delta.x > m_follow.deadzone.width / 2.f)
                        targetPos.x -= m_follow.deadzone.width / 2.f;
                    else if (delta.x < -m_follow.deadzone.width / 2.f)
                        targetPos.x += m_follow.deadzone.width / 2.f;
                }
                if (m_follow.deadzone.height > 0.f) {
                    if (delta.y > m_follow.deadzone.height / 2.f)
                        targetPos.y -= m_follow.deadzone.height / 2.f;
                    else if (delta.y < -m_follow.deadzone.height / 2.f)
                        targetPos.y += m_follow.deadzone.height / 2.f;
                }

                float t = 1.0f - std::exp(-m_follow.lerpSpeed * dt);
                m_smoothPosition = m_smoothPosition + (targetPos - m_smoothPosition) * t;
            }

            if (m_follow.clampToBounds) {
                float halfW = m_size.x * m_zoom / 2.f;
                float halfH = m_size.y * m_zoom / 2.f;
                m_smoothPosition.x = std::max(m_follow.bounds.left + halfW,
                    std::min(m_follow.bounds.left + m_follow.bounds.width - halfW, m_smoothPosition.x));
                m_smoothPosition.y = std::max(m_follow.bounds.top + halfH,
                    std::min(m_follow.bounds.top + m_follow.bounds.height - halfH, m_smoothPosition.y));
            }

            m_position = m_smoothPosition;
        }

        bool Camera::isFollowing() const {
            return m_follow.enabled;
        }

        FollowConfig Camera::getFollowConfig() const {
            return m_follow;
        }

        void Camera::setFollowBounds(const sf::FloatRect& bounds) {
            m_follow.bounds = bounds;
            m_follow.clampToBounds = true;
        }

        void Camera::setFollowLerpSpeed(float speed) {
            m_follow.lerpSpeed = speed;
        }

        void Camera::update(float dt) {
            updateShake(dt);
            updateFollow(dt);

            m_viewport.setCenter(m_position + m_shakeOffset);
            m_viewport.setSize(m_size * m_zoom);
            m_viewport.setRotation(m_rotation);
        }

        sf::FloatRect Camera::getBounds() const {
            sf::Vector2f pos = applyShake(m_position);
            float halfW = m_size.x * m_zoom / 2.f;
            float halfH = m_size.y * m_zoom / 2.f;
            return sf::FloatRect(pos.x - halfW, pos.y - halfH, halfW * 2.f, halfH * 2.f);
        }

        sf::View Camera::getSFMLView() const {
            return m_viewport.toSFML();
        }

        void Camera::applyTo(sf::RenderTarget& target) const {
            target.setView(getSFMLView());
        }

        Viewport& Camera::getViewport() {
            return m_viewport;
        }

        const Viewport& Camera::getViewport() const {
            return m_viewport;
        }

        bool Frustum::isVisible(const sf::FloatRect& aabb) const {
            sf::Vector2f corners[4] = {
                sf::Vector2f(aabb.left, aabb.top),
                sf::Vector2f(aabb.left + aabb.width, aabb.top),
                sf::Vector2f(aabb.left + aabb.width, aabb.top + aabb.height),
                sf::Vector2f(aabb.left, aabb.top + aabb.height)
            };
            for (const auto& plane : planes) {
                bool allOutside = true;
                for (int i = 0; i < 4; ++i) {
                    float dot = corners[i].x * plane.normal.x + corners[i].y * plane.normal.y;
                    if (dot + plane.distance > 0) {
                        allOutside = false;
                        break;
                    }
                }
                if (allOutside) return false;
            }
            return true;
        }

        bool Frustum::isVisible(const sf::Vector2f& center, float radius) const {
            for (const auto& plane : planes) {
                float dot = center.x * plane.normal.x + center.y * plane.normal.y;
                if (dot + plane.distance + radius <= 0) return false;
            }
            return true;
        }

        Frustum Camera::getFrustum() const {
            Frustum frustum;
            float halfW = m_size.x * m_zoom / 2.f;
            float halfH = m_size.y * m_zoom / 2.f;
            sf::Vector2f pos = applyShake(m_position);

            float cr = std::cos(m_rotation * 3.14159265f / 180.f);
            float sr = std::sin(m_rotation * 3.14159265f / 180.f);

            sf::Vector2f right(cr, sr);
            sf::Vector2f up(-sr, cr);

            // Left plane
            sf::Vector2f leftNormal = sf::Vector2f(-right.x, -right.y);
            frustum.planes[0] = FrustumPlane(leftNormal, halfW);
            // Right plane
            frustum.planes[1] = FrustumPlane(right, -halfW);
            // Bottom plane
            sf::Vector2f bottomNormal = sf::Vector2f(-up.x, -up.y);
            frustum.planes[2] = FrustumPlane(bottomNormal, halfH);
            // Top plane
            frustum.planes[3] = FrustumPlane(up, -halfH);

            return frustum;
        }

        bool Camera::isVisible(const sf::FloatRect& aabb) const {
            return getFrustum().isVisible(aabb);
        }

        bool Camera::isVisible(const sf::Vector2f& center, float radius) const {
            return getFrustum().isVisible(center, radius);
        }

        void Camera::setOrthographicSize(float size) {
            m_orthographicSize = size;
            float aspect = m_size.x / m_size.y;
            m_size = sf::Vector2f(size * aspect, size);
        }

        float Camera::getOrthographicSize() const {
            return m_orthographicSize;
        }

        void Camera::setAspectRatioLock(bool lock) {
            m_aspectRatioLock = lock;
            if (lock) {
                m_aspectRatio = m_size.x / m_size.y;
            }
        }

        bool Camera::isAspectRatioLocked() const {
            return m_aspectRatioLock;
        }

        void Camera::setAspectRatio(float ratio) {
            m_aspectRatio = ratio;
            if (m_aspectRatioLock) {
                m_size.x = m_size.y * ratio;
            }
        }

        float Camera::getAspectRatio() const {
            return m_aspectRatio;
        }

        void Camera::setPixelPerfect(bool enabled) {
            m_pixelPerfect = enabled;
        }

        bool Camera::isPixelPerfect() const {
            return m_pixelPerfect;
        }

        void Camera::setRenderOrder(RenderOrder order) {
            m_renderOrder = order;
        }

        RenderOrder Camera::getRenderOrder() const {
            return m_renderOrder;
        }

        sf::Vector2f Camera::applyShake(const sf::Vector2f& pos) const {
            return pos + m_shakeOffset;
        }

    }
}
