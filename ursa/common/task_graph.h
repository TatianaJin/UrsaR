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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/constants.h"
#include "common/source_data.h"
#include "common/task.h"
#include "metadata/metadata.h"

namespace axe {
namespace common {

using metadata::Metadata;

class TaskGraph {
 public:
  TaskGraph() {}
  std::shared_ptr<Task> CreateTask(const std::string& name, ResourceType type);
  inline DataIdType CreateDataset() { return dataset_counter_++; }
  inline void RegisterClosure(TaskIdType task_id, const Closure& closure) { closure_map_.insert({task_id, closure}); }
  void AddMetaData(DataIdType data_id, const Metadata& metadata) { data_.insert({data_id, metadata}); }

  inline const std::shared_ptr<Task>& GetTaskById(TaskIdType task_id) const { return tasks_.at(task_id); }
  inline TaskIdType GetNumTasks() const { return task_counter_; }
  inline DataIdType GetNumDatasets() const { return dataset_counter_; }
  inline const std::unordered_map<TaskIdType, std::shared_ptr<Task>>& GetTasks() const { return tasks_; }
  inline const ClosureMap& GetClosureMap() const { return closure_map_; }
  inline const auto& GetMetadata() const { return data_; }

  inline void AddSourceData(SourceData&& source_data) { source_data_.push_back(std::move(source_data)); }
  inline auto& GetSourceData() { return source_data_; }

 private:
  std::vector<SourceData> source_data_;
  ClosureMap closure_map_;
  DataIdType dataset_counter_ = 0;
  std::map<DataIdType, metadata::Metadata> data_;
  TaskIdType task_counter_ = 0;
  std::unordered_map<TaskIdType, std::shared_ptr<Task>> tasks_;
};

}  // namespace common
}  // namespace axe
