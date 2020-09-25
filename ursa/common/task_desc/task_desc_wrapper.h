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

#include "base/bin_stream.h"
#include "common/constants.h"
#include "common/task_desc/channel_network_task_desc.h"
#include "common/task_desc/disk_read_task_desc.h"
#include "common/task_desc/hdfs_disk_read_task_desc.h"
#include "common/task_desc/hdfs_network_task_desc.h"
#include "common/task_desc/network_task_desc.h"
#include "common/task_desc/task_desc.h"

namespace axe {
namespace common {

// Wrapper of task desc to implement serialization and deserialization
class TaskDescWrapper {
 public:
  enum class TaskDescType : uint32_t { HdfsNetworkTaskDesc = 0, HdfsDiskReadTaskDesc, CPUTaskDesc, ChannelNetworkTaskDesc };
  TaskDescWrapper() = default;
  explicit TaskDescWrapper(const std::shared_ptr<TaskDesc>& task_desc);

  base::BinStream& serialize(base::BinStream& bin_stream) const;
  base::BinStream& deserialize(base::BinStream& bin_stream);

  auto& GetTaskDesc() const { return task_desc_; }
  TaskDescType GetType() const { return type_; }

 private:
  TaskDescType type_;
  std::shared_ptr<TaskDesc> task_desc_;
};

}  // namespace common
}  // namespace axe
