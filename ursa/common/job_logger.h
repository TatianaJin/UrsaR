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

#include "common/constants.h"
#include "common/simulation_job_logger.h"
#include "common/task_desc/task_desc.h"
#include "common/task_graph.h"

namespace axe {
namespace common {

#if JobAnalysis == 0
#define JobLogger DummyJobLogger
#define JobLoggers DummyJobLoggers
#elif JobAnalysis == 1
#define JobLogger SimpleJobLogger
#define JobLoggers SimpleJobLoggers
#elif JobAnalysis == 2
#define JobLogger SimulationJobLogger
#define JobLoggers SimulationJobLoggers
#endif

class DummyJobLogger {
 public:
  DummyJobLogger(JobIdType, const std::string&) {}

  // @Worker
  void TaskAssigned(const TaskDesc& task_desc) {}
  void TaskFinished(TaskIdType tid, ShardIdType sid, const InstanceId& instance) {}
  void SetTaskTime(TaskIdType tid, ShardIdType sid, const InstanceId& instance, int ms) {}

  // @JM
  void TaskRunnable(const TaskDesc& task_desc) {}
  void TasksRunnable(const std::vector<std::shared_ptr<TaskDesc>>& task_descs) {}
  void TaskSubmitted(const TaskDesc& task_desc) {}
  void JMLaunched() {}
  void JobFinished(const TaskGraph&, const void*) {}

  void Flush() {}
};

class DummyJobLoggers {
 public:
  explicit DummyJobLoggers(const std::string& hostname) {}
  void SetTaskTime(JobIdType jid, TaskIdType tid, ShardIdType sid, const InstanceId& instance, int ms) {}
  void TaskFinished(JobIdType jid, TaskIdType tid, ShardIdType sid, const InstanceId& instance) {}
  void TaskAssigned(const TaskDesc& task_desc) {}
  void Flush(JobIdType jid) {}
  void TaskRunnable(const TaskDesc& task_desc) {}
};

class SimpleJobLogger {
 public:
  SimpleJobLogger(JobIdType job_id, const std::string& file)
      : file_(file), out_(std::make_shared<std::ofstream>()), ss_(std::make_shared<std::stringstream>()) {
    out_->open(file_.c_str());
    CHECK(out_->is_open()) << "Failed to open " << file << " for writing.";
  }

  // @Worker
  void TaskAssigned(const TaskDesc& task_desc);
  void TaskFinished(TaskIdType tid, ShardIdType sid, const InstanceId& instance);
  void SetTaskTime(TaskIdType tid, ShardIdType sid, const InstanceId& instance, int ms);

  // @JM
  void TaskRunnable(const TaskDesc& task_desc);
  void TasksRunnable(const std::vector<std::shared_ptr<TaskDesc>>& task_descs);
  void TaskSubmitted(const TaskDesc& task_desc);
  void JMLaunched();
  void JobFinished(const TaskGraph&, const void*);

  void Flush();

 protected:
  void FlushInternal();
  void AppendTaskTime(TaskIdType tid, ShardIdType sid, const InstanceId& instance);
  void AppendTaskTime(const TaskDesc& task_desc);
  void AppendTaskInfo(TaskIdType tid, ShardIdType sid, const InstanceId& instance);
  void AppendTaskInfo(const TaskDesc& task_desc);
  std::shared_ptr<std::stringstream> ss_;
  std::shared_ptr<std::ofstream> out_;
  int line_count_ = 0;
  std::string file_;
  const int flush_interval_ = 10000;
};

class SimpleJobLoggers {
 public:
  explicit SimpleJobLoggers(const std::string& hostname) { prefix_ = hostname + ".job"; }
  void SetTaskTime(JobIdType jid, TaskIdType tid, ShardIdType sid, const InstanceId& instance, int ms) {
    GetLogger(jid).SetTaskTime(tid, sid, instance, ms);
  }
  void TaskFinished(JobIdType jid, TaskIdType tid, ShardIdType sid, const InstanceId& instance) { GetLogger(jid).TaskFinished(tid, sid, instance); }

  void TaskAssigned(const TaskDesc& task_desc) { GetLogger(task_desc.GetJobId()).TaskAssigned(task_desc); }

  void TaskRunnable(const TaskDesc& task_desc) { GetLogger(task_desc.GetJobId()).TaskRunnable(task_desc); }

  void Flush(JobIdType jid) { GetLogger(jid).Flush(); }

 private:
  SimpleJobLogger& GetLogger(JobIdType jid) {
    auto pos = loggers_.find(jid);
    if (pos == loggers_.end()) {
      loggers_.insert({jid, SimpleJobLogger(jid, prefix_ + std::to_string(jid) + surfix_)});
      return loggers_.at(jid);
    }
    return pos->second;
  }
  std::unordered_map<JobIdType, SimpleJobLogger> loggers_;
  std::string prefix_;
  std::string surfix_ = ".log";
};

}  // namespace common
}  // namespace axe
