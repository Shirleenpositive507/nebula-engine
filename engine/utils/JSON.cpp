#include "JSON.h"
#include <sstream>
#include <iomanip>
#include <cctype>

namespace nebula {

    bool JSONValue::asBool() const {
        if (m_type != JSONType::Bool) throw JSONParseError("Value is not a bool");
        return std::get<bool>(m_data);
    }

    double JSONValue::asNumber() const {
        if (m_type != JSONType::Number) throw JSONParseError("Value is not a number");
        return std::get<double>(m_data);
    }

    int JSONValue::asInt() const {
        return static_cast<int>(asNumber());
    }

    const std::string& JSONValue::asString() const {
        if (m_type != JSONType::String) throw JSONParseError("Value is not a string");
        return std::get<std::string>(m_data);
    }

    const JSONArray& JSONValue::asArray() const {
        if (m_type != JSONType::Array) throw JSONParseError("Value is not an array");
        return std::get<JSONArray>(m_data);
    }

    const JSONObject& JSONValue::asObject() const {
        if (m_type != JSONType::Object) throw JSONParseError("Value is not an object");
        return std::get<JSONObject>(m_data);
    }

    size_t JSONValue::size() const {
        if (m_type == JSONType::Array) return std::get<JSONArray>(m_data).size();
        if (m_type == JSONType::Object) return std::get<JSONObject>(m_data).size();
        return 0;
    }

    bool JSONValue::hasKey(const std::string& key) const {
        if (m_type != JSONType::Object) return false;
        return std::get<JSONObject>(m_data).find(key) != std::get<JSONObject>(m_data).end();
    }

    const JSONValue& JSONValue::get(const std::string& key) const {
        static JSONValue s_null;
        if (m_type != JSONType::Object) return s_null;
        auto& obj = std::get<JSONObject>(m_data);
        auto it = obj.find(key);
        if (it == obj.end()) return s_null;
        return it->second;
    }

    void JSONValue::set(const std::string& key, const JSONValue& value) {
        if (m_type != JSONType::Object) {
            m_type = JSONType::Object;
            m_data = JSONObject();
        }
        std::get<JSONObject>(m_data)[key] = value;
    }

    void JSONValue::pushBack(const JSONValue& value) {
        if (m_type != JSONType::Array) {
            m_type = JSONType::Array;
            m_data = JSONArray();
        }
        std::get<JSONArray>(m_data).push_back(value);
    }

    const JSONValue& JSONValue::operator[](const std::string& key) const {
        return get(key);
    }

    const JSONValue& JSONValue::operator[](size_t index) const {
        static JSONValue s_null;
        if (m_type != JSONType::Array) return s_null;
        auto& arr = std::get<JSONArray>(m_data);
        if (index >= arr.size()) return s_null;
        return arr[index];
    }

