#include "Time.h"
#include "Platform.h"

namespace nebula {

    Time& Time::instance() {
        static Time inst;
        return inst;
    }

    Time::Time() {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_lastFrame = m_startTime;
    }

    void Time::update() {
        auto now = std::chrono::high_resolution_clock::now();

        auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastFrame);
        m_unscaledDeltaTime = static_cast<float>(frameDuration.count()) / 1000000.0f;

        if (m_unscaledDeltaTime > 0.1f) {
            m_unscaledDeltaTime = 0.1f;
        }

        m_deltaTime = m_unscaledDeltaTime * m_timeScale;

        auto totalDuration = std::chrono::duration_cast<std::chrono::duration<float>>(now - m_startTime);
        m_totalTime = totalDuration.count();

        m_elapsedTime += m_deltaTime;

        m_lastFrame = now;
        m_frameCount++;
        m_fpsFrameCount++;

        m_fpsTimer += m_unscaledDeltaTime;
        if (m_fpsTimer >= 0.5f) {
            m_fps = static_cast<float>(m_fpsFrameCount) / m_fpsTimer;
            m_fpsFrameCount = 0;
            m_fpsTimer = 0.0f;
        }
    }

    void Time::reset() {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_lastFrame = m_startTime;
        m_deltaTime = 0.0f;
        m_unscaledDeltaTime = 0.0f;
        m_elapsedTime = 0.0f;
        m_totalTime = 0.0f;
        m_frameCount = 0;
        m_fps = 0.0f;
        m_fpsTimer = 0.0f;
        m_fpsFrameCount = 0;
    }

}
