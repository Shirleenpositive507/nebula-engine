#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include "Viewport.h"
#include <functional>
#include <array>

namespace nebula {
    namespace graphics {

        enum class ProjectionMode {
            Orthographic,
            Perspective
        };

        enum class RenderOrder {
            Default,
            BackToFront,
            FrontToBack,
            Custom
        };

        struct FrustumPlane {
            sf::Vector2f normal;
            float distance;

            FrustumPlane() : normal(0.f, 0.f), distance(0.f) {}
            FrustumPlane(const sf::Vector2f& n, float d) : normal(n), distance(d) {}
        };

        struct Frustum {
            std::array<FrustumPlane, 4> planes; // left, right, bottom, top

            bool isVisible(const sf::FloatRect& aabb) const;
            bool isVisible(const sf::Vector2f& center, float radius) const;
        };

        struct ScreenShakeConfig {
            float intensity;
            float duration;
            float frequency;
            bool fadeOut;

            ScreenShakeConfig()
                : intensity(5.0f), duration(0.5f), frequency(10.0f), fadeOut(true) {}
        };

        struct FollowConfig {
            sf::Vector2f target;
            float lerpSpeed;
            sf::FloatRect deadzone;
            bool enabled;
            bool clampToBounds;
            sf::FloatRect bounds;

            FollowConfig()
                : target(0.f, 0.f), lerpSpeed(5.0f), deadzone(0.f, 0.f, 0.f, 0.f)
                , enabled(false), clampToBounds(false)
                , bounds(0.f, 0.f, 0.f, 0.f) {}
        };

        class Camera {
        public:
            Camera();
            explicit Camera(const sf::Vector2f& position);
            Camera(const sf::Vector2f& position, float rotation, float zoom);

            void setPosition(const sf::Vector2f& position);
            void setPosition(float x, float y);
            sf::Vector2f getPosition() const;

            void setRotation(float degrees);
            float getRotation() const;

            void setZoom(float zoom);
            float getZoom() const;

            void setViewport(const sf::FloatRect& viewport);
            sf::FloatRect getViewport() const;

            void setProjectionMode(ProjectionMode mode);
            ProjectionMode getProjectionMode() const;

            void setSize(const sf::Vector2f& size);
            sf::Vector2f getSize() const;

            sf::Transform getViewMatrix() const;
            sf::Transform getProjectionMatrix() const;
            sf::Transform getViewProjectionMatrix() const;

            void lookAt(const sf::Vector2f& target);
            void orbit(const sf::Vector2f& center, float angleDelta, float radiusDelta = 0.f);

            sf::Vector2f screenToWorld(const sf::Vector2f& screenPoint, const sf::Vector2u& windowSize) const;
            sf::Vector2f worldToScreen(const sf::Vector2f& worldPoint, const sf::Vector2u& windowSize) const;

            void shake(float intensity, float duration, float frequency = 10.0f);
            void updateShake(float dt);
            bool isShaking() const;
            ScreenShakeConfig getShakeConfig() const;

            void startFollow(const sf::Vector2f& target, float lerpSpeed = 5.0f);
            void startFollow(const sf::Vector2f& target, const sf::FloatRect& deadzone, float lerpSpeed = 5.0f);
            void stopFollow();
            void updateFollow(float dt);
            bool isFollowing() const;
            FollowConfig getFollowConfig() const;
            void setFollowBounds(const sf::FloatRect& bounds);
            void setFollowLerpSpeed(float speed);

            void update(float dt);

            sf::FloatRect getBounds() const;
            sf::View getSFMLView() const;
            void applyTo(sf::RenderTarget& target) const;

            Frustum getFrustum() const;
            bool isVisible(const sf::FloatRect& aabb) const;
            bool isVisible(const sf::Vector2f& center, float radius) const;

            void setOrthographicSize(float size);
            float getOrthographicSize() const;

            void setAspectRatioLock(bool lock);
            bool isAspectRatioLocked() const;
            void setAspectRatio(float ratio);
            float getAspectRatio() const;

            void setPixelPerfect(bool enabled);
            bool isPixelPerfect() const;

            void setRenderOrder(RenderOrder order);
            RenderOrder getRenderOrder() const;

            Viewport& getViewport();
            const Viewport& getViewport() const;

        private:
            sf::Vector2f m_position;
            sf::Vector2f m_size;
            float m_rotation;
            float m_zoom;
            ProjectionMode m_projectionMode;
            Viewport m_viewport;

            float m_orthographicSize;
            bool m_aspectRatioLock;
            float m_aspectRatio;
            bool m_pixelPerfect;
            RenderOrder m_renderOrder;

            ScreenShakeConfig m_shake;
            float m_shakeTime;
            sf::Vector2f m_shakeOffset;

            FollowConfig m_follow;
            sf::Vector2f m_smoothPosition;

            sf::Vector2f applyShake(const sf::Vector2f& pos) const;
        };

    }
}
