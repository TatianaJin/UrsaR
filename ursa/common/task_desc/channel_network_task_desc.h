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
#include <utility>
#include <vector>

#include "glog/logging.h"

#include "common/task_desc/network_task_desc.h"

namespace axe {
namespace common {

class ChannelNetworkTaskDesc : public NetworkTaskDesc {
 public:
  struct RemoteInfo {
    std::string remote_host_ = "";
    ShardIdType remote_shard_ = 0;
    // TODO(tatiana): remote instance id
    double data_size_ = 1.0;  // remote data size (KB)
    friend base::BinStream& operator<<(base::BinStream& binstream, const RemoteInfo& info) {
      binstream << info.remote_host_ << info.remote_shard_;
      return binstream;
    }
    friend base::BinStream& operator>>(base::BinStream& binstream, RemoteInfo& info) {
      binstream >> info.remote_host_ >> info.remote_shard_;
      return binstream;
    }
  };
  ChannelNetworkTaskDesc() {}
  ChannelNetworkTaskDesc(JobIdType job_id, TaskIdType task_id, ShardIdType shard_id, double network_usage = 0)
      : NetworkTaskDesc(job_id, task_id, shard_id, network_usage) {}

  base::BinStream& serialize(base::BinStream& bin_stream) const override {
    CHECK_NE(remote_data_.size(), 0) << "Remote data not set";
    NetworkTaskDesc::serialize(bin_stream);
    bin_stream << remote_infos_;
    bin_stream << remote_data_;
    bin_stream << remote_instances_;
    bin_stream << data_;
    bin_stream << n_remote_shard_;
    bin_stream << is_broadcast_;
    bin_stream << priority_;
    return bin_stream;
  }

  base::BinStream& deserialize(base::BinStream& bin_stream) override {
    NetworkTaskDesc::deserialize(bin_stream);
    bin_stream >> remote_infos_;
    bin_stream >> remote_data_;
    bin_stream >> remote_instances_;
    bin_stream >> data_;
    bin_stream >> n_remote_shard_;
    bin_stream >> is_broadcast_;
    bin_stream >> priority_;
    return bin_stream;
  }

  inline DataIdType GetDataId() const { return data_; }
  inline const auto& GetRemoteInfos() const { return remote_infos_; }
  inline ShardIdType GetRemoteShardSize() const { return n_remote_shard_; }
  inline auto GetRemoteDataIds() const { return remote_data_; }
  inline const auto& GetRemoteInstanceIds() const { return remote_instances_; }
  TaskIdType GetPriority() const override { return priority_; }

  void SetPriority(TaskIdType priority) { priority_ = priority; }
  void SetData(DataIdType data_id) { data_ = data_id; }
  /* TODO(tatiana): deprecate */
  void SetRemoteData(const std::vector<std::pair<std::string, ShardIdType>>& remote_info, DataIdType data_id,
                     const std::shared_ptr<InstanceId>& instance_id) {
    AddRemoteData(remote_info, data_id, instance_id);
  }
  void AddRemoteData(const std::vector<std::pair<std::string, ShardIdType>>& remote_info, DataIdType data_id,
                     const std::shared_ptr<InstanceId>& instance_id) {
    n_remote_shard_ += remote_info.size();
    std::unordered_map<std::string, std::vector<ShardIdType>> info_map;
    for (auto& info : remote_info) {
      info_map[info.first].push_back(info.second);
    }
    remote_infos_.push_back(std::move(info_map));
    remote_data_.push_back(data_id);
    remote_instances_.push_back(instance_id);
  }

  bool IsBroadcast() const { return is_broadcast_; }
  void SetBroadcast(bool is_broadcast) { is_broadcast_ = is_broadcast; }

 private:
  TaskIdType priority_;  // priority is set to its child cpu task id
  std::vector<std::unordered_map<std::string, std::vector<ShardIdType>>> remote_infos_;
  uint32_t n_remote_shard_ = 0;
  DataIdType data_ = 0;
  std::vector<DataIdType> remote_data_;
  std::vector<std::shared_ptr<InstanceId>> remote_instances_;
  bool is_broadcast_ = false;
};

}  // namespace common
}  // namespace axe
