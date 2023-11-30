#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <mutex>

namespace nebula {

struct CrashReport {
    std::string timestamp;
    std::string exceptionCode;
    std::string exceptionAddress;
    std::string exceptionDescription;

    std::vector<std::string> stackTrace;

    std::string engineVersion;
    float fps;
    std::string currentScene;
    std::vector<std::string> lastLogMessages;

    std::string systemInfo;
    std::string gpuInfo;
    uint64_t memoryUsageMB;
    uint64_t availableMemoryMB;

    bool minidumpCreated;
    std::string minidumpPath;

    std::string toFormattedString() const;
};

class CrashHandler {
public:
    static CrashHandler& instance();

    void install();
    void uninstall();
    bool isInstalled() const { return m_installed; }

    void setCrashCallback(std::function<void(const CrashReport&)> callback);
    void setDumpPath(const std::string& path);
    std::string getDumpPath() const;

    void setEngineVersion(const std::string& version) { m_engineVersion = version; }
    void setCurrentScene(const std::string& scene) { m_currentScene = scene; }
    void setCurrentFPS(float fps) { m_currentFps = fps; }

    void addLogMessage(const std::string& message);
    std::vector<std::string> getRecentLogMessages(std::size_t count = 50) const;

    void generateCrashReport();
    void writeMinidump();
    std::string getStackTrace(std::size_t skip = 0) const;

    static std::string getExceptionCodeString(uint32_t code);

private:
    CrashHandler();
    ~CrashHandler();
    CrashHandler(const CrashHandler&) = delete;
    CrashHandler& operator=(const CrashHandler&) = delete;

    void handleSignal(int signal);
    void writeCrashLog(const CrashReport& report);

    static void signalHandler(int signal);
    static long __stdcall sehHandler(unsigned int exceptionCode, void* exceptionInfo);

    bool m_installed;
    std::string m_dumpPath;
    std::string m_engineVersion;
    std::string m_currentScene;
    float m_currentFps;

    std::vector<std::string> m_logMessages;
    mutable std::mutex m_logMutex;

    std::function<void(const CrashReport&)> m_crashCallback;

    static CrashHandler* s_instance;
    static constexpr std::size_t MAX_LOG_MESSAGES = 256;
};

}

