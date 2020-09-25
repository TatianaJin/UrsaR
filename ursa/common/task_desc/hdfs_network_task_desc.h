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
#include <utility>
#include <vector>

#include "common/task_desc/network_task_desc.h"

namespace axe {
namespace common {

class HdfsNetworkTaskDesc : public NetworkTaskDesc {
 public:
  HdfsNetworkTaskDesc() {}
  HdfsNetworkTaskDesc(JobIdType job_id, TaskIdType task_id, ShardIdType shard_id, const std::vector<std::pair<std::string, size_t>>& block_desc,
                      double network_usage, double input_data_size)
      : NetworkTaskDesc(job_id, task_id, shard_id, network_usage), block_desc_(block_desc), input_data_size_(input_data_size) {
    // Note: change the resource type of reading data task to cpu
    resource_type_ = ResourceType::CPU;
  }

  base::BinStream& serialize(base::BinStream& bin_stream) const override {
    NetworkTaskDesc::serialize(bin_stream);
    size_t size = block_desc_.size();
    bin_stream << size;
    for (auto& blk : block_desc_)
      bin_stream << blk.first << blk.second;
    return bin_stream;
  }

  base::BinStream& deserialize(base::BinStream& bin_stream) override {
    NetworkTaskDesc::deserialize(bin_stream);
    size_t size;
    bin_stream >> size;
    for (int i = 0; i < size; i++) {
      std::string filename;
      size_t offset;
      bin_stream >> filename >> offset;
      block_desc_.push_back(std::make_pair(filename, offset));
    }
    return bin_stream;
  }

  inline const auto& GetBlockDesc() { return block_desc_; }

  double GetInputDataSize() const { return input_data_size_; }

 private:
  double input_data_size_;
  std::vector<std::pair<std::string, size_t>> block_desc_;
};

}  // namespace common
}  // namespace axe
