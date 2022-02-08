#pragma once

#include "Engine.h"
#include <memory>

namespace nebula {

    class Application {
    public:
        Application();
        virtual ~Application();

        virtual bool initialize();
        virtual void shutdown();
        virtual void update(float dt);
        virtual void render(float dt);

        void run();
        void quit();

        Engine& getEngine() { return *m_engine; }
        const Engine& getEngine() const { return *m_engine; }

        bool isRunning() const { return m_running; }

    protected:
        std::unique_ptr<Engine> m_engine;
        bool m_running = false;
        bool m_initialized = false;
    };

}
