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

#include <string>
#include <vector>

#include "base/bin_stream.h"

namespace axe {
namespace common {

class InstanceId;

// Allowing putting an InstanceId into an std::map/std::set
bool operator<(const InstanceId& a, const InstanceId& b);
bool operator==(const InstanceId& a, const InstanceId& b);

base::BinStream& operator<<(base::BinStream& bin_stream, const InstanceId& instance_id);
base::BinStream& operator>>(base::BinStream& bin_stream, InstanceId& instance_id);

class InstanceId {
 public:
  InstanceId() = default;
  explicit InstanceId(const std::vector<int>& vec) : vec_(vec) {}

  size_t Size() const { return vec_.size(); }
  int Get(int idx) const { return vec_.at(idx); }

  void Append(int num) { vec_.push_back(num); }
  void Pop() { vec_.pop_back(); }
  void Set(int idx, int num) { vec_.at(idx) = num; }
  void SetBack(int num) { vec_.back() = num; }

  // For Debug Purpose
  std::string ToString() const {
    std::string ret = std::to_string(vec_.size());
    ret += ":";
    for (auto v : vec_)
      ret += " " + std::to_string(v);
    return ret;
  }

  friend base::BinStream& operator<<(base::BinStream& bin_stream, const InstanceId& instance_id);
  friend base::BinStream& operator>>(base::BinStream& bin_stream, InstanceId& instance_id);

 protected:
  std::vector<int> vec_;
};

}  // namespace common
}  // namespace axe
