#pragma once

#include "Time.h"
#include <functional>
#include <chrono>
#include <thread>

namespace nebula {

    enum class LoopMode {
        FIXED,
        VARIABLE,
        HYBRID
    };

    class GameLoop {
    public:
        GameLoop();

        void setLoopMode(LoopMode mode);
        LoopMode getLoopMode() const { return m_mode; }

        void setFixedTimestep(float timestep);
        float getFixedTimestep() const { return m_fixedTimestep; }

        void setMaxFrameRate(float fps);
        float getMaxFrameRate() const { return m_maxFramerate; }

        void start(std::function<void(float)> updateCallback,
                   std::function<void(float)> renderCallback);
        void stop();

        bool isRunning() const { return m_running; }

        void setMaxUpdateSteps(int steps);
        int getMaxUpdateSteps() const { return m_maxUpdateSteps; }

        float getUpdateTime() const { return m_updateTime; }
        float getRenderTime() const { return m_renderTime; }
        float getFrameTime() const { return m_frameTime; }

    private:
        void fixedLoop(std::function<void(float)>& update,
                       std::function<void(float)>& render);
        void variableLoop(std::function<void(float)>& update,
                          std::function<void(float)>& render);
        void hybridLoop(std::function<void(float)>& update,
                        std::function<void(float)>& render);

        LoopMode m_mode = LoopMode::HYBRID;
        float m_fixedTimestep = 1.0f / 60.0f;
        float m_maxFramerate = 0.0f;
        int m_maxUpdateSteps = 5;
        bool m_running = false;

        float m_updateTime = 0.0f;
        float m_renderTime = 0.0f;
        float m_frameTime = 0.0f;
    };

}
