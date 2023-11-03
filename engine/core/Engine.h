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
#include "CommandLine.h"
#include "TaskSystem.h"

#include <memory>
#include <string>
#include <functional>
#include <array>

namespace nebula {

    enum class EngineState {
        Uninitialized,
        Initializing,
        Running,
        Paused,
        Stopped
    };

    class EventBus;

    class Engine {
    public:
        Engine();
        ~Engine();

        bool initialize(const std::string& configPath = "");
        void shutdown();
        void run();
        void stop();

        void pause();
        void resume();

        Window& getWindow() { return *m_window; }
        Input& getInput() { return Input::instance(); }
        Time& getTime() { return Time::instance(); }
        EventDispatcher& getEventDispatcher() { return *m_eventDispatcher; }
        Settings& getSettings() { return Settings::instance(); }
        Config& getConfig() { return Config::instance(); }
        Profiler& getProfiler() { return Profiler::instance(); }
        CommandLine& getCommandLine() { return CommandLine::instance(); }
        TaskSystem& getTaskSystem() { return *m_taskSystem; }

        bool isRunning() const { return m_state == EngineState::Running; }
        bool isInitialized() const { return m_state != EngineState::Uninitialized; }
        bool isPaused() const { return m_state == EngineState::Paused; }
        EngineState getState() const { return m_state; }
        const std::string& getVersion() const { return m_version; }

        void setWindowTitle(const std::string& title);
        void setWindowSize(int width, int height);
        void setFullscreen(bool fullscreen);

        void setClearColor(const sf::Color& color) { m_clearColor = color; }
        const sf::Color& getClearColor() const { return m_clearColor; }

        void setUpdateCallback(std::function<void(float)> callback) { m_updateCallback = std::move(callback); }
        void setRenderCallback(std::function<void(float)> callback) { m_renderCallback = std::move(callback); }

        void setFixedTimestep(float dt) { m_fixedTimestep = dt; }
        float getFixedTimestep() const { return m_fixedTimestep; }

        float getAccumulator() const { return m_accumulator; }
        void setAccumulator(float acc) { m_accumulator = acc; }

    private:
        void onEvent(Event& event);
        void update(float dt);
        void render(float dt);
        void handleWindowEvent(const Event& event);

        std::unique_ptr<Window> m_window;
        std::unique_ptr<EventDispatcher> m_eventDispatcher;
        std::unique_ptr<GameLoop> m_gameLoop;
        std::unique_ptr<EventBus> m_eventBus;
        std::unique_ptr<TaskSystem> m_taskSystem;

        std::function<void(float)> m_updateCallback;
        std::function<void(float)> m_renderCallback;

        EngineState m_state = EngineState::Uninitialized;
        sf::Color m_clearColor = sf::Color::Black;
        float m_fixedTimestep = 1.0f / 60.0f;
        float m_accumulator = 0.0f;
        std::string m_version = "1.0.0";
    };

}

