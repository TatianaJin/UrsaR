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

#include "common/constants.h"
#include "common/task.h"

namespace axe {
namespace metadata {

using common::TaskIdType;
using common::DataIdType;

class Metadata {
 public:
  Metadata(DataIdType id, const std::string& name) : id_(id), name_(name) {}

  inline auto GetParallelism() const { return parallelism_; }
  inline auto GetId() const { return id_; }
  inline auto GetProducer() const { return producer_; }
  inline auto& GetName() const { return name_; }

  inline void SetParallelism(int parallelism) { parallelism_ = parallelism; }
  inline void SetProducer(TaskIdType task_id) { producer_ = task_id; }

 private:
  DataIdType id_;
  int parallelism_ = 10;
  std::string name_;
  TaskIdType producer_ = 0;
};

}  // namespace metadata
}  // namespace axe
