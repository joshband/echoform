#include "TokenLoader.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace ui_runtime {
namespace {

struct JsonValue {
    enum class Type { Null, Object, String, Number, Bool };
    Type type { Type::Null };
    std::unordered_map<std::string, JsonValue> object;
    std::string stringValue;
    double numberValue { 0.0 };
    bool boolValue { false };
};

class JsonParser {
public:
    explicit JsonParser(const std::string& input)
        : input_(input)
    {
    }

    JsonValue parse()
    {
        skipWhitespace();
        JsonValue value = parseValue();
        skipWhitespace();
        if (pos_ != input_.size())
            throw std::runtime_error("Unexpected trailing content in JSON.");
        return value;
    }

private:
    JsonValue parseValue()
    {
        skipWhitespace();
        if (pos_ >= input_.size())
            throw std::runtime_error("Unexpected end of JSON.");

        const char ch = input_[pos_];
        if (ch == '{')
            return parseObject();
        if (ch == '"')
            return parseString();
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)))
            return parseNumber();
        if (startsWith("true"))
            return parseBool(true);
        if (startsWith("false"))
            return parseBool(false);
        if (startsWith("null"))
            return parseNull();

        throw std::runtime_error("Invalid JSON token.");
    }

    JsonValue parseObject()
    {
        JsonValue value;
        value.type = JsonValue::Type::Object;
        expect('{');
        skipWhitespace();
        if (peek() == '}')
        {
            expect('}');
            return value;
        }

        while (true)
        {
            skipWhitespace();
            JsonValue key = parseString();
            skipWhitespace();
            expect(':');
            skipWhitespace();
            JsonValue child = parseValue();
            value.object.emplace(key.stringValue, std::move(child));
            skipWhitespace();
            if (peek() == ',')
            {
                expect(',');
                continue;
            }
            if (peek() == '}')
            {
                expect('}');
                break;
            }
            throw std::runtime_error("Expected ',' or '}' in JSON object.");
        }
        return value;
    }

    JsonValue parseString()
    {
        JsonValue value;
        value.type = JsonValue::Type::String;
        expect('"');
        std::string result;
        while (pos_ < input_.size())
        {
            char ch = input_[pos_++];
            if (ch == '"')
                break;
            if (ch == '\\')
            {
                if (pos_ >= input_.size())
                    throw std::runtime_error("Invalid escape sequence in string.");
                char escaped = input_[pos_++];
                switch (escaped)
                {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    default: throw std::runtime_error("Unsupported escape sequence in string.");
                }
            }
            else
            {
                result.push_back(ch);
            }
        }
        value.stringValue = result;
        return value;
    }

    JsonValue parseNumber()
    {
        JsonValue value;
        value.type = JsonValue::Type::Number;
        size_t start = pos_;
        if (input_[pos_] == '-')
            ++pos_;
        while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_])))
            ++pos_;
        if (pos_ < input_.size() && input_[pos_] == '.')
        {
            ++pos_;
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_])))
                ++pos_;
        }
        if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E'))
        {
            ++pos_;
            if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-'))
                ++pos_;
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_])))
                ++pos_;
        }

        const std::string token = input_.substr(start, pos_ - start);
        value.numberValue = std::stod(token);
        return value;
    }

    JsonValue parseBool(bool value)
    {
        JsonValue result;
        result.type = JsonValue::Type::Bool;
        if (value)
            expectSequence("true");
        else
            expectSequence("false");
        result.boolValue = value;
        return result;
    }

    JsonValue parseNull()
    {
        JsonValue result;
        result.type = JsonValue::Type::Null;
        expectSequence("null");
        return result;
    }

    void skipWhitespace()
    {
        while (pos_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[pos_])))
            ++pos_;
    }

    char peek() const
    {
        if (pos_ >= input_.size())
            return '\0';
        return input_[pos_];
    }

    void expect(char expected)
    {
        if (pos_ >= input_.size() || input_[pos_] != expected)
            throw std::runtime_error("Unexpected character in JSON.");
        ++pos_;
    }

    bool startsWith(const char* text) const
    {
        size_t len = std::strlen(text);
        if (pos_ + len > input_.size())
            return false;
        return input_.compare(pos_, len, text) == 0;
    }

    void expectSequence(const char* text)
    {
        if (!startsWith(text))
            throw std::runtime_error("Unexpected token in JSON.");
        pos_ += std::strlen(text);
    }

    const std::string& input_;
    size_t pos_ { 0 };
};

const JsonValue* findObject(const JsonValue& root, const std::string& key)
{
    if (root.type != JsonValue::Type::Object)
        return nullptr;
    auto it = root.object.find(key);
    if (it == root.object.end())
        return nullptr;
    return &it->second;
}

const JsonValue* findPath(const JsonValue& root, const std::vector<std::string>& path)
{
    const JsonValue* current = &root;
    for (const auto& key : path)
    {
        if (!current || current->type != JsonValue::Type::Object)
            return nullptr;
        auto it = current->object.find(key);
        if (it == current->object.end())
            return nullptr;
        current = &it->second;
    }
    return current;
}

