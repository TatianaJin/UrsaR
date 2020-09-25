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
#include "common/resource_pack.h"

namespace axe {
namespace common {

class ResourceRelease {
 public:
  ResourceRelease() = default;

  ResourceRelease(JobIdType job_id, int req_id, const std::string& worker_addr, const ResourcePack& resource, bool is_subgraph = true)
      : job_id_(job_id), req_id_(req_id), worker_addr_(worker_addr), resource_(resource), is_subgraph_(is_subgraph) {}

  JobIdType GetJobId() const { return job_id_; }
  std::string GetWorkerAddr() const { return worker_addr_; }
  ResourcePack GetResource() const { return resource_; }
  int GetReqId() const { return req_id_; }
  double GetMemory() const { return resource_.GetMemory(); }
  bool IsSubgraph() const { return is_subgraph_; }

  void SetRealCPUUsage(double real_cpu_usage) { real_cpu_usage_ = real_cpu_usage; }
  void SetRealNetUsage(double real_net_usage) { real_net_usage_ = real_net_usage; }
  void SetRemainingDataCount(int count) { remaining_data_count_ = count; }

  double GetRealCPUUsage() const { return real_cpu_usage_; }
  double GetRealNetUsage() const { return real_net_usage_; }
  int GetRemainingDataCount() const { return remaining_data_count_; }

  friend base::BinStream& operator<<(base::BinStream& bin_stream, const ResourceRelease& rel) {
    bin_stream << rel.job_id_ << rel.req_id_ << rel.worker_addr_ << rel.resource_ << rel.is_subgraph_ << rel.real_cpu_usage_ << rel.real_net_usage_;
    return bin_stream;
  }
  friend base::BinStream& operator>>(base::BinStream& bin_stream, ResourceRelease& rel) {
    bin_stream >> rel.job_id_ >> rel.req_id_ >> rel.worker_addr_ >> rel.resource_ >> rel.is_subgraph_ >> rel.real_cpu_usage_ >> rel.real_net_usage_;
    return bin_stream;
  }

 private:
  bool is_subgraph_;
  JobIdType job_id_;
  int req_id_;  // TODO Set req id properly
  std::string worker_addr_;
  ResourcePack resource_;
  double real_cpu_usage_;
  double real_net_usage_;
  int remaining_data_count_ = 0;
};

}  // namespace common
}  // namespace axe
