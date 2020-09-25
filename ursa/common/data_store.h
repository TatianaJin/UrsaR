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

#include <mutex>
#include <unordered_map>

#include "glog/logging.h"

#include "common/constants.h"
#include "common/dataset/dataset_partition.h"

namespace axe {
namespace common {

class DataStore {
 public:
  template <typename Val>
  void InsertDatasetPartition(DataIdType data_id, ShardIdType shard_id, std::shared_ptr<DatasetPartition<Val>> data) {
    std::lock_guard<std::mutex> lock(mu_);
    /* TODO tatiana: version control */
    // DLOG(INFO) << " insert data " << data_id << "." << shard_id;
    // google::FlushLogFiles(google::INFO);
    store_[data_id][shard_id] = data;
  }

  /** Get immutable dataset partition from data store.
   *
   * @tparam Val    dataset partition value type
   * @param data_id the id of the dataset partition
   */
  template <typename Val>
  std::shared_ptr<DatasetPartition<Val>> GetDatasetPartition(DataIdType data_id, ShardIdType shard_id) {
    std::lock_guard<std::mutex> lock(mu_);
    // Now we do not skip tasks that one of the input is empty
    // CHECK(store_.count(data_id) > 0) << "[DataStore] Cannot get data " << data_id;
    // CHECK(store_.at(data_id).count(shard_id) > 0) << "[DataStore] Cannot get data " << data_id << "-" << shard_id;
    if (store_.count(data_id) == 0) {
      LOG(WARNING) << "[DataStore] Cannot get data " << data_id;
      return nullptr;
    }
    if (store_.at(data_id).count(shard_id) == 0) {
      LOG(WARNING) << "[DataStore] Cannot get data " << data_id << "-" << shard_id;
      return nullptr;
    }
    auto ptr = std::dynamic_pointer_cast<DatasetPartition<Val>>(store_.at(data_id).at(shard_id));
    CHECK(ptr != nullptr) << "[DataStore] Get invalid type of data partition:" << data_id << '-' << shard_id;
    return ptr;
  }

  template <typename Val>
  const auto GetDataset(DataIdType data_id) {
    std::lock_guard<std::mutex> lock(mu_);
    CHECK(store_.count(data_id) > 0) << "[DataStore] Cannot get data " << data_id;
    auto map = store_.at(data_id);
    return map;
  }

  const auto GetDataset(DataIdType data_id) {
    std::lock_guard<std::mutex> lock(mu_);
    CHECK(store_.count(data_id) > 0) << "[DataStore] Cannot get data " << data_id;
    auto map = store_.at(data_id);
    return map;
  }

  /** Get mutable dataset partition from data store.
   *
   * TODO(tatiana): Supports only element update now, cannot insert or delete
   *
   * @tparam Val    dataset partition value type
   * @param data_id the id of the dataset partition
   */
  template <typename Val>
  auto GetMutableDatasetPartition(DataIdType data_id, ShardIdType shard_id) {
    std::lock_guard<std::mutex> lock(mu_);
    CHECK(store_.count(data_id) > 0) << "[DataStore] Cannot get data " << data_id;
    CHECK(store_.at(data_id).count(shard_id) > 0) << "[DataStore] Cannot get data " << data_id << "-" << shard_id;
    auto ptr = std::dynamic_pointer_cast<DatasetPartition<Val>>(store_.at(data_id).at(shard_id));
    CHECK(ptr != nullptr) << "[DataStore] Get invalid type of data partition:" << data_id << '-' << shard_id;
    return ptr;
  }

  void InsertData(DataIdType data_id, ShardIdType shard_id, std::shared_ptr<AbstractData> data) {
    std::lock_guard<std::mutex> lock(mu_);
    /* TODO tatiana: version control */
    // DLOG(INFO) << " insert data " << data_id << "." << shard_id;
    // google::FlushLogFiles(google::INFO);
    store_[data_id][shard_id] = data;
  }

  void InsertProcessLevelData(DataIdType data_id, std::shared_ptr<AbstractData> data) {
    std::lock_guard<std::mutex> lock(mu_);
    // DLOG(INFO) << " insert process level data " << data_id;
    // google::FlushLogFiles(google::INFO);
    store_[data_id][0] = data;
  }

  const std::shared_ptr<AbstractData> GetProcessLevelData(DataIdType data_id) {
    std::lock_guard<std::mutex> lock(mu_);
    // CHECK(store_.count(data_id) > 0) << "[DataStore] Cannot get data " << data_id;
    // CHECK(store_.at(data_id).count(0) > 0) << "[DataStore] Cannot get data " << data_id;
    if (store_.count(data_id) == 0) {
      LOG(WARNING) << "[DataStore] Cannot get data " << data_id;
      return nullptr;
    }
    if (store_.at(data_id).count(0) == 0) {
      LOG(WARNING) << "[DataStore] Cannot get data " << data_id;
      return nullptr;
    }
    auto ptr = store_.at(data_id).at(0);
    CHECK(ptr != nullptr) << "[DataStore] Get invalid type of data :" << data_id;
    return ptr;
  }

  /** Get immutable dataset partition from data store.
   *
   * @param data_id the id of the dataset partition
   */
  const std::shared_ptr<AbstractData> GetData(DataIdType data_id, ShardIdType shard_id) {
    std::lock_guard<std::mutex> lock(mu_);
    // Now we do not skip tasks that one of the input is empty
    if (store_.count(data_id) == 0) {
      LOG(WARNING) << "[DataStore] Cannot get data " << data_id;
      return nullptr;
    }
    if (store_.at(data_id).count(shard_id) == 0) {
      LOG(WARNING) << "[DataStore] Cannot get data " << data_id << '.' << shard_id;
      return nullptr;
    }
    return store_.at(data_id).at(shard_id);
  }

  /** Get mutable dataset partition from data store.
   *
   * TODO(tatiana): Supports only element update now, cannot insert or delete
   *
   * @param data_id the id of the dataset partition
   */
  auto GetMutableData(DataIdType data_id, ShardIdType shard_id) {
    std::lock_guard<std::mutex> lock(mu_);
    CHECK(store_.count(data_id) > 0) << "[DataStore] Cannot get data " << data_id;
    CHECK(store_.at(data_id).count(shard_id) > 0) << "[DataStore] Cannot get data " << data_id << "-" << shard_id;
    return store_.at(data_id).at(shard_id);
  }

  bool CheckProcessLevelDataExist(DataIdType data_id) { return CheckDataExist(data_id, 0); }

  bool CheckDataExist(DataIdType data_id, ShardIdType shard_id) {
    std::lock_guard<std::mutex> lock(mu_);
    if (store_.count(data_id) == 0)
      return false;
    if (store_.at(data_id).count(shard_id) == 0)
      return false;
    return true;
  }

  void RemoveData(DataIdType data_id, ShardIdType shard_id) {
    std::lock_guard<std::mutex> lock(mu_);
    CHECK(store_.count(data_id) > 0) << "[DataStore] Cannot remove data " << data_id;
    CHECK(store_.at(data_id).count(shard_id) > 0) << "[DataStore] Cannot remove data " << data_id << "-" << shard_id;
    store_.at(data_id).erase(shard_id);
    // DLOG(INFO) << " delete data " << data_id << "." << shard_id;
    google::FlushLogFiles(google::INFO);
    if (store_.at(data_id).size() == 0)
      store_.erase(data_id);
  }

 private:
  std::mutex mu_;
  std::unordered_map<DataIdType, std::unordered_map<ShardIdType, std::shared_ptr<AbstractData>>> store_;
};

}  // namespace common
}  // namespace axe
