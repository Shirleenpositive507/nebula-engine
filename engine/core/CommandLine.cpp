#include "CommandLine.h"
#include <algorithm>

namespace nebula {

CommandLine& CommandLine::instance() {
    static CommandLine cmd;
    return cmd;
}

void CommandLine::parse(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    parse(args);
}

void CommandLine::parse(const std::vector<std::string>& args) {
    m_rawArgs.clear();
    m_parsed = ParsedCommandLine();

    for (const auto& arg : args) {
        if (arg.size() < 2 || arg[0] != '-') continue;

        size_t eqPos = arg.find('=');
        std::string key, value;

        if (eqPos != std::string::npos) {
            key = arg.substr(0, eqPos);
            value = arg.substr(eqPos + 1);
        } else {
            key = arg;
            value = "true";
        }

        while (key.size() > 1 && key[0] == '-') {
            key = key.substr(1);
        }

        m_rawArgs[key] = value;

        if (key == "config" || key == "config-path") {
            m_parsed.configPath = value;
            m_parsed.hasConfig = true;
        } else if (key == "width") {
            m_parsed.width = std::stoi(value);
            m_parsed.hasWidth = true;
        } else if (key == "height") {
            m_parsed.height = std::stoi(value);
            m_parsed.hasHeight = true;
        } else if (key == "fullscreen") {
            m_parsed.fullscreen = (value == "true" || value == "1");
            m_parsed.hasFullscreen = true;
        } else if (key == "vsync") {
            m_parsed.vsync = (value == "true" || value == "1");
            m_parsed.hasVsync = true;
        } else if (key == "scene") {
            m_parsed.scene = value;
            m_parsed.hasScene = true;
        } else if (key == "editor") {
            m_parsed.editor = (value == "true" || value == "1");
            m_parsed.hasEditor = true;
        } else if (key == "log-level") {
            m_parsed.logLevel = std::stoi(value);
            m_parsed.hasLogLevel = true;
        } else if (key == "port") {
            m_parsed.port = std::stoi(value);
            m_parsed.hasPort = true;
        }
    }
}

bool CommandLine::has(const std::string& key) const {
    return m_rawArgs.find(key) != m_rawArgs.end();
}

std::string CommandLine::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_rawArgs.find(key);
    if (it != m_rawArgs.end()) {
        return it->second;
    }
    return defaultValue;
}

int CommandLine::getInt(const std::string& key, int defaultValue) const {
    auto it = m_rawArgs.find(key);
    if (it != m_rawArgs.end()) {
        return std::stoi(it->second);
    }
    return defaultValue;
}

bool CommandLine::getFlag(const std::string& key) const {
    auto it = m_rawArgs.find(key);
    if (it != m_rawArgs.end()) {
        return it->second == "true";
    }
    return false;
}

}

