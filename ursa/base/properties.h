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

#include "glog/logging.h"

namespace axe {
namespace base {

class Properties {
 public:
  const std::string& Get(const std::string& key);
  const std::string& Get(const std::string& key, const std::string& default_value);
  const std::string& GetOrSet(const std::string& key, const std::string& default_value);

  inline void Add(const std::string& key, const std::string& value) { properties_[key] = value; }

  void PrintProperties() {
    for (auto& pair : properties_) {
      LOG(INFO) << pair.first << "\t=\t" << pair.second;
    }
  }

 private:
  std::map<std::string, std::string> properties_;  // key, value
};

}  // namespace base
}  // namespace axe
