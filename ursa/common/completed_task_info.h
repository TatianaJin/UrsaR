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

#include "base/bin_stream.h"
#include "common/constants.h"
#include "common/data_memory_record.h"
#include "common/instance_id.h"

namespace axe {
namespace common {

struct CompletedTaskInfo {
  int ms;
  TaskIdType task_id;
  ShardIdType shard_id;
  InstanceId instance_id;
  std::string locality;  // jp addr
  InstanceId watermark;
  bool has_watermark;
  DataMemory data_memory;
  friend base::BinStream& operator<<(base::BinStream& binstream, const CompletedTaskInfo& info) {
    binstream << info.ms << info.task_id << info.shard_id << info.instance_id << info.locality;
    binstream << info.has_watermark;
    if (info.has_watermark)
      binstream << info.watermark;
    binstream << info.data_memory;
    return binstream;
  }
  friend base::BinStream& operator>>(base::BinStream& binstream, CompletedTaskInfo& info) {
    binstream >> info.ms >> info.task_id >> info.shard_id >> info.instance_id >> info.locality;
    binstream >> info.has_watermark;
    if (info.has_watermark)
      binstream >> info.watermark;
    binstream >> info.data_memory;
    return binstream;
  }
};

}  // namespace common
}  // namespace axe
