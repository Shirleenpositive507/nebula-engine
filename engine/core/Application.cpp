#include "Application.h"
#include "Logger.h"

namespace nebula {

    Application::Application() {
        m_engine = std::make_unique<Engine>();
    }

    Application::~Application() {
        if (m_initialized) {
            shutdown();
        }
    }

    bool Application::initialize() {
        NEBULA_INFO("Application initializing...");

        if (!m_engine->initialize()) {
            NEBULA_FATAL("Engine initialization failed");
            return false;
        }

        m_engine->setUpdateCallback([this](float dt) { update(dt); });
        m_engine->setRenderCallback([this](float dt) { render(dt); });

        m_initialized = true;
        NEBULA_INFO("Application initialized successfully");
        return true;
    }

    void Application::shutdown() {
        if (!m_initialized) return;

        NEBULA_INFO("Application shutting down...");
        m_engine->shutdown();
        m_initialized = false;
    }

    void Application::update(float dt) {
    }

    void Application::render(float dt) {
    }

    void Application::run() {
        if (!m_initialized) {
            if (!initialize()) {
                NEBULA_FATAL("Failed to initialize application");
                return;
            }
        }

        m_running = true;
        NEBULA_INFO("Application started");

        m_engine->run();

        m_running = false;
    }

    void Application::quit() {
        NEBULA_INFO("Application quit requested");
        m_engine->stop();
        m_running = false;
    }

}
