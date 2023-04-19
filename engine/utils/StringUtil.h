#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <functional>
#include <cstdint>
#include <regex>
#include <memory>

namespace nebula {

    class StringBuilder {
    public:
        StringBuilder();
        explicit StringBuilder(size_t reserveSize);

        StringBuilder& append(const std::string& str);
        StringBuilder& append(const char* str);
        StringBuilder& append(char c);
        StringBuilder& append(int value);
        StringBuilder& append(float value);
        StringBuilder& appendLine(const std::string& str);
        StringBuilder& appendLine();

        std::string toString() const;
        size_t length() const;
        void clear();
        bool isEmpty() const;

    private:
        std::string m_buffer;
    };

    class Regex {
    public:
        Regex();
        explicit Regex(const std::string& pattern);
        ~Regex() = default;

        bool compile(const std::string& pattern);
        bool isValid() const;

        bool match(const std::string& str) const;
        bool search(const std::string& str) const;
        std::vector<std::string> find(const std::string& str) const;
        std::vector<std::vector<std::string>> findAll(const std::string& str) const;

        std::string replace(const std::string& str, const std::string& replacement) const;
        std::string replaceAll(const std::string& str, const std::string& replacement) const;

        std::vector<std::string> split(const std::string& str) const;

        std::vector<std::string> getGroupNames() const;

    private:
        std::shared_ptr<std::regex> m_regex;
        std::string m_pattern;
        bool m_valid;
    };

    class StringSlice {
    public:
        StringSlice(const std::string& str, size_t start = 0, size_t end = std::string::npos);

        size_t size() const;
        size_t length() const;
        bool empty() const;
        char at(size_t index) const;
        char operator[](size_t index) const;

        std::string str() const;
        std::string substr(size_t start, size_t count = std::string::npos) const;

        StringSlice trim() const;
        StringSlice trimLeft() const;
        StringSlice trimRight() const;

        std::vector<StringSlice> split(char delimiter) const;
        bool startsWith(const std::string& prefix) const;

        size_t find(const std::string& substr, size_t pos = 0) const;

    private:
        const std::string* m_source;
        size_t m_start;
        size_t m_end;
    };

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

        static std::vector<uint8_t> utf8Encode(uint32_t codepoint);
        static std::string utf8EncodeString(const std::vector<uint32_t>& codepoints);
        static uint32_t utf8Decode(const std::string& str, size_t& offset);
        static std::vector<uint32_t> utf8DecodeString(const std::string& str);
        static size_t utf8Length(const std::string& str);

        static bool globMatch(const std::string& str, const std::string& pattern);
    };

}
