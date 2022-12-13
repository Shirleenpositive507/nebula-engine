#pragma once

#include <chrono>
#include <cstdint>
#include <array>

namespace nebula {

    enum class TimeScaleGroup {
        Global = 0,
        Physics,
        Rendering,
        Audio,
        UI,
        Network,
        Count
    };

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

        void setGroupTimeScale(TimeScaleGroup group, float scale);
        float getGroupTimeScale(TimeScaleGroup group) const;
        float getGroupDeltaTime(TimeScaleGroup group) const;

        int getFrameCount() const { return m_frameCount; }
        float getFPS() const { return m_fps; }

        void setFixedDeltaTime(float dt) { m_fixedDeltaTime = dt; }

        float getTotalTime() const { return m_totalTime; }

        float getAccumulator() const { return m_accumulator; }
        void setAccumulator(float acc) { m_accumulator = acc; }

        void setFrameLimit(float fps);
        float getFrameLimit() const { return m_frameLimit; }

        float getFrameTimeMin() const { return m_frameTimeMin; }
        float getFrameTimeMax() const { return m_frameTimeMax; }
        float getFrameTimeAvg() const { return m_frameTimeAvg; }

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
        float m_accumulator = 0.0f;
        float m_frameLimit = 0.0f;

        float m_groupTimeScales[static_cast<int>(TimeScaleGroup::Count)];
        float m_groupDeltaTimes[static_cast<int>(TimeScaleGroup::Count)];

        int m_frameCount = 0;
        float m_fps = 0.0f;
        float m_fpsTimer = 0.0f;
        int m_fpsFrameCount = 0;

        static constexpr int kHistorySize = 60;
        std::array<float, kHistorySize> m_frameTimeHistory{};
        int m_historyIndex = 0;
        int m_historyCount = 0;
        float m_frameTimeMin = 0.0f;
        float m_frameTimeMax = 0.0f;
        float m_frameTimeAvg = 0.0f;

        void updateFrameTimeHistory(float dt);
    };

}

