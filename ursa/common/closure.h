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

#include <functional>
#include <map>

#include "common/constants.h"
#include "common/task_context.h"

namespace axe {
namespace common {

class Closure {
 public:
  static Closure CreateClosure(std::function<void(TaskContext*)> function);
  void Execute(TaskContext*) const;

 protected:
  std::function<void(TaskContext*)> function_ = nullptr;
};

using ClosureMap = std::map<TaskIdType, Closure>;

}  // namespace common
}  // namespace axe
