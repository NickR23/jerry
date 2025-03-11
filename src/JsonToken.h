#pragma once
#include <variant>
#include <string>
#include <sstream>
#include <ostream>

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
        std::variant<std::monostate, std::string, double, bool> value;

        // Factory methods for creating tokens
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
            return {JsonTokenType::Null, std::monostate{}};
        }

        static JsonToken makeStructural(JsonTokenType type) {
            return {type, std::monostate{}};
        }

        // Equality operator
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
        
        // String representation for debuggin 
        std::string toString() const {
            std::stringstream ss;
            switch(type) {
                case JsonTokenType::ObjectStart: ss << "ObjectStart"; break;
                case JsonTokenType::ObjectEnd: ss << "ObjectEnd"; break;
                case JsonTokenType::ArrayStart: ss << "ArrayStart"; break;
                case JsonTokenType::ArrayEnd: ss << "ArrayEnd"; break;
                case JsonTokenType::String: 
                    if (std::holds_alternative<std::string>(value)) {
                        ss << "String(\"" << std::get<std::string>(value) << "\")";
                    } else {
                        ss << "String(invalid)";
                    }
                    break;
                case JsonTokenType::Number:
                    if (std::holds_alternative<double>(value)) {
                        ss << "Number(" << std::get<double>(value) << ")";
                    } else {
                        ss << "Number(invalid)";
                    }
                    break;
                case JsonTokenType::Boolean:
                    if (std::holds_alternative<bool>(value)) {
                        ss << "Boolean(" << (std::get<bool>(value) ? "true" : "false") << ")";
                    } else {
                        ss << "Boolean(invalid)";
                    }
                    break;
                case JsonTokenType::Null: ss << "Null"; break;
                case JsonTokenType::Colon: ss << "Colon"; break;
                case JsonTokenType::Comma: ss << "Comma"; break;
            }
            
            return ss.str();
        }
    };
    
    // Stream operator for easy printing
    inline std::ostream& operator<<(std::ostream& os, const JsonToken& token) {
        return os << token.toString();
    }
}