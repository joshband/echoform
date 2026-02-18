#include "ComponentSpecLoader.h"
#include "ComponentSpec.h"

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
    enum class Type { Null, Object, Array, String, Number, Bool };
    Type type { Type::Null };
    std::unordered_map<std::string, JsonValue> object;
    std::vector<JsonValue> array;
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
        if (ch == '[')
            return parseArray();
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

    JsonValue parseArray()
    {
        JsonValue value;
        value.type = JsonValue::Type::Array;
        expect('[');
        skipWhitespace();
        if (peek() == ']')
        {
            expect(']');
            return value;
        }

        while (true)
        {
            skipWhitespace();
            value.array.push_back(parseValue());
            skipWhitespace();
            if (peek() == ',')
            {
                expect(',');
                continue;
            }
            if (peek() == ']')
            {
                expect(']');
                break;
            }
            throw std::runtime_error("Expected ',' or ']' in JSON array.");
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

const JsonValue& requireObjectField(const JsonValue& root, const std::string& key)
{
    const JsonValue* value = findObject(root, key);
    if (!value)
        throw std::runtime_error("Missing required field: " + key);
    return *value;
}

std::string requireString(const JsonValue& value, const std::string& label)
{
    if (value.type != JsonValue::Type::String)
        throw std::runtime_error("Expected string for field: " + label);
    return value.stringValue;
}

std::string requireStringField(const JsonValue& root, const std::string& key)
{
    return requireString(requireObjectField(root, key), key);
}

std::vector<std::string> parseStringArray(const JsonValue& value, const std::string& label)
{
    if (value.type != JsonValue::Type::Array)
        throw std::runtime_error("Expected array for field: " + label);
    std::vector<std::string> result;
    result.reserve(value.array.size());
    for (const auto& entry : value.array)
        result.push_back(requireString(entry, label));
    return result;
}

std::map<std::string, std::string> parseStringMap(const JsonValue& value, const std::string& label)
{
    if (value.type != JsonValue::Type::Object)
        throw std::runtime_error("Expected object for field: " + label);
    std::map<std::string, std::string> result;
    for (const auto& entry : value.object)
        result.emplace(entry.first, requireString(entry.second, label));
    return result;
}

ComponentSpec::LayerSpec parseLayer(const JsonValue& value)
{
    if (value.type != JsonValue::Type::Object)
        throw std::runtime_error("Layer entry must be an object.");
    ComponentSpec::LayerSpec layer;
    layer.id = requireStringField(value, "id");
    layer.role = requireStringField(value, "role");
    layer.transform = requireStringField(value, "transform");
    return layer;
}

ComponentSpec::InteractionSpec parseInteraction(const JsonValue& value)
{
    if (value.type != JsonValue::Type::Object)
        throw std::runtime_error("Interaction must be an object.");
    ComponentSpec::InteractionSpec interaction;
    interaction.inputType = requireStringField(value, "inputType");
    interaction.valueRange = requireStringField(value, "valueRange");
    interaction.gestureCurve = requireStringField(value, "gestureCurve");
    return interaction;
}

ComponentSpec::MotionSpec parseMotion(const JsonValue& value)
{
    if (value.type != JsonValue::Type::Object)
        throw std::runtime_error("Motion must be an object.");
    ComponentSpec::MotionSpec motion;
    motion.rotationMin = requireStringField(value, "rotationMin");
    motion.rotationMax = requireStringField(value, "rotationMax");
    return motion;
}

} // namespace

bool ComponentSpecLoader::loadFromFile(const std::string& path,
                                       ComponentSpec& outSpec,
                                       std::string& error)
{
    std::ifstream file(path);
    if (!file.good())
    {
        error = "Unable to open component spec file: " + path;
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromJson(buffer.str(), outSpec, error);
}

bool ComponentSpecLoader::loadFromJson(const std::string& jsonStr,
                                       ComponentSpec& outSpec,
                                       std::string& error)
{
    try
    {
        JsonParser parser(jsonStr);
        JsonValue root = parser.parse();
        if (root.type != JsonValue::Type::Object)
        {
            error = "Component spec root must be an object.";
            return false;
        }

        outSpec.id = requireStringField(root, "id");
        outSpec.category = requireStringField(root, "category");
        outSpec.hierarchy = parseStringArray(requireObjectField(root, "hierarchy"), "hierarchy");
        outSpec.assets = parseStringMap(requireObjectField(root, "assets"), "assets");

        const JsonValue& layersValue = requireObjectField(root, "layers");
        if (layersValue.type != JsonValue::Type::Array)
            throw std::runtime_error("Expected array for field: layers");
        outSpec.layers.clear();
        outSpec.layers.reserve(layersValue.array.size());
        for (const auto& entry : layersValue.array)
            outSpec.layers.push_back(parseLayer(entry));

        outSpec.interaction = parseInteraction(requireObjectField(root, "interaction"));
        outSpec.motion = parseMotion(requireObjectField(root, "motion"));
        outSpec.tokens = parseStringMap(requireObjectField(root, "tokens"), "tokens");

        return true;
    }
    catch (const std::exception& e)
    {
        error = e.what();
        return false;
    }
}

} // namespace ui_runtime
