#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>

namespace nebula {

    enum class LogLevel {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    class Logger {
    public:
        static void init(const std::string& logFile = "nebula.log");
        static void shutdown();
        static void setLevel(LogLevel level);
        static LogLevel getLevel() { return s_level; }

        static void trace(const std::string& message);
        static void debug(const std::string& message);
        static void info(const std::string& message);
        static void warn(const std::string& message);
        static void error(const std::string& message);
        static void fatal(const std::string& message);

        static void setFlushOnWrite(bool flush) { s_flushOnWrite = flush; }
        static void addSink(std::function<void(LogLevel, const std::string&)> sink);

    private:
        static void log(LogLevel level, const std::string& message);
        static std::string getTimestamp();
        static std::string levelToString(LogLevel level);
        static const char* levelToAnsiColor(LogLevel level);

        static LogLevel s_level;
        static std::ofstream s_file;
        static std::mutex s_mutex;
        static bool s_initialized;
        static bool s_flushOnWrite;
        static std::vector<std::function<void(LogLevel, const std::string&)>> s_sinks;
    };

}

#define NEBULA_TRACE(...) nebula::Logger::trace(__VA_ARGS__)
#define NEBULA_DEBUG(...) nebula::Logger::debug(__VA_ARGS__)
#define NEBULA_INFO(...)  nebula::Logger::info(__VA_ARGS__)
#define NEBULA_WARN(...)  nebula::Logger::warn(__VA_ARGS__)
#define NEBULA_ERROR(...) nebula::Logger::error(__VA_ARGS__)
#define NEBULA_FATAL(...) nebula::Logger::fatal(__VA_ARGS__)
