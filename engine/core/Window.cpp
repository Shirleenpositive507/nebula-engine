#include "Window.h"
#include "Logger.h"
#include "Input.h"

namespace nebula {

    Window::Window() {
        NEBULA_TRACE("Window created (not yet initialized)");
    }

    Window::~Window() {
        close();
    }

    bool Window::create(int width, int height, const std::string& title,
                        bool fullscreen, int antialiasing) {
        m_width = width;
        m_height = height;
        m_title = title;
        m_fullscreen = fullscreen;

        m_contextSettings.antialiasingLevel = static_cast<unsigned int>(antialiasing);
        m_contextSettings.majorVersion = 4;
        m_contextSettings.minorVersion = 1;

        sf::Uint32 style = sf::Style::Default;
        if (fullscreen) {
            style = sf::Style::Fullscreen;
        }

        sf::VideoMode videoMode(width, height);
        if (fullscreen) {
            videoMode = sf::VideoMode::getDesktopMode();
            m_width = static_cast<int>(videoMode.width);
            m_height = static_cast<int>(videoMode.height);
        }

        m_window.create(videoMode, title, style, m_contextSettings);

        if (!m_window.isOpen()) {
            NEBULA_ERROR("Failed to create window: " + title);
            return false;
        }

        m_window.setKeyRepeatEnabled(true);
        m_window.setActive(true);

        loadCursor(CursorStyle::Arrow);
        loadCursor(CursorStyle::Hand);
        loadCursor(CursorStyle::Text);
        loadCursor(CursorStyle::Cross);
        loadCursor(CursorStyle::Wait);

        NEBULA_INFO("Window created: " + title + " (" + std::to_string(m_width) + "x" + std::to_string(m_height) + ")");
        return true;
    }

    void Window::close() {
        if (m_window.isOpen()) {
            m_window.close();
            NEBULA_INFO("Window closed");
        }
    }

    bool Window::isOpen() const {
        return m_window.isOpen();
    }

