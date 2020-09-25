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

#include <fstream>
#include <string>
#include <vector>

#include "glog/logging.h"
#include "nlohmann/json.hpp"

#include "common/constants.h"
#include "common/task_desc/task_desc.h"
#include "common/task_graph.h"

namespace axe {
namespace common {

using nlohmann::json;

/**
 * Job info format
 * {
 *    "job_id":0,
 *    "user_id":"",
 *    "submission_time":0,
 *    "finish_time":0,
 *    "task":[
 *       {
 *          "task_id":0,
 *          "parallelism":200,
 *          "children":[1],
 *          "child_types":[0]
 *       }
 *    ]
 * }
 *
 *
 * Task shard info format
 * {
 *    "job_id":0,
 *    "shard_task":[
 *       {
 *          "task_id":0,
 *          "shard_id":0,
 *          "instance_id":0,
 *          "resource_type":2,
 *          "memory":2,          # not yet
 *          "resource_req":5,
 *          "runnable": 2999934
 *          "submitted": 2999955
 *          "assigned": 3000000,
 *          "finished": 3741000,
 *          "cpu_time": 741000,
 *          ...
 *       }
 *    ]
 * }
 */
class SimulationJobLogger {
 public:
  SimulationJobLogger(JobIdType job_id, const std::string& file);

  // @Worker
  void TaskAssigned(const TaskDesc& task_desc);
  void TaskFinished(TaskIdType tid, ShardIdType sid, const InstanceId& instance);
  void SetTaskTime(TaskIdType tid, ShardIdType sid, const InstanceId& instance, int ms);
  void TaskRunnable(const TaskDesc& task_desc);  // when task arrives at worker
  void TasksRunnable(const std::vector<std::shared_ptr<TaskDesc>>& task_descs);

  // @JM
  void TaskSubmitted(const TaskDesc& task_desc);
  void JobFinished(const TaskGraph&, const std::map<DataIdType, std::vector<std::shared_ptr<ShardedMetadata>>>* data = nullptr);
  void JMLaunched() { job_submission_time_ = Now(); }

  void Flush();

 protected:
  inline int64_t Now() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  }

  inline void AppendTaskInfo(TaskIdType tid, ShardIdType sid, const InstanceId& instance) {
    *ss_ << tid << "." << sid << "." << (instance.Size() == 0 ? 0 : instance.Get(0));
  }
  void AppendTaskTime(TaskIdType tid, ShardIdType sid, const InstanceId& instance);
  void FlushInternal();

  std::shared_ptr<std::ofstream> out_;
  std::string file_;
  json j_info_;
  JobIdType job_id_;
  int64_t job_submission_time_ = 0;

  std::shared_ptr<std::stringstream> ss_;
  const int flush_interval_ = 10000;
  int line_count_ = 0;
};

class SimulationJobLoggers {
 public:
  explicit SimulationJobLoggers(const std::string& hostname) { prefix_ = hostname + "."; }
  void SetTaskTime(JobIdType jid, TaskIdType tid, ShardIdType sid, const InstanceId& instance, int ms) {
    GetLogger(jid).SetTaskTime(tid, sid, instance, ms);
  }
  void TaskFinished(JobIdType jid, TaskIdType tid, ShardIdType sid, const InstanceId& instance) { GetLogger(jid).TaskFinished(tid, sid, instance); }

  void TaskAssigned(const TaskDesc& task_desc) { GetLogger(task_desc.GetJobId()).TaskAssigned(task_desc); }
  void TaskRunnable(const TaskDesc& task_desc) { GetLogger(task_desc.GetJobId()).TaskRunnable(task_desc); }

  void Flush(JobIdType jid) { GetLogger(jid).Flush(); }

 private:
  SimulationJobLogger& GetLogger(JobIdType jid) {
    auto pos = loggers_.find(jid);
    if (pos == loggers_.end()) {
      loggers_.insert({jid, SimulationJobLogger(jid, prefix_ + std::to_string(jid) + surfix_)});
      return loggers_.at(jid);
    }
    return pos->second;
  }
  std::unordered_map<JobIdType, SimulationJobLogger> loggers_;
  std::string prefix_;
  std::string surfix_ = ".log";
};

}  // namespace common
}  // namespace axe
