#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <unordered_map>
#include <functional>
#include <vector>
#include <cstdint>

namespace nebula {

    class Input {
    public:
        static Input& instance();

        void update();

        bool isKeyDown(sf::Keyboard::Key key) const;
        bool isKeyPressed(sf::Keyboard::Key key) const;
        bool isKeyReleased(sf::Keyboard::Key key) const;

        bool isMouseButtonDown(sf::Mouse::Button button) const;
        bool isMouseButtonPressed(sf::Mouse::Button button) const;
        bool isMouseButtonReleased(sf::Mouse::Button button) const;

        float getMouseX() const { return m_mouseX; }
        float getMouseY() const { return m_mouseY; }
        float getMouseDeltaX() const { return m_mouseDeltaX; }
        float getMouseDeltaY() const { return m_mouseDeltaY; }
        float getScrollDelta() const { return m_scrollDelta; }

        void setMouseVisible(bool visible);
        void setMousePosition(float x, float y);

        void onKeyPressed(sf::Keyboard::Key key);
        void onKeyReleased(sf::Keyboard::Key key);
        void onMousePressed(sf::Mouse::Button button);
        void onMouseReleased(sf::Mouse::Button button);
        void onMouseMoved(float x, float y);
        void onMouseScrolled(float delta);

        bool isGamepadConnected(int id) const;
        float getGamepadAxis(int id, sf::Joystick::Axis axis) const;
        bool isGamepadButtonDown(int id, int button) const;

        void setClipboardText(const std::string& text);
        std::string getClipboardText() const;

    private:
        Input() = default;
        Input(const Input&) = delete;
        Input& operator=(const Input&) = delete;

        std::unordered_map<sf::Keyboard::Key, bool> m_keysDown;
        std::unordered_map<sf::Keyboard::Key, bool> m_keysPressed;
        std::unordered_map<sf::Keyboard::Key, bool> m_keysReleased;

        std::unordered_map<sf::Mouse::Button, bool> m_mouseDown;
        std::unordered_map<sf::Mouse::Button, bool> m_mousePressed;
        std::unordered_map<sf::Mouse::Button, bool> m_mouseReleased;

        float m_mouseX = 0.0f;
        float m_mouseY = 0.0f;
        float m_mouseDeltaX = 0.0f;
        float m_mouseDeltaY = 0.0f;
        float m_scrollDelta = 0.0f;
    };

}
