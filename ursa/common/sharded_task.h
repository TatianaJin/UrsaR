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
#include "common/task_desc/task_desc.h"
#include "metadata/sharded_metadata.h"

namespace axe {
namespace common {

using metadata::ShardedMetadata;

class ShardedTask : public TaskBase {
 public:
  ShardedTask(TaskIdType id, const TaskNameType& name, ShardIdType shard_id, ResourceType type) : TaskBase(id, name), shard_id_(shard_id) {
    TaskBase::SetTaskType(type);
  }

  ShardIdType GetShardId() const { return shard_id_; }

  void SetProduceData(const std::vector<std::shared_ptr<ShardedMetadata>>& produce_data) { produce_data_ = produce_data; }
  void SetReadData(const std::vector<std::shared_ptr<ShardedMetadata>>& read_data) { read_data_ = read_data; }
  void SetWriteData(const std::vector<std::shared_ptr<ShardedMetadata>>& write_data) { write_data_ = write_data; }
  void AddReadData(const std::vector<std::shared_ptr<ShardedMetadata>>& read_data) {
    read_data_.insert(read_data_.end(), read_data.begin(), read_data.end());
  }

  const auto& GetProduceData() const { return produce_data_; }
  const auto& GetReadData() const { return read_data_; }
  const auto& GetWriteData() const { return write_data_; }

  std::string GetLocality() const {
    std::string ret;
    if ((ret = GetLocality(write_data_)) != "")
      return ret;
    if ((ret = GetLocality(read_data_)) != "")
      return ret;
    return "";
  }

  void SetBroadcast() { is_broadcast_ = true; }
  const bool IsBroadcast() const { return is_broadcast_; }

 private:
  std::string GetLocality(const std::vector<std::shared_ptr<ShardedMetadata>>& data_vec) const {
    for (auto data : data_vec) {
      if (data->GetLocality() != "")
        return data->GetLocality();
    }
    return "";
  }

  bool is_broadcast_ = 0;
  ShardIdType shard_id_;
  std::vector<std::shared_ptr<ShardedMetadata>> produce_data_;
  std::vector<std::shared_ptr<ShardedMetadata>> read_data_;
  std::vector<std::shared_ptr<ShardedMetadata>> write_data_;
};

}  // namespace common
}  // namespace axe
