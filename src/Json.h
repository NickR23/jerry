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

  static std::optional<Json> fromString(std::string input) {
    auto json = Json();
    json.value = {"hello"};
    return json;
  }

 private:
  explicit Json() = default;
  JsonValue value;
};

}  // namespace jerry
