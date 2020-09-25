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

class ResourceRequest {
 public:
  ResourceRequest() = default;
  ResourceRequest(JobIdType job_id, int req_id, const std::string& locality = "") : job_id_(job_id), req_id_(req_id), locality_(locality) {}
  ResourceRequest(JobIdType job_id, int req_id, const std::string& locality, const ResourcePack& resource, bool relaxable_locality = false)
      : job_id_(job_id), req_id_(req_id), locality_(locality), resource_(resource), relaxable_locality_(relaxable_locality) {}

  const std::string& GetLocality() const { return locality_; }
  JobIdType GetJobId() const { return job_id_; }
  int GetReqId() const { return req_id_; }

  const auto& GetResource() const { return resource_; }
  void SetResource(const ResourcePack& resource) { resource_ = resource; }

  void SetScore(double score) { score_ = score; }
  double GetScore() const { return score_; }

  void SetMemory(double memory) { resource_.Memory() = memory; }

  void SetBarrierScore(double score) { barrier_score_ = score; }
  double GetBarrierScore() const { return barrier_score_; }

  void SetCpuPortion(float cpu_portion) { cpu_portion_ = cpu_portion; }
  void SetNetworkPortion(float net_portion) { net_portion_ = net_portion; }
  void SetDiskPortion(float disk_portion) { disk_portion_ = disk_portion; }

  double GetCpuPortion() const { return cpu_portion_; }
  double GetNetworkPortion() const { return net_portion_; }
  double GetDiskPortion() const { return disk_portion_; }
  bool GetRelaxableLocality() const { return relaxable_locality_; }

  friend base::BinStream& operator<<(base::BinStream& bin_stream, const ResourceRequest& req) {
    bin_stream << req.job_id_ << req.req_id_ << req.locality_ << req.resource_ << req.score_ << req.barrier_score_ << req.cpu_portion_
               << req.net_portion_ << req.disk_portion_;
    return bin_stream;
  }
  friend base::BinStream& operator>>(base::BinStream& bin_stream, ResourceRequest& req) {
    bin_stream >> req.job_id_ >> req.req_id_ >> req.locality_ >> req.resource_ >> req.score_ >> req.barrier_score_ >> req.cpu_portion_ >>
        req.net_portion_ >> req.disk_portion_;
    return bin_stream;
  }

 private:
  ResourcePack resource_;
  JobIdType job_id_;
  int req_id_;
  std::string locality_;
  float cpu_portion_ = 0;
  float net_portion_ = 0;
  float disk_portion_ = 0;
  float score_ = 0;          // from jm
  float barrier_score_ = 0;  // from jm
  bool relaxable_locality_ = false;
};

}  // namespace common
}  // namespace axe
