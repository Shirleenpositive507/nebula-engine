#include "Logger.h"
#include "Platform.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>

namespace nebula {

    LogLevel Logger::s_level = LogLevel::TRACE;
    std::ofstream Logger::s_file;
    std::mutex Logger::s_mutex;
    bool Logger::s_initialized = false;
    bool Logger::s_flushOnWrite = true;
    std::vector<std::function<void(LogLevel, const std::string&)>> Logger::s_sinks;

    void Logger::init(const std::string& logFile) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_initialized) return;

        s_file.open(logFile, std::ios::out | std::ios::trunc);
        if (!s_file.is_open()) {
            std::cerr << "[Logger] Failed to open log file: " << logFile << std::endl;
        }

        s_initialized = true;

        NEBULA_INFO("Logger initialized, writing to: " + logFile);
        NEBULA_INFO("Platform: " + std::string(PlatformInfo::getName()));
        NEBULA_INFO("Cores: " + std::to_string(PlatformInfo::getCoreCount()));
    }

    void Logger::shutdown() {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (!s_initialized) return;

        NEBULA_INFO("Logger shutting down");

        if (s_file.is_open()) {
            s_file.flush();
            s_file.close();
        }

        s_initialized = false;
    }

    void Logger::setLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_level = level;
    }

    void Logger::addSink(std::function<void(LogLevel, const std::string&)> sink) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_sinks.push_back(std::move(sink));
    }

    void Logger::trace(const std::string& message) { log(LogLevel::TRACE, message); }
    void Logger::debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void Logger::info(const std::string& message)  { log(LogLevel::INFO, message); }
    void Logger::warn(const std::string& message)  { log(LogLevel::WARN, message); }
    void Logger::error(const std::string& message) { log(LogLevel::ERROR, message); }
    void Logger::fatal(const std::string& message) { log(LogLevel::FATAL, message); }

    void Logger::log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(s_mutex);

        if (static_cast<int>(level) < static_cast<int>(s_level)) return;

        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);
        std::string formatted = "[" + timestamp + "][" + levelStr + "] " + message;

        if (s_file.is_open()) {
            s_file << formatted << std::endl;
            if (s_flushOnWrite) s_file.flush();
        }

#ifdef NEBULA_PLATFORM_WINDOWS
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console != INVALID_HANDLE_VALUE) {
            WORD attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            switch (level) {
                case LogLevel::TRACE: attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
                case LogLevel::DEBUG: attributes = FOREGROUND_GREEN | FOREGROUND_BLUE; break;
                case LogLevel::INFO:  attributes = FOREGROUND_GREEN; break;
                case LogLevel::WARN:  attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
                case LogLevel::ERROR: attributes = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
                case LogLevel::FATAL: attributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
            }
            SetConsoleTextAttribute(console, attributes);
            std::cout << formatted << std::endl;
            SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        } else {
            std::cout << formatted << std::endl;
        }
#else
        std::cout << levelToAnsiColor(level) << formatted << "\033[0m" << std::endl;
#endif

        for (auto& sink : s_sinks) {
            sink(level, formatted);
        }
    }

    std::string Logger::getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch() % 1000).count();

        std::tm tm;
#ifdef NEBULA_PLATFORM_WINDOWS
        localtime_s(&tm, &timeT);
#else
        localtime_r(&timeT, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms;
        return oss.str();
    }

    std::string Logger::levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default:              return "?????";
        }
    }

    const char* Logger::levelToAnsiColor(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "\033[37m";
            case LogLevel::DEBUG: return "\033[36m";
            case LogLevel::INFO:  return "\033[32m";
            case LogLevel::WARN:  return "\033[33m";
            case LogLevel::ERROR: return "\033[31m";
            case LogLevel::FATAL: return "\033[35m";
            default:              return "\033[0m";
        }
    }

}
