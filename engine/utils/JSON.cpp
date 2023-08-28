#include "JSON.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <regex>

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
                if (peek() == '\\' && m_pos + 1 < m_input.size()) {
                    result += advance();
                    result += advance();
                } else {
                    result += advance();
                }
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

    JSONValue JSONValue::query(const JSONValue& root, const std::string& path) {
        if (path.empty() || path == "$") return root;

        std::string p = path;
        if (p[0] == '$') {
            if (p.size() == 1) return root;
            p = p.substr(1);
        }
        if (p[0] == '.') p = p.substr(1);

        const JSONValue* current = &root;
        std::string segment;
        std::istringstream stream(p);

        while (std::getline(stream, segment, '.')) {
            if (segment.empty()) continue;

            size_t bracketPos = segment.find('[');
            if (bracketPos != std::string::npos) {
                std::string key = segment.substr(0, bracketPos);
                if (!key.empty()) {
                    if (!current->isObject()) return JSONValue(nullptr);
                    auto& obj = current->asObject();
                    auto it = obj.find(key);
                    if (it == obj.end()) return JSONValue(nullptr);
                    current = &it->second;
                }

                std::string idxStr = segment.substr(bracketPos + 1);
                size_t closeBracket = idxStr.find(']');
                if (closeBracket != std::string::npos) {
                    idxStr = idxStr.substr(0, closeBracket);
                }

                if (!current->isArray()) return JSONValue(nullptr);
                auto& arr = current->asArray();
                size_t idx = std::stoul(idxStr);
                if (idx >= arr.size()) return JSONValue(nullptr);
                current = &arr[idx];
            } else {
                if (!current->isObject()) return JSONValue(nullptr);
                auto& obj = current->asObject();
                auto it = obj.find(segment);
                if (it == obj.end()) return JSONValue(nullptr);
                current = &it->second;
            }
        }

        return *current;
    }

    std::vector<JSONValue> JSONValue::queryAll(const JSONValue& root, const std::string& path) {
        std::vector<JSONValue> results;

        if (path.find('*') == std::string::npos) {
            results.push_back(query(root, path));
            return results;
        }

        std::string p = path;
        if (p[0] == '$') {
            if (p.size() > 1) p = p.substr(1);
        }
        if (p[0] == '.') p = p.substr(1);

        size_t wildcard = p.find('*');
        if (wildcard == std::string::npos) {
            results.push_back(query(root, path));
            return results;
        }

        std::string prefix = p.substr(0, wildcard);
        JSONValue prefixResult = query(root, prefix);

        if (prefixResult.isArray()) {
            for (auto& item : prefixResult.asArray()) {
                results.push_back(item);
            }
        } else if (prefixResult.isObject()) {
            for (auto& [key, val] : prefixResult.asObject()) {
                results.push_back(val);
            }
        }

        return results;
    }

    JSONValue JSONValue::merge(const JSONValue& base, const JSONValue& override_) {
        if (!base.isObject() || !override_.isObject()) {
            return override_;
        }

        JSONValue result = base;
        auto& resultObj = std::get<JSONObject>(result.m_data);

        for (auto& [key, val] : override_.asObject()) {
            auto baseIt = resultObj.find(key);
            if (baseIt != resultObj.end() && baseIt->second.isObject() && val.isObject()) {
                resultObj[key] = merge(baseIt->second, val);
            } else {
                resultObj[key] = val;
            }
        }

        return result;
    }

    JSONValue JSONValue::diff(const JSONValue& a, const JSONValue& b) {
        JSONObject diffObj;

        if (a.getType() != b.getType()) {
            diffObj["_typeChanged"] = JSONValue(true);
            diffObj["_oldType"] = JSONValue(static_cast<int>(a.getType()));
            diffObj["_newType"] = JSONValue(static_cast<int>(b.getType()));
            return JSONValue(diffObj);
        }

        if (a.isObject() && b.isObject()) {
            auto& aObj = a.asObject();
            auto& bObj = b.asObject();

            for (auto& [key, val] : aObj) {
                auto bIt = bObj.find(key);
                if (bIt == bObj.end()) {
                    diffObj[key] = JSONObject{{"_removed", val}};
                } else {
                    JSONValue d = diff(val, bIt->second);
                    if (!d.isObject() || d.size() > 0) {
                        diffObj[key] = d;
                    }
                }
            }

            for (auto& [key, val] : bObj) {
                if (aObj.find(key) == aObj.end()) {
                    diffObj[key] = JSONObject{{"_added", val}};
                }
            }
        } else if (a.isArray() && b.isArray()) {
            auto& aArr = a.asArray();
            auto& bArr = b.asArray();

            size_t maxSize = std::max(aArr.size(), bArr.size());
            for (size_t i = 0; i < maxSize; ++i) {
                std::string idxKey = std::to_string(i);
                if (i >= aArr.size()) {
                    diffObj[idxKey] = JSONObject{{"_added", bArr[i]}};
                } else if (i >= bArr.size()) {
                    diffObj[idxKey] = JSONObject{{"_removed", aArr[i]}};
                } else {
                    JSONValue d = diff(aArr[i], bArr[i]);
                    if (!d.isObject() || d.size() > 0) {
                        diffObj[idxKey] = d;
                    }
                }
            }
        } else {
            if (a.toString() != b.toString()) {
                diffObj["_old"] = a;
                diffObj["_new"] = b;
            }
        }

        return JSONValue(diffObj);
    }

    JSONValue JSONValue::patch(const JSONValue& base, const JSONValue& patchData) {
        if (!patchData.isObject()) return patchData;
        if (!base.isObject()) return patchData;

        JSONValue result = base;
        auto& resultObj = std::get<JSONObject>(result.m_data);

        for (auto& [key, val] : patchData.asObject()) {
            if (val.isObject() && val.hasKey("_removed")) {
                resultObj.erase(key);
            } else if (val.isObject() && val.hasKey("_added")) {
                resultObj[key] = val.get("_added");
            } else if (val.isObject() && resultObj.find(key) != resultObj.end() && resultObj[key].isObject()) {
                resultObj[key] = patch(resultObj[key], val);
            } else {
                resultObj[key] = val;
            }
        }

        return result;
    }

    bool JSONValue::validateSchema(const JSONValue& value, const JSONValue& schema, std::string* errorOut) {
        if (!schema.isObject()) return true;

        auto& schemaObj = schema.asObject();

        if (schema.hasKey("type")) {
            std::string expectedType = schema.get("type").asString();
            std::string actualType;

            switch (value.getType()) {
                case JSONType::Null:   actualType = "null"; break;
                case JSONType::Bool:   actualType = "boolean"; break;
                case JSONType::Number: actualType = "number"; break;
                case JSONType::String: actualType = "string"; break;
                case JSONType::Array:  actualType = "array"; break;
                case JSONType::Object: actualType = "object"; break;
            }

            if (expectedType == "integer" && actualType == "number") {
                double num = value.asNumber();
                if (num != static_cast<int64_t>(num)) {
                    if (errorOut) *errorOut = "Expected integer, got float";
                    return false;
                }
            } else if (expectedType != actualType) {
                if (errorOut) *errorOut = "Expected type " + expectedType + ", got " + actualType;
                return false;
            }
        }

        if (schema.hasKey("properties") && value.isObject()) {
            auto& props = schema.get("properties").asObject();
            for (auto& [propName, propSchema] : props) {
                if (value.hasKey(propName)) {
                    if (!validateSchema(value.get(propName), propSchema, errorOut)) {
                        if (errorOut) *errorOut = "Property '" + propName + "': " + *errorOut;
                        return false;
                    }
                } else if (propSchema.isObject() && propSchema.hasKey("required")) {
                    bool required = propSchema.get("required").asBool();
                    if (required) {
                        if (errorOut) *errorOut = "Missing required property '" + propName + "'";
                        return false;
                    }
                }
            }
        }

        if (schema.hasKey("items") && value.isArray()) {
            auto& items = schema.get("items");
            for (auto& item : value.asArray()) {
                if (!validateSchema(item, items, errorOut)) {
                    return false;
                }
            }
        }

        if (schema.hasKey("enum") && schema.get("enum").isArray()) {
            bool found = false;
            for (auto& enumVal : schema.get("enum").asArray()) {
                if (enumVal.toString() == value.toString()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (errorOut) *errorOut = "Value not in enum";
                return false;
            }
        }

        if (schema.hasKey("minimum") && value.isNumber()) {
            double min = schema.get("minimum").asNumber();
            if (value.asNumber() < min) {
                if (errorOut) *errorOut = "Value below minimum " + std::to_string(min);
                return false;
            }
        }

        if (schema.hasKey("maximum") && value.isNumber()) {
            double max = schema.get("maximum").asNumber();
            if (value.asNumber() > max) {
                if (errorOut) *errorOut = "Value above maximum " + std::to_string(max);
                return false;
            }
        }

        if (schema.hasKey("minLength") && value.isString()) {
            size_t minLen = static_cast<size_t>(schema.get("minLength").asInt());
            if (value.asString().size() < minLen) {
                if (errorOut) *errorOut = "String too short";
                return false;
            }
        }

        if (schema.hasKey("maxLength") && value.isString()) {
            size_t maxLen = static_cast<size_t>(schema.get("maxLength").asInt());
            if (value.asString().size() > maxLen) {
                if (errorOut) *errorOut = "String too long";
                return false;
            }
        }

        if (schema.hasKey("pattern") && value.isString()) {
            std::string pattern = schema.get("pattern").asString();
            try {
                std::regex re(pattern);
                if (!std::regex_match(value.asString(), re)) {
                    if (errorOut) *errorOut = "String does not match pattern";
                    return false;
                }
            } catch (...) {
                if (errorOut) *errorOut = "Invalid regex pattern";
                return false;
            }
        }

        return true;
    }

    JSONValue::StreamingParser::StreamingParser()
        : m_braceDepth(0), m_bracketDepth(0), m_inString(false) {}

    void JSONValue::StreamingParser::feed(const std::string& chunk) {
        m_buffer += chunk;

        for (size_t i = 0; i < chunk.size(); ++i) {
            char c = chunk[i];

            if (m_inString) {
                if (c == '\\' && i + 1 < chunk.size()) {
                    ++i;
                    continue;
                }
                if (c == '"') {
                    m_inString = false;
                }
                continue;
            }

            switch (c) {
                case '"':
                    m_inString = true;
                    break;
                case '{':
                    m_braceDepth++;
                    break;
                case '}':
                    m_braceDepth--;
                    if (m_braceDepth == 0 && m_bracketDepth == 0) {
                        size_t endPos = m_buffer.size() - (chunk.size() - i);
                        std::string complete = m_buffer.substr(0, endPos + 1);
                        try {
                            m_completedValues.push_back(JSONValue::parse(complete));
                            m_buffer = m_buffer.substr(endPos + 1);
                        } catch (...) {}
                    }
                    break;
                case '[':
                    m_bracketDepth++;
                    break;
                case ']':
                    m_bracketDepth--;
                    if (m_braceDepth == 0 && m_bracketDepth == 0) {
                        size_t endPos = m_buffer.size() - (chunk.size() - i);
                        std::string complete = m_buffer.substr(0, endPos + 1);
                        try {
                            m_completedValues.push_back(JSONValue::parse(complete));
                            m_buffer = m_buffer.substr(endPos + 1);
                        } catch (...) {}
                    }
                    break;
            }
        }
    }

    bool JSONValue::StreamingParser::hasValue() const {
        return !m_completedValues.empty();
    }

    JSONValue JSONValue::StreamingParser::popValue() {
        if (m_completedValues.empty()) return JSONValue(nullptr);
        JSONValue val = m_completedValues.front();
        m_completedValues.erase(m_completedValues.begin());
        return val;
    }

    void JSONValue::StreamingParser::reset() {
        m_buffer.clear();
        m_completedValues.clear();
        m_braceDepth = 0;
        m_bracketDepth = 0;
        m_inString = false;
    }

}
