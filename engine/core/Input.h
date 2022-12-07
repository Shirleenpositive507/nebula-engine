#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <unordered_map>
#include <functional>
#include <vector>
#include <string>
#include <cstdint>
#include <stack>

namespace nebula {

    struct GamepadState {
        bool connected = false;
        float axes[sf::Joystick::AxisCount] = {};
        bool buttons[sf::Joystick::ButtonCount] = {};
        bool buttonsPressed[sf::Joystick::ButtonCount] = {};
        bool buttonsReleased[sf::Joystick::ButtonCount] = {};
    };

    struct InputAction {
        std::string name;
        int primaryKey = -1;
        int secondaryKey = -1;
        int gamepadButton = -1;
        int mouseButton = -1;
        float deadZone = 0.2f;
    };

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

        void onTextEntered(uint32_t codepoint);
        bool hasTextInput() const { return !m_textInputBuffer.empty(); }
        std::string getTextInput();
        void clearTextInput();

        bool isGamepadConnected(int id) const;
        int getConnectedGamepadCount() const;
        float getGamepadAxis(int id, sf::Joystick::Axis axis) const;
        bool isGamepadButtonDown(int id, int button) const;
        bool isGamepadButtonPressed(int id, int button) const;
        bool isGamepadButtonReleased(int id, int button) const;
        const GamepadState& getGamepadState(int id) const;
        void pollGamepads();

        void bindAction(const std::string& name, int primaryKey, int secondaryKey = -1,
                        int gamepadButton = -1, int mouseButton = -1, float deadZone = 0.2f);
        void unbindAction(const std::string& name);
        bool isActionTriggered(const std::string& name) const;
        bool isActionDown(const std::string& name) const;
        bool isActionReleased(const std::string& name) const;

        void pushContext(const std::string& context);
        void popContext();
        void setContext(const std::string& context);
        const std::string& getCurrentContext() const;
        bool hasContext(const std::string& context) const;

        void setClipboardText(const std::string& text);
        std::string getClipboardText() const;

    private:
        Input() = default;
        Input(const Input&) = delete;
        Input& operator=(const Input&) = delete;

        bool resolveActionKey(const InputAction& action, bool (*keyCheck)(sf::Keyboard::Key),
                              bool (*mouseCheck)(sf::Mouse::Button),
                              bool (*gamepadCheck)(int, int)) const;

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

        std::vector<uint32_t> m_textInputBuffer;

        std::unordered_map<int, GamepadState> m_gamepadStates;

        std::unordered_map<std::string, InputAction> m_actions;
        std::stack<std::string> m_contextStack;
        std::unordered_map<std::string, std::vector<std::string>> m_contextActions;
    };

}

