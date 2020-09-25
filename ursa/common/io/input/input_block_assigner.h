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
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gflags/gflags.h"
#include "hdfs/hdfs.h"
#include "zmq.hpp"

#include "common/constants.h"
#include "common/io/input/input_block_info.h"
#include "common/sharded_task.h"
#include "common/task_desc/task_desc.h"

namespace axe {
namespace common {

class InputBlockAssigner {
 public:
  /**
   * Initialize HDFS
   */
  InputBlockAssigner(JobIdType job_id, TaskIdType task_id, TaskNameType name);

  // pair < job_process, weight>, BlockInfo <filename, host, offset>
  std::vector<std::shared_ptr<TaskDesc>> Assigning(const std::vector<std::pair<std::string, size_t>>& job_processes,
                                                   const std::shared_ptr<common::AbstractInputBlockInfo>& block_info, size_t shard_num);

 private:
  void AssignBlockToProcess(const std::vector<std::pair<std::string, size_t>>& job_processes, size_t shard_num);
  std::vector<std::shared_ptr<TaskDesc>> GenerateShardedTask(const std::vector<std::pair<std::string, size_t>>& job_processes,
                                                             const std::shared_ptr<common::AbstractInputBlockInfo>& block_info);

  static std::string GetPrefix(const std::string& hostname) {
    auto pos = hostname.find(":");
    if (pos == std::string::npos)
      return hostname;
    return hostname.substr(0, pos);
  }

  TaskNameType name_;
  JobIdType job_id_;
  TaskIdType task_id_;
  std::map<std::pair<std::string, size_t>, std::vector<std::string>> block_to_hosts_;
  std::unordered_map<std::string, std::vector<std::pair<std::string, size_t>>> hosts_blocks_disk_;
  std::unordered_map<std::string, std::vector<std::pair<std::string, size_t>>> hosts_blocks_network_;
  std::vector<size_t> each_process_block_num_;
  std::vector<size_t> each_process_shard_num_;
  int num_blocks_ = 0;
};

}  // namespace common
}  // namespace axe
