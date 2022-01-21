#include "Config.h"
#include "Logger.h"
#include <iostream>
#include <regex>

namespace nebula {

    Config& Config::instance() {
        static Config inst;
        return inst;
    }

    bool Config::load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            NEBULA_WARN("Config file not found: " + path + ", using defaults");
            return false;
        }

        m_values.clear();
        std::string line;
        std::string currentSection;
        int lineNum = 0;

        while (std::getline(file, line)) {
            lineNum++;
            std::string trimmed = trim(line);

            if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') continue;

            if (trimmed[0] == '[') {
                size_t end = trimmed.find(']');
                if (end != std::string::npos) {
                    currentSection = trim(trimmed.substr(1, end - 1));
                }
                continue;
            }

            size_t eqPos = trimmed.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key = trim(trimmed.substr(0, eqPos));
            std::string value = trim(trimmed.substr(eqPos + 1));

            if (key.empty()) continue;

            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            std::string fullKey = currentSection.empty() ? key : currentSection + "." + key;
            m_values[fullKey] = value;
        }

        NEBULA_INFO("Loaded " + std::to_string(m_values.size()) + " config keys from " + path);
        return true;
    }

    bool Config::save(const std::string& path) {
        std::ofstream file(path);
        if (!file.is_open()) {
            NEBULA_ERROR("Failed to open config file for writing: " + path);
            return false;
        }

        std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> sections;

        for (const auto& [key, value] : m_values) {
            size_t dotPos = key.find('.');
            if (dotPos != std::string::npos) {
                std::string section = key.substr(0, dotPos);
                std::string subKey = key.substr(dotPos + 1);
                sections[section].emplace_back(subKey, value);
            } else {
                sections[""].emplace_back(key, value);
            }
        }

        for (const auto& [section, keys] : sections) {
            if (!section.empty()) {
                file << "[" << section << "]" << std::endl;
            }
            for (const auto& [key, value] : keys) {
                bool needsQuotes = value.find(' ') != std::string::npos || value.find('=') != std::string::npos;
                if (needsQuotes) {
                    file << key << " = \"" << value << "\"" << std::endl;
                } else {
                    file << key << " = " << value << std::endl;
                }
            }
            file << std::endl;
        }

        NEBULA_INFO("Saved " + std::to_string(m_values.size()) + " config keys to " + path);
        return true;
    }

    bool Config::hasKey(const std::string& key) const {
        return m_values.find(key) != m_values.end();
    }

    void Config::removeKey(const std::string& key) {
        m_values.erase(key);
    }

    void Config::clear() {
        m_values.clear();
    }

    std::vector<std::string> Config::getKeys() const {
        std::vector<std::string> keys;
        keys.reserve(m_values.size());
        for (const auto& [key, _] : m_values) {
            keys.push_back(key);
        }
        return keys;
    }

    std::vector<std::string> Config::getKeysInSection(const std::string& section) const {
        std::vector<std::string> keys;
        std::string prefix = section + ".";
        for (const auto& [key, _] : m_values) {
            if (key.find(prefix) == 0) {
                keys.push_back(key.substr(prefix.size()));
            }
        }
        return keys;
    }

    void Config::setString(const std::string& key, const std::string& value) {
        m_values[key] = value;
    }

    std::string Config::getString(const std::string& key, const std::string& defaultValue) const {
        auto it = m_values.find(key);
        return it != m_values.end() ? it->second : defaultValue;
    }

    int Config::getInt(const std::string& key, int defaultValue) const {
        return get<int>(key, defaultValue);
    }

    float Config::getFloat(const std::string& key, float defaultValue) const {
        return get<float>(key, defaultValue);
    }

    bool Config::getBool(const std::string& key, bool defaultValue) const {
        return get<bool>(key, defaultValue);
    }

    std::string Config::trim(const std::string& str) const {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }

}
