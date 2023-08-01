#include "Engine.h"
#include "Logger.h"
#include "Platform.h"
#include <SFML/Window/Event.hpp>

namespace nebula {

    Engine::Engine() {
        Logger::trace("Engine created");
    }

    Engine::~Engine() {
        if (m_state != EngineState::Uninitialized) {
            shutdown();
        }
    }

    bool Engine::initialize(const std::string& configPath) {
        if (m_state != EngineState::Uninitialized) {
            Logger::warn("Engine already initialized");
            return true;
        }

        m_state = EngineState::Initializing;

        Logger::init("nebula.log");
        Logger::info("Nebula Engine v" + m_version + " initializing...");
        Logger::info("Platform: " + std::string(PlatformInfo::getName()));
        Logger::info("CPU Cores: " + std::to_string(PlatformInfo::getCoreCount()));
        Logger::info("Page Size: " + std::to_string(PlatformInfo::getPageSize()) + " bytes");
        Logger::info("Available Memory: " + std::to_string(PlatformInfo::getAvailableMemory() / (1024 * 1024)) + " MB");

        Config& config = Config::instance();
        if (!configPath.empty()) {
            config.load(configPath);
        }

        Settings& settings = Settings::instance();
        settings.loadDefaults();

        Input::instance();
        Time::instance();

        m_eventBus = std::make_unique<EventBus>();

        int width = settings.getWidth();
        int height = settings.getHeight();
        bool fullscreen = settings.getFullscreen();
        int aa = settings.getAntialiasingLevel();

        m_window = std::make_unique<Window>();
        if (!m_window->create(width, height, "Nebula Engine", fullscreen, aa)) {
            Logger::fatal("Failed to create window");
            m_state = EngineState::Uninitialized;
            return false;
        }

        m_window->setVSync(settings.getVSync());

        m_eventDispatcher = std::make_unique<EventDispatcher>();

        m_eventDispatcher->addListener(EventType::WindowClose, [this](Event& e) {
            (void)e;
            Logger::info("Window close requested");
            stop();
        });

        m_gameLoop = std::make_unique<GameLoop>();

        m_state = EngineState::Running;
        Logger::info("Engine initialization complete");
        return true;
    }

    void Engine::shutdown() {
        if (m_state == EngineState::Uninitialized) return;

        Logger::info("Engine shutting down...");

        if (m_state == EngineState::Running || m_state == EngineState::Paused) {
            stop();
        }

        m_state = EngineState::Stopped;

        if (m_eventDispatcher) {
            m_eventDispatcher->removeAllListeners();
        }

        m_gameLoop.reset();
        m_eventDispatcher.reset();
        m_eventBus.reset();
        m_window.reset();

        m_state = EngineState::Uninitialized;

        Logger::shutdown();
    }

    void Engine::run() {
        if (m_state == EngineState::Uninitialized) {
            Logger::fatal("Cannot run engine before initialize()");
            return;
        }

        if (m_state == EngineState::Running) {
            Logger::warn("Engine already running");
            return;
        }

        m_state = EngineState::Running;
        Logger::info("Engine entering main loop");

        NEBULA_PROFILE_SCOPE("Engine::run");

        auto updateFn = [this](float dt) { update(dt); };
        auto renderFn = [this](float dt) { render(dt); };

        m_gameLoop->start(updateFn, renderFn);

        if (m_state == EngineState::Running) {
            m_state = EngineState::Stopped;
        }
    }

    void Engine::stop() {
        if (m_state != EngineState::Running && m_state != EngineState::Paused) return;
        m_state = EngineState::Stopped;
        if (m_gameLoop) {
            m_gameLoop->stop();
        }
        Logger::info("Engine stopped");
    }

    void Engine::pause() {
        if (m_state != EngineState::Running) {
            Logger::warn("Cannot pause engine in current state");
            return;
        }
        m_state = EngineState::Paused;
        Logger::info("Engine paused");
    }

    void Engine::resume() {
        if (m_state != EngineState::Paused) {
            Logger::warn("Cannot resume engine in current state");
            return;
        }
        m_state = EngineState::Running;
        Logger::info("Engine resumed");
    }

    void Engine::onEvent(Event& event) {
        Logger::trace("Event: " + std::string(event.getName()));
    }

    void Engine::update(float dt) {
        NEBULA_PROFILE_FUNCTION();

        if (m_state == EngineState::Paused) {
            PlatformInfo::sleep(0.001);
            return;
        }

        Time::instance().update();
        Input::instance().update();

        if (m_updateCallback) {
            m_updateCallback(dt);
        }

        if (Input::instance().isKeyPressed(sf::Keyboard::Escape)) {
            Logger::info("Escape pressed - stopping engine");
            stop();
        }
    }

    void Engine::render(float dt) {
        NEBULA_PROFILE_FUNCTION();

        if (m_state == EngineState::Paused) return;

        m_window->clear(m_clearColor);

        if (m_renderCallback) {
            m_renderCallback(dt);
        }

        m_window->display();
    }

    void Engine::handleWindowEvent(const Event& event) {
        switch (event.getType()) {
            case EventType::WindowResize: {
                auto& resizeEvent = static_cast<const WindowResizeEvent&>(event);
                Logger::debug("Window resized: " + std::to_string(resizeEvent.width) +
                            "x" + std::to_string(resizeEvent.height));
                break;
            }
            case EventType::WindowClose:
                stop();
                break;
            default:
                break;
        }
    }

    void Engine::setWindowTitle(const std::string& title) {
        if (m_window) {
            m_window->setTitle(title);
        }
    }

    void Engine::setWindowSize(int width, int height) {
        if (m_window) {
            m_window->setSize(width, height);
            Settings::instance().setResolution(width, height);
        }
    }

    void Engine::setFullscreen(bool fullscreen) {
        if (m_window) {
            m_window->setFullscreen(fullscreen);
            Settings::instance().setFullscreen(fullscreen);
        }
    }

}

