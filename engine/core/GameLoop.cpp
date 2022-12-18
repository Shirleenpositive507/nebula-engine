#include "GameLoop.h"
#include "Logger.h"
#include "Platform.h"
#include <algorithm>

namespace nebula {

    GameLoop::GameLoop() {
        NEBULA_TRACE("GameLoop created");
    }

    void GameLoop::setLoopMode(LoopMode mode) {
        m_mode = mode;
        NEBULA_INFO("GameLoop mode set to: " +
            std::string(mode == LoopMode::FIXED ? "FIXED" :
                        mode == LoopMode::VARIABLE ? "VARIABLE" : "HYBRID"));
    }

    void GameLoop::setFramePacingMode(FramePacingMode mode) {
        m_pacingMode = mode;
        NEBULA_INFO("Frame pacing mode set to: " +
            std::string(mode == FramePacingMode::None ? "None" :
                        mode == FramePacingMode::Sleep ? "Sleep" :
                        mode == FramePacingMode::BusyWait ? "BusyWait" : "Adaptive"));
    }

    void GameLoop::setFixedTimestep(float timestep) {
        if (timestep > 0.0f) {
            m_fixedTimestep = timestep;
            NEBULA_INFO("Fixed timestep set to: " + std::to_string(timestep) +
                       " (" + std::to_string(1.0f / timestep) + " UPS)");
        }
    }

    void GameLoop::setMaxFrameRate(float fps) {
        m_maxFramerate = fps;
        if (fps > 0.0f) {
            NEBULA_INFO("Max framerate set to: " + std::to_string(fps) + " FPS");
        } else {
            NEBULA_INFO("Max framerate: unlimited");
        }
    }

    void GameLoop::start(std::function<void(float)> updateCallback,
                          std::function<void(float)> renderCallback) {
        if (m_running) {
            NEBULA_WARN("GameLoop already running");
            return;
        }

        if (!updateCallback || !renderCallback) {
            NEBULA_ERROR("GameLoop requires both update and render callbacks");
            return;
        }

        m_running = true;
        resetStatistics();
        NEBULA_INFO("GameLoop started (mode: " +
            std::string(m_mode == LoopMode::FIXED ? "FIXED" :
                        m_mode == LoopMode::VARIABLE ? "VARIABLE" : "HYBRID") + ")");

        switch (m_mode) {
            case LoopMode::FIXED:
                fixedLoop(updateCallback, renderCallback);
                break;
            case LoopMode::VARIABLE:
                variableLoop(updateCallback, renderCallback);
                break;
            case LoopMode::HYBRID:
                hybridLoop(updateCallback, renderCallback);
                break;
        }
    }

    void GameLoop::stop() {
        m_running = false;
        NEBULA_INFO("GameLoop stopped");
    }

    void GameLoop::resetStatistics() {
        m_stats = LoopStatistics{};
    }

    void GameLoop::fixedLoop(std::function<void(float)>& update,
                              std::function<void(float)>& render) {
        auto previous = std::chrono::high_resolution_clock::now();
        float lag = 0.0f;

        while (m_running) {
            auto current = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(current - previous);
            previous = current;

            lag += elapsed.count();

            int steps = 0;
            while (lag >= m_fixedTimestep && steps < m_maxUpdateSteps) {
                if (m_profileBegin) m_profileBegin("Update");
                auto updateStart = std::chrono::high_resolution_clock::now();
                update(m_fixedTimestep);
                auto updateEnd = std::chrono::high_resolution_clock::now();
                m_updateTime = std::chrono::duration_cast<std::chrono::duration<float>>(updateEnd - updateStart).count();
                if (m_profileEnd) m_profileEnd("Update");

                lag -= m_fixedTimestep;
                steps++;
            }

            if (steps >= m_maxUpdateSteps) {
                lag = 0.0f;
            }

            float interpolation = lag / m_fixedTimestep;

            if (m_profileBegin) m_profileBegin("Render");
            auto renderStart = std::chrono::high_resolution_clock::now();
            render(interpolation);
            auto renderEnd = std::chrono::high_resolution_clock::now();
            m_renderTime = std::chrono::duration_cast<std::chrono::duration<float>>(renderEnd - renderStart).count();
            if (m_profileEnd) m_profileEnd("Render");

            m_frameTime = std::chrono::duration_cast<std::chrono::duration<float>>(renderEnd - current).count();
            updateStatistics(m_updateTime, m_renderTime, m_frameTime);

            applyFramePacing(m_frameTime);
        }
    }

    void GameLoop::variableLoop(std::function<void(float)>& update,
                                 std::function<void(float)>& render) {
        auto previous = std::chrono::high_resolution_clock::now();

        while (m_running) {
            auto current = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(current - previous);
            previous = current;

            float dt = elapsed.count();
            if (dt > 0.1f) dt = 0.1f;

            {
                if (m_profileBegin) m_profileBegin("Update");
                auto updateStart = std::chrono::high_resolution_clock::now();
                update(dt);
                auto updateEnd = std::chrono::high_resolution_clock::now();
                m_updateTime = std::chrono::duration_cast<std::chrono::duration<float>>(updateEnd - updateStart).count();
                if (m_profileEnd) m_profileEnd("Update");
            }

            {
                if (m_profileBegin) m_profileBegin("Render");
                auto renderStart = std::chrono::high_resolution_clock::now();
                render(dt);
                auto renderEnd = std::chrono::high_resolution_clock::now();
                m_renderTime = std::chrono::duration_cast<std::chrono::duration<float>>(renderEnd - renderStart).count();
                if (m_profileEnd) m_profileEnd("Render");
            }

            m_frameTime = std::chrono::duration_cast<std::chrono::duration<float>>(
                std::chrono::high_resolution_clock::now() - current).count();
            updateStatistics(m_updateTime, m_renderTime, m_frameTime);

            applyFramePacing(m_frameTime);
        }
    }

