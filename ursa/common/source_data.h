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
#include <utility>
#include <vector>

#include "common/constants.h"
#include "common/io/input/input_block_assigner.h"
#include "common/io/input/input_block_info.h"
#include "common/task_desc/hdfs_disk_read_task_desc.h"
#include "common/task_desc/hdfs_network_task_desc.h"
#include "common/task_desc/task_desc.h"

namespace axe {
namespace common {

// Only consider hdfs first
// May extend to the different type of data source later
class SourceData {
 public:
  SourceData(const std::shared_ptr<common::AbstractInputBlockInfo>& input_block_info, const std::shared_ptr<Task>& task)
      : task_(task), input_block_info_(input_block_info) {}

  static const std::vector<std::pair<std::string, size_t>>& GetBlockDesc(const std::shared_ptr<TaskDesc>& task_desc) {
    if (std::dynamic_pointer_cast<HdfsNetworkTaskDesc>(task_desc) != nullptr) {
      return std::dynamic_pointer_cast<HdfsNetworkTaskDesc>(task_desc)->GetBlockDesc();
    } else {
      CHECK(std::dynamic_pointer_cast<HdfsDiskReadTaskDesc>(task_desc) != nullptr) << "Unknown hdfs fetch type";
      return std::dynamic_pointer_cast<HdfsDiskReadTaskDesc>(task_desc)->GetBlockDesc();
    }
  }
  const auto& GetTask() const { return task_; }
  const auto& GetUrl() const { return input_block_info_->GetUrl(); }
  const auto& GetTaskDescs() const { return task_descs_; }

  void SetInputBlockInfoCache(InputBlockInfoCache* cache) { input_block_info_->SetInputBlockInfoCache(cache); }

  virtual void BuildTaskDesc(JobIdType job_id, const std::vector<std::pair<std::string, size_t>>& job_processes, size_t parallelism) {
    auto start = std::chrono::steady_clock::now();
    input_block_info_->FetchBlocksInfo();
    auto end = std::chrono::steady_clock::now();
    common::InputBlockAssigner assigner(job_id, task_->GetId(), task_->GetName());
    LOG(INFO) << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms for " << input_block_info_->GetUrls().size()
              << " urls, input blocks " << input_block_info_->GetBlocks().size() << " - " << input_block_info_->GetUrl();
    task_descs_ = assigner.Assigning(job_processes, input_block_info_, parallelism);
    DCHECK_EQ(task_descs_.size(), parallelism);
  }

  const std::shared_ptr<common::AbstractInputBlockInfo>& GetInputBlockInfo() const { return input_block_info_; }

 protected:
  std::shared_ptr<common::AbstractInputBlockInfo> input_block_info_;
  std::vector<std::shared_ptr<TaskDesc>> task_descs_;
  std::shared_ptr<Task> task_;
};

}  // namespace common
}  // namespace axe
