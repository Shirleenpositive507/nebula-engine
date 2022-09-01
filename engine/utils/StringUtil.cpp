#include "StringUtil.h"
#include <cstdarg>
#include <cwchar>
#include <vector>
#include <locale>
#include <codecvt>

#ifdef NEBULA_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace nebula {

    std::vector<std::string> StringUtil::split(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            result.push_back(token);
        }
        return result;
    }

    std::vector<std::string> StringUtil::split(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end;
        while ((end = str.find(delimiter, start)) != std::string::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
        }
        result.push_back(str.substr(start));
        return result;
    }

    std::string StringUtil::join(const std::vector<std::string>& parts, const std::string& delimiter) {
        std::string result;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) result += delimiter;
            result += parts[i];
        }
        return result;
    }

    std::string StringUtil::trim(const std::string& str) {
        return trimLeft(trimRight(str));
    }

    std::string StringUtil::trimLeft(const std::string& str) {
        auto it = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        return std::string(it, str.end());
    }

    std::string StringUtil::trimRight(const std::string& str) {
        auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        return std::string(str.begin(), it.base());
    }

    std::string StringUtil::toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return result;
    }

    std::string StringUtil::toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });
        return result;
    }

    std::string StringUtil::replace(const std::string& str, const std::string& from, const std::string& to) {
        size_t pos = str.find(from);
        if (pos == std::string::npos) return str;
        std::string result = str;
        result.replace(pos, from.length(), to);
        return result;
    }

    std::string StringUtil::replaceAll(const std::string& str, const std::string& from, const std::string& to) {
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
        return result;
    }

    bool StringUtil::startsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }

    bool StringUtil::endsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool StringUtil::contains(const std::string& str, const std::string& substr) {
        return str.find(substr) != std::string::npos;
    }

    int StringUtil::toInt(const std::string& str, int base) {
        return static_cast<int>(std::stoll(str, nullptr, base));
    }

    float StringUtil::toFloat(const std::string& str) {
        return std::stof(str);
    }

    bool StringUtil::toBool(const std::string& str) {
        std::string lower = toLower(trim(str));
        return lower == "true" || lower == "1" || lower == "yes" || lower == "on";
    }

    std::string StringUtil::format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        std::string result = vformat(fmt, args);
        va_end(args);
        return result;
    }

    std::string StringUtil::vformat(const char* fmt, va_list args) {
        va_list copy;
        va_copy(copy, args);
        int size = vsnprintf(nullptr, 0, fmt, copy);
        va_end(copy);

        if (size < 0) return std::string();

        std::string result(size, '\0');
        vsnprintf(&result[0], size + 1, fmt, args);
        return result;
    }

    std::wstring StringUtil::toWide(const std::string& str) {
#ifdef NEBULA_PLATFORM_WINDOWS
        if (str.empty()) return std::wstring();
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
        std::wstring result(size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], size);
        return result;
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(str);
#endif
    }

    std::string StringUtil::fromWide(const std::wstring& str) {
#ifdef NEBULA_PLATFORM_WINDOWS
        if (str.empty()) return std::string();
        int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
        std::string result(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], size, nullptr, nullptr);
        return result;
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(str);
#endif
    }

    static const uint32_t FNV1A_BASIS_32 = 2166136261u;
    static const uint32_t FNV1A_PRIME_32 = 16777619u;
    static const uint64_t FNV1A_BASIS_64 = 14695981039346656037ull;
    static const uint64_t FNV1A_PRIME_64 = 1099511628211ull;

    uint32_t StringUtil::hash(const std::string& str) {
        uint32_t h = FNV1A_BASIS_32;
        for (unsigned char c : str) {
            h ^= c;
            h *= FNV1A_PRIME_32;
        }
        return h;
    }

    uint64_t StringUtil::hash64(const std::string& str) {
        uint64_t h = FNV1A_BASIS_64;
        for (unsigned char c : str) {
            h ^= c;
            h *= FNV1A_PRIME_64;
        }
        return h;
    }

    static const char BASE64_CHARS[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string StringUtil::base64Encode(const std::vector<uint8_t>& data) {
        std::string result;
        result.reserve(((data.size() + 2) / 3) * 4);

        size_t i = 0;
        while (i < data.size()) {
            uint32_t octet_a = i < data.size() ? data[i++] : 0;
            uint32_t octet_b = i < data.size() ? data[i++] : 0;
            uint32_t octet_c = i < data.size() ? data[i++] : 0;

            uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

            result.push_back(BASE64_CHARS[(triple >> 18) & 0x3F]);
            result.push_back(BASE64_CHARS[(triple >> 12) & 0x3F]);
            result.push_back(BASE64_CHARS[(triple >> 6) & 0x3F]);
            result.push_back(BASE64_CHARS[triple & 0x3F]);
        }

        size_t padding = (3 - (data.size() % 3)) % 3;
        for (size_t p = 0; p < padding; ++p) {
            result[result.size() - 1 - p] = '=';
        }

        return result;
    }

    static uint8_t base64Index(char c) {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return 0;
    }

    std::vector<uint8_t> StringUtil::base64Decode(const std::string& str) {
        std::vector<uint8_t> result;
        result.reserve((str.size() / 4) * 3);

        size_t i = 0;
        while (i < str.size() && str[i] != '=') {
            uint8_t sextet_a = base64Index(str[i++]);
            uint8_t sextet_b = i < str.size() && str[i] != '=' ? base64Index(str[i++]) : 0;
            uint8_t sextet_c = i < str.size() && str[i] != '=' ? base64Index(str[i++]) : 0;
            uint8_t sextet_d = i < str.size() && str[i] != '=' ? base64Index(str[i++]) : 0;

            uint32_t triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6) | sextet_d;

            result.push_back((triple >> 16) & 0xFF);
            if (sextet_c != 0 || (i < str.size() && str[i - 1] != '='))
                result.push_back((triple >> 8) & 0xFF);
            if (sextet_d != 0 || (i < str.size() && str[i - 1] != '='))
                result.push_back(triple & 0xFF);

            while (i < str.size() && str[i] == '=') ++i;
        }

        return result;
    }

    bool StringUtil::wildcardMatch(const std::string& str, const std::string& pattern) {
        size_t s = 0, p = 0;
        size_t starIdx = std::string::npos;
        size_t match = 0;

        while (s < str.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
                ++s;
                ++p;
            } else if (p < pattern.size() && pattern[p] == '*') {
                starIdx = p;
                match = s;
                ++p;
            } else if (starIdx != std::string::npos) {
                p = starIdx + 1;
                ++match;
                s = match;
            } else {
                return false;
            }
        }

        while (p < pattern.size() && pattern[p] == '*') {
            ++p;
        }

        return p == pattern.size();
    }

    std::string StringUtil::padLeft(const std::string& str, size_t length, char padding) {
        if (str.size() >= length) return str;
        return std::string(length - str.size(), padding) + str;
    }

    std::string StringUtil::padRight(const std::string& str, size_t length, char padding) {
        if (str.size() >= length) return str;
        return str + std::string(length - str.size(), padding);
    }

    std::string StringUtil::truncate(const std::string& str, size_t maxLength, const std::string& suffix) {
        if (str.size() <= maxLength) return str;
        return str.substr(0, maxLength - suffix.size()) + suffix;
    }

    std::string StringUtil::repeat(const std::string& str, size_t count) {
        std::string result;
        result.reserve(str.size() * count);
        for (size_t i = 0; i < count; ++i) {
            result += str;
        }
        return result;
    }

}
