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
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "glog/logging.h"

#include "base/bin_stream.h"
#include "common/dataset/abstract_data.h"
#include "common/dataset/dataset_partition.h"
#include "common/task.h"
#include "common/task_context.h"
#include "common/task_graph.h"

namespace axe {
namespace common {

using base::BinStream;

template <typename Key>
auto hash(Key key) {
  return std::hash<Key>{}(key);
}

class AbstractDataset {
 public:
  static DatasetPartition<std::shared_ptr<BinStream>> CreateMessagePartition(int num_partitions, DataIdType msg_id, ShardIdType shard_id,
                                                                             const std::shared_ptr<InstanceId>& instance_id) {
    DatasetPartition<std::shared_ptr<BinStream>> msg(num_partitions);
    for (ShardIdType i = 0; i < num_partitions; ++i) {
      msg[i].reset(new BinStream());
      *msg[i] << common::JobProcessEventType::JPReceiveData;
      *msg[i] << msg_id << shard_id << instance_id << i;  // msg data id, sender shard id, msg instance id (timestamp), destination shard id
    }
    return msg;
  }

  explicit AbstractDataset(TaskGraph* tg) : task_graph_(tg), id_(tg->CreateDataset()) {}
  AbstractDataset(const std::shared_ptr<Task>& producer, TaskGraph* task_graph) : task_graph_(task_graph), id_(task_graph->CreateDataset()) {
    SetProducer(producer);
  }

  inline DataIdType GetId() const { return id_; }
  inline int GetParallelism() const { return parallelism_; }
  inline void SetParallelism(int parallelism) { parallelism_ = parallelism; }

  /* Data & task dependency */
  void SetProducer(const std::shared_ptr<Task>& producer) {
    producer_is_set_ = true;
    parallelism_ = producer->GetParallelism();
    write_precedence_ = producer;
    producer->ProduceData(id_);
    // metadata
    Metadata meta(id_, name_);
    meta.SetParallelism(parallelism_);
    meta.SetProducer(producer->GetId());
    task_graph_->AddMetaData(id_, meta);
  }

  void WriteBy(const std::shared_ptr<Task>& task) {
    task->WriteData(id_);
    if (read_precedence_.empty()) {
      if (write_precedence_ != nullptr) {
        write_precedence_->Then(task);
      }
    } else {
      for (auto& t : read_precedence_) {
        t->Then(task);
      }
      read_precedence_.clear();
    }
    write_precedence_ = task;
  }

  void ReadBy(const std::shared_ptr<Task>& task) {
    task->ReadData(id_);
    read_precedence_.push_back(task);
    write_precedence_->Then(task);
  }

  void BroadcastBy(const std::shared_ptr<Task>& task) {
    task->ReadData(id_);
    read_precedence_.push_back(task);
    write_precedence_->BroadcastThen(task);
  }

  void LocalAggregateBy(const std::shared_ptr<Task>& task) {
    task->ReadData(id_);
    read_precedence_.push_back(task);
    write_precedence_->LocalAggregateThen(task);
  }

  inline const std::shared_ptr<Task>& GetWriteDependence() { return write_precedence_; }

 protected:
  inline void SetWriteDependence(const std::shared_ptr<Task>& task) { write_precedence_ = task; }

  void SanityCheck() const { CHECK(producer_is_set_) << "The producer of the dataset is not set yet"; }

  /* TaskGraph utils */

  std::shared_ptr<Task> CreateTask(const std::string& func_name, ResourceType type = CPU) const {
    auto task = task_graph_->CreateTask(name_ + "." + func_name, type);
    task->SetParallelism(parallelism_);
    return task;
  }

  std::shared_ptr<Task> CreateTask(const std::string& func_name, int parallelism, ResourceType type = CPU) const {
    auto task = task_graph_->CreateTask(name_ + "." + func_name, type);
    task->SetParallelism(parallelism);
    return task;
  }

  template <typename Lambda>
  void RegisterClosure(TaskIdType tid, Lambda lambda) const {
    task_graph_->RegisterClosure(tid, Closure::CreateClosure(lambda));
  }

 protected:
  TaskGraph* task_graph_ = nullptr;
  std::string name_ = "";
  DataIdType id_;
  std::shared_ptr<Task> write_precedence_ = nullptr;
  std::vector<std::shared_ptr<Task>> read_precedence_;
  bool producer_is_set_ = false;
  int parallelism_ = 10;
};

}  // namespace common
}  // namespace axe
