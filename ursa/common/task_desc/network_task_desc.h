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

class NetworkTaskDesc : public TaskDesc {
 public:
  NetworkTaskDesc() = default;
  NetworkTaskDesc(JobIdType job_id, TaskIdType task_id, ShardIdType shard_id, double network_usage)
      : TaskDesc(job_id, task_id, shard_id, ResourceType::NetWork) {}

  virtual ~NetworkTaskDesc() = default;

  base::BinStream& serialize(base::BinStream& bin_stream) const override {
    TaskDesc::serialize(bin_stream);
    return bin_stream;
  }

  base::BinStream& deserialize(base::BinStream& bin_stream) override {
    TaskDesc::deserialize(bin_stream);
    return bin_stream;
  }
};

}  // namespace common
}  // namespace axe
