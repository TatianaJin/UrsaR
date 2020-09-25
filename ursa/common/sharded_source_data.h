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

#include "glog/logging.h"

#include "common/constants.h"
#include "common/task_desc/hdfs_disk_read_task_desc.h"
#include "common/task_desc/hdfs_network_task_desc.h"
#include "common/task_desc/task_desc.h"

namespace axe {
namespace common {

class ShardedSourceData {
 public:
  ShardedSourceData(TaskIdType task_id, ShardIdType shard_id, const std::shared_ptr<TaskDesc>& task_desc, ResourceType type)
      : task_id_(task_id), shard_id_(shard_id), task_desc_(task_desc), resource_type_(type) {}
  const std::shared_ptr<TaskDesc>& GetTaskDesc() const { return task_desc_; }

  const std::string& GetLocality() const { return task_desc_->GetLocality(); }

  double GetInputDataSize() const {
    if (std::dynamic_pointer_cast<HdfsDiskReadTaskDesc>(task_desc_) != nullptr) {
      return std::dynamic_pointer_cast<HdfsDiskReadTaskDesc>(task_desc_)->GetInputDataSize();
    }
    DCHECK(std::dynamic_pointer_cast<HdfsNetworkTaskDesc>(task_desc_) != nullptr);
    return std::dynamic_pointer_cast<HdfsNetworkTaskDesc>(task_desc_)->GetInputDataSize();
  }

 private:
  TaskIdType task_id_;
  ShardIdType shard_id_;
  ResourceType resource_type_;
  std::shared_ptr<TaskDesc> task_desc_;
};

}  // namespace common
}  // namespace axe
