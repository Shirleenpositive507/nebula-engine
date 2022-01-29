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
                case sf::Event::Resized:
                    m_width = event.size.width;
                    m_height = event.size.height;
                    m_window.setView(sf::View(sf::FloatRect(0, 0,
                        static_cast<float>(m_width), static_cast<float>(m_height))));
                    break;
                case sf::Event::Closed:
                    m_window.close();
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

    void Window::applySettings() {
        m_window.setVerticalSyncEnabled(m_vsync);
    }

}
