#include "CrashHandler.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace nebula {

CrashHandler* CrashHandler::s_instance = nullptr;

CrashHandler& CrashHandler::instance() {
    static CrashHandler handler;
    return handler;
}

CrashHandler::CrashHandler()
    : m_installed(false)
    , m_dumpPath("crashes")
    , m_currentFps(0.0f)
{
    s_instance = this;
}

CrashHandler::~CrashHandler() {
    if (m_installed) uninstall();
    s_instance = nullptr;
}

void CrashHandler::install() {
    if (m_installed) return;

    std::signal(SIGSEGV, CrashHandler::signalHandler);
    std::signal(SIGABRT, CrashHandler::signalHandler);
    std::signal(SIGFPE, CrashHandler::signalHandler);
    std::signal(SIGILL, CrashHandler::signalHandler);
    std::signal(SIGTERM, CrashHandler::signalHandler);

#ifdef _WIN32
    SetUnhandledExceptionFilter(sehHandler);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

    m_installed = true;
}

void CrashHandler::uninstall() {
    if (!m_installed) return;

    std::signal(SIGSEGV, SIG_DFL);
    std::signal(SIGABRT, SIG_DFL);
    std::signal(SIGFPE, SIG_DFL);
    std::signal(SIGILL, SIG_DFL);
    std::signal(SIGTERM, SIG_DFL);

#ifdef _WIN32
    SetUnhandledExceptionFilter(nullptr);
#endif

    m_installed = false;
}

void CrashHandler::setCrashCallback(std::function<void(const CrashReport&)> callback) {
    m_crashCallback = std::move(callback);
}

void CrashHandler::setDumpPath(const std::string& path) {
    m_dumpPath = path;
}

std::string CrashHandler::getDumpPath() const {
    return m_dumpPath;
}

void CrashHandler::addLogMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logMessages.push_back(message);
    if (m_logMessages.size() > MAX_LOG_MESSAGES) {
        m_logMessages.erase(m_logMessages.begin());
    }
}

std::vector<std::string> CrashHandler::getRecentLogMessages(std::size_t count) const {
    std::lock_guard<std::mutex> lock(m_logMutex);
    if (m_logMessages.size() <= count) return m_logMessages;
    return std::vector<std::string>(m_logMessages.end() - static_cast<ptrdiff_t>(count), m_logMessages.end());
}

void CrashHandler::signalHandler(int signal) {
    if (s_instance) {
        s_instance->handleSignal(signal);
    }
}

