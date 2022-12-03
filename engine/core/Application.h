#pragma once

#include "Engine.h"
#include <memory>
#include <string>
#include <vector>

namespace nebula {

    struct ApplicationConfig {
        std::string name = "Nebula Application";
        std::string version = "1.0.0";
        std::string organization = "Nebula Engine";
        int windowWidth = 1280;
        int windowHeight = 720;
        bool fullscreen = false;
        int antialiasingLevel = 0;
        bool vsync = true;
        std::string configFilePath;
        std::string logFilePath = "nebula.log";
        std::vector<std::string> commandLineArgs;
    };

    class Application {
    public:
        Application();
        explicit Application(const ApplicationConfig& config);
        virtual ~Application();

        virtual bool onInitialize();
        virtual void onShutdown();
        virtual void onUpdate(float dt);
        virtual void onRender(float dt);
        virtual void onEvent(Event& event);

        bool initialize();
        void shutdown();
        void update(float dt);
        void render(float dt);

        void run();
        void quit();

        void parseCommandLine(int argc, char* argv[]);

        Engine& getEngine() { return *m_engine; }
        const Engine& getEngine() const { return *m_engine; }

        const ApplicationConfig& getConfig() const { return m_config; }
        void setConfig(const ApplicationConfig& config) { m_config = config; }

        const std::string& getName() const { return m_config.name; }
        const std::string& getVersion() const { return m_config.version; }
        const std::string& getOrganization() const { return m_config.organization; }

        bool isRunning() const { return m_running; }

    protected:
        std::unique_ptr<Engine> m_engine;
        ApplicationConfig m_config;
        bool m_running = false;
        bool m_initialized = false;
    };

}

