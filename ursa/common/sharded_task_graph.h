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
#include <vector>

#include "glog/logging.h"

#include "common/constants.h"
#include "common/sharded_task.h"
#include "common/task.h"
#include "metadata/sharded_metadata.h"

namespace axe {
namespace common {

using metadata::ShardedMetadata;

class ShardedTaskGraph {
 public:
  void AddTasks(TaskIdType task_id, std::vector<ShardedTask>&& tasks) { tasks_.insert({task_id, std::move(tasks)}); }
  void AddData(DataIdType data_id, std::vector<std::shared_ptr<ShardedMetadata>>&& sharded_metadata) {
    data_.insert({data_id, std::move(sharded_metadata)});
  }

  const auto& GetTasksById(TaskIdType task_id) {
    auto pos = tasks_.find(task_id);
    CHECK(pos != tasks_.end()) << "Task " << task_id << " does not exist";
    return pos->second;
  }

  const auto& GetTaskById(TaskIdType task_id, ShardIdType shard_id) {
    auto pos = tasks_.find(task_id);
    CHECK(pos != tasks_.end()) << "Task " << task_id << " does not exist";
    const auto& vec = pos->second;
    CHECK(shard_id < pos->second.size()) << "Task " << task_id << " shard id " << shard_id << " does not exist";
    return vec.at(shard_id);
  }

  const auto& GetDataById(DataIdType data_id, ShardIdType shard_id) {
    const auto& vec = GetDataById(data_id);
    CHECK(shard_id < vec.size()) << "Data " << data_id << " shard " << shard_id << " does not exist";
    return vec.at(shard_id);
  }

  const std::vector<std::shared_ptr<ShardedMetadata>>& GetDataById(DataIdType data_id) {
    auto pos = data_.find(data_id);
    CHECK(pos != data_.end()) << "Data " << data_id << " does not exist";
    return pos->second;
  }

  auto GetDataColumns(const std::vector<DataIdType>& data) {
    std::vector<std::shared_ptr<ShardedMetadata>> ret;
    for (auto data_id : data) {
      auto& dataset = GetDataById(data_id);
      ret.insert(ret.end(), dataset.begin(), dataset.end());
    }
    return ret;
  }

  auto GetDataColumn(const std::vector<DataIdType>& data, ShardIdType shard_id) {
    std::vector<std::shared_ptr<ShardedMetadata>> ret;
    for (auto data_id : data) {
      ret.push_back(GetDataById(data_id, shard_id));
    }
    return ret;
  }

  const auto& GetData() const { return data_; }

  bool HaveTask(TaskIdType task_id) const { return tasks_.count(task_id) > 0; }

 private:
  std::map<TaskIdType, std::vector<ShardedTask>> tasks_;
  std::map<DataIdType, std::vector<std::shared_ptr<ShardedMetadata>>> data_;
};

}  // namespace common
}  // namespace axe