long __stdcall CrashHandler::sehHandler(unsigned int exceptionCode, void* exceptionInfo) {
    (void)exceptionInfo;
    if (s_instance) {
        CrashReport report;
        report.exceptionCode = std::to_string(exceptionCode);
        report.exceptionDescription = getExceptionCodeString(exceptionCode);
        report.timestamp = "NOW";
        report.engineVersion = s_instance->m_engineVersion;
        report.currentScene = s_instance->m_currentScene;
        report.fps = s_instance->m_currentFps;
        report.lastLogMessages = s_instance->getRecentLogMessages();

        s_instance->writeCrashLog(report);
        s_instance->writeMinidump();

        if (s_instance->m_crashCallback) {
            s_instance->m_crashCallback(report);
        }
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashHandler::handleSignal(int signal) {
    CrashReport report;
    report.timestamp = "NOW";

    switch (signal) {
        case SIGSEGV: report.exceptionCode = "SIGSEGV"; report.exceptionDescription = "Segmentation fault (invalid memory access)"; break;
        case SIGABRT: report.exceptionCode = "SIGABRT"; report.exceptionDescription = "Abnormal termination (abort)"; break;
        case SIGFPE:  report.exceptionCode = "SIGFPE";  report.exceptionDescription = "Floating-point exception (divide by zero, etc.)"; break;
        case SIGILL:  report.exceptionCode = "SIGILL";  report.exceptionDescription = "Illegal instruction"; break;
        case SIGTERM: report.exceptionCode = "SIGTERM"; report.exceptionDescription = "Termination request"; break;
        default:      report.exceptionCode = "UNKNOWN"; report.exceptionDescription = "Unknown signal"; break;
    }

    report.engineVersion = m_engineVersion;
    report.currentScene = m_currentScene;
    report.fps = m_currentFps;
    report.stackTrace = { "Stack trace unavailable" };
    report.lastLogMessages = getRecentLogMessages();

#ifdef _WIN32
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    report.memoryUsageMB = memStatus.dwTotalPhys / (1024 * 1024) - memStatus.dwAvailPhys / (1024 * 1024);
    report.availableMemoryMB = memStatus.dwAvailPhys / (1024 * 1024);
#endif

    writeCrashLog(report);
    writeMinidump();

    if (m_crashCallback) {
        m_crashCallback(report);
    }

    std::_Exit(EXIT_FAILURE);
}

void CrashHandler::generateCrashReport() {
    handleSignal(SIGABRT);
}

void CrashHandler::writeMinidump() {
#ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    DWORD pid = GetCurrentProcessId();

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << m_dumpPath << "/crash_" << pid << "_" << time_t << ".dmp";

    std::string dumpFilePath = ss.str();

    HANDLE file = CreateFileA(
        dumpFilePath.c_str(),
        GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );

    if (file != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = nullptr;
        mei.ClientPointers = FALSE;

        MiniDumpWriteDump(
            process, pid, file,
            MiniDumpNormal,
            nullptr, nullptr, nullptr
        );
        CloseHandle(file);
    }
#else
    std::ofstream dump(m_dumpPath + "/crash.dump", std::ios::binary);
    if (dump) {
        dump << "Crash dump (simulated)" << std::endl;
    }
#endif
}

std::string CrashHandler::getStackTrace(std::size_t skip) const {
    (void)skip;
    return "Stack trace not available";
}

std::string CrashHandler::getExceptionCodeString(uint32_t code) {
    switch (code) {
        case 0xC0000005: return "Access violation (read/write)";
        case 0xC0000094: return "Integer divide by zero";
        case 0xC000008C: return "Array bounds exceeded";
        case 0xC0000008: return "Invalid handle";
        case 0xC0000022: return "Access denied";
        case 0xC000001D: return "Illegal instruction";
        case 0xC0000025: return "Stack overflow";
        default:         return "Unknown exception";
    }
}

void CrashHandler::writeCrashLog(const CrashReport& report) {
    std::string logContent = report.toFormattedString();

    std::string logPath = m_dumpPath + "/crash.log";
    std::ofstream logFile(logPath, std::ios::app);
    if (logFile) {
        logFile << logContent << std::endl;
        logFile << "---" << std::endl;
    }

    std::cerr << "CRASH: " << report.exceptionCode << " - " << report.exceptionDescription << std::endl;
    std::cerr << "Engine: " << report.engineVersion << " | Scene: " << report.currentScene << std::endl;
    std::cerr << "Crash log written to: " << logPath << std::endl;
}

std::string CrashReport::toFormattedString() const {
    std::stringstream ss;
    ss << "=== CRASH REPORT ===" << std::endl;
    ss << "Timestamp: " << timestamp << std::endl;
    ss << "Exception: " << exceptionCode << " (" << exceptionDescription << ")" << std::endl;
    ss << "Address: " << exceptionAddress << std::endl;
    ss << std::endl;
    ss << "--- Engine State ---" << std::endl;
    ss << "Version: " << engineVersion << std::endl;
    ss << "FPS: " << fps << std::endl;
    ss << "Scene: " << currentScene << std::endl;
    ss << std::endl;
    ss << "--- System ---" << std::endl;
    ss << "Memory Usage: " << memoryUsageMB << " MB" << std::endl;
    ss << "Available Memory: " << availableMemoryMB << " MB" << std::endl;
    ss << std::endl;
    ss << "--- Stack Trace ---" << std::endl;
    for (const auto& frame : stackTrace) {
        ss << frame << std::endl;
    }
    ss << std::endl;
    ss << "--- Recent Log Messages ---" << std::endl;
    for (const auto& msg : lastLogMessages) {
        ss << msg << std::endl;
    }
    ss << std::endl;
    ss << "Minidump: " << (minidumpCreated ? minidumpPath : "Not created") << std::endl;
    ss << "=========================" << std::endl;
    return ss.str();
}

}