    void GameLoop::hybridLoop(std::function<void(float)>& update,
                               std::function<void(float)>& render) {
        auto previous = std::chrono::high_resolution_clock::now();
        float accumulator = 0.0f;

        while (m_running) {
            auto current = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(current - previous);
            previous = current;

            float frameTime = elapsed.count();
            if (frameTime > 0.1f) frameTime = 0.1f;

            accumulator += frameTime;

            int steps = 0;
            while (accumulator >= m_fixedTimestep && steps < m_maxUpdateSteps) {
                if (m_profileBegin) m_profileBegin("Update");
                auto updateStart = std::chrono::high_resolution_clock::now();
                update(m_fixedTimestep);
                auto updateEnd = std::chrono::high_resolution_clock::now();
                m_updateTime = std::chrono::duration_cast<std::chrono::duration<float>>(updateEnd - updateStart).count();
                if (m_profileEnd) m_profileEnd("Update");

                accumulator -= m_fixedTimestep;
                steps++;
            }

            if (steps >= m_maxUpdateSteps) {
                accumulator = 0.0f;
            }

            float interpolation = accumulator / m_fixedTimestep;

            {
                if (m_profileBegin) m_profileBegin("Render");
                auto renderStart = std::chrono::high_resolution_clock::now();
                render(interpolation);
                auto renderEnd = std::chrono::high_resolution_clock::now();
                m_renderTime = std::chrono::duration_cast<std::chrono::duration<float>>(renderEnd - renderStart).count();
                if (m_profileEnd) m_profileEnd("Render");
            }

            m_frameTime = std::chrono::duration_cast<std::chrono::duration<float>>(
                std::chrono::high_resolution_clock::now() - current).count();
            updateStatistics(m_updateTime, m_renderTime, m_frameTime);

            applyFramePacing(m_frameTime);
        }
    }

    void GameLoop::applyFramePacing(float frameTime) {
        if (m_maxFramerate <= 0.0f) return;

        float targetFrameTime = 1.0f / m_maxFramerate;
        float sleepTime = targetFrameTime - frameTime;

        if (sleepTime <= 0.0f) return;

        switch (m_pacingMode) {
            case FramePacingMode::Sleep:
                platformSleep(sleepTime);
                break;
            case FramePacingMode::BusyWait: {
                auto waitStart = std::chrono::high_resolution_clock::now();
                while (std::chrono::duration_cast<std::chrono::duration<float>>(
                    std::chrono::high_resolution_clock::now() - waitStart).count() < sleepTime) {
                }
                break;
            }
            case FramePacingMode::Adaptive: {
                if (sleepTime > 0.003f) {
                    platformSleep(sleepTime - 0.002f);
                }
                auto waitStart = std::chrono::high_resolution_clock::now();
                while (std::chrono::duration_cast<std::chrono::duration<float>>(
                    std::chrono::high_resolution_clock::now() - waitStart).count() < sleepTime) {
                }
                break;
            }
            default:
                break;
        }
    }

    void GameLoop::updateStatistics(float updateTime, float renderTime, float frameTime) {
        if (m_stats.frameCount == 0) {
            m_stats.minFrameTime = frameTime;
            m_stats.maxFrameTime = frameTime;
            m_stats.minUpdateTime = updateTime;
            m_stats.maxUpdateTime = updateTime;
            m_stats.minRenderTime = renderTime;
            m_stats.maxRenderTime = renderTime;
        } else {
            m_stats.minFrameTime = std::min(m_stats.minFrameTime, frameTime);
            m_stats.maxFrameTime = std::max(m_stats.maxFrameTime, frameTime);
            m_stats.minUpdateTime = std::min(m_stats.minUpdateTime, updateTime);
            m_stats.maxUpdateTime = std::max(m_stats.maxUpdateTime, updateTime);
            m_stats.minRenderTime = std::min(m_stats.minRenderTime, renderTime);
            m_stats.maxRenderTime = std::max(m_stats.maxRenderTime, renderTime);
        }

        m_stats.avgFrameTime = (m_stats.avgFrameTime * static_cast<float>(m_stats.frameCount) + frameTime) /
                               static_cast<float>(m_stats.frameCount + 1);
        m_stats.avgUpdateTime = (m_stats.avgUpdateTime * static_cast<float>(m_stats.frameCount) + updateTime) /
                                static_cast<float>(m_stats.frameCount + 1);
        m_stats.avgRenderTime = (m_stats.avgRenderTime * static_cast<float>(m_stats.frameCount) + renderTime) /
                                static_cast<float>(m_stats.frameCount + 1);

        m_stats.frameCount++;
        m_stats.totalTime += frameTime;
    }

}

