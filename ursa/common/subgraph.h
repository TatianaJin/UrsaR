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

#include <algorithm>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/constants.h"
#include "common/resource_pack.h"
#include "common/resource_request.h"
#include "common/resource_usage_estimator.h"
#include "common/sharded_source_data.h"
#include "common/sharded_task.h"
#include "common/task_desc/task_desc.h"

namespace axe {
namespace common {

class Subgraph {
 public:
  Subgraph(JobIdType job_id, int subgraph_id, ShardIdType shard_id,
           std::map<DataIdType, std::vector<std::shared_ptr<ShardedMetadata>>>* sharded_metadata_map, int parallelism,
           ResourceType load_data_task_type = ResourceType::CPU);

  inline void SetResourceUsageEstimator(ResourceUsageEstimator* estimator) {
    DCHECK(estimator != nullptr);
    resource_usage_estimator_ = estimator;
  }
  inline void SetTask(std::map<TaskIdType, ShardedTask>&& tasks) { tasks_ = std::move(tasks); }
  inline void SetReqId(int req_id) { req_id_ = req_id; }
  inline void AddShardedSourceData(TaskIdType task_id, const ShardedSourceData& source_data) { source_data_.insert({task_id, source_data}); }
  inline void SetLocality(const std::string& locality) { locality_ = locality; }
  inline void SetBroadcastExecutor(bool if_executor = true) { broadcast_executor_ = if_executor; }

  inline const ShardedTask& GetTask(TaskIdType task_id) const { return tasks_.at(task_id); }
  inline int GetSubgraphId() const { return subgraph_id_; }
  inline ShardIdType GetShardId() const { return shard_id_; }
  inline int GetReqId() const { return req_id_; }
  inline ResourcePack GetResourceUsage() const { return resource_usage_; }
  inline double GetRealCPUUsage() const { return real_cpu_usage_; }
  inline double GetRealNetUsage() const { return real_net_usage_; }
  inline bool IsFinished() const { return task_finished_counter_ == tasks_.size(); }
  inline bool HasInput() const { return input_data_size_ > 0; }
  inline std::string GetLocality() const { return locality_; }  // TODO(tatiana): return const ref?

  void AddDependency(TaskIdType parent_id, TaskIdType child_id);

  ResourceRequest GetResourceRequest(int req_id, std::map<std::string, std::string>* jp_to_worker_map) const;

  std::string GetResourceRequestLocality() const;

  std::vector<std::shared_ptr<TaskDesc>> GetInitTaskDesc(const std::string& locality, bool except_broadcast = 0,
                                                         std::map<TaskIdType, int>* task_finish_counter = nullptr,
                                                         std::vector<TaskIdType>* broadcast_id = nullptr);

  std::vector<std::shared_ptr<TaskDesc>> TaskFinish(TaskIdType task_id, const std::string& locality, double usage = 0);

  bool CanSkip(TaskIdType task_id) const;

  double GetTaskInputSize(TaskIdType task_id, const ShardedTask& task) const;

  void EstimateResourceUsage(double network_bandwith = 0, double disk_bandwith = 0, double memory = 0);

  std::set<std::pair<DataIdType, ShardIdType>> GetRemainingData() const;

 private:
  // dummy instance id now
  std::shared_ptr<TaskDesc> GetTaskDescInternal(TaskIdType task_id, const std::string& locality);

  void GetTopoOrderInternal();

  std::string GetLocalityDependency() const;

  double real_cpu_usage_ = 0, real_net_usage_ = 0;
  JobIdType job_id_;
  std::map<DataIdType, std::vector<std::shared_ptr<ShardedMetadata>>>* sharded_metadata_map_;
  std::string locality_;  // worker addr

  int subgraph_id_;
  int req_id_;
  ShardIdType shard_id_;
  std::map<TaskIdType, TaskDependencyType> tasks_dependency_type_;
  std::map<TaskIdType, ShardedTask> tasks_;
  std::map<TaskIdType, ShardedSourceData> source_data_;
  std::map<TaskIdType, double> predicted_output_size_;
  std::map<TaskIdType, double> task_resource_usage_;
  std::map<TaskIdType, double> task_input_data_size_;
  int task_finished_counter_;
  int parallelism_;

  std::map<TaskIdType, int> task_dep_;
  std::map<TaskIdType, std::vector<TaskIdType>> dep_;
  std::map<TaskIdType, std::vector<TaskIdType>> parents_;
  std::map<TaskIdType, int> task_finished_dep_;

  std::vector<TaskIdType> topo_order_;
  ResourcePack resource_usage_;
  double input_data_size_ = 0;

  ResourceType load_data_task_type_;
  ResourceUsageEstimator* resource_usage_estimator_;  // not owned
  bool broadcast_executor_ = false;
};

}  // namespace common
}  // namespace axe
