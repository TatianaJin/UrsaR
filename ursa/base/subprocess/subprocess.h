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
#include <vector>

#include "glog/logging.h"

namespace axe {
namespace subprocess {

class Subprocess {
 public:
  explicit Subprocess(int pid) : pid_(pid) {}
  int GetPid() const { return pid_; }

 private:
  int pid_;
};

class SubprocessBuilder {
 public:
  explicit SubprocessBuilder(const char* filename);
  /**
   * Add argument for process.
   * First argument should be the filepath
   *
   * @param argment
   */
  void AddArg(const std::string& arg);
  void AddEnv(const std::string& env);

  std::string GetCommand() const {
    std::stringstream ss;
    for (auto& env : envp_) {
      ss << env << " ";
    }
    ss << "./" << filename_;
    for (auto& arg : argv_) {
      ss << " " << arg;
    }
    return ss.str();
  }

  Subprocess Start();

 private:
  std::vector<std::string> argv_;
  std::vector<std::string> envp_;
  std::string filename_;
};

int KillSubprocess(const Subprocess& subprocess);
void WaitSubprocess(const Subprocess& subprocess);

}  // namespace subprocess
}  // namespace axe
