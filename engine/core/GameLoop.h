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

    enum class FramePacingMode {
        None,
        Sleep,
        BusyWait,
        Adaptive
    };

    struct LoopStatistics {
        float minFrameTime = 0.0f;
        float maxFrameTime = 0.0f;
        float avgFrameTime = 0.0f;
        float minUpdateTime = 0.0f;
        float maxUpdateTime = 0.0f;
        float avgUpdateTime = 0.0f;
        float minRenderTime = 0.0f;
        float maxRenderTime = 0.0f;
        float avgRenderTime = 0.0f;
        int frameCount = 0;
        float totalTime = 0.0f;
    };

    class GameLoop {
    public:
        GameLoop();

        void setLoopMode(LoopMode mode);
        LoopMode getLoopMode() const { return m_mode; }

        void setFramePacingMode(FramePacingMode mode);
        FramePacingMode getFramePacingMode() const { return m_pacingMode; }

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

        const LoopStatistics& getStatistics() const { return m_stats; }
        void resetStatistics();

        using ProfilingCallback = std::function<void(const std::string&)>;
        void setProfilingBeginCallback(ProfilingCallback callback) { m_profileBegin = std::move(callback); }
        void setProfilingEndCallback(ProfilingCallback callback) { m_profileEnd = std::move(callback); }

    private:
        void fixedLoop(std::function<void(float)>& update,
                       std::function<void(float)>& render);
        void variableLoop(std::function<void(float)>& update,
                          std::function<void(float)>& render);
        void hybridLoop(std::function<void(float)>& update,
                        std::function<void(float)>& render);

        void applyFramePacing(float frameTime);
        void updateStatistics(float updateTime, float renderTime, float frameTime);

        LoopMode m_mode = LoopMode::HYBRID;
        FramePacingMode m_pacingMode = FramePacingMode::Sleep;
        float m_fixedTimestep = 1.0f / 60.0f;
        float m_maxFramerate = 0.0f;
        int m_maxUpdateSteps = 5;
        bool m_running = false;

        float m_updateTime = 0.0f;
        float m_renderTime = 0.0f;
        float m_frameTime = 0.0f;

        LoopStatistics m_stats;

        ProfilingCallback m_profileBegin;
        ProfilingCallback m_profileEnd;
    };

}

