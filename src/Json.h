#include <string>
#include <variant>
#include <optional>

#include "Tokenizer.h"

namespace jerry {

struct JsonValue {
  // JsonValues or either a literal (bool, string, etc...) or a mapping of a
  // key (std::string) to another value.
  std::variant<std::monostate, bool, double, std::string, std::vector<JsonValue>,
               std::unordered_map<std::string, JsonValue>>
      value;
  
  JsonValue() : value(std::monostate()) {}
  JsonValue(bool b) : value(b) {}
  JsonValue(int n) : value((double) n) {}
  JsonValue(double d) : value(d) {}
  JsonValue(const std::string& s) : value(s) {}
  JsonValue(const char* s) : value(std::string(s)) {}
  JsonValue(const std::vector<JsonValue>& v) : value(v) {}
  JsonValue(const std::unordered_map<std::string, JsonValue>& m) : value(m) {}
  
  JsonValue(std::initializer_list<std::string> strings) {
    std::vector<JsonValue> values;
    for (const auto& s : strings) {
      values.push_back(JsonValue(s));
    }
    value = values;
  }
  
  bool operator==(const JsonValue &other) const {
    return value == other.value;
  }

  static std::optional<JsonValue> fromJsonToken(JsonToken token) {
    if (token.type == JsonTokenType::Null) {
      return JsonValue();
    }
    auto jVal = std::visit([](auto&& val) -> std::optional<JsonValue> {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::string>) {
          return JsonValue(val);
        }
        if constexpr (std::is_same_v<T, double>) {
          return JsonValue(val);
        }
        if constexpr (std::is_same_v<T, bool>) {
          return JsonValue(val);
        }
        return std::nullopt;
    }, token.value);
    return jVal;
  }

  std::optional<std::string> toString() const {
    auto s = std::visit([](auto&& val) -> std::optional<std::string> {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::string>) {
          return val;
        }
        if constexpr (std::is_same_v<T, double>) {
          return std::to_string(val);
        }
        if constexpr (std::is_same_v<T, bool>) {
          return std::to_string(val);
        }
        if constexpr (std::is_same_v<T, std::vector<JsonValue>>) {
          return "array";
        }
        if constexpr (std::is_same_v<T, std::map<std::string, JsonValue>>) {
          return "object";
        }
        return std::nullopt;
    }, value);
    return s;
  }
};

class Json {
 public:
  bool operator==(const Json &other) const {
    return value == other.getValue();
  }

  JsonValue getValue() const {
    return value;
  }

  static std::optional<std::pair<std::vector<JsonValue>, TokenizerState>> parseList(TokenizerState& state) {
    std::vector<JsonValue> values;
    
    // Skip whitespace
    auto whitespacesResult = manyOf(whitespace()).run(state);
    if (whitespacesResult) {
      state = whitespacesResult->second;
    }
    
    // Try to parse values until we hit the end of the array
    while (true) {
      // Check for end of array
      auto endResult = arrayEnd().run(state);
      if (endResult) {
        // Array is complete, update state and return values
        state = endResult->second;
        return std::make_pair(values, state);
      }
      
      auto innerJsonResult = Json::fromState(state);
      if (!innerJsonResult) {
        return std::nullopt;
      }

      auto innerJson = innerJsonResult.value().first;
      state = innerJsonResult.value().second;
      
      
      // Add value to our list and update state
      values.push_back(innerJson.getValue());
      
      // Skip whitespace
      whitespacesResult = manyOf(whitespace()).run(state);
      if (whitespacesResult) {
        state = whitespacesResult->second;
      }
      
      // Check for comma
      auto commaResult = comma().run(state);
      if (!commaResult) {
        // If no comma, must be end of array
        endResult = arrayEnd().run(state);
        if (endResult) {
          // Array is complete, update state and return values
          state = endResult->second;
          return std::make_pair(values, state);
        }
        // Expected comma or end of array
        return std::nullopt;
      }
      
      // Found comma, update state and continue
      state = commaResult->second;
      
      // Skip whitespace after comma
      whitespacesResult = manyOf(whitespace()).run(state);
      if (whitespacesResult) {
        state = whitespacesResult->second;
      }
    }
  }

