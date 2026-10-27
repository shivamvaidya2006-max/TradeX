#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cctype>

namespace tradex::utils {

enum class JsonType {
    NUL,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT
};

class Json {
public:
    JsonType type{JsonType::NUL};
    bool boolValue{false};
    double numberValue{0.0};
    std::string stringValue;
    std::vector<Json> arrayValue;
    std::map<std::string, Json> objectValue;

    Json() : type(JsonType::NUL) {}
    Json(std::nullptr_t) : type(JsonType::NUL) {}
    Json(bool b) : type(JsonType::BOOLEAN), boolValue(b) {}
    Json(int n) : type(JsonType::NUMBER), numberValue(n) {}
    Json(int64_t n) : type(JsonType::NUMBER), numberValue(static_cast<double>(n)) {}
    Json(uint64_t n) : type(JsonType::NUMBER), numberValue(static_cast<double>(n)) {}
    Json(double n) : type(JsonType::NUMBER), numberValue(n) {}
    Json(const char* s) : type(JsonType::STRING), stringValue(s) {}
    Json(std::string s) : type(JsonType::STRING), stringValue(std::move(s)) {}
    Json(std::vector<Json> arr) : type(JsonType::ARRAY), arrayValue(std::move(arr)) {}
    Json(std::map<std::string, Json> obj) : type(JsonType::OBJECT), objectValue(std::move(obj)) {}
    Json(std::initializer_list<std::pair<const std::string, Json>> init)
        : type(JsonType::OBJECT), objectValue(init.begin(), init.end()) {}

    static Json array() {
        Json j;
        j.type = JsonType::ARRAY;
        return j;
    }

    static Json object() {
        Json j;
        j.type = JsonType::OBJECT;
        return j;
    }

    bool isNull() const { return type == JsonType::NUL; }
    bool isBool() const { return type == JsonType::BOOLEAN; }
    bool isNumber() const { return type == JsonType::NUMBER; }
    bool isString() const { return type == JsonType::STRING; }
    bool isArray() const { return type == JsonType::ARRAY; }
    bool isObject() const { return type == JsonType::OBJECT; }

    std::string getString(const std::string& def = "") const {
        return (type == JsonType::STRING) ? stringValue : def;
    }
    double getNumber(double def = 0.0) const {
        return (type == JsonType::NUMBER) ? numberValue : def;
    }
    int getInt(int def = 0) const {
        return (type == JsonType::NUMBER) ? static_cast<int>(numberValue) : def;
    }
    bool getBool(bool def = false) const {
        return (type == JsonType::BOOLEAN) ? boolValue : def;
    }

    bool contains(const std::string& key) const {
        return (type == JsonType::OBJECT) && (objectValue.find(key) != objectValue.end());
    }

    Json& operator[](const std::string& key) {
        if (type != JsonType::OBJECT) {
            type = JsonType::OBJECT;
            objectValue.clear();
        }
        return objectValue[key];
    }

    const Json& operator[](const std::string& key) const {
        static const Json nullJson;
        if (type != JsonType::OBJECT) return nullJson;
        auto it = objectValue.find(key);
        return (it != objectValue.end()) ? it->second : nullJson;
    }

    Json& operator[](size_t index) {
        if (type != JsonType::ARRAY) {
            type = JsonType::ARRAY;
            arrayValue.clear();
        }
        if (index >= arrayValue.size()) arrayValue.resize(index + 1);
        return arrayValue[index];
    }

    const Json& operator[](size_t index) const {
        static const Json nullJson;
        if (type != JsonType::ARRAY || index >= arrayValue.size()) return nullJson;
        return arrayValue[index];
    }

    void push_back(Json val) {
        if (type != JsonType::ARRAY) {
            type = JsonType::ARRAY;
            arrayValue.clear();
        }
        arrayValue.push_back(std::move(val));
    }

    std::string dump() const {
        std::ostringstream ss;
        serialize(ss);
        return ss.str();
    }

    static Json parse(const std::string& str) {
        size_t idx = 0;
        skipWhitespace(str, idx);
        return parseValue(str, idx);
    }

private:
    void serialize(std::ostringstream& ss) const {
        switch (type) {
            case JsonType::NUL: ss << "null"; break;
            case JsonType::BOOLEAN: ss << (boolValue ? "true" : "false"); break;
            case JsonType::NUMBER: {
                if (numberValue == static_cast<int64_t>(numberValue)) {
                    ss << static_cast<int64_t>(numberValue);
                } else {
                    ss << std::fixed << std::setprecision(2) << numberValue;
                }
                break;
            }
            case JsonType::STRING: {
                ss << '"';
                for (char c : stringValue) {
                    if (c == '"') ss << "\\\"";
                    else if (c == '\\') ss << "\\\\";
                    else if (c == '\b') ss << "\\b";
                    else if (c == '\f') ss << "\\f";
                    else if (c == '\n') ss << "\\n";
                    else if (c == '\r') ss << "\\r";
                    else if (c == '\t') ss << "\\t";
                    else ss << c;
                }
                ss << '"';
                break;
            }
            case JsonType::ARRAY: {
                ss << '[';
                for (size_t i = 0; i < arrayValue.size(); ++i) {
                    if (i > 0) ss << ',';
                    arrayValue[i].serialize(ss);
                }
                ss << ']';
                break;
            }
            case JsonType::OBJECT: {
                ss << '{';
                bool first = true;
                for (const auto& pair : objectValue) {
                    if (!first) ss << ',';
                    first = false;
                    ss << '"' << pair.first << "\":";
                    pair.second.serialize(ss);
                }
                ss << '}';
                break;
            }
        }
    }