TokenColor parseColor(const JsonValue* value)
{
    TokenColor color {};
    if (!value || value->type != JsonValue::Type::String)
        throw std::runtime_error("Expected color token string.");

    std::string text = value->stringValue;
    if (!text.empty() && text[0] == '#')
        text = text.substr(1);

    if (text.size() == 6)
        text = "ff" + text;
    if (text.size() != 8)
        throw std::runtime_error("Invalid color token format.");

    const auto hexToFloat = [](const std::string& hex) {
        const int value = std::stoi(hex, nullptr, 16);
        return static_cast<float>(value) / 255.0f;
    };

    color.a = hexToFloat(text.substr(0, 2));
    color.r = hexToFloat(text.substr(2, 2));
    color.g = hexToFloat(text.substr(4, 2));
    color.b = hexToFloat(text.substr(6, 2));
    return color;
}

float parseNumber(const JsonValue* value)
{
    if (!value)
        throw std::runtime_error("Missing numeric token.");
    if (value->type == JsonValue::Type::Number)
        return static_cast<float>(value->numberValue);
    if (value->type == JsonValue::Type::String)
        return std::stof(value->stringValue);
    throw std::runtime_error("Invalid numeric token.");
}

JsonValue resolveScope(const JsonValue& root,
                       const std::string& pluginName,
                       const std::string& componentName)
{
    const JsonValue* global = findObject(root, "global");
    const JsonValue* globalScope = global ? global : &root;

    JsonValue merged = *globalScope;

    if (!pluginName.empty())
    {
        const JsonValue* pluginScope = nullptr;
        if (const JsonValue* plugins = findObject(root, "plugins"))
        {
            pluginScope = findObject(*plugins, pluginName);
        }
        if (!pluginScope)
            pluginScope = findObject(root, "plugin");

        if (pluginScope && pluginScope->type == JsonValue::Type::Object)
        {
            for (const auto& entry : pluginScope->object)
                merged.object[entry.first] = entry.second;
        }
    }

    if (!componentName.empty())
    {
        if (const JsonValue* components = findObject(root, "components"))
        {
            if (const JsonValue* componentScope = findObject(*components, componentName))
            {
                if (componentScope->type == JsonValue::Type::Object)
                {
                    for (const auto& entry : componentScope->object)
                        merged.object[entry.first] = entry.second;
                }
            }
        }
    }

    return merged;
}

TokenSet buildTokenSet(const JsonValue& root)
{
    TokenSet tokens;
    const std::vector<std::pair<std::string, TokenColor*>> colorTokens = {
        { "background", &tokens.background },
        { "panel", &tokens.panel },
        { "accent", &tokens.accent },
        { "text", &tokens.text },
        { "mutedText", &tokens.mutedText },
        { "track", &tokens.track }
    };

    for (const auto& entry : colorTokens)
    {
        const JsonValue* value = findPath(root, { "colors", entry.first, "value" });
        if (!value)
            throw std::runtime_error("Missing required color token: " + entry.first);
        *entry.second = parseColor(value);
    }

    tokens.fontSize = parseNumber(findPath(root, { "font", "size", "value" }));
    tokens.fontSizeSmall = parseNumber(findPath(root, { "font", "sizeSmall", "value" }));
    tokens.fontSizeLarge = parseNumber(findPath(root, { "font", "sizeLarge", "value" }));

    tokens.spacing.xs = parseNumber(findPath(root, { "spacing", "xs", "value" }));
    tokens.spacing.sm = parseNumber(findPath(root, { "spacing", "sm", "value" }));
    tokens.spacing.md = parseNumber(findPath(root, { "spacing", "md", "value" }));
    tokens.spacing.lg = parseNumber(findPath(root, { "spacing", "lg", "value" }));

    tokens.radius.sm = parseNumber(findPath(root, { "radius", "sm", "value" }));
    tokens.radius.md = parseNumber(findPath(root, { "radius", "md", "value" }));
    tokens.radius.lg = parseNumber(findPath(root, { "radius", "lg", "value" }));

    return tokens;
}

} // namespace

TokenLoadResult TokenLoader::loadFromFile(const std::string& path,
                                          const std::string& pluginName,
                                          const std::string& componentName) const
{
    std::ifstream file(path);
    if (!file.good())
        return { {}, false, "Unable to open token file: " + path };

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromJson(buffer.str(), pluginName, componentName);
}

TokenLoadResult TokenLoader::loadFromJson(const std::string& json,
                                          const std::string& pluginName,
                                          const std::string& componentName) const
{
    try
    {
        JsonParser parser(json);
        JsonValue root = parser.parse();
        if (root.type != JsonValue::Type::Object)
            return { {}, false, "Token JSON root must be an object." };

        const JsonValue resolved = resolveScope(root, pluginName, componentName);
        TokenSet tokens = buildTokenSet(resolved);
        return { tokens, true, {} };
    }
    catch (const std::exception& e)
    {
        return { {}, false, e.what() };
    }
}

} // namespace ui_runtime