    void Window::processEvents() {
        sf::Event event;
        while (m_window.pollEvent(event)) {
            if (m_eventCallback) {
                m_eventCallback(event);
            }

            switch (event.type) {
                case sf::Event::KeyPressed:
                    Input::instance().onKeyPressed(event.key.code);
                    break;
                case sf::Event::KeyReleased:
                    Input::instance().onKeyReleased(event.key.code);
                    break;
                case sf::Event::MouseButtonPressed:
                    Input::instance().onMousePressed(event.mouseButton.button);
                    break;
                case sf::Event::MouseButtonReleased:
                    Input::instance().onMouseReleased(event.mouseButton.button);
                    break;
                case sf::Event::MouseMoved:
                    Input::instance().onMouseMoved(
                        static_cast<float>(event.mouseMove.x),
                        static_cast<float>(event.mouseMove.y));
                    break;
                case sf::Event::MouseWheelScrolled:
                    Input::instance().onMouseScrolled(event.mouseWheelScroll.delta);
                    break;
                case sf::Event::Resized: {
                    if (event.size.width == 0 || event.size.height == 0) break;
                    m_width = event.size.width;
                    m_height = event.size.height;
                    sf::View newView(sf::FloatRect(0, 0,
                        static_cast<float>(m_width), static_cast<float>(m_height)));
                    newView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
                    m_window.setView(newView);
                    if (m_resizeCallback) {
                        m_resizeCallback(m_width, m_height);
                    }
                    break;
                }
                case sf::Event::Closed:
                    if (m_closeCallback) {
                        m_closeCallback();
                    } else {
                        m_window.close();
                    }
                    break;
                case sf::Event::GainedFocus:
                    if (m_focusCallback) {
                        m_focusCallback(true);
                    }
                    break;
                case sf::Event::LostFocus:
                    if (m_focusCallback) {
                        m_focusCallback(false);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void Window::clear(const sf::Color& color) {
        m_window.clear(color);
    }

    void Window::display() {
        m_window.display();
    }

    void Window::setVSync(bool enabled) {
        m_vsync = enabled;
        m_window.setVerticalSyncEnabled(enabled);
        NEBULA_INFO("VSync " + std::string(enabled ? "enabled" : "disabled"));
    }

    void Window::setTitle(const std::string& title) {
        m_title = title;
        m_window.setTitle(title);
    }

    void Window::setIcon(const std::string& path) {
        sf::Image image;
        if (image.loadFromFile(path)) {
            m_window.setIcon(image.getSize().x, image.getSize().y, image.getPixelsPtr());
            NEBULA_INFO("Window icon set from: " + path);
        } else {
            NEBULA_WARN("Failed to load window icon: " + path);
        }
    }

    void Window::setIconFromImage(const sf::Image& image) {
        m_window.setIcon(image.getSize().x, image.getSize().y, image.getPixelsPtr());
        NEBULA_INFO("Window icon set from image");
    }

    void Window::setFullscreen(bool fullscreen) {
        if (m_fullscreen == fullscreen) return;
        m_fullscreen = fullscreen;

        sf::Uint32 style = fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
        sf::VideoMode videoMode = fullscreen
            ? sf::VideoMode::getDesktopMode()
            : sf::VideoMode(static_cast<unsigned int>(m_width), static_cast<unsigned int>(m_height));

        m_window.create(videoMode, m_title, style, m_contextSettings);
        NEBULA_INFO("Window switched to " + std::string(fullscreen ? "fullscreen" : "windowed"));
    }

    void Window::setFramerateLimit(unsigned int limit) {
        m_window.setFramerateLimit(limit);
    }

    unsigned int Window::getFramerateLimit() const {
        return m_window.getFramerateLimit();
    }

    void Window::setSize(int width, int height) {
        m_width = width;
        m_height = height;
        m_window.setSize(sf::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height)));
    }

    std::pair<float, float> Window::getSize() const {
        auto size = m_window.getSize();
        return { static_cast<float>(size.x), static_cast<float>(size.y) };
    }

    void Window::minimize() {
        m_window.setVisible(false);
    }

    void Window::maximize() {
        m_window.setVisible(true);
    }

    void Window::restore() {
        m_window.setVisible(true);
    }

    void Window::setPosition(int x, int y) {
        m_window.setPosition(sf::Vector2i(x, y));
    }

    sf::Vector2i Window::getPosition() const {
        return m_window.getPosition();
    }

    void Window::setVisible(bool visible) {
        m_window.setVisible(visible);
    }

    void Window::requestFocus() {
        m_window.requestFocus();
    }

    bool Window::hasFocus() const {
        return m_window.hasFocus();
    }

    bool Window::setCursorStyle(CursorStyle style) {
        if (!loadCursor(style)) {
            NEBULA_WARN("Failed to load cursor style: " + std::to_string(static_cast<int>(style)));
            return false;
        }
        auto it = m_cursors.find(static_cast<int>(style));
        if (it != m_cursors.end() && it->second) {
            m_window.setMouseCursor(*it->second);
            m_currentCursor = style;
            return true;
        }
        return false;
    }

    void Window::setCursorVisible(bool visible) {
        m_cursorVisible = visible;
        m_window.setMouseCursorVisible(visible);
    }

    void Window::setCursorGrabbed(bool grabbed) {
        m_cursorGrabbed = grabbed;
        m_window.setMouseCursorGrabbed(grabbed);
    }

    void Window::setTitleBarColor(const sf::Color& color) {
        (void)color;
    #ifdef NEBULA_PLATFORM_WINDOWS
        HWND hwnd = m_window.getSystemHandle();
        if (hwnd) {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(hwnd, RGB(color.r, color.g, color.b), 0, LWA_COLORKEY);
        }
    #endif
    }

    void Window::setTitleBarHeight(int height) {
        (void)height;
    #ifdef NEBULA_PLATFORM_WINDOWS
        HWND hwnd = m_window.getSystemHandle();
        if (hwnd) {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            SetWindowPos(hwnd, nullptr, rect.left, rect.top,
                         rect.right - rect.left, rect.bottom - rect.top + height - 30,
                         SWP_FRAMECHANGED | SWP_NOZORDER);
        }
    #endif
    }

    int Window::getMonitorCount() const {
        return static_cast<int>(sf::VideoMode::getFullscreenModes().size());
    }

    sf::VideoMode Window::getMonitorMode(int monitor) const {
        auto modes = sf::VideoMode::getFullscreenModes();
        if (monitor >= 0 && monitor < static_cast<int>(modes.size())) {
            return modes[monitor];
        }
        return sf::VideoMode::getDesktopMode();
    }

    std::string Window::getMonitorName(int monitor) const {
        (void)monitor;
        return "Monitor " + std::to_string(monitor);
    }

    bool Window::loadCursor(CursorStyle style) {
        int key = static_cast<int>(style);
        auto it = m_cursors.find(key);
        if (it != m_cursors.end()) {
            return it->second != nullptr;
        }

        auto cursor = std::make_unique<sf::Cursor>();
        sf::Cursor::Type sfType;

        switch (style) {
            case CursorStyle::Arrow: sfType = sf::Cursor::Arrow; break;
            case CursorStyle::Hand: sfType = sf::Cursor::Hand; break;
            case CursorStyle::Text: sfType = sf::Cursor::Text; break;
            case CursorStyle::Cross: sfType = sf::Cursor::Cross; break;
            case CursorStyle::Wait: sfType = sf::Cursor::Wait; break;
            case CursorStyle::SizeHorizontal: sfType = sf::Cursor::SizeHorizontal; break;
            case CursorStyle::SizeVertical: sfType = sf::Cursor::SizeVertical; break;
            case CursorStyle::SizeTopLeftBottomRight: sfType = sf::Cursor::SizeTopLeftBottomRight; break;
            case CursorStyle::SizeBottomLeftTopRight: sfType = sf::Cursor::SizeBottomLeftTopRight; break;
            case CursorStyle::Help: sfType = sf::Cursor::Help; break;
            case CursorStyle::NotAllowed: sfType = sf::Cursor::NotAllowed; break;
            default: return false;
        }

        if (cursor->loadFromSystem(sfType)) {
            m_cursors[key] = std::move(cursor);
            return true;
        }

        m_cursors[key] = nullptr;
        return false;
    }

    void Window::applySettings() {
        m_window.setVerticalSyncEnabled(m_vsync);
    }

}

