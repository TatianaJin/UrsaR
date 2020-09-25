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

namespace axe {
namespace base {

class Shard {
 public:
  Shard(int local_shard_id, int process_id) : local_shard_id_(local_shard_id), process_id_(process_id) {}

  int GetLocalShardId() const { return local_shard_id_; }
  int GetProcessId() const { return process_id_; }

  bool operator<(const Shard& shard) const {
    if (process_id_ < shard.GetProcessId() || (process_id_ == shard.GetProcessId() && local_shard_id_ < shard.GetLocalShardId()))
      return true;
    return false;
  }

 protected:
  int local_shard_id_;
  int process_id_;
};

}  // namespace base
}  // namespace axe
