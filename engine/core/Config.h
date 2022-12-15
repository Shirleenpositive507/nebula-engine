#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <functional>
#include <chrono>
#include <ctime>

namespace nebula {

    struct ConfigSection {
        std::string name;
        std::unordered_map<std::string, std::string> values;
    };

    class Config {
    public:
        static Config& instance();

        bool load(const std::string& path);
        bool save(const std::string& path);
        bool reload();

        void setWatchChanges(bool watch);
        bool isWatching() const { return m_watchChanges; }
        void setChangeCallback(std::function<void()> callback);

        template<typename T>
        T get(const std::string& key, const T& defaultValue = T()) const {
            auto it = m_values.find(key);
            if (it == m_values.end()) return defaultValue;

            T value;
            std::istringstream iss(it->second);
            if constexpr (std::is_same_v<T, bool>) {
                std::string str = it->second;
                std::transform(str.begin(), str.end(), str.begin(), ::tolower);
                return str == "true" || str == "1" || str == "yes";
            } else if constexpr (std::is_same_v<T, std::string>) {
                return it->second;
            } else {
                iss >> value;
            }
            return value;
        }

        template<typename T>
        void set(const std::string& key, const T& value) {
            if constexpr (std::is_same_v<T, bool>) {
                m_values[key] = value ? "true" : "false";
            } else {
                std::ostringstream oss;
                oss << value;
                m_values[key] = oss.str();
            }
        }

        bool hasKey(const std::string& key) const;
        void removeKey(const std::string& key);
        void clear();
        std::vector<std::string> getKeys() const;
        std::vector<std::string> getKeysInSection(const std::string& section) const;

        std::vector<ConfigSection> getSections() const;
        bool hasSection(const std::string& section) const;
        void removeSection(const std::string& section);

        void setString(const std::string& key, const std::string& value);
        std::string getString(const std::string& key, const std::string& defaultValue = "") const;
        int getInt(const std::string& key, int defaultValue = 0) const;
        float getFloat(const std::string& key, float defaultValue = 0.0f) const;
        bool getBool(const std::string& key, bool defaultValue = false) const;

        std::string getLastError() const { return m_lastError; }
        bool hasValidationErrors() const { return !m_validationErrors.empty(); }
        std::vector<std::string> getValidationErrors() const { return m_validationErrors; }

    private:
        Config() = default;
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        std::string trim(const std::string& str) const;
        std::string loadFile(const std::string& path);

        std::unordered_map<std::string, std::string> m_values;
        std::string m_configPath;
        std::string m_lastError;
        std::vector<std::string> m_validationErrors;
        bool m_watchChanges = false;
        std::function<void()> m_changeCallback;
        std::time_t m_lastWriteTime = 0;
    };

}

