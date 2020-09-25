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

#include "common/constants.h"
#include "common/task_desc/task_desc.h"

namespace axe {
namespace common {

class DiskReadTaskDesc : public TaskDesc {
 public:
  DiskReadTaskDesc() = default;
  DiskReadTaskDesc(JobIdType job_id, TaskIdType task_id, ShardIdType shard_id) : TaskDesc(job_id, task_id, shard_id, ResourceType::Disk) {}

  ~DiskReadTaskDesc() override = default;

  base::BinStream& serialize(base::BinStream& bin_stream) const override { return TaskDesc::serialize(bin_stream); }

  base::BinStream& deserialize(base::BinStream& bin_stream) override { return TaskDesc::deserialize(bin_stream); }
};

}  // namespace common
}  // namespace axe
