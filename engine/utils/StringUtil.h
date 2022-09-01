#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <functional>
#include <cstdint>

namespace nebula {

    class StringUtil {
    public:
        static std::vector<std::string> split(const std::string& str, char delimiter);
        static std::vector<std::string> split(const std::string& str, const std::string& delimiter);

        static std::string join(const std::vector<std::string>& parts, const std::string& delimiter);

        static std::string trim(const std::string& str);
        static std::string trimLeft(const std::string& str);
        static std::string trimRight(const std::string& str);

        static std::string toLower(const std::string& str);
        static std::string toUpper(const std::string& str);

        static std::string replace(const std::string& str, const std::string& from, const std::string& to);
        static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

        static bool startsWith(const std::string& str, const std::string& prefix);
        static bool endsWith(const std::string& str, const std::string& suffix);
        static bool contains(const std::string& str, const std::string& substr);

        template<typename T>
        static std::string toString(T value) {
            if constexpr (std::is_same_v<T, bool>) {
                return value ? "true" : "false";
            } else {
                return std::to_string(value);
            }
        }

        static int toInt(const std::string& str, int base = 10);
        static float toFloat(const std::string& str);
        static bool toBool(const std::string& str);

        static std::string format(const char* fmt, ...);
        static std::string vformat(const char* fmt, va_list args);

        static std::wstring toWide(const std::string& str);
        static std::string fromWide(const std::wstring& str);

        static uint32_t hash(const std::string& str);
        static uint64_t hash64(const std::string& str);

        static std::string base64Encode(const std::vector<uint8_t>& data);
        static std::vector<uint8_t> base64Decode(const std::string& str);

        static bool wildcardMatch(const std::string& str, const std::string& pattern);

        static std::string padLeft(const std::string& str, size_t length, char padding = ' ');
        static std::string padRight(const std::string& str, size_t length, char padding = ' ');

        static std::string truncate(const std::string& str, size_t maxLength, const std::string& suffix = "...");

        static std::string repeat(const std::string& str, size_t count);
    };

}
