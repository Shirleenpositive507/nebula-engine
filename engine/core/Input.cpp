#include "Input.h"
#include <SFML/System/Clipboard.hpp>
#include <cstring>

namespace nebula {

    Input& Input::instance() {
        static Input inst;
        return inst;
    }

    void Input::update() {
        m_keysPressed.clear();
        m_keysReleased.clear();

        for (auto& [key, down] : m_keysDown) {
            (void)down;
        }

        m_mousePressed.clear();
        m_mouseReleased.clear();
        m_mouseDeltaX = 0.0f;
        m_mouseDeltaY = 0.0f;
        m_scrollDelta = 0.0f;
    }

    bool Input::isKeyDown(sf::Keyboard::Key key) const {
        auto it = m_keysDown.find(key);
        return it != m_keysDown.end() && it->second;
    }

    bool Input::isKeyPressed(sf::Keyboard::Key key) const {
        auto it = m_keysPressed.find(key);
        return it != m_keysPressed.end() && it->second;
    }

    bool Input::isKeyReleased(sf::Keyboard::Key key) const {
        auto it = m_keysReleased.find(key);
        return it != m_keysReleased.end() && it->second;
    }

    bool Input::isMouseButtonDown(sf::Mouse::Button button) const {
        auto it = m_mouseDown.find(button);
        return it != m_mouseDown.end() && it->second;
    }

    bool Input::isMouseButtonPressed(sf::Mouse::Button button) const {
        auto it = m_mousePressed.find(button);
        return it != m_mousePressed.end() && it->second;
    }

    bool Input::isMouseButtonReleased(sf::Mouse::Button button) const {
        auto it = m_mouseReleased.find(button);
        return it != m_mouseReleased.end() && it->second;
    }

    void Input::setMouseVisible(bool visible) {
        sf::Mouse::setVisible(visible);
    }

    void Input::setMousePosition(float x, float y) {
        sf::Mouse::setPosition(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)));
    }

    void Input::onKeyPressed(sf::Keyboard::Key key) {
        if (!m_keysDown[key]) {
            m_keysPressed[key] = true;
        }
        m_keysDown[key] = true;
    }

    void Input::onKeyReleased(sf::Keyboard::Key key) {
        m_keysDown[key] = false;
        m_keysReleased[key] = true;
    }

    void Input::onMousePressed(sf::Mouse::Button button) {
        if (!m_mouseDown[button]) {
            m_mousePressed[button] = true;
        }
        m_mouseDown[button] = true;
    }

    void Input::onMouseReleased(sf::Mouse::Button button) {
        m_mouseDown[button] = false;
        m_mouseReleased[button] = true;
    }

    void Input::onMouseMoved(float x, float y) {
        m_mouseDeltaX = x - m_mouseX;
        m_mouseDeltaY = y - m_mouseY;
        m_mouseX = x;
        m_mouseY = y;
    }

    void Input::onMouseScrolled(float delta) {
        m_scrollDelta += delta;
    }

    bool Input::isGamepadConnected(int id) const {
        return sf::Joystick::isConnected(id);
    }

    float Input::getGamepadAxis(int id, sf::Joystick::Axis axis) const {
        return sf::Joystick::getAxisPosition(id, axis);
    }

    bool Input::isGamepadButtonDown(int id, int button) const {
        return sf::Joystick::isButtonPressed(id, button);
    }

    void Input::setClipboardText(const std::string& text) {
        sf::Clipboard::setString(sf::String::fromUtf8(text.begin(), text.end()));
    }

    std::string Input::getClipboardText() const {
        sf::String clipString = sf::Clipboard::getString();
        std::string result;
        result.resize(clipString.getSize());
        for (size_t i = 0; i < clipString.getSize(); i++) {
            result[i] = static_cast<char>(clipString[i]);
        }
        return result;
    }

}
