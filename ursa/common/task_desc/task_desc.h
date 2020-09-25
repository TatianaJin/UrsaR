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
#include <sstream>
#include <string>

#include "base/bin_stream.h"
#include "common/constants.h"
#include "common/instance_id.h"

namespace axe {
namespace common {

class TaskDesc {
 public:
  TaskDesc() {}
  TaskDesc(JobIdType job_id, TaskIdType task_id, ShardIdType shard_id, ResourceType type)
      : job_id_(job_id), task_id_(task_id), shard_id_(shard_id), resource_type_(type) {}
  virtual ~TaskDesc() {}

  inline const auto& GetResourceType() const { return resource_type_; }
  inline const auto& GetJobId() const { return job_id_; }
  inline const auto& GetTaskId() const { return task_id_; }
  inline const auto& GetShardId() const { return shard_id_; }
  inline const auto& GetInstanceId() const { return instance_id_; }
  inline const auto& GetLocality() const { return locality_; }
  virtual TaskIdType GetPriority() const { return task_id_; }

  inline void SetLocality(const std::string& locality) { locality_ = locality; }
  inline void SetInstanceId(const std::shared_ptr<InstanceId>& instance_id) { instance_id_ = instance_id; }
  inline void SetResourceType(ResourceType type) { resource_type_ = type; }

  inline std::string DebugString(bool print_instance = false) const {
    std::stringstream ss;
    ss << "Task " << job_id_ << "." << task_id_ << "." << shard_id_ << " Resource " << resource_type_;
    if (print_instance && instance_id_ != nullptr) {
      ss << " " << instance_id_->ToString();
    }
    return ss.str();
  }

  virtual base::BinStream& serialize(base::BinStream& bin_stream) const;
  virtual base::BinStream& deserialize(base::BinStream& bin_stream);

  double GetResourceUsage() const { return resource_usage_; }
  void SetResourceUsage(double usage) { resource_usage_ = usage; }

 protected:
  common::ResourceType resource_type_;
  double resource_usage_ = 1.;

 private:
  TaskIdType task_id_;
  int job_id_;
  ShardIdType shard_id_;
  std::shared_ptr<InstanceId> instance_id_ = std::make_shared<InstanceId>();
  std::string locality_ = "";
};

}  // namespace common
}  // namespace axe
