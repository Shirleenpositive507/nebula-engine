#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace nebula {

    struct ProfileResult {
        std::string name;
        float time;
        unsigned int threadId;
    };

    struct ProfileSession {
        std::string name;
        std::vector<ProfileResult> results;
    };

    class Profiler {
    public:
        static Profiler& instance();

        void beginSession(const std::string& name);
        void endSession();
        void beginFrame();
        void endFrame();

        void addResult(const ProfileResult& result);
        void dumpResults();
        void dumpResultsToFile(const std::string& path);

        const std::vector<ProfileResult>& getSessionResults() const { return m_results; }
        void clearResults();

        bool isSessionActive() const { return m_sessionActive; }
        unsigned int getFrameCount() const { return m_frameCount; }

    private:
        Profiler() = default;
        Profiler(const Profiler&) = delete;
        Profiler& operator=(const Profiler&) = delete;

        std::string m_sessionName;
        std::vector<ProfileResult> m_results;
        std::chrono::high_resolution_clock::time_point m_frameStart;
        std::chrono::high_resolution_clock::time_point m_sessionStart;
        bool m_sessionActive = false;
        unsigned int m_frameCount = 0;
    };

    class ScopedTimer {
    public:
        ScopedTimer(const std::string& name);
        ~ScopedTimer();

        void stop();

    private:
        std::string m_name;
        std::chrono::high_resolution_clock::time_point m_start;
        bool m_stopped = false;

#ifdef _MSC_VER
        unsigned int m_threadId;
#endif
    };

}

#define NEBULA_PROFILE_SCOPE(name) nebula::ScopedTimer timer##__LINE__(name)
#define NEBULA_PROFILE_FUNCTION() NEBULA_PROFILE_SCOPE(__FUNCTION__)
