// Copyright 2020 HDL
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
#include <unordered_map>
#include <vector>

#include "glog/logging.h"

#include "base/properties_reader.h"
#include "common/task_graph.h"

namespace axe {
namespace common {

using base::Properties;
using base::PropertiesReader;

/** The user program interface **/
struct Job {
  /** Specify the application logic in the Run function **/
  virtual void Run(TaskGraph* task_graph, const std::shared_ptr<Properties>& config) const = 0;
};

namespace JobDriver {

#ifdef IsJobManager
void LaunchJobManager(const TaskGraph& task_graph, const std::shared_ptr<Properties>& config);
#else
void LaunchJobProcess(const TaskGraph& task_graph, const std::shared_ptr<Properties>& config);
#endif

int Run(int argc, char** argv, const Job& job, PropertiesReader* reader = nullptr);

void PrintTask(const std::shared_ptr<Task> task, const TaskGraph& task_graph, const std::string& indent);

/* @deprecated Replaced by {@link ReversePrintTaskGraph}*/
void PrintTaskGraph(const TaskGraph& task_graph);

void PrintTask(const std::shared_ptr<Task> task, const TaskGraph& task_graph, std::string indent,
               const std::unordered_map<TaskIdType, std::vector<TaskIdType>>& edges, bool remove_indent = false);

void ReversePrintTaskGraph(const TaskGraph& task_graph);

}  // namespace JobDriver

}  // namespace common
}  // namespace axe
