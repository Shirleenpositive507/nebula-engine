#include "Viewport.h"
#include <algorithm>
#include <cmath>

namespace nebula {
    namespace graphics {

        Viewport::Viewport() {}

        Viewport::Viewport(const sf::View& view)
            : m_view(view) {}

        Viewport::Viewport(const sf::FloatRect& viewport)
            : m_view(viewport) {}

        Viewport::Viewport(const sf::Vector2f& center, const sf::Vector2f& size)
            : m_view(center, size) {}

        void Viewport::setCenter(const sf::Vector2f& center) {
            m_view.setCenter(center);
        }

        void Viewport::setCenter(float x, float y) {
            m_view.setCenter(x, y);
        }

        void Viewport::setSize(const sf::Vector2f& size) {
            m_view.setSize(size);
        }

        void Viewport::setSize(float width, float height) {
            m_view.setSize(sf::Vector2f(width, height));
        }

        void Viewport::setRotation(float angle) {
            m_view.setRotation(angle);
        }

        void Viewport::setViewport(const sf::FloatRect& viewport) {
            m_view.setViewport(viewport);
        }

        void Viewport::move(const sf::Vector2f& offset) {
            m_view.move(offset);
        }

        void Viewport::move(float offsetX, float offsetY) {
            m_view.move(offsetX, offsetY);
        }

        void Viewport::zoom(float factor) {
            m_view.zoom(factor);
        }

        void Viewport::rotate(float angle) {
            m_view.rotate(angle);
        }

        void Viewport::reset(const sf::FloatRect& rectangle) {
            m_view.reset(rectangle);
        }

        sf::Vector2f Viewport::getCenter() const {
            return m_view.getCenter();
        }

        sf::Vector2f Viewport::getSize() const {
            return m_view.getSize();
        }

        float Viewport::getRotation() const {
            return m_view.getRotation();
        }

        sf::FloatRect Viewport::getViewport() const {
            return m_view.getViewport();
        }

        sf::Transform Viewport::getTransform() const {
            return m_view.getTransform();
        }

        sf::Transform Viewport::getInverseTransform() const {
            return m_view.getInverseTransform();
        }

        sf::View Viewport::toSFML() const {
            return m_view;
        }

        Viewport Viewport::fromSFML(const sf::View& view) {
            return Viewport(view);
        }

        void Viewport::setSplitScreen(const Viewport& other, float splitRatio, bool horizontal) {
            sf::FloatRect vp1, vp2;
            splitRatio = std::max(0.1f, std::min(0.9f, splitRatio));

            if (horizontal) {
                vp1 = sf::FloatRect(0.f, 0.f, splitRatio, 1.f);
                vp2 = sf::FloatRect(splitRatio, 0.f, 1.f - splitRatio, 1.f);
            } else {
                vp1 = sf::FloatRect(0.f, 0.f, 1.f, splitRatio);
                vp2 = sf::FloatRect(0.f, splitRatio, 1.f, 1.f - splitRatio);
            }

            setViewport(vp1);
            const_cast<Viewport&>(other).setViewport(vp2);
        }

        void Viewport::fitToAspectRatio(float aspectRatio) {
            sf::Vector2f size = getSize();
            float currentAspect = size.x / size.y;

            if (currentAspect > aspectRatio) {
                size.x = size.y * aspectRatio;
            } else {
                size.y = size.x / aspectRatio;
            }

            setSize(size);
        }

        void Viewport::setBounds(float left, float top, float right, float bottom) {
            sf::Vector2f center((left + right) / 2.f, (top + bottom) / 2.f);
            sf::Vector2f size(right - left, bottom - top);
            setCenter(center);
            setSize(size);
        }

        sf::FloatRect Viewport::getBounds() const {
            sf::Vector2f center = getCenter();
            sf::Vector2f size = getSize();
            return sf::FloatRect(
                center.x - size.x / 2.f,
                center.y - size.y / 2.f,
                size.x,
                size.y
            );
        }

        bool Viewport::contains(const sf::Vector2f& point) const {
            return getBounds().contains(point);
        }

        sf::Vector2f Viewport::screenToWorld(const sf::Vector2f& screenPoint, const sf::Vector2u& windowSize) const {
            sf::Vector2f normalized(
                screenPoint.x / static_cast<float>(windowSize.x),
                screenPoint.y / static_cast<float>(windowSize.y)
            );

            sf::FloatRect vp = getViewport();
            sf::Vector2f viewCoords(
                (normalized.x - vp.left) / vp.width,
                (normalized.y - vp.top) / vp.height
            );

            sf::Vector2f viewSize = getSize();
            sf::Vector2f viewCenter = getCenter();
            float viewRotation = getRotation() * 3.14159265f / 180.f;

            sf::Vector2f worldPos(
                (viewCoords.x - 0.5f) * viewSize.x,
                (viewCoords.y - 0.5f) * viewSize.y
            );

            float cosR = std::cos(-viewRotation);
            float sinR = std::sin(-viewRotation);
            return sf::Vector2f(
                worldPos.x * cosR - worldPos.y * sinR + viewCenter.x,
                worldPos.x * sinR + worldPos.y * cosR + viewCenter.y
            );
        }

        sf::Vector2f Viewport::worldToScreen(const sf::Vector2f& worldPoint, const sf::Vector2u& windowSize) const {
            sf::Vector2f viewSize = getSize();
            sf::Vector2f viewCenter = getCenter();
            float viewRotation = getRotation() * 3.14159265f / 180.f;

            sf::Vector2f offset = worldPoint - viewCenter;
            float cosR = std::cos(viewRotation);
            float sinR = std::sin(viewRotation);
            sf::Vector2f rotated(
                offset.x * cosR - offset.y * sinR,
                offset.x * sinR + offset.y * cosR
            );

            sf::Vector2f normalized(
                rotated.x / viewSize.x + 0.5f,
                rotated.y / viewSize.y + 0.5f
            );

            sf::FloatRect vp = getViewport();
            return sf::Vector2f(
                (normalized.x * vp.width + vp.left) * static_cast<float>(windowSize.x),
                (normalized.y * vp.height + vp.top) * static_cast<float>(windowSize.y)
            );
        }

    }
}
