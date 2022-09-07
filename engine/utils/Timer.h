#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace nebula {

    class Timer {
    public:
        Timer();

        void start();
        void stop();
        void reset();

        double getElapsedSeconds() const;
        double getElapsedMilliseconds() const;
        double getElapsedMicroseconds() const;

        bool isRunning() const { return m_running; }

    private:
        using Clock = std::chrono::high_resolution_clock;

        Clock::time_point m_startTime;
        Clock::duration m_accumulated{0};
        bool m_running = false;
    };

    class ScopedTimer {
    public:
        explicit ScopedTimer(const std::string& name = "ScopedTimer");
        ~ScopedTimer();

        void stop();

    private:
        Timer m_timer;
        std::string m_name;
        bool m_stopped = false;
    };

    class ManualTimer {
    public:
        ManualTimer();

        void startLap();
        double getLapSeconds();
        double getLapMilliseconds();
        double getTotalSeconds() const;
        double getTotalMilliseconds() const;
        size_t getLapCount() const { return m_laps.size(); }
        const std::vector<double>& getLaps() const { return m_laps; }

    private:
        using Clock = std::chrono::high_resolution_clock;

        Clock::time_point m_startTime;
        Clock::time_point m_lastLapTime;
        std::vector<double> m_laps;
    };

    class Alarm {
    public:
        using Callback = std::function<void()>;

        Alarm();
        explicit Alarm(double durationSeconds, Callback callback = nullptr, bool repeat = false);

        void start(double durationSeconds, Callback callback = nullptr, bool repeat = false);
        void stop();
        void pause();
        void resume();
        void reset();

        void update(double deltaSeconds);

        bool isRunning() const { return m_running; }
        bool isPaused() const { return m_paused; }
        bool hasElapsed() const { return m_elapsed; }
        double getRemaining() const { return m_remaining; }
        double getDuration() const { return m_duration; }
        float getProgress() const {
            return m_duration > 0.0 ? static_cast<float>(1.0 - m_remaining / m_duration) : 0.0f;
        }

    private:
        double m_duration = 0.0;
        double m_remaining = 0.0;
        bool m_running = false;
        bool m_paused = false;
        bool m_elapsed = false;
        bool m_repeat = false;
        Callback m_callback;
    };

    class Clock {
    public:
        Clock();

        void update();
        void setTimeScale(double scale) { m_timeScale = scale; }
        double getTimeScale() const { return m_timeScale; }

        double getDeltaTime() const { return m_deltaTime; }
        double getRawDeltaTime() const { return m_rawDeltaTime; }
        double getTotalTime() const { return m_totalTime; }
        double getFPS() const { return m_fps; }
        double getFrameCount() const { return m_frameCount; }

        void setMaxDeltaTime(double maxDt) { m_maxDeltaTime = maxDt; }
        double getMaxDeltaTime() const { return m_maxDeltaTime; }

    private:
        using ClockType = std::chrono::high_resolution_clock;

        ClockType::time_point m_lastTime;
        double m_deltaTime = 0.0;
        double m_rawDeltaTime = 0.0;
        double m_totalTime = 0.0;
        double m_timeScale = 1.0;
        double m_maxDeltaTime = 0.1;

        double m_fps = 0.0;
        double m_fpsAccumulator = 0.0;
        int m_frameCount = 0;
        int m_fpsFrameCount = 0;
    };

}