  static std::optional<std::pair<Json, TokenizerState>> fromState(TokenizerState& state) {
    auto consumeWhitespace = [](TokenizerState& state) {
      auto r = manyOf(whitespace()).run(state);
      // manyOf should not return nullopt ever but lets be safe.
      return r ? r->second : state;
    };

    state = consumeWhitespace(state);

    // Check if this is a bool
    auto jsonBoolResult = boolean().run(state);
    if (jsonBoolResult) {
      auto jVal = JsonValue::fromJsonToken(jsonBoolResult->first);
      if (jVal) {
        return std::make_pair(Json(jVal.value()), jsonBoolResult->second);
      }
    }

    auto jsonNullResult = jsonNull().run(state);
    if(jsonNullResult) {
      auto jVal = JsonValue::fromJsonToken(JsonToken::makeNull());
      if (jVal) {
        return std::make_pair(Json(jVal.value()), jsonNullResult->second);
      }
    }

    // Check if this is a literal string
    auto jsonStringResult = jsonString().run(state);
    if (jsonStringResult) {
      auto jVal = JsonValue::fromJsonToken(jsonStringResult->first);
      if (jVal) {
        return std::make_pair(Json(jVal.value()), jsonStringResult->second);
      }
    }
    
    // Check if this is a number
    auto jsonNumberResult = jsonNumber().run(state);
    if (jsonNumberResult) {
      auto jVal = JsonValue::fromJsonToken(jsonNumberResult->first);
      if (jVal) {
        return std::make_pair(Json(jVal.value()), jsonNumberResult->second);
      }
    }

    // Check if this is a list
    auto arrayStartResult = arrayStart().run(state);
    if (arrayStartResult) {
      state = arrayStartResult->second;
      auto parseListResult = parseList(state);
      if (!parseListResult) {
        return std::nullopt;
      }
      return std::make_pair(Json(parseListResult->first), parseListResult->second);
    }

    // Check if json.
    auto objectStartResult = objectStart().run(state);
    if (objectStartResult)  {
      state = objectStartResult->second;
      std::unordered_map<std::string, JsonValue> objectMap;

      // Empty objects are valid
      auto objectCloseResult = objectEnd().run(state);
      if (objectCloseResult) {
        return std::make_pair(Json(JsonValue(objectMap)), state);
      }

      bool continueObjectParsing = true;
      while (continueObjectParsing) {
        state = consumeWhitespace(state);
        // Next should be a key (string)
        // TODO write "consume" func in Tokenizer that does this.
        auto keyResult = jsonString().run(state);
        if (!keyResult) {
          return std::nullopt;
        }
        state = keyResult->second;
        state = consumeWhitespace(state);
        auto colonResult = colon().run(state);
        if (!colonResult) {
          return std::nullopt;
        }
        state = colonResult->second;
        state = consumeWhitespace(state);

        //Recurse
        auto innerJsonResult = fromState(state);
        if (!innerJsonResult) {
          return std::nullopt;
        }

        state = innerJsonResult->second;

        objectMap[keyResult->first.toString().value()] = innerJsonResult->first.getValue();

        auto commaResult = comma().run(state);
        if (commaResult) {
          state = commaResult->second;
        } else {
          continueObjectParsing = false;
        }

      }

      state = consumeWhitespace(state);

      auto closingBraceResult = braceClose().run(state);
      if (!closingBraceResult) {
        return std::nullopt;
      }
      state = closingBraceResult->second;

      return std::make_pair(Json(JsonValue(objectMap)), state);
    }

    return std::nullopt;
  }


  static std::optional<Json> fromString(const std::string& input) {
    auto state = TokenizerState(input, 0);
    auto result = fromState(state);
    if (!result) {
      return std::nullopt;
    }
    return result->first;
  }

  explicit Json(JsonValue v) : value(v) {};
  explicit Json(std::vector<JsonValue> v) : value(JsonValue{v}) {};

 private:
  JsonValue value;
};

}  // namespace jerry
