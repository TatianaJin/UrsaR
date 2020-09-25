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

#include "common/constants.h"
#include "common/instance_id.h"
#include "common/task.h"

namespace axe {
namespace metadata {

using common::DataIdType;
using common::ShardIdType;
using common::TaskIdType;
using common::DataStatus;
using common::InstanceId;

class ShardedMetadata {
 public:
  ShardedMetadata(DataIdType id, ShardIdType shard_id, TaskIdType producer)
      : id_(id), shard_id_(shard_id), producer_(producer), status_(DataStatus::NotCreated) {}

  void ChangeStatus(const InstanceId& instance_id, DataStatus status, const std::string& locality, const std::vector<double>& size_in_bytes) {
    status_ = status;
    locality_ = locality;
    partitioned_size_ = size_in_bytes;
    for (auto s : partitioned_size_) {
      size_in_bytes_ += s;
    }
  }

  DataStatus GetStatus() const { return status_; }
  void Clean() { status_ = DataStatus::Cleaned; }
  inline const std::string& GetLocality() const { return locality_; }
  inline DataIdType GetId() const { return id_; }
  inline ShardIdType GetShardId() const { return shard_id_; }
  inline double GetSize() const { return size_in_bytes_; }
  inline const auto& GetPartitionSize() const { return partitioned_size_; }
  inline TaskIdType GetProducer() const { return producer_; }

 private:
  DataIdType id_;
  ShardIdType shard_id_;
  TaskIdType producer_;
  // TODO(czk): include instance id
  DataStatus status_;
  std::string locality_;
  double size_in_bytes_ = 0;
  std::vector<double> partitioned_size_;
};

}  // namespace metadata
}  // namespace axe
