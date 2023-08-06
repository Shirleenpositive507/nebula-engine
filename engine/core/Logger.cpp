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
    std::vector<LogSink> Logger::s_namedSinks;
    std::unordered_map<std::string, LogLevel> Logger::s_categoryLevels;
    LogFormatter Logger::s_formatter;
    std::thread Logger::s_asyncThread;
    std::queue<std::string> Logger::s_asyncQueue;
    std::mutex Logger::s_asyncMutex;
    std::condition_variable Logger::s_asyncCv;
    std::atomic<bool> Logger::s_asyncRunning(false);

    void Logger::init(const std::string& logFile) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_initialized) return;

        s_file.open(logFile, std::ios::out | std::ios::trunc);
        if (!s_file.is_open()) {
            std::cerr << "[Logger] Failed to open log file: " << logFile << std::endl;
        }

        s_initialized = true;

        s_asyncRunning = true;
        s_asyncThread = std::thread(asyncWorker);

        NEBULA_INFO("Logger initialized, writing to: " + logFile);
        NEBULA_INFO("Platform: " + std::string(PlatformInfo::getName()));
        NEBULA_INFO("Cores: " + std::to_string(PlatformInfo::getCoreCount()));
    }

    void Logger::shutdown() {
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_initialized) return;
        }

        {
            std::lock_guard<std::mutex> lock(s_asyncMutex);
            s_asyncRunning = false;
        }
        s_asyncCv.notify_one();
        if (s_asyncThread.joinable()) {
            s_asyncThread.join();
        }

        std::lock_guard<std::mutex> lock(s_mutex);
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

    int Logger::addNamedSink(const LogSink& sink) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_namedSinks.push_back(sink);
        return static_cast<int>(s_namedSinks.size()) - 1;
    }

    void Logger::removeSink(int sinkId) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (sinkId >= 0 && sinkId < static_cast<int>(s_namedSinks.size())) {
            s_namedSinks.erase(s_namedSinks.begin() + sinkId);
        }
    }

    void Logger::setSinkLevel(int sinkId, LogLevel level) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (sinkId >= 0 && sinkId < static_cast<int>(s_namedSinks.size())) {
            s_namedSinks[sinkId].levelFilter = level;
        }
    }

    void Logger::setSinkEnabled(int sinkId, bool enabled) {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (sinkId >= 0 && sinkId < static_cast<int>(s_namedSinks.size())) {
            s_namedSinks[sinkId].enabled = enabled;
        }
    }

    void Logger::setFormatter(const LogFormatter& formatter) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_formatter = formatter;
    }

    void Logger::setCategoryLevel(const std::string& category, LogLevel level) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_categoryLevels[category] = level;
    }

    LogLevel Logger::getCategoryLevel(const std::string& category) {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_categoryLevels.find(category);
        if (it != s_categoryLevels.end()) return it->second;
        return s_level;
    }

    void Logger::trace(const std::string& message, const std::string& category) { log(LogLevel::TRACE, message, category); }
    void Logger::debug(const std::string& message, const std::string& category) { log(LogLevel::DEBUG, message, category); }
    void Logger::info(const std::string& message, const std::string& category)  { log(LogLevel::INFO, message, category); }
    void Logger::warn(const std::string& message, const std::string& category)  { log(LogLevel::WARN, message, category); }
    void Logger::error(const std::string& message, const std::string& category) { log(LogLevel::ERROR, message, category); }
    void Logger::fatal(const std::string& message, const std::string& category) { log(LogLevel::FATAL, message, category); }

    void Logger::log(LogLevel level, const std::string& message, const std::string& category) {
        std::lock_guard<std::mutex> lock(s_mutex);

        if (static_cast<int>(level) < static_cast<int>(s_level)) return;

        if (!category.empty()) {
            auto it = s_categoryLevels.find(category);
            if (it != s_categoryLevels.end() && static_cast<int>(level) < static_cast<int>(it->second)) return;
        }

        std::string formatted = formatMessage(level, message, category);

        if (s_file.is_open()) {
            s_file << formatted << std::endl;
            if (s_flushOnWrite) s_file.flush();
        }

#ifdef NEBULA_PLATFORM_WINDOWS
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console != INVALID_HANDLE_VALUE && s_formatter.useColor) {
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
        if (s_formatter.useColor) {
            std::cout << levelToAnsiColor(level) << formatted << "\033[0m" << std::endl;
        } else {
            std::cout << formatted << std::endl;
        }
#endif

        for (auto& sink : s_sinks) {
            sink(level, formatted);
        }

        for (auto& namedSink : s_namedSinks) {
            if (!namedSink.enabled) continue;
            if (static_cast<int>(level) < static_cast<int>(namedSink.levelFilter)) continue;

            if (namedSink.writeCallback) {
                namedSink.writeCallback(formatted);
            }
        }

        {
            std::lock_guard<std::mutex> asyncLock(s_asyncMutex);
            s_asyncQueue.push(formatted);
        }
        s_asyncCv.notify_one();
    }

    std::string Logger::formatMessage(LogLevel level, const std::string& message, const std::string& category) {
        std::string result;

        if (s_formatter.showTimestamp) {
            result += "[" + getTimestamp() + "]";
        }
        if (s_formatter.showLevel) {
            result += "[" + levelToString(level) + "]";
        }
        if (s_formatter.showCategory && !category.empty()) {
            result += "[" + category + "]";
        }
        if (s_formatter.showThreadId) {
            std::ostringstream tid;
            tid << std::this_thread::get_id();
            result += "[" + tid.str() + "]";
        }
        if (!result.empty()) result += " ";
        result += message;

        return result;
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

    void Logger::asyncWorker() {
        while (s_asyncRunning) {
            std::unique_lock<std::mutex> lock(s_asyncMutex);
            s_asyncCv.wait_for(lock, std::chrono::milliseconds(100), [] { return !s_asyncQueue.empty() || !s_asyncRunning; });

            while (!s_asyncQueue.empty()) {
                std::string msg = s_asyncQueue.front();
                s_asyncQueue.pop();
                lock.unlock();
                {
                    std::lock_guard<std::mutex> fileLock(s_mutex);
                    if (s_file.is_open()) {
                        s_file << msg << std::endl;
                        s_file.flush();
                    }
                }
                lock.lock();
            }
        }
    }

}

