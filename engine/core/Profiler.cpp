#include "Profiler.h"
#include "Logger.h"
#include "Platform.h"
#include <thread>
#include <iostream>

namespace nebula {

    Profiler& Profiler::instance() {
        static Profiler inst;
        return inst;
    }

    void Profiler::beginSession(const std::string& name) {
        m_sessionName = name;
        m_results.clear();
        m_sessionActive = true;
        m_sessionStart = std::chrono::high_resolution_clock::now();
        m_frameCount = 0;

        NEBULA_INFO("Profiler session started: " + name);
    }

    void Profiler::endSession() {
        if (!m_sessionActive) return;

        m_sessionActive = false;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(end - m_sessionStart);

        NEBULA_INFO("Profiler session ended: " + m_sessionName +
                    " (duration: " + std::to_string(duration.count()) + "s, frames: " +
                    std::to_string(m_frameCount) + ")");

        dumpResults();
    }

    void Profiler::beginFrame() {
        if (!m_sessionActive) return;
        m_frameStart = std::chrono::high_resolution_clock::now();
    }

    void Profiler::endFrame() {
        if (!m_sessionActive) return;
        m_frameCount++;
    }

    void Profiler::addResult(const ProfileResult& result) {
        if (!m_sessionActive) return;
        m_results.push_back(result);
    }

    void Profiler::dumpResults() {
        if (m_results.empty()) {
            NEBULA_INFO("No profiling results to display");
            return;
        }

        std::unordered_map<std::string, std::pair<float, int>> aggregates;

        for (const auto& result : m_results) {
            auto& agg = aggregates[result.name];
            agg.first += result.time;
            agg.second++;
        }

        std::vector<std::pair<std::string, std::pair<float, int>>> sorted(
            aggregates.begin(), aggregates.end());

        std::sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) {
                return a.second.first > b.second.first;
            });

        NEBULA_INFO("=== Profiling Results ===");
        NEBULA_INFO("Total samples: " + std::to_string(m_results.size()));
        NEBULA_INFO("Unique blocks: " + std::to_string(sorted.size()));
        NEBULA_INFO("");

        for (const auto& [name, data] : sorted) {
            float totalMs = data.first * 1000.0f;
            float avgMs = (totalMs / data.second);
            float avgPerFrame = data.first / static_cast<float>(m_frameCount) * 1000.0f;

            NEBULA_INFO("  " + name +
                " | total: " + std::to_string(totalMs) + "ms" +
                " | avg: " + std::to_string(avgMs) + "ms" +
                " | calls: " + std::to_string(data.second) +
                " | per-frame: " + std::to_string(avgPerFrame) + "ms");
        }

        NEBULA_INFO("=== End Profiling ===");
    }

    void Profiler::dumpResultsToFile(const std::string& path) {
        std::ofstream file(path);
        if (!file.is_open()) {
            NEBULA_ERROR("Failed to open profiling output file: " + path);
            return;
        }

        file << "name,time_ms,thread_id" << std::endl;
        for (const auto& result : m_results) {
            file << result.name << ","
                 << std::fixed << std::setprecision(6) << (result.time * 1000.0f) << ","
                 << result.threadId << std::endl;
        }

        NEBULA_INFO("Profiling results exported to: " + path);
    }

    void Profiler::clearResults() {
        m_results.clear();
    }

    ScopedTimer::ScopedTimer(const std::string& name) : m_name(name) {
        m_start = std::chrono::high_resolution_clock::now();
#ifdef _MSC_VER
        m_threadId = static_cast<unsigned int>(GetCurrentThreadId());
#endif
    }

    ScopedTimer::~ScopedTimer() {
        if (!m_stopped) {
            stop();
        }
    }

    void ScopedTimer::stop() {
        if (m_stopped) return;
        m_stopped = true;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<float>>(end - m_start);

        ProfileResult result;
        result.name = m_name;
        result.time = duration.count();
#ifdef _MSC_VER
        result.threadId = m_threadId;
#else
        result.threadId = 0;
#endif

        Profiler::instance().addResult(result);
    }

}
