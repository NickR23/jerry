#include <string>
#include <variant>

#include "Tokenizer.h"

namespace jerry {

struct JsonValue {
  // JsonValues or either a literal (bool, string, etc...) or a mapping of a
  // key (std::string) to another value.
  std::variant<bool, double, std::string, std::vector<JsonValue>,
               std::map<std::string, JsonValue>>
      value;
  bool operator==(const JsonValue &other) const {
    return value == other.value;
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

  static std::optional<Json> fromState(TokenizerState& state) {
    auto consumeWhitespace = [&state]() {
      auto r = manyOf(whitespace()).run(state);
      // manyOf should not return nullopt ever but lets be safe.
      return r ? r->second : state;
    };

    state = consumeWhitespace();

    //Check if the next token is an obj or literal
    // When this is run either a '{', a JsonNumber(), or a JsonString is in the
    // resulting Jsontoken
    auto isNumberOrString = orElse(jsonNumber(), jsonString());
    auto r = isNumberOrString.run(state);
    if (r)  {
      // TODO fix the jsontoken (variant) to value api. 
      // Also refactor this.
      if (r->first.type == JsonTokenType::String) {
        std::optional<std::string> s = r->first.toString();
        if (!s) {
          return std::nullopt;
        }
        return Json(JsonValue{s.value()});
      } else if (r->first.type == JsonTokenType::Number) {
        std::optional<double> n = r->first.toNumber();
        if (!n) {
          return std::nullopt;
        }
        return Json(JsonValue{n.value()});
      }
      return std::nullopt;
    }

    // auto isObectOrLiteral = orElse(openBrace(), orElse(jsonNumber(), jsonString()));
    return std::nullopt;
  }


  static std::optional<Json> fromString(std::string& input) {
    auto state = TokenizerState(input, 0);
    return fromState(state);
  }

 private:
  explicit Json(JsonValue v) : value(v) {};
  JsonValue value;
};

}  // namespace jerry
