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

#include <cstdint>
#include <string>
#include <utility>

namespace axe {
namespace common {

enum ResourceType : uint32_t { CPU = 0, NetWork, Disk, GPU };

enum MailboxReservedChannel : uint32_t {
  Worker = 1000,
  JobManager,
  Master,
  JobProcess,
  Scheduler,
  ChannelService,  // now only one handler thread for user data channel
};

using ClientEventType = int;
const ClientEventType ClientShutdown = 0;
const ClientEventType ClientNewJob = 1;

enum MasterEventType : uint32_t {
  MasterShutDown = 0,
  MasterAddWorker,
  MasterAddScheduler,
};

enum SchedulerEventType : uint32_t {
  SchedulerShutDown = 0,
  SchedulerAddWorkers,
  SchedulerAddJobManager,
  SchedulerNewJob,
  SchedulerFinishJob,
  SchedulerRequestResource,
  SchedulerReleaseResource,
  SchedulerSchedule,
  SchedulerSchedEffReport,
};

enum JobProcessEventType : uint32_t {
  JPShutDown = 100,
  JPAssignTask,
  JPTaskFinish,
  JPAddJobProcess,
  JPRequestData,
  JPRequestDataFinish,
  JPReceiveData,
  JPDeleteData,
  JPChangeResource,
};

enum WorkerEventType : uint32_t {
  WorkerBroadcastWorkerProcessId = 200,
  WorkerRemoveJobProcess,
  WorkerAddJobProcess,
  WorkerShutDown,
  WorkerTaskComplete,
  WorkerRecvTaskDesc,
  WorkerStartJobManager,
  WorkerStopJobManager,
  WorkerStartJobProcess,
  WorkerStopJobProcess,
  WorkerReceiveFile,
  WorkerUpdateTaskStatus,
  WorkerChannelService,
  WorkerPrioritizeJob,
  WorkerAddScheduler,
};

enum class TaskDependencyType : uint32_t {
  Sync,            // each child shard depends on all parent shards
  Async,           // each child shard depends on its corresponding parent shard
  AsyncComm,       // one parent shard triggers all child shards once, add instance dim
  Aggregate,       // each child shard collects all instances of its parent shard, remove instance dim
  Broadcast,       // broadcast
  LocalAggregate,  // LocalAggregate, Sync
};

const uint64_t kJobFileChunkSize = 65536;  // 65536 = 1024 * 64 ~ 64k.

enum JobManagerEventType : uint32_t {
  JMShutDown = 0,
  JMAddJobProcess,
  JMTaskFinish,
  JMFinishJob,
  JMReceiveAllocation,
};

enum DataStatus : uint32_t {
  NotCreated,
  InMemory,
  InFile,
  Cleaned,
};

// NOTE: Unscoped enum needs to avoid redeclaration but supports implicit convertion.

using TaskIdType = uint32_t;
using ShardIdType = uint32_t;
using DataIdType = uint32_t;
using JobIdType = int;
using TaskNameType = std::string;

// HDFS Block
using Block = std::pair<std::string, size_t>;
enum FS : uint8_t { HDFS, NFS };

}  // namespace common
}  // namespace axe
