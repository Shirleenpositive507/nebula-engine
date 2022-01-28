#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Image.hpp>
#include <string>
#include <functional>
#include <memory>

namespace nebula {

    class Window {
    public:
        Window();
        ~Window();

        bool create(int width, int height, const std::string& title,
                    bool fullscreen = false, int antialiasing = 0);
        void close();
        bool isOpen() const;
        void processEvents();
        void clear(const sf::Color& color = sf::Color::Black);
        void display();

        sf::RenderWindow& getHandle() { return m_window; }
        const sf::RenderWindow& getHandle() const { return m_window; }

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }
        float getAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

        void setVSync(bool enabled);
        void setTitle(const std::string& title);
        void setIcon(const std::string& path);
        void setFullscreen(bool fullscreen);
        bool isFullscreen() const { return m_fullscreen; }
        void setVerticalSync(bool vsync) { setVSync(vsync); }

        void setFramerateLimit(unsigned int limit);
        unsigned int getFramerateLimit() const;

        void setSize(int width, int height);

        std::pair<float, float> getSize() const;
        sf::Vector2u getSizeU() const { return m_window.getSize(); }

        void minimize();
        void maximize();
        void restore();
        void setPosition(int x, int y);
        sf::Vector2i getPosition() const;

        void setVisible(bool visible);
        void requestFocus();
        bool hasFocus() const;

        using EventCallback = std::function<void(sf::Event&)>;
        void setEventCallback(EventCallback callback) { m_eventCallback = std::move(callback); }
        void clearEventCallback() { m_eventCallback = nullptr; }

    private:
        void applySettings();

        sf::RenderWindow m_window;
        sf::ContextSettings m_contextSettings;
        int m_width = 1280;
        int m_height = 720;
        std::string m_title = "Nebula Engine";
        bool m_fullscreen = false;
        bool m_vsync = false;
        EventCallback m_eventCallback;
    };

}
