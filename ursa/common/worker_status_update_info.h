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

#include <vector>

#include "base/bin_stream.h"

namespace axe {
namespace common {

class WorkerStatusUpdateInfo {
 public:
  void SetPendingQueueCount(const std::vector<int>& pending_queue_count) { pending_queue_count_ = pending_queue_count; }
  void SetPendingQueueResourceUsage(const std::vector<double>& pending_queue_resource_usage) {
    pending_queue_resource_usage_ = pending_queue_resource_usage;
  }
  void SetRunningTaskCount(const std::vector<int>& running_task_count) { running_task_count_ = running_task_count; }
  void SetResourceFinish(const std::vector<double>& resource_finish) { resource_finish_ = resource_finish; }

  const std::vector<int>& GetPendingQueueCount() const { return pending_queue_count_; }
  const std::vector<double> GetPendingQueueResourceUsage() const { return pending_queue_resource_usage_; }
  const std::vector<int>& GetRunningTaskCount() const { return running_task_count_; }
  const std::vector<double>& GetResourceFinish() const { return resource_finish_; }

  base::BinStream& serialize(base::BinStream& bin_stream) const {
    bin_stream << pending_queue_count_ << running_task_count_ << pending_queue_resource_usage_ << resource_finish_;
    return bin_stream;
  }

  base::BinStream& deserialize(base::BinStream& bin_stream) {
    bin_stream >> pending_queue_count_ >> running_task_count_ >> pending_queue_resource_usage_ >> resource_finish_;
    return bin_stream;
  }

 private:
  std::vector<int> pending_queue_count_;
  std::vector<double> pending_queue_resource_usage_;
  std::vector<int> running_task_count_;
  std::vector<double> resource_finish_;
};

}  // namespace common
}  // namespace axe
