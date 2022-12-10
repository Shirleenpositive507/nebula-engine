#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <any>
#include <queue>
#include <mutex>
#include <algorithm>

namespace nebula {

    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus,
        WindowMoved, WindowMinimized, WindowRestored,
        KeyPressed, KeyReleased, KeyTyped,
        MouseMoved, MousePressed, MouseReleased, MouseScrolled,
        MouseEntered, MouseLeft,
        GamepadConnected, GamepadDisconnected,
        GamepadButtonPressed, GamepadButtonReleased,
        GamepadAxisMoved,
        TextEntered,
        FileDropped,
        PhysicsCollision, PhysicsTrigger,
        UIAction, UIHover, UIClick,
        NetworkConnected, NetworkDisconnected, NetworkDataReceived,
        AppTick, AppUpdate, AppRender,
        UserEvent = 1000
    };

    enum class EventCategory {
        None = 0,
        Window = 1,
        Input = 2,
        Keyboard = 4,
        Mouse = 8,
        Gamepad = 16,
        Application = 32,
        Physics = 64,
        UI = 128,
        Network = 256
    };

    enum class EventPriority {
        Lowest = 0,
        Low = 1,
        Normal = 2,
        High = 3,
        Highest = 4
    };

    struct Event {
        EventType type = EventType::None;
        bool handled = false;
        bool propagating = true;
        EventPriority priority = EventPriority::Normal;
        bool filtered = false;

        virtual ~Event() = default;

        EventType getType() const { return type; }
        virtual const char* getName() const { return "Event"; }
        virtual EventCategory getCategory() const { return EventCategory::None; }
        virtual std::string toString() const { return "Event"; }

        bool isInCategory(EventCategory category) const {
            return (static_cast<int>(getCategory()) & static_cast<int>(category)) != 0;
        }

        void stopPropagation() { propagating = false; }
        void filter() { filtered = true; }
    };

    struct WindowResizeEvent : public Event {
        int width;
        int height;

        WindowResizeEvent(int w, int h) : width(w), height(h) {
            type = EventType::WindowResize;
        }

        const char* getName() const override { return "WindowResize"; }
        EventCategory getCategory() const override { return EventCategory::Window; }
        std::string toString() const override {
            return "WindowResize: " + std::to_string(width) + "x" + std::to_string(height);
        }
    };

    struct WindowCloseEvent : public Event {
        WindowCloseEvent() { type = EventType::WindowClose; }
        const char* getName() const override { return "WindowClose"; }
        EventCategory getCategory() const override { return EventCategory::Window; }
    };

    struct WindowFocusEvent : public Event {
        bool focused;
        WindowFocusEvent(bool f) : focused(f) {
            type = focused ? EventType::WindowFocus : EventType::WindowLostFocus;
        }
        const char* getName() const override { return "WindowFocus"; }
        EventCategory getCategory() const override { return EventCategory::Window; }
    };

    struct KeyEvent : public Event {
        int keyCode;
        int scancode;
        int mods;
        bool repeat;

        bool isShiftDown() const { return (mods & 0x0001) != 0; }
        bool isControlDown() const { return (mods & 0x0002) != 0; }
        bool isAltDown() const { return (mods & 0x0004) != 0; }
        bool isSuperDown() const { return (mods & 0x0008) != 0; }
        bool isCapsLockOn() const { return (mods & 0x0010) != 0; }
        bool isNumLockOn() const { return (mods & 0x0020) != 0; }

        EventCategory getCategory() const override {
            return static_cast<EventCategory>(static_cast<int>(EventCategory::Input) | static_cast<int>(EventCategory::Keyboard));
        }
    };

    struct KeyPressedEvent : public KeyEvent {
        KeyPressedEvent(int k, int s, int m, bool r) {
            type = EventType::KeyPressed;
            keyCode = k; scancode = s; mods = m; repeat = r;
        }
        const char* getName() const override { return "KeyPressed"; }
        std::string toString() const override {
            return "KeyPressed: " + std::to_string(keyCode) + " (repeat=" + (repeat ? "true" : "false") + ")";
        }
    };

    struct KeyReleasedEvent : public KeyEvent {
        KeyReleasedEvent(int k, int s, int m) {
            type = EventType::KeyReleased;
            keyCode = k; scancode = s; mods = m; repeat = false;
        }
        const char* getName() const override { return "KeyReleased"; }
        std::string toString() const override {
            return "KeyReleased: " + std::to_string(keyCode);
        }
    };

    struct KeyTypedEvent : public Event {
        unsigned int codepoint;
        KeyTypedEvent(unsigned int cp) : codepoint(cp) {
            type = EventType::KeyTyped;
        }
        const char* getName() const override { return "KeyTyped"; }
        EventCategory getCategory() const override { return EventCategory::Keyboard; }
        std::string toString() const override {
            return "KeyTyped: " + std::to_string(codepoint) + " ('" + static_cast<char>(codepoint) + "')";
        }
    };

    struct MouseMovedEvent : public Event {
        float x;
        float y;
        float deltaX;
        float deltaY;

        MouseMovedEvent(float mx, float my, float dx = 0, float dy = 0)
            : x(mx), y(my), deltaX(dx), deltaY(dy) {
            type = EventType::MouseMoved;
        }

        const char* getName() const override { return "MouseMoved"; }
        EventCategory getCategory() const override {
            return static_cast<EventCategory>(static_cast<int>(EventCategory::Input) | static_cast<int>(EventCategory::Mouse));
        }
        std::string toString() const override {
            return "MouseMoved: (" + std::to_string(x) + ", " + std::to_string(y) + ")";
        }
    };

    struct MouseButtonEvent : public Event {
        int button;
        float x;
        float y;

        EventCategory getCategory() const override {
            return static_cast<EventCategory>(static_cast<int>(EventCategory::Input) | static_cast<int>(EventCategory::Mouse));
        }
    };

    struct MousePressedEvent : public MouseButtonEvent {
        MousePressedEvent(int btn, float mx, float my) {
            type = EventType::MousePressed;
            button = btn; x = mx; y = my;
        }
        const char* getName() const override { return "MousePressed"; }
        std::string toString() const override {
            return "MousePressed: button=" + std::to_string(button) + " at (" + std::to_string(x) + ", " + std::to_string(y) + ")";
        }
    };

    struct MouseReleasedEvent : public MouseButtonEvent {
        MouseReleasedEvent(int btn, float mx, float my) {
            type = EventType::MouseReleased;
            button = btn; x = mx; y = my;
        }
        const char* getName() const override { return "MouseReleased"; }
        std::string toString() const override {
            return "MouseReleased: button=" + std::to_string(button);
        }
    };

    struct MouseScrolledEvent : public Event {
        float deltaX;
        float deltaY;

        MouseScrolledEvent(float dx, float dy) : deltaX(dx), deltaY(dy) {
            type = EventType::MouseScrolled;
        }

        const char* getName() const override { return "MouseScrolled"; }
        EventCategory getCategory() const override {
            return static_cast<EventCategory>(static_cast<int>(EventCategory::Input) | static_cast<int>(EventCategory::Mouse));
        }
        std::string toString() const override {
            return "MouseScrolled: (" + std::to_string(deltaX) + ", " + std::to_string(deltaY) + ")";
        }
    };

    struct GamepadConnectedEvent : public Event {
        int gamepadId;
        GamepadConnectedEvent(int id) : gamepadId(id) { type = EventType::GamepadConnected; }
        const char* getName() const override { return "GamepadConnected"; }
        EventCategory getCategory() const override { return EventCategory::Gamepad; }
    };

    struct GamepadDisconnectedEvent : public Event {
        int gamepadId;
        GamepadDisconnectedEvent(int id) : gamepadId(id) { type = EventType::GamepadDisconnected; }
        const char* getName() const override { return "GamepadDisconnected"; }
        EventCategory getCategory() const override { return EventCategory::Gamepad; }
    };

    struct TextEnteredEvent : public Event {
        unsigned int unicode;
        TextEnteredEvent(unsigned int u) : unicode(u) { type = EventType::TextEntered; }
        const char* getName() const override { return "TextEntered"; }
        EventCategory getCategory() const override { return EventCategory::Input; }
    };

    struct FileDroppedEvent : public Event {
        std::vector<std::string> files;
        FileDroppedEvent(const std::vector<std::string>& f) : files(f) { type = EventType::FileDropped; }
        const char* getName() const override { return "FileDropped"; }
        EventCategory getCategory() const override { return EventCategory::Window; }
    };

    struct UserEvent : public Event {
        int userType;
        std::any data;

        UserEvent(int t, const std::any& d = {}) : userType(t), data(d) {
            type = EventType::UserEvent;
        }

        const char* getName() const override { return "UserEvent"; }
        EventCategory getCategory() const override { return EventCategory::None; }
        std::string toString() const override {
            return "UserEvent: type=" + std::to_string(userType);
        }
    };

    using EventCallback = std::function<void(Event&)>;
    using EventFilterCallback = std::function<bool(Event&)>;

    class EventBus;

    class EventDispatcher {
    public:
        void addListener(EventType type, EventCallback callback, EventPriority priority = EventPriority::Normal);
        void removeListener(EventType type);
        void removeListener(EventType type, const EventCallback& callback);
        void clear();

        void dispatch(Event& event);
        void dispatch(EventType type, Event& event);

        template<typename T>
        void dispatch(T& event) {
            dispatch(static_cast<Event&>(event));
        }

        bool hasListeners(EventType type) const;
        int getListenerCount(EventType type) const;

        void addFilter(EventCategory category, EventFilterCallback filter);
        void removeFilter(EventCategory category);

    private:
        struct ListenerEntry {
            EventCallback callback;
            EventPriority priority;
        };

        std::unordered_map<EventType, std::vector<ListenerEntry>> m_listeners;
        std::unordered_map<int, EventFilterCallback> m_filters;
    };

    class EventQueue {
    public:
        void enqueue(EventType type, std::unique_ptr<Event> event);
        void enqueueBlocking(EventType type, std::unique_ptr<Event> event);
        std::unique_ptr<Event> dequeue();
        bool tryDequeue(Event& event);
        size_t size() const;
        bool isEmpty() const;
        void clear();
        void processAll(EventDispatcher& dispatcher);

    private:
        struct QueuedEvent {
            EventType type;
            std::unique_ptr<Event> event;
            EventPriority priority;
            bool operator<(const QueuedEvent& other) const {
                return static_cast<int>(priority) < static_cast<int>(other.priority);
            }
        };

        std::vector<QueuedEvent> m_queue;
        std::vector<QueuedEvent> m_blockingQueue;
        mutable std::mutex m_mutex;
    };

    class EventBus {
    public:
        void publish(std::unique_ptr<Event> event);
        void subscribe(EventType type, EventCallback callback);
        void unsubscribe(EventType type);
        void process();

        EventDispatcher& getDispatcher() { return m_dispatcher; }

    private:
        EventQueue m_queue;
        EventDispatcher m_dispatcher;
    };

}

