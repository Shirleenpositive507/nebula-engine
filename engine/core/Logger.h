#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <unordered_map>

namespace nebula {

    enum class LogLevel {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    enum class SinkType {
        Console,
        File,
        Network,
        Callback
    };

    struct LogSink {
        SinkType type;
        std::string name;
        LogLevel levelFilter = LogLevel::TRACE;
        bool enabled = true;

        std::function<void(const std::string&)> writeCallback;
        std::string filePath;
        std::string networkHost;
        int networkPort = 0;
    };

    struct LogFormatter {
        bool showTimestamp = true;
        bool showLevel = true;
        bool showCategory = true;
        bool showThreadId = false;
        bool useColor = true;
        std::string timestampFormat = "%Y-%m-%d %H:%M:%S";
    };

    class Logger {
    public:
        static void init(const std::string& logFile = "nebula.log");
        static void shutdown();
        static void setLevel(LogLevel level);
        static LogLevel getLevel() { return s_level; }

        static void trace(const std::string& message, const std::string& category = "");
        static void debug(const std::string& message, const std::string& category = "");
        static void info(const std::string& message, const std::string& category = "");
        static void warn(const std::string& message, const std::string& category = "");
        static void error(const std::string& message, const std::string& category = "");
        static void fatal(const std::string& message, const std::string& category = "");

        static void setFlushOnWrite(bool flush) { s_flushOnWrite = flush; }
        static void addSink(std::function<void(LogLevel, const std::string&)> sink);
        static int addNamedSink(const LogSink& sink);
        static void removeSink(int sinkId);
        static void setSinkLevel(int sinkId, LogLevel level);
        static void setSinkEnabled(int sinkId, bool enabled);
        static void setFormatter(const LogFormatter& formatter);

        static void setCategoryLevel(const std::string& category, LogLevel level);
        static LogLevel getCategoryLevel(const std::string& category);

    private:
        static void log(LogLevel level, const std::string& message, const std::string& category);
        static std::string getTimestamp();
        static std::string levelToString(LogLevel level);
        static const char* levelToAnsiColor(LogLevel level);
        static std::string formatMessage(LogLevel level, const std::string& message, const std::string& category);

        static void asyncWorker();

        static LogLevel s_level;
        static std::ofstream s_file;
        static std::mutex s_mutex;
        static bool s_initialized;
        static bool s_flushOnWrite;
        static std::vector<std::function<void(LogLevel, const std::string&)>> s_sinks;
        static std::vector<LogSink> s_namedSinks;
        static std::unordered_map<std::string, LogLevel> s_categoryLevels;
        static LogFormatter s_formatter;

        static std::thread s_asyncThread;
        static std::queue<std::string> s_asyncQueue;
        static std::mutex s_asyncMutex;
        static std::condition_variable s_asyncCv;
        static std::atomic<bool> s_asyncRunning;
    };

}

#define NEBULA_TRACE(...) nebula::Logger::trace(__VA_ARGS__)
#define NEBULA_DEBUG(...) nebula::Logger::debug(__VA_ARGS__)
#define NEBULA_INFO(...)  nebula::Logger::info(__VA_ARGS__)
#define NEBULA_WARN(...)  nebula::Logger::warn(__VA_ARGS__)
#define NEBULA_ERROR(...) nebula::Logger::error(__VA_ARGS__)
#define NEBULA_FATAL(...) nebula::Logger::fatal(__VA_ARGS__)

