# Jerry

A fast, lightweight JSON parser written in modern C++.

## Usage

```cpp
#include "Json.h"
#include <iostream>
#include <unordered_map>
#include <string>

int main() {
  std::string input = "\"hello world\" : \"how are you?\"";
  std::unordered_map<std::string, json> result = Json::fromString(input);

  // Print the parsed JSON key-value pairs
  for (const auto& [key, value] : result) {
    std::cout << key << " : " << value << std::endl;
  }
  return 0;
}
```

This will output:

```
hello world : how are you?
```

## Currently broken features:
- `double`s  currently have rounding errors
- No unicode support (i.e. "\u263A")
- No support for quotation escape sequences (i.e. \\")

## Building

```bash
# Clone the repository
git clone https://github.com/username/jerry.git
cd jerry

# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run tests
ctest
```

<p align="center">
  <img src="https://github.com/user-attachments/assets/9548e1ca-bf4f-46aa-892b-4054a36f7441" alt="Jerry JSON Parser" width="200"/>
  <br>
  <em>Literally jerry rn</em>
</p>
