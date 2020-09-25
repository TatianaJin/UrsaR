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

#include <string>

namespace axe {
namespace common {

class WorkerInfo {
 public:
  WorkerInfo(const std::string& hostname, int port, int pid) : hostname_(hostname), port_(port), pid_(pid) {}

  inline int GetPort() const { return port_; }
  inline const std::string& GetHostName() const { return hostname_; }

 private:
  std::string hostname_;
  int port_;
  int pid_;
};

}  // namespace common
}  // namespace axe
