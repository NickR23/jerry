#pragma once
#include <variant>
#include <string>

namespace jerry {
    enum class JsonTokenType {
        ObjectStart,  
        ObjectEnd,
        ArrayStart,
        ArrayEnd,
        String,
        Number,
        Boolean,
        Null,
        Colon,
        Comma
    };

    struct JsonToken {
        JsonTokenType type;
        // Use monostate to represent tokens that don't have an associated value.
        // i.e. ',' or '{' 
        std::variant<std::monostate, std::string, double, bool> value;

        static JsonToken fromString(const std::string& s) {
            return {JsonTokenType::String, s};
        }

        static JsonToken fromBool(bool b) {
            return {JsonTokenType::Boolean, b};
        }

        static JsonToken fromNumber(double n) {
            return {JsonTokenType::Number, n};
        }

        static JsonToken makeNull() {
            return {JsonTokenType::String, std::monostate{}};
        }

        static JsonToken makeStructural(JsonTokenType type) {
            return {type, std::monostate{}};
        }

        bool operator==(const JsonToken& other) const {
            if (type != other.type) {
                return false;
            }

            if (type == JsonTokenType::ObjectStart ||
                type == JsonTokenType::ObjectEnd   ||
                type == JsonTokenType::ArrayStart  ||
                type == JsonTokenType::ArrayEnd    ||
                type == JsonTokenType::Colon       ||
                type == JsonTokenType::Comma       ||
                type == JsonTokenType::Null          ) {
                return true;
            }
            return value == other.value;
        }
    };
}