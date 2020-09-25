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

#include <memory>
#include <string>
#include <vector>

#include "common/constants.h"
#include "common/task_base.h"
#include "common/task_dependency.h"
#include "metadata/metadata.h"

namespace axe {
namespace common {

class Task : public TaskBase {
 public:
  Task(TaskIdType id, const std::string& name);

  void Then(const Task& child) {
    if (child.GetTaskType() == NetWork) {  // the message data should be communicated asynchronously
      children_.push_back({child.GetId(), TaskDependencyType::AsyncComm});
    } else {
      children_.push_back({child.GetId(), TaskDependencyType::Async});
    }
  }
  inline void Then(const std::shared_ptr<Task>& child) { Then(*child); }

  inline void AggregateThen(const Task& child) { children_.push_back({child.GetId(), TaskDependencyType::Aggregate}); }
  inline void AggregateThen(const std::shared_ptr<Task>& child) { AggregateThen(*child); }

  inline void LocalAggregateThen(const Task& child) { children_.push_back({child.GetId(), TaskDependencyType::LocalAggregate}); }
  inline void LocalAggregateThen(const std::shared_ptr<Task>& child) { LocalAggregateThen(*child); }

  inline void BroadcastThen(const Task& child) { children_.push_back({child.GetId(), TaskDependencyType::Broadcast}); }
  inline void BroadcastThen(const std::shared_ptr<Task>& child) { BroadcastThen(*child); }

  inline void SyncThen(const Task& child) { children_.push_back({child.GetId(), TaskDependencyType::Sync}); }
  inline void SyncThen(const std::shared_ptr<Task>& child) { SyncThen(*child); }

  const std::vector<TaskDependency>& GetChildren() const { return children_; }

  void SetParallelism(int parallelism) { parallelism_ = parallelism; }
  int GetParallelism() const { return parallelism_; }

  void ProduceData(DataIdType data_id) { produce_data_.push_back(data_id); }
  void ReadData(DataIdType data_id) { read_data_.push_back(data_id); }
  void WriteData(DataIdType data_id) { write_data_.push_back(data_id); }

  const auto& GetProduceData() { return produce_data_; }
  const auto& GetReadData() { return read_data_; }
  const auto& GetWriteData() { return write_data_; }

 private:
  std::vector<TaskDependency> children_;
  int parallelism_;
  std::vector<DataIdType> produce_data_;
  std::vector<DataIdType> read_data_;
  std::vector<DataIdType> write_data_;
};

}  // namespace common
}  // namespace axe
