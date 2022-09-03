#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>
#include <stdexcept>

namespace nebula {

    class JSONValue;
    using JSONArray = std::vector<JSONValue>;
    using JSONObject = std::unordered_map<std::string, JSONValue>;

    enum class JSONType {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    class JSONValue {
    public:
        JSONValue() : m_type(JSONType::Null) {}
        JSONValue(std::nullptr_t) : m_type(JSONType::Null) {}
        JSONValue(bool value) : m_type(JSONType::Bool), m_data(value) {}
        JSONValue(double value) : m_type(JSONType::Number), m_data(value) {}
        JSONValue(int value) : m_type(JSONType::Number), m_data(static_cast<double>(value)) {}
        JSONValue(const char* value) : m_type(JSONType::String), m_data(std::string(value)) {}
        JSONValue(const std::string& value) : m_type(JSONType::String), m_data(value) {}
        JSONValue(const JSONArray& value) : m_type(JSONType::Array), m_data(value) {}
        JSONValue(const JSONObject& value) : m_type(JSONType::Object), m_data(value) {}

        JSONType getType() const { return m_type; }
        bool isNull() const { return m_type == JSONType::Null; }
        bool isBool() const { return m_type == JSONType::Bool; }
        bool isNumber() const { return m_type == JSONType::Number; }
        bool isString() const { return m_type == JSONType::String; }
        bool isArray() const { return m_type == JSONType::Array; }
        bool isObject() const { return m_type == JSONType::Object; }

        bool asBool() const;
        double asNumber() const;
        int asInt() const;
        const std::string& asString() const;
        const JSONArray& asArray() const;
        const JSONObject& asObject() const;

        size_t size() const;
        bool hasKey(const std::string& key) const;

        const JSONValue& get(const std::string& key) const;
        void set(const std::string& key, const JSONValue& value);

        void pushBack(const JSONValue& value);

        const JSONValue& operator[](const std::string& key) const;
        const JSONValue& operator[](size_t index) const;

        std::string toString() const;
        std::string prettyPrint(int indent = 0) const;

    static JSONValue parse(const std::string& json);
    static JSONValue parseStream(std::istream& stream);

    static std::string unescapeString(const std::string& str);

private:
    JSONType m_type;
    std::variant<bool, double, std::string, JSONArray, JSONObject> m_data;

    std::string escapeString(const std::string& str) const;
    };

    class JSONParseError : public std::runtime_error {
    public:
        explicit JSONParseError(const std::string& msg) : std::runtime_error(msg) {}
    };

}
