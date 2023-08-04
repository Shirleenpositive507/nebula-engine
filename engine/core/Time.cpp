#include "Time.h"
#include "Platform.h"
#include <algorithm>

namespace nebula {

    Time& Time::instance() {
        static Time inst;
        return inst;
    }

    Time::Time() {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_lastFrame = m_startTime;
        for (int i = 0; i < static_cast<int>(TimeScaleGroup::Count); ++i) {
            m_groupTimeScales[i] = 1.0f;
            m_groupDeltaTimes[i] = 0.0f;
        }
    }

    void Time::update() {
        auto now = std::chrono::high_resolution_clock::now();

        auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastFrame);
        m_unscaledDeltaTime = static_cast<float>(frameDuration.count()) / 1000000.0f;

        if (m_unscaledDeltaTime > 0.1f) {
            m_unscaledDeltaTime = 0.1f;
            m_lastFrame = now;
        }

        m_deltaTime = m_unscaledDeltaTime * m_timeScale;

        for (int i = 0; i < static_cast<int>(TimeScaleGroup::Count); ++i) {
            m_groupDeltaTimes[i] = m_unscaledDeltaTime * m_timeScale * m_groupTimeScales[i];
        }

        auto totalDuration = std::chrono::duration_cast<std::chrono::duration<float>>(now - m_startTime);
        m_totalTime = totalDuration.count();

        m_elapsedTime += m_deltaTime;
        m_accumulator += m_deltaTime;

        m_lastFrame = now;
        m_frameCount++;
        m_fpsFrameCount++;

        updateFrameTimeHistory(m_unscaledDeltaTime);

        m_fpsTimer += m_unscaledDeltaTime;
        if (m_fpsTimer >= 0.5f) {
            m_fps = static_cast<float>(m_fpsFrameCount) / m_fpsTimer;
            m_fpsFrameCount = 0;
            m_fpsTimer -= 0.5f;
        }

        if (m_frameLimit > 0.0f) {
            float targetFrameTime = 1.0f / m_frameLimit;
            float frameTime = static_cast<float>(frameDuration.count()) / 1000000.0f;
            float sleepTime = targetFrameTime - frameTime;
            if (sleepTime > 0.001f) {
                platformSleep(sleepTime);
            }
        }
    }

    void Time::setGroupTimeScale(TimeScaleGroup group, float scale) {
        int idx = static_cast<int>(group);
        if (idx >= 0 && idx < static_cast<int>(TimeScaleGroup::Count)) {
            m_groupTimeScales[idx] = scale;
        }
    }

    float Time::getGroupTimeScale(TimeScaleGroup group) const {
        int idx = static_cast<int>(group);
        if (idx >= 0 && idx < static_cast<int>(TimeScaleGroup::Count)) {
            return m_groupTimeScales[idx];
        }
        return 1.0f;
    }

    float Time::getGroupDeltaTime(TimeScaleGroup group) const {
        int idx = static_cast<int>(group);
        if (idx >= 0 && idx < static_cast<int>(TimeScaleGroup::Count)) {
            return m_groupDeltaTimes[idx];
        }
        return m_deltaTime;
    }

    void Time::setFrameLimit(float fps) {
        m_frameLimit = fps;
    }

    void Time::updateFrameTimeHistory(float dt) {
        m_frameTimeHistory[m_historyIndex] = dt;
        m_historyIndex = (m_historyIndex + 1) % kHistorySize;
        if (m_historyCount < kHistorySize) {
            m_historyCount++;
        }

        m_frameTimeMin = m_frameTimeHistory[0];
        m_frameTimeMax = m_frameTimeHistory[0];
        float sum = 0.0f;

        for (int i = 0; i < m_historyCount; ++i) {
            float val = m_frameTimeHistory[i];
            sum += val;
            if (val < m_frameTimeMin) m_frameTimeMin = val;
            if (val > m_frameTimeMax) m_frameTimeMax = val;
        }

        m_frameTimeAvg = sum / static_cast<float>(m_historyCount);
    }

    void Time::reset() {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_lastFrame = m_startTime;
        m_deltaTime = 0.0f;
        m_unscaledDeltaTime = 0.0f;
        m_elapsedTime = 0.0f;
        m_totalTime = 0.0f;
        m_accumulator = 0.0f;
        m_frameCount = 0;
        m_fps = 0.0f;
        m_fpsTimer = 0.0f;
        m_fpsFrameCount = 0;
        m_historyIndex = 0;
        m_historyCount = 0;
        m_frameTimeMin = 0.0f;
        m_frameTimeMax = 0.0f;
        m_frameTimeAvg = 0.0f;
    }

}

