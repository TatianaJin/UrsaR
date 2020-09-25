// Copyright 2018 H-AXE
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <map>
#include <string>
#include <vector>

#include "base/properties.h"

namespace axe {
namespace base {

class PropertiesReader {
 public:
  static PropertiesReader GetSystemPropertiesReader();

  Properties Read(const std::string& file);
  std::string Trim(const std::string& line);

  inline void SetCompulsoryProperty(const std::string& key) { compulsory_properties_.push_back(key); }
  inline void SetDefaultProperty(const std::string& key, const std::string& value = "") { default_properties_[key] = value; }

 private:
  inline bool IsComment(const std::string& line) { return line[0] == '#'; }

  const std::string whitespace_ = " \f\n\r\t\v";
  std::map<std::string, std::string> default_properties_;  // key-value pairs
  std::vector<std::string> compulsory_properties_;         // keys
};

}  // namespace base
}  // namespace axe
