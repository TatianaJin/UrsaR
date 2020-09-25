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

#include <map>

#include "base/bin_stream.h"
#include "common/instance_id.h"

namespace axe {
namespace common {

class InstanceIdCollection;
base::BinStream& operator<<(base::BinStream& bin_stream, const InstanceIdCollection& collection);
base::BinStream& operator>>(base::BinStream& bin_stream, InstanceIdCollection& collection);

class InstanceIdCollection {
 public:
  struct TrieNode {
    std::map<int, std::unique_ptr<TrieNode>> children;
    int max_suffix = -1;
    int counter = 0;  // counter number of instance id with same prefix
  };
  InstanceIdCollection() { trie_root_.reset(new TrieNode()); }

  void Insert(const InstanceId& instance_id);
  void Remove(const InstanceId& instance_id);

  // Here suffix refers to the last number of the instance_id
  // Prefix refers to everything except for the last number
  int GetMaxSuffix(const InstanceId& prefix);

  bool PrefixExist(const InstanceId& prefix);

  friend base::BinStream& operator<<(base::BinStream& bin_stream, const InstanceIdCollection& collection);
  friend base::BinStream& operator>>(base::BinStream& bin_stream, InstanceIdCollection& collection);

 protected:
  std::unique_ptr<TrieNode> trie_root_;
};

}  // namespace common
}  // namespace axe
