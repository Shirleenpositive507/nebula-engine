#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace nebula {

struct ParsedCommandLine {
    std::string configPath;
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool vsync = true;
    std::string scene;
    bool editor = false;
    int logLevel = 2;
    int port = 0;

    bool hasConfig = false;
    bool hasWidth = false;
    bool hasHeight = false;
    bool hasFullscreen = false;
    bool hasVsync = false;
    bool hasScene = false;
    bool hasEditor = false;
    bool hasLogLevel = false;
    bool hasPort = false;
};

class CommandLine {
public:
    static CommandLine& instance();

    void parse(int argc, char* argv[]);
    void parse(const std::vector<std::string>& args);

    const ParsedCommandLine& getParsed() const { return m_parsed; }

    bool has(const std::string& key) const;
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getFlag(const std::string& key) const;

private:
    CommandLine() = default;

    ParsedCommandLine m_parsed;
    std::unordered_map<std::string, std::string> m_rawArgs;
};

}