    std::string JSONValue::escapeString(const std::string& str) const {
        std::string result;
        result.reserve(str.size());
        for (char c : str) {
            switch (c) {
                case '"':  result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[8];
                        snprintf(buf, sizeof(buf), "\\u%04x", c);
                        result += buf;
                    } else {
                        result += c;
                    }
            }
        }
        return result;
    }

    std::string JSONValue::toString() const {
        switch (m_type) {
            case JSONType::Null:
                return "null";
            case JSONType::Bool:
                return std::get<bool>(m_data) ? "true" : "false";
            case JSONType::Number:
                return std::to_string(std::get<double>(m_data));
            case JSONType::String:
                return "\"" + escapeString(std::get<std::string>(m_data)) + "\"";
            case JSONType::Array: {
                std::string result = "[";
                auto& arr = std::get<JSONArray>(m_data);
                for (size_t i = 0; i < arr.size(); ++i) {
                    if (i > 0) result += ",";
                    result += arr[i].toString();
                }
                result += "]";
                return result;
            }
            case JSONType::Object: {
                std::string result = "{";
                auto& obj = std::get<JSONObject>(m_data);
                bool first = true;
                for (auto& [key, val] : obj) {
                    if (!first) result += ",";
                    first = false;
                    result += "\"" + escapeString(key) + "\":" + val.toString();
                }
                result += "}";
                return result;
            }
        }
        return "null";
    }

    std::string JSONValue::prettyPrint(int indent) const {
        std::string pad(indent, ' ');
        switch (m_type) {
            case JSONType::Null:
                return "null";
            case JSONType::Bool:
                return std::get<bool>(m_data) ? "true" : "false";
            case JSONType::Number:
                return std::to_string(std::get<double>(m_data));
            case JSONType::String:
                return "\"" + escapeString(std::get<std::string>(m_data)) + "\"";
            case JSONType::Array: {
                auto& arr = std::get<JSONArray>(m_data);
                if (arr.empty()) return "[]";
                std::string result = "[\n";
                for (size_t i = 0; i < arr.size(); ++i) {
                    if (i > 0) result += ",\n";
                    result += pad + "  " + arr[i].prettyPrint(indent + 2);
                }
                result += "\n" + pad + "]";
                return result;
            }
            case JSONType::Object: {
                auto& obj = std::get<JSONObject>(m_data);
                if (obj.empty()) return "{}";
                std::string result = "{\n";
                bool first = true;
                for (auto& [key, val] : obj) {
                    if (!first) result += ",\n";
                    first = false;
                    result += pad + "  \"" + escapeString(key) + "\": " + val.prettyPrint(indent + 2);
                }
                result += "\n" + pad + "}";
                return result;
            }
        }
        return "null";
    }

    std::string JSONValue::unescapeString(const std::string& str) {
        std::string result;
        result.reserve(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '\\' && i + 1 < str.size()) {
                switch (str[i + 1]) {
                    case '"':  result += '"'; ++i; break;
                    case '\\': result += '\\'; ++i; break;
                    case '/':  result += '/'; ++i; break;
                    case 'b':  result += '\b'; ++i; break;
                    case 'f':  result += '\f'; ++i; break;
                    case 'n':  result += '\n'; ++i; break;
                    case 'r':  result += '\r'; ++i; break;
                    case 't':  result += '\t'; ++i; break;
                    case 'u': {
                        if (i + 5 < str.size()) {
                            std::string hex = str.substr(i + 2, 4);
                            unsigned int cp = std::stoul(hex, nullptr, 16);
                            result += static_cast<char>(cp);
                            i += 5;
                        }
                        break;
                    }
                    default: result += str[i + 1]; ++i; break;
                }
            } else {
                result += str[i];
            }
        }
        return result;
    }

    class JSONParser {
    public:
        JSONParser(const std::string& json) : m_input(json), m_pos(0) {}

        JSONValue parse() {
            skipWhitespace();
            if (m_pos >= m_input.size())
                throw JSONParseError("Unexpected end of input");
            return parseValue();
        }

    private:
        const std::string& m_input;
        size_t m_pos;

        char peek() {
            return m_pos < m_input.size() ? m_input[m_pos] : '\0';
        }

        char advance() {
            return m_pos < m_input.size() ? m_input[m_pos++] : '\0';
        }

        void skipWhitespace() {
            while (m_pos < m_input.size() && std::isspace(static_cast<unsigned char>(m_input[m_pos])))
                ++m_pos;
        }

        void expect(char c) {
            skipWhitespace();
            if (advance() != c)
                throw JSONParseError(std::string("Expected '") + c + "' but got '" + peek() + "'");
        }

        JSONValue parseValue() {
            skipWhitespace();
            char c = peek();
            switch (c) {
                case 'n': return parseNull();
                case 't': case 'f': return parseBool();
                case '"': return parseString();
                case '[': return parseArray();
                case '{': return parseObject();
                default:
                    if (c == '-' || std::isdigit(static_cast<unsigned char>(c)))
                        return parseNumber();
                    throw JSONParseError(std::string("Unexpected character: ") + c);
            }
        }

        JSONValue parseNull() {
            if (m_input.substr(m_pos, 4) == "null") {
                m_pos += 4;
                return JSONValue(nullptr);
            }
            throw JSONParseError("Expected 'null'");
        }

        JSONValue parseBool() {
            if (m_input.substr(m_pos, 4) == "true") {
                m_pos += 4;
                return JSONValue(true);
            }
            if (m_input.substr(m_pos, 5) == "false") {
                m_pos += 5;
                return JSONValue(false);
            }
            throw JSONParseError("Expected 'true' or 'false'");
        }

        JSONValue parseNumber() {
            size_t start = m_pos;
            if (peek() == '-') advance();
            while (m_pos < m_input.size() && std::isdigit(static_cast<unsigned char>(peek())))
                advance();
            if (peek() == '.') {
                advance();
                while (m_pos < m_input.size() && std::isdigit(static_cast<unsigned char>(peek())))
                    advance();
            }
            if (peek() == 'e' || peek() == 'E') {
                advance();
                if (peek() == '+' || peek() == '-') advance();
                while (m_pos < m_input.size() && std::isdigit(static_cast<unsigned char>(peek())))
                    advance();
            }
            return JSONValue(std::stod(m_input.substr(start, m_pos - start)));
        }

        JSONValue parseString() {
            expect('"');
            std::string result;
            while (m_pos < m_input.size() && peek() != '"') {
                result += advance();
            }
            if (m_pos >= m_input.size())
                throw JSONParseError("Unterminated string");
            advance();
            return JSONValue(JSONValue::unescapeString(result));
        }

        JSONValue parseArray() {
            expect('[');
            JSONArray arr;
            skipWhitespace();
            if (peek() != ']') {
                arr.push_back(parseValue());
                skipWhitespace();
                while (peek() == ',') {
                    advance();
                    arr.push_back(parseValue());
                    skipWhitespace();
                }
            }
            expect(']');
            return JSONValue(arr);
        }

        JSONValue parseObject() {
            expect('{');
            JSONObject obj;
            skipWhitespace();
            if (peek() != '}') {
                auto key = parseString();
                expect(':');
                obj[key.asString()] = parseValue();
                skipWhitespace();
                while (peek() == ',') {
                    advance();
                    key = parseString();
                    expect(':');
                    obj[key.asString()] = parseValue();
                    skipWhitespace();
                }
            }
            expect('}');
            return JSONValue(obj);
        }
    };

    JSONValue JSONValue::parse(const std::string& json) {
        JSONParser parser(json);
        JSONValue result = parser.parse();
        return result;
    }

    JSONValue JSONValue::parseStream(std::istream& stream) {
        std::string json((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return parse(json);
    }

}
