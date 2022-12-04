#include "Application.h"
#include "Logger.h"
#include <algorithm>
#include <cstring>

namespace nebula {

    Application::Application() {
        m_engine = std::make_unique<Engine>();
    }

    Application::Application(const ApplicationConfig& config)
        : m_config(config) {
        m_engine = std::make_unique<Engine>();
    }

    Application::~Application() {
        if (m_initialized) {
            onShutdown();
            shutdown();
        }
    }

    bool Application::onInitialize() {
        return true;
    }

    void Application::onShutdown() {
    }

    void Application::onUpdate(float dt) {
        (void)dt;
    }

    void Application::onRender(float dt) {
        (void)dt;
    }

    void Application::onEvent(Event& event) {
        (void)event;
    }

    bool Application::initialize() {
        NEBULA_INFO(m_config.name + " v" + m_config.version + " initializing...");
        NEBULA_INFO("Organization: " + m_config.organization);

        if (!m_engine->initialize(m_config.configFilePath)) {
            NEBULA_FATAL("Engine initialization failed");
            return false;
        }

        m_engine->setUpdateCallback([this](float dt) { update(dt); });
        m_engine->setRenderCallback([this](float dt) { render(dt); });

        if (!onInitialize()) {
            NEBULA_FATAL("Application onInitialize failed");
            return false;
        }

        m_initialized = true;
        NEBULA_INFO(m_config.name + " initialized successfully");
        return true;
    }

    void Application::shutdown() {
        if (!m_initialized) return;

        NEBULA_INFO(m_config.name + " shutting down...");
        onShutdown();
        m_engine->shutdown();
        m_initialized = false;
    }

    void Application::update(float dt) {
        onUpdate(dt);
    }

    void Application::render(float dt) {
        onRender(dt);
    }

    void Application::run() {
        if (!m_initialized) {
            if (!initialize()) {
                NEBULA_FATAL("Failed to initialize application");
                return;
            }
        }

        m_running = true;
        NEBULA_INFO(m_config.name + " started");

        m_engine->run();

        m_running = false;
    }

    void Application::quit() {
        NEBULA_INFO(m_config.name + " quit requested");
        m_engine->stop();
        m_running = false;
    }

    void Application::parseCommandLine(int argc, char* argv[]) {
        m_config.commandLineArgs.clear();
        for (int i = 0; i < argc; ++i) {
            m_config.commandLineArgs.push_back(argv[i]);
        }

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg == "--width" || arg == "-w") {
                if (i + 1 < argc) {
                    m_config.windowWidth = std::max(1, std::atoi(argv[++i]));
                }
            } else if (arg == "--height" || arg == "-h") {
                if (i + 1 < argc) {
                    m_config.windowHeight = std::max(1, std::atoi(argv[++i]));
                }
            } else if (arg == "--fullscreen" || arg == "-f") {
                m_config.fullscreen = true;
            } else if (arg == "--vsync") {
                m_config.vsync = true;
            } else if (arg == "--no-vsync") {
                m_config.vsync = false;
            } else if (arg == "--config" || arg == "-c") {
                if (i + 1 < argc) {
                    m_config.configFilePath = argv[++i];
                }
            } else if (arg == "--log" || arg == "-l") {
                if (i + 1 < argc) {
                    m_config.logFilePath = argv[++i];
                }
            } else if (arg == "--antialiasing" || arg == "-a") {
                if (i + 1 < argc) {
                    m_config.antialiasingLevel = std::max(0, std::atoi(argv[++i]));
                }
            }
        }

        NEBULA_INFO("Parsed " + std::to_string(argc) + " command line arguments");
    }

}