    static void skipWhitespace(const std::string& str, size_t& idx) {
        while (idx < str.size() && std::isspace(static_cast<unsigned char>(str[idx]))) {
            idx++;
        }
    }

    static Json parseValue(const std::string& str, size_t& idx) {
        skipWhitespace(str, idx);
        if (idx >= str.size()) return Json();

        char c = str[idx];
        if (c == 'n') { idx += 4; return Json(); }
        if (c == 't') { idx += 4; return Json(true); }
        if (c == 'f') { idx += 5; return Json(false); }
        if (c == '"') return parseString(str, idx);
        if (c == '[') return parseArray(str, idx);
        if (c == '{') return parseObject(str, idx);
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') return parseNumber(str, idx);

        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = idx;
            while (idx < str.size() && (std::isalnum(static_cast<unsigned char>(str[idx])) || str[idx] == '_' || str[idx] == '@' || str[idx] == '.' || str[idx] == '-')) {
                idx++;
            }
            return Json(str.substr(start, idx - start));
        }

        return Json();
    }

    static Json parseString(const std::string& str, size_t& idx) {
        idx++; // skip opening '"'
        std::string res;
        while (idx < str.size()) {
            char c = str[idx++];
            if (c == '"') break;
            if (c == '\\' && idx < str.size()) {
                char next = str[idx++];
                if (next == '"') res += '"';
                else if (next == '\\') res += '\\';
                else if (next == 'n') res += '\n';
                else if (next == 'r') res += '\r';
                else if (next == 't') res += '\t';
                else res += next;
            } else {
                res += c;
            }
        }
        return Json(res);
    }

    static Json parseNumber(const std::string& str, size_t& idx) {
        size_t start = idx;
        if (str[idx] == '-') idx++;
        while (idx < str.size() && (std::isdigit(static_cast<unsigned char>(str[idx])) || str[idx] == '.' || str[idx] == 'e' || str[idx] == 'E' || str[idx] == '+' || str[idx] == '-')) {
            idx++;
        }
        std::string numStr = str.substr(start, idx - start);
        double val = 0.0;
        try {
            val = std::stod(numStr);
        } catch (...) {}
        return Json(val);
    }

    static Json parseArray(const std::string& str, size_t& idx) {
        idx++; // skip '['
        Json arr = Json::array();
        skipWhitespace(str, idx);
        if (idx < str.size() && str[idx] == ']') {
            idx++;
            return arr;
        }

        while (idx < str.size()) {
            arr.push_back(parseValue(str, idx));
            skipWhitespace(str, idx);
            if (idx < str.size() && str[idx] == ',') {
                idx++;
                skipWhitespace(str, idx);
            } else if (idx < str.size() && str[idx] == ']') {
                idx++;
                break;
            } else {
                idx++;
            }
        }
        return arr;
    }

    static Json parseObject(const std::string& str, size_t& idx) {
        idx++; // skip '{'
        Json obj = Json::object();
        skipWhitespace(str, idx);
        if (idx < str.size() && str[idx] == '}') {
            idx++;
            return obj;
        }

        while (idx < str.size()) {
            skipWhitespace(str, idx);
            if (idx >= str.size() || str[idx] == '}') {
                if (idx < str.size() && str[idx] == '}') idx++;
                break;
            }

            std::string key;
            if (str[idx] == '"') {
                key = parseString(str, idx).getString();
            } else if (std::isalpha(static_cast<unsigned char>(str[idx])) || str[idx] == '_') {
                size_t start = idx;
                while (idx < str.size() && (std::isalnum(static_cast<unsigned char>(str[idx])) || str[idx] == '_')) {
                    idx++;
                }
                key = str.substr(start, idx - start);
            } else {
                idx++;
                continue;
            }

            skipWhitespace(str, idx);
            if (idx < str.size() && str[idx] == ':') idx++;
            skipWhitespace(str, idx);
            obj[key] = parseValue(str, idx);
            skipWhitespace(str, idx);
            if (idx < str.size() && str[idx] == ',') {
                idx++;
                skipWhitespace(str, idx);
            } else if (idx < str.size() && str[idx] == '}') {
                idx++;
                break;
            }
        }
        return obj;
    }
};

} // namespace tradex::utils
