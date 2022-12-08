#include "Input.h"
#include "Event.h"
#include "Logger.h"
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

        m_mousePressed.clear();
        m_mouseReleased.clear();
        m_mouseDeltaX = 0.0f;
        m_mouseDeltaY = 0.0f;
        m_scrollDelta = 0.0f;

        pollGamepads();
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

    void Input::onTextEntered(uint32_t codepoint) {
        m_textInputBuffer.push_back(codepoint);
    }

    std::string Input::getTextInput() {
        std::string result;
        for (uint32_t cp : m_textInputBuffer) {
            if (cp <= 0x7F) {
                result += static_cast<char>(cp);
            }
        }
        m_textInputBuffer.clear();
        return result;
    }

    void Input::clearTextInput() {
        m_textInputBuffer.clear();
    }

    bool Input::isGamepadConnected(int id) const {
        return sf::Joystick::isConnected(id);
    }

    int Input::getConnectedGamepadCount() const {
        int count = 0;
        for (int i = 0; i < sf::Joystick::Count; ++i) {
            if (sf::Joystick::isConnected(i)) ++count;
        }
        return count;
    }

    float Input::getGamepadAxis(int id, sf::Joystick::Axis axis) const {
        auto it = m_gamepadStates.find(id);
        if (it != m_gamepadStates.end() && it->second.connected) {
            return it->second.axes[axis];
        }
        return 0.0f;
    }

    bool Input::isGamepadButtonDown(int id, int button) const {
        auto it = m_gamepadStates.find(id);
        if (it != m_gamepadStates.end() && it->second.connected) {
            return it->second.buttons[button];
        }
        return false;
    }

    bool Input::isGamepadButtonPressed(int id, int button) const {
        auto it = m_gamepadStates.find(id);
        if (it != m_gamepadStates.end() && it->second.connected) {
            return it->second.buttonsPressed[button];
        }
        return false;
    }

    bool Input::isGamepadButtonReleased(int id, int button) const {
        auto it = m_gamepadStates.find(id);
        if (it != m_gamepadStates.end() && it->second.connected) {
            return it->second.buttonsReleased[button];
        }
        return false;
    }

    const GamepadState& Input::getGamepadState(int id) const {
        static GamepadState defaultState;
        auto it = m_gamepadStates.find(id);
        if (it != m_gamepadStates.end()) {
            return it->second;
        }
        return defaultState;
    }

    void Input::pollGamepads() {
        for (int i = 0; i < sf::Joystick::Count; ++i) {
            GamepadState& state = m_gamepadStates[i];
            bool wasConnected = state.connected;
            state.connected = sf::Joystick::isConnected(i);

            for (int b = 0; b < sf::Joystick::ButtonCount; ++b) {
                bool wasDown = state.buttons[b];
                state.buttons[b] = state.connected ? sf::Joystick::isButtonPressed(i, b) : false;
                state.buttonsPressed[b] = state.connected && !wasDown && state.buttons[b];
                state.buttonsReleased[b] = state.connected && wasDown && !state.buttons[b];
            }

            for (int a = 0; a < sf::Joystick::AxisCount; ++a) {
                state.axes[a] = state.connected ? sf::Joystick::getAxisPosition(i, static_cast<sf::Joystick::Axis>(a)) : 0.0f;
            }

            if (state.connected != wasConnected) {
                if (state.connected) {
                    NEBULA_INFO("Gamepad connected: " + std::to_string(i));
                } else {
                    NEBULA_INFO("Gamepad disconnected: " + std::to_string(i));
                }
            }
        }
    }

    void Input::bindAction(const std::string& name, int primaryKey, int secondaryKey,
                           int gamepadButton, int mouseButton, float deadZone) {
        InputAction action;
        action.name = name;
        action.primaryKey = primaryKey;
        action.secondaryKey = secondaryKey;
        action.gamepadButton = gamepadButton;
        action.mouseButton = mouseButton;
        action.deadZone = deadZone;
        m_actions[name] = action;

        if (!m_contextStack.empty()) {
            m_contextActions[m_contextStack.top()].push_back(name);
        }
    }

    void Input::unbindAction(const std::string& name) {
        m_actions.erase(name);
    }

    bool Input::resolveActionKey(const InputAction& action, bool (*keyCheck)(sf::Keyboard::Key),
                                  bool (*mouseCheck)(sf::Mouse::Button),
                                  bool (*gamepadCheck)(int, int)) const {
        if (action.primaryKey >= 0) {
            sf::Keyboard::Key key = static_cast<sf::Keyboard::Key>(action.primaryKey);
            if (keyCheck(key)) return true;
        }
        if (action.secondaryKey >= 0) {
            sf::Keyboard::Key key = static_cast<sf::Keyboard::Key>(action.secondaryKey);
            if (keyCheck(key)) return true;
        }
        if (action.mouseButton >= 0) {
            sf::Mouse::Button btn = static_cast<sf::Mouse::Button>(action.mouseButton);
            if (mouseCheck(btn)) return true;
        }
        if (action.gamepadButton >= 0) {
            for (int i = 0; i < sf::Joystick::Count; ++i) {
                if (gamepadCheck(i, action.gamepadButton)) return true;
            }
        }
        return false;
    }

    bool Input::isActionTriggered(const std::string& name) const {
        auto it = m_actions.find(name);
        if (it == m_actions.end()) return false;
        return resolveActionKey(it->second,
            [this](sf::Keyboard::Key k) { return isKeyPressed(k); },
            [this](sf::Mouse::Button b) { return isMouseButtonPressed(b); },
            [this](int id, int btn) { return isGamepadButtonPressed(id, btn); });
    }

    bool Input::isActionDown(const std::string& name) const {
        auto it = m_actions.find(name);
        if (it == m_actions.end()) return false;
        return resolveActionKey(it->second,
            [this](sf::Keyboard::Key k) { return isKeyDown(k); },
            [this](sf::Mouse::Button b) { return isMouseButtonDown(b); },
            [this](int id, int btn) { return isGamepadButtonDown(id, btn); });
    }

    bool Input::isActionReleased(const std::string& name) const {
        auto it = m_actions.find(name);
        if (it == m_actions.end()) return false;
        return resolveActionKey(it->second,
            [this](sf::Keyboard::Key k) { return isKeyReleased(k); },
            [this](sf::Mouse::Button b) { return isMouseButtonReleased(b); },
            [this](int id, int btn) { return isGamepadButtonReleased(id, btn); });
    }

    void Input::pushContext(const std::string& context) {
        m_contextStack.push(context);
        NEBULA_DEBUG("Input context pushed: " + context);
    }

    void Input::popContext() {
        if (!m_contextStack.empty()) {
            std::string top = m_contextStack.top();
            m_contextStack.pop();
            NEBULA_DEBUG("Input context popped: " + top);
        }
    }

    void Input::setContext(const std::string& context) {
        while (!m_contextStack.empty()) {
            m_contextStack.pop();
        }
        m_contextStack.push(context);
        NEBULA_DEBUG("Input context set to: " + context);
    }

    const std::string& Input::getCurrentContext() const {
        static std::string defaultContext = "default";
        if (m_contextStack.empty()) return defaultContext;
        return m_contextStack.top();
    }

    bool Input::hasContext(const std::string& context) const {
        std::stack<std::string> temp = m_contextStack;
        while (!temp.empty()) {
            if (temp.top() == context) return true;
            temp.pop();
        }
        return false;
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

