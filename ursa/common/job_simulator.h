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

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "glog/logging.h"

#ifdef WITH_GPERF
#include "gperftools/profiler.h"
#endif

#include "common/job_driver.h"
#include "common/resource_request.h"
#include "job_manager/simulation_job_manager.h"
#include "job_process/simulation_job_process.h"

#ifdef WITH_GPERF
DEFINE_string(gperf, "/tmp/axe_prof", "gperf output path");
#endif

namespace axe {
namespace common {

using base::BinStream;
using job_manager::SimulationJobManager;
using job_manager::SimulationJobManagerCommEnv;
using job_process::SimulationChannelService;
using job_process::SimulationJobProcess;
using job_process::SimulationJobProcessCommEnv;

using MsgQueue = network::ThreadsafeQueue<std::shared_ptr<BinStream>>;

namespace SimulationConnector {

std::thread AssignTask(MsgQueue* jm_queue, const std::map<std::string, MsgQueue*>& jp_queues, MsgQueue* jm_receive_queue) {
  return std::thread([=]() {
    while (true) {
      auto payload = jm_queue->WaitAndPop();
      auto to_scheduler = std::make_shared<BinStream>(*payload);
      common::SchedulerEventType event_type;
      *to_scheduler >> event_type;
      if (event_type == common::SchedulerEventType::SchedulerRequestResource) {
        ResourceRequest req;
        *to_scheduler >> req;
        auto to_jm = std::make_shared<BinStream>();
        *to_jm << common::JobManagerEventType::JMReceiveAllocation;
        *to_jm << req.GetReqId();
        if (req.GetLocality().size() > 0)
          *to_jm << req.GetLocality();
        else
          *to_jm << jp_queues.begin()->first;
        jm_receive_queue->Push(to_jm);
      } else {
        CHECK(event_type == common::SchedulerEventType::SchedulerFinishJob) << "Unexpected type " << event_type;
        JobIdType job_id;
        *to_scheduler >> job_id;
        google::FlushLogFiles(google::INFO);
        return;
      }
    }
  });
}

std::thread TaskComplete(MsgQueue* jp_queue, MsgQueue* jm_queue) {
  return std::thread([=]() {
    while (true) {
      auto payload = jp_queue->WaitAndPop();
      auto to_worker = std::make_shared<BinStream>(*payload);
      common::WorkerEventType event_type;
      int ms;
      JobIdType job_id;
      ResourceType resource_type;

      *to_worker >> event_type;
      CHECK_EQ(event_type, common::WorkerEventType::WorkerTaskComplete);
      *to_worker >> ms >> job_id >> resource_type;
      auto to_jm = std::make_shared<base::BinStream>();
      *to_jm << common::JobManagerEventType::JMTaskFinish;
      to_jm->append(*to_worker);
      jm_queue->Push(to_jm);
    }
  });
}

std::thread Deliver(MsgQueue* src_queue, MsgQueue* dst_queue) {
  return std::thread([=]() {
    while (true) {
      dst_queue->Push(src_queue->WaitAndPop());
    }
  });
}

std::thread SendTask(MsgQueue* worker_queue, MsgQueue* jp_queue) {
  return std::thread([=]() {
    while (true) {
      auto payload = worker_queue->WaitAndPop();
      common::WorkerEventType event;
      *payload >> event;
      CHECK_EQ(event, common::WorkerEventType::WorkerRecvTaskDesc);
      TaskDescWrapper wrapper;
      *payload >> wrapper;
      auto to_jp = std::make_shared<BinStream>();
      *to_jp << common::JobProcessEventType::JPAssignTask;
      *to_jp << wrapper;
      jp_queue->Push(to_jp);
    }
  });
}

}  // namespace SimulationConnector

class SimulationJob {
 public:
  SimulationJob(int argc, char** argv, uint32_t n_job_processes, const Job& job) {
#ifdef AXE_DEBUG_MODE
    FLAGS_logbuflevel = -1;  // -1 means don't buffer.
#endif
    FLAGS_config_file = "./test/resources/sql.ini";
    google::InitGoogleLogging(argv[0]);
    google::LogToStderr();
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    Init(n_job_processes, job);
  }

  SimulationJob(uint32_t n_job_processes, const Job& job) {
#ifdef AXE_DEBUG_MODE
    FLAGS_logbuflevel = -1;  // -1 means don't buffer.
#endif
    google::InitGoogleLogging("JobSimulation");
    google::LogToStderr();

    Init(n_job_processes, job);
  }

  std::shared_ptr<Properties> GetConfig(PropertiesReader* reader) {
    if (FLAGS_config_file == "") {
      return nullptr;
    }
    if (reader == nullptr) {
      return std::make_shared<Properties>(PropertiesReader().Read(FLAGS_config_file));
    }
    return std::make_shared<Properties>(reader->Read(FLAGS_config_file));
  }

  void Init(uint32_t n_job_processes, const Job& job, PropertiesReader* reader = nullptr) {
#ifdef WITH_GPERF
    ProfilerStart(FLAGS_gperf.c_str());
#endif
    TaskGraph task_graph;
    job.Run(&task_graph, GetConfig(reader));

    SimulationJobManagerCommEnv jm_comm_env;
    SimulationJobManager job_manager(&jm_comm_env);
    job_manager.SetTaskGraph(task_graph);

    std::vector<std::string> jp_addrs(n_job_processes);
    uint32_t i = 0;
    for (auto& addr : jp_addrs) {
      addr = "jp:" + std::to_string(i++);
    }

    std::map<std::string, MsgQueue*> jp_queues;
    // connect jp to jm
    for (i = 0; i < n_job_processes; ++i) {
      auto& my_addr = jp_addrs[i];
      auto& jp_comm_env = jp_comm_envs_[my_addr];
      auto& jp = *(jps_.insert({my_addr, std::make_shared<SimulationJobProcess>(&jp_comm_env, my_addr)}).first->second);

      job_manager.AddJobProcess("jp", i, i);
      threads_.emplace_back(SimulationConnector::TaskComplete(jp_comm_env.GetLocalWorkerQueue(), job_manager.GetQueue()));
      threads_.emplace_back(SimulationConnector::SendTask(jm_comm_env.GetToWorkersQueue(my_addr), jp.GetQueue()));
      jp_queues.insert({my_addr, jp.GetQueue()});

      jp.SetClosureMap(task_graph.GetClosureMap());
      jp.Serve();
    }

    auto wait_thread = SimulationConnector::AssignTask(jm_comm_env.GetSchedulerQueue(), jp_queues, job_manager.GetQueue());

    // channel service
    for (i = 0; i < n_job_processes; ++i) {
      auto& my_addr = jp_addrs[i];
      auto& jp_comm_env = jp_comm_envs_[my_addr];
      for (auto& addr : jp_addrs) {
        jp_comm_env.AddJobProcess(addr);
        threads_.emplace_back(SimulationConnector::Deliver(jp_comm_env.GetChannelServiceQueue(addr), jps_.at(addr)->GetChannelServiceQueue()));
      }
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    job_manager.Serve();

    wait_thread.join();
    auto end_time = std::chrono::high_resolution_clock::now();
    LOG(INFO) << "Simulation job completion time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms";
#ifdef WITH_GPERF
    ProfilerStop();
#endif
    exit(0);
    // the program does not exit as jm and jps are not shutdown
  }

 private:
  std::map<std::string, SimulationJobProcessCommEnv> jp_comm_envs_;
  std::map<std::string, std::shared_ptr<SimulationJobProcess>> jps_;
  std::vector<std::thread> threads_;
};

}  // namespace common
}  // namespace axe
