#pragma once

#include "Window.h"
#include "Input.h"
#include "Event.h"
#include "Time.h"
#include "Logger.h"
#include "Config.h"
#include "Profiler.h"
#include "Settings.h"
#include "GameLoop.h"
#include "Platform.h"

#include <memory>
#include <string>
#include <functional>

namespace nebula {

    class Engine {
    public:
        Engine();
        ~Engine();

        bool initialize(const std::string& configPath = "");
        void shutdown();
        void run();
        void stop();

        Window& getWindow() { return *m_window; }
        Input& getInput() { return Input::instance(); }
        Time& getTime() { return Time::instance(); }
        EventDispatcher& getEventDispatcher() { return *m_eventDispatcher; }
        Settings& getSettings() { return Settings::instance(); }
        Config& getConfig() { return Config::instance(); }
        Profiler& getProfiler() { return Profiler::instance(); }

        bool isRunning() const { return m_running; }
        bool isInitialized() const { return m_initialized; }
        const std::string& getVersion() const { return m_version; }

        void setWindowTitle(const std::string& title);
        void setWindowSize(int width, int height);
        void setFullscreen(bool fullscreen);

        void setUpdateCallback(std::function<void(float)> callback) { m_updateCallback = std::move(callback); }
        void setRenderCallback(std::function<void(float)> callback) { m_renderCallback = std::move(callback); }

    private:
        void onEvent(Event& event);
        void update(float dt);
        void render(float dt);
        void handleWindowEvent(const Event& event);

        std::unique_ptr<Window> m_window;
        std::unique_ptr<EventDispatcher> m_eventDispatcher;
        std::unique_ptr<GameLoop> m_gameLoop;

        std::function<void(float)> m_updateCallback;
        std::function<void(float)> m_renderCallback;

        bool m_running = false;
        bool m_initialized = false;
        std::string m_version = "1.0.0";
    };

}
