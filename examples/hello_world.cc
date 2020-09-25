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

#include "glog/logging.h"

#include "common/closure.h"
#include "common/constants.h"
#include "common/job_driver.h"
#include "common/task.h"
#include "common/task_graph.h"

using axe::base::Properties;
using axe::common::Closure;
using axe::common::Job;
using axe::common::ResourceType;
using axe::common::TaskContext;
using axe::common::TaskGraph;

class BasicConstructsJob : public Job {
 public:
  void Run(TaskGraph* tg, const std::shared_ptr<Properties>& config) const override {
    auto task = tg->CreateTask("A CPU task", ResourceType::CPU);
    tg->RegisterClosure(task->GetId(), Closure::CreateClosure([](TaskContext* tc) { LOG(INFO) << "A Hello World!"; }));
    task->SetParallelism(10);

    auto task1 = tg->CreateTask("B CPU task", ResourceType::CPU);
    tg->RegisterClosure(task1->GetId(), Closure::CreateClosure([](auto*) { LOG(INFO) << "B Hello World!"; }));
    task->SyncThen(task1);
    task1->SetParallelism(10);

    axe::common::JobDriver::PrintTaskGraph(*tg);
  }
};

int main(int argc, char** argv) { return axe::common::JobDriver::Run(argc, argv, BasicConstructsJob()); }
