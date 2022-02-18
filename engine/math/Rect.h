#pragma once
#include <cmath>
#include <iostream>
#include <algorithm>
#include "core/Types.h"
#include "Vector2.h"

namespace nebula {

template<typename T>
class Rect {
public:
    T left;
    T top;
    T width;
    T height;

    Rect() : left(0), top(0), width(0), height(0) {}
    Rect(T left, T top, T width, T height)
        : left(left), top(top), width(width), height(height) {}

    static Rect fromMinMax(T minX, T minY, T maxX, T maxY) {
        return Rect(minX, minY, maxX - minX, maxY - minY);
    }

    static Rect fromCenter(const Vector2<T>& center, const Vector2<T>& size) {
        return Rect(
            center.x - size.x / static_cast<T>(2),
            center.y - size.y / static_cast<T>(2),
            size.x, size.y
        );
    }

    T getMinX() const { return left; }
    T getMinY() const { return top; }
    T getMaxX() const { return left + width; }
    T getMaxY() const { return top + height; }

    void setMinX(T v) { T maxX = getMaxX(); left = v; width = maxX - left; }
    void setMinY(T v) { T maxY = getMaxY(); top = v; height = maxY - top; }
    void setMaxX(T v) { width = v - left; }
    void setMaxY(T v) { height = v - top; }

    Vector2<T> getMin() const { return Vector2<T>(left, top); }
    Vector2<T> getMax() const { return Vector2<T>(getMaxX(), getMaxY()); }
    Vector2<T> getSize() const { return Vector2<T>(width, height); }
    Vector2<T> getCenter() const {
        return Vector2<T>(left + width / static_cast<T>(2), top + height / static_cast<T>(2));
    }

    bool isEmpty() const { return width <= 0 || height <= 0; }

    bool contains(const Vector2<T>& point) const {
        return point.x >= left && point.x <= getMaxX() &&
               point.y >= top && point.y <= getMaxY();
    }

    bool contains(const Rect& other) const {
        return other.left >= left && other.top >= top &&
               other.getMaxX() <= getMaxX() && other.getMaxY() <= getMaxY();
    }

    bool intersects(const Rect& other) const {
        return left < other.getMaxX() && getMaxX() > other.left &&
               top < other.getMaxY() && getMaxY() > other.top;
    }

    Rect intersection(const Rect& other) const {
        T minX = std::max(left, other.left);
        T minY = std::max(top, other.top);
        T maxX = std::min(getMaxX(), other.getMaxX());
        T maxY = std::min(getMaxY(), other.getMaxY());

        if (maxX < minX || maxY < minY)
            return Rect(0, 0, 0, 0);

        return fromMinMax(minX, minY, maxX, maxY);
    }

    Rect united(const Rect& other) const {
        T minX = std::min(left, other.left);
        T minY = std::min(top, other.top);
        T maxX = std::max(getMaxX(), other.getMaxX());
        T maxY = std::max(getMaxY(), other.getMaxY());
        return fromMinMax(minX, minY, maxX, maxY);
    }

    void expand(const Vector2<T>& amount) {
        left -= amount.x;
        top -= amount.y;
        width += amount.x * static_cast<T>(2);
        height += amount.y * static_cast<T>(2);
    }

    void inflate(T dx, T dy) {
        left -= dx;
        top -= dy;
        width += dx * static_cast<T>(2);
        height += dy * static_cast<T>(2);
    }

    void offset(const Vector2<T>& delta) {
        left += delta.x;
        top += delta.y;
    }

    void offset(T dx, T dy) {
        left += dx;
        top += dy;
    }

    Rect translated(const Vector2<T>& delta) const {
        Rect r = *this;
        r.offset(delta);
        return r;
    }

    Rect scaled(T factor) const {
        Vector2<T> c = getCenter();
        Vector2<T> s = getSize() * factor;
        return fromCenter(c, s);
    }

    bool operator==(const Rect& other) const {
        return left == other.left && top == other.top &&
               width == other.width && height == other.height;
    }
    bool operator!=(const Rect& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Rect& r) {
        os << "Rect(" << r.left << ", " << r.top << ", " << r.width << ", " << r.height << ")";
        return os;
    }

    static const Rect Zero;
    static const Rect One;
};

template<typename T>
const Rect<T> Rect<T>::Zero = Rect<T>(0, 0, 0, 0);

template<typename T>
const Rect<T> Rect<T>::One = Rect<T>(0, 0, 1, 1);

using Rectf = Rect<f32>;
using Recti = Rect<i32>;
using Rectu = Rect<u32>;
using Rectd = Rect<f64>;

} // namespace nebula
