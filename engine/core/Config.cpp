#include "Config.h"
#include "Logger.h"
#include <iostream>
#include <regex>
#include <filesystem>
#include <sstream>
#include <iomanip>

namespace nebula {

    Config& Config::instance() {
        static Config inst;
        return inst;
    }

    bool Config::load(const std::string& path) {
        m_configPath = path;
        m_validationErrors.clear();
        m_lastError.clear();

        std::string content = loadFile(path);
        if (content.empty()) {
            NEBULA_WARN("Config file not found or empty: " + path + ", using defaults");
            return false;
        }

        m_values.clear();
        std::istringstream stream(content);
        std::string line;
        std::string currentSection;
        int lineNum = 0;

        while (std::getline(stream, line)) {
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

        try {
            m_lastWriteTime = std::filesystem::last_write_time(path);
        } catch (...) {
            m_lastWriteTime = 0;
        }

        NEBULA_INFO("Loaded " + std::to_string(m_values.size()) + " config keys from " + path);
        return true;
    }

    bool Config::save(const std::string& path) {
        std::ofstream file(path);
        if (!file.is_open()) {
            NEBULA_ERROR("Failed to open config file for writing: " + path);
            m_lastError = "Cannot open file for writing: " + path;
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

    bool Config::reload() {
        if (m_configPath.empty()) return false;
        return load(m_configPath);
    }

    void Config::setWatchChanges(bool watch) {
        m_watchChanges = watch;
    }

    void Config::setChangeCallback(std::function<void()> callback) {
        m_changeCallback = std::move(callback);
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

    std::vector<ConfigSection> Config::getSections() const {
        std::unordered_map<std::string, ConfigSection> sectionMap;
        for (const auto& [key, value] : m_values) {
            size_t dotPos = key.find('.');
            std::string sectionName = (dotPos != std::string::npos) ? key.substr(0, dotPos) : "";
            std::string subKey = (dotPos != std::string::npos) ? key.substr(dotPos + 1) : key;
            sectionMap[sectionName].name = sectionName;
            sectionMap[sectionName].values[subKey] = value;
        }
        std::vector<ConfigSection> result;
        result.reserve(sectionMap.size());
        for (auto& [name, section] : sectionMap) {
            result.push_back(std::move(section));
        }
        return result;
    }

    bool Config::hasSection(const std::string& section) const {
        std::string prefix = section + ".";
        for (const auto& [key, _] : m_values) {
            if (key.find(prefix) == 0) return true;
        }
        return m_values.find(section) != m_values.end();
    }

    void Config::removeSection(const std::string& section) {
        std::string prefix = section + ".";
        auto it = m_values.begin();
        while (it != m_values.end()) {
            if (it->first.find(prefix) == 0 || it->first == section) {
                it = m_values.erase(it);
            } else {
                ++it;
            }
        }
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

    bool Config::migrate(uint32_t fromVersion, uint32_t toVersion) {
        uint32_t current = fromVersion;
        while (current < toVersion) {
            auto it = m_migrations.find(current);
            if (it == m_migrations.end()) {
                NEBULA_ERROR("No migration path from version " + std::to_string(current));
                return false;
            }
            uint32_t nextVersion = it->second.first;
            auto& migrationFn = it->second.second;
            if (!migrationFn(*this)) {
                NEBULA_ERROR("Migration from version " + std::to_string(current) + " failed");
                return false;
            }
            current = nextVersion;
        }
        m_version = toVersion;
        NEBULA_INFO("Config migrated to version " + std::to_string(toVersion));
        return true;
    }

    void Config::registerMigration(uint32_t fromVersion, uint32_t toVersion, std::function<bool(Config&)> migrationFn) {
        m_migrations[fromVersion] = {toVersion, migrationFn};
    }

    void Config::addSensitiveKey(const std::string& key) {
        m_sensitiveKeys.push_back(key);
    }

    std::string Config::encrypt(const std::string& value) const {
        if (m_encryptionKey.empty()) return value;

        std::string result = value;
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] ^= m_encryptionKey[i % m_encryptionKey.size()];
        }

        std::ostringstream hexStream;
        for (unsigned char c : result) {
            hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
        return hexStream.str();
    }

    std::string Config::decrypt(const std::string& value) const {
        if (m_encryptionKey.empty()) return value;
        if (value.size() % 2 != 0) return value;

        std::string decoded;
        for (size_t i = 0; i < value.size(); i += 2) {
            std::string byteStr = value.substr(i, 2);
            char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
            decoded += byte;
        }

        for (size_t i = 0; i < decoded.size(); ++i) {
            decoded[i] ^= m_encryptionKey[i % m_encryptionKey.size()];
        }
        return decoded;
    }

    std::string Config::trim(const std::string& str) const {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }

    std::string Config::loadFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

}

