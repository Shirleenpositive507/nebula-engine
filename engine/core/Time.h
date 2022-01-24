#pragma once

#include <chrono>
#include <cstdint>

namespace nebula {

    class Time {
    public:
        static Time& instance();

        void update();

        float getDeltaTime() const { return m_deltaTime; }
        float getElapsedTime() const { return m_elapsedTime; }
        float getFixedDeltaTime() const { return m_fixedDeltaTime; }
        float getUnscaledDeltaTime() const { return m_unscaledDeltaTime; }

        void setTimeScale(float scale) { m_timeScale = scale; }
        float getTimeScale() const { return m_timeScale; }

        int getFrameCount() const { return m_frameCount; }
        float getFPS() const { return m_fps; }

        void setFixedDeltaTime(float dt) { m_fixedDeltaTime = dt; }

        float getTotalTime() const { return m_totalTime; }

        void reset();

    private:
        Time();
        Time(const Time&) = delete;
        Time& operator=(const Time&) = delete;

        std::chrono::high_resolution_clock::time_point m_startTime;
        std::chrono::high_resolution_clock::time_point m_lastFrame;

        float m_deltaTime = 0.0f;
        float m_unscaledDeltaTime = 0.0f;
        float m_elapsedTime = 0.0f;
        float m_totalTime = 0.0f;
        float m_fixedDeltaTime = 1.0f / 60.0f;
        float m_timeScale = 1.0f;

        int m_frameCount = 0;
        float m_fps = 0.0f;
        float m_fpsTimer = 0.0f;
        int m_fpsFrameCount = 0;
    };

}
