#include "Timer.h"
#include <iostream>

namespace nebula {

    Timer::Timer() {
        reset();
    }

    void Timer::start() {
        if (!m_running) {
            m_startTime = Clock::now();
            m_running = true;
        }
    }

    void Timer::stop() {
        if (m_running) {
            m_accumulated += Clock::now() - m_startTime;
            m_running = false;
        }
    }

    void Timer::reset() {
        m_running = false;
        m_accumulated = Clock::duration::zero();
        m_startTime = Clock::time_point();
    }

    double Timer::getElapsedSeconds() const {
        auto total = m_accumulated;
        if (m_running) {
            total += Clock::now() - m_startTime;
        }
        return std::chrono::duration<double>(total).count();
    }

    double Timer::getElapsedMilliseconds() const {
        return getElapsedSeconds() * 1000.0;
    }

    double Timer::getElapsedMicroseconds() const {
        return getElapsedSeconds() * 1000000.0;
    }

    ScopedTimer::ScopedTimer(const std::string& name)
        : m_name(name)
    {
        m_timer.start();
    }

    ScopedTimer::~ScopedTimer() {
        if (!m_stopped) {
            stop();
        }
    }

    void ScopedTimer::stop() {
        m_timer.stop();
        m_stopped = true;
        std::cout << "[Timer] " << m_name << ": "
                  << m_timer.getElapsedMilliseconds() << " ms" << std::endl;
    }

    ManualTimer::ManualTimer() {
        m_startTime = Clock::now();
        m_lastLapTime = m_startTime;
    }

    void ManualTimer::startLap() {
        auto now = Clock::now();
        auto lapDuration = std::chrono::duration<double>(now - m_lastLapTime).count();
        m_laps.push_back(lapDuration);
        m_lastLapTime = now;
    }

    double ManualTimer::getLapSeconds() {
        auto now = Clock::now();
        double lap = std::chrono::duration<double>(now - m_lastLapTime).count();
        m_lastLapTime = now;
        m_laps.push_back(lap);
        return lap;
    }

    double ManualTimer::getLapMilliseconds() {
        return getLapSeconds() * 1000.0;
    }

    double ManualTimer::getTotalSeconds() const {
        return std::chrono::duration<double>(Clock::now() - m_startTime).count();
    }

    double ManualTimer::getTotalMilliseconds() const {
        return getTotalSeconds() * 1000.0;
    }

    Alarm::Alarm() = default;

    Alarm::Alarm(double durationSeconds, Callback callback, bool repeat)
        : m_duration(durationSeconds), m_remaining(durationSeconds),
          m_repeat(repeat), m_callback(std::move(callback))
    {
        m_running = true;
    }

    void Alarm::start(double durationSeconds, Callback callback, bool repeat) {
        m_duration = durationSeconds;
        m_remaining = durationSeconds;
        m_repeat = repeat;
        m_callback = std::move(callback);
        m_running = true;
        m_paused = false;
        m_elapsed = false;
    }

    void Alarm::stop() {
        m_running = false;
        m_paused = false;
        m_elapsed = false;
    }

    void Alarm::pause() {
        m_paused = true;
    }

    void Alarm::resume() {
        m_paused = false;
    }

    void Alarm::reset() {
        m_remaining = m_duration;
        m_elapsed = false;
        m_running = true;
        m_paused = false;
    }

    void Alarm::update(double deltaSeconds) {
        if (!m_running || m_paused || m_elapsed) return;

        m_remaining -= deltaSeconds;
        if (m_remaining <= 0.0) {
            m_elapsed = true;
            m_running = m_repeat;
            if (m_repeat) {
                m_remaining = m_duration;
            }
            if (m_callback) {
                m_callback();
            }
        }
    }

    Clock::Clock() {
        m_lastTime = ClockType::now();
    }

    void Clock::update() {
        auto now = ClockType::now();
        m_rawDeltaTime = std::chrono::duration<double>(now - m_lastTime).count();
        m_lastTime = now;

        if (m_rawDeltaTime > m_maxDeltaTime) {
            m_rawDeltaTime = m_maxDeltaTime;
        }

        m_deltaTime = m_rawDeltaTime * m_timeScale;
        m_totalTime += m_deltaTime;
        ++m_frameCount;

        m_fpsAccumulator += m_rawDeltaTime;
        ++m_fpsFrameCount;
        if (m_fpsAccumulator >= 1.0) {
            m_fps = m_fpsFrameCount / m_fpsAccumulator;
            m_fpsAccumulator = 0.0;
            m_fpsFrameCount = 0;
        }
    }

}
