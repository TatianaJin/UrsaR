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

#include <memory>
#include <string>
#include <unordered_map>

#include "base/properties.h"
#include "common/constants.h"
#include "common/data_memory_record.h"
#include "common/data_store.h"
#include "common/dataset/abstract_data.h"
#include "common/dataset/dataset_partition.h"
#include "common/flags.h"
#include "common/instance_id.h"
#include "common/task_desc/task_desc.h"

#include "glog/logging.h"

namespace axe {
namespace common {

using base::Properties;

class TaskContext {
 public:
  explicit TaskContext(const std::shared_ptr<TaskDesc>& task_desc) : task_desc_(task_desc) {}

  /** Constructor with data store. **/
  TaskContext(const std::shared_ptr<TaskDesc>& task_desc, DataStore* data_store) : task_desc_(task_desc), data_store_(data_store) {}

  /** Add dataset partition to data store.
   *
   * @tparam Val    dataset partition value type
   * @param data_id the id of the dataset partition to add
   * @param data    the source dataset partition
   */
  template <typename Val>
  void InsertDatasetPartition(DataIdType data_id, std::shared_ptr<DatasetPartition<Val>> data) {
    InsertData(data_id, data);
  }

  /** Get immutable dataset partition from data store.
   *
   * @tparam Val    dataset partition value type
   * @param data_id the id of the dataset partition
   */
  template <typename Val>
  const auto GetDatasetPartition(DataIdType data_id) {
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetDatasetPartition<Val>(data_id, task_desc_->GetShardId());
  }

  template <typename Val>
  const auto GetDatasetPartition(DataIdType data_id, ShardIdType shard_id) {
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetDatasetPartition<Val>(data_id, shard_id);
  }

  /** Get immutable dataset partitions from data store.
   *
   * TODO(tatiana): deprecate
   * @tparam Val    dataset partitions value type
   * @param data_id the id of the dataset partitions
   * @return all dataset partitions of the data_id
   */
  template <typename Val>
  const auto GetDataset(DataIdType data_id) {
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetDataset<Val>(data_id);
  }

  /** Get immutable dataset partitions from data store.
   *
   * @param data_id the id of the dataset partitions
   * @return all dataset partitions of the data_id
   */
  const auto GetDataset(DataIdType data_id) {
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetDataset(data_id);
  }

  /** Get mutable dataset partition from data store.
   *
   * TODO(tatiana): Supports only element update now, cannot insert or delete
   *
   * @tparam Val    dataset partition value type
   * @param data_id the id of the dataset partition
   */
  template <typename Val>
  auto GetMutableDatasetPartition(DataIdType data_id) {
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetMutableDatasetPartition<Val>(data_id, task_desc_->GetShardId());
  }

  /** Add dataset partition to data store.
   *
   * @param data_id the id of the dataset partition to add
   * @param data    the source dataset partition
   */
  void InsertData(DataIdType data_id, std::shared_ptr<AbstractData> data) {
    /* TODO tatiana: version control */
    if (data == nullptr) {
      LOG(WARNING) << "data to insert is null: " << data_id << " from " << task_desc_->DebugString();
      return;
    }
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    auto message_ptr = std::dynamic_pointer_cast<DatasetPartition<std::shared_ptr<base::BinStream>>>(data);
    if (FLAGS_enable_message_size_report && message_ptr != nullptr) {
      std::vector<double> downstream_memory;
      downstream_memory.reserve(message_ptr->size());
      for (auto& bin : *message_ptr) {
        downstream_memory.push_back(bin->size());
      }
      data_memory_.emplace_back(data_id, std::move(downstream_memory));
    } else {
      data_memory_.emplace_back(data_id, data->GetMemory());
    }
    data_store_->InsertData(data_id, task_desc_->GetShardId(), data);
  }

  /** Add process-level dataset partition to data store.
   *
   * @param data_id the id of the dataset partition to add
   * @param data    the source dataset partition
   */
  void InsertProcessLevelData(DataIdType data_id, std::shared_ptr<AbstractData> data) {
    if (data == nullptr)
      return;
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    data_memory_.emplace_back(data_id, data->GetMemory());
    data_store_->InsertProcessLevelData(data_id, data);
  }

  const auto GetProcessLevelData(DataIdType data_id) {
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetProcessLevelData(data_id);
  }

  /** Get immutable dataset partition from data store.
   *
   * @param data_id the id of the dataset partition
   */
  const auto GetData(DataIdType data_id) {
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetData(data_id, task_desc_->GetShardId());
  }

  /** Get mutable dataset partition from data store.
   *
   * TODO(tatiana): Supports only element update now, cannot insert or delete
   *
   * @param data_id the id of the dataset partition
   */
  auto GetMutable(DataIdType data_id) {
    /* TODO tatiana: version control */
    CHECK(data_store_ != nullptr) << "[TaskContext] data_store_ not set";
    return data_store_->GetMutableData(data_id, task_desc_->GetShardId());
  }

  inline const std::shared_ptr<InstanceId>& GetInstanceId() const { return task_desc_->GetInstanceId(); }
  inline ShardIdType GetShardId() const { return task_desc_->GetShardId(); }
  inline const auto& GetTaskDesc() const { return task_desc_; }
  inline const auto& GetInjectedWatermark() const { return watermark_; }
  inline const bool HasWatermark() const { return has_watermark_; }

  inline void SetConfig(const std::shared_ptr<Properties>& config) { config_ = config; }
  inline const std::string& GetConfig(const std::string& key) { return config_->Get(key); }

  // Assume the watermark is the prefix of instance id
  void InjectWatermark() {
    watermark_ = *task_desc_->GetInstanceId();
    CHECK_GT(watermark_.Size(), 0) << "When injecting watermark, the size of instance should be larger than 0";
    watermark_.Pop();
    has_watermark_ = true;
  }

  const auto& GetDataMemory() const { return data_memory_; }

 private:
  std::shared_ptr<TaskDesc> task_desc_;
  bool has_watermark_ = false;
  InstanceId watermark_;
  DataStore* const data_store_ = nullptr;  // not owned
  std::shared_ptr<Properties> config_;

  DataMemory data_memory_;
};

}  // namespace common
}  // namespace axe
