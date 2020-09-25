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

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "glog/logging.h"

#include "common/dataset/abstract_data.h"

namespace axe {
namespace common {

class PartitionIterator {
 public:
  virtual std::pair<const void*, size_t> GetPtr() const = 0;
  virtual void operator++() = 0;

  bool operator!=(const PartitionIterator& rhs) { return GetPtr().first != rhs.GetPtr().first; }
  bool operator==(const PartitionIterator& rhs) { return GetPtr().first == rhs.GetPtr().first; }
};

class Partition : public AbstractData {
 public:
  virtual ~Partition() = default;

  virtual void clear() {
    has_null_ = false;
    not_null_.clear();
  }

  virtual bool empty() const = 0;
  virtual size_t size() const = 0;
  virtual void resize(size_t size) = 0;
  virtual void reserve(size_t size) = 0;
  virtual void push_back(const std::pair<const void*, size_t>& val) = 0;
  virtual void shrink_to_fit() = 0;

  virtual void push_back(const char* data, uint32_t length) = 0;
  virtual std::pair<const void*, size_t> At(size_t pos) = 0;

  virtual std::string Print(size_t pos) = 0;

  // For sorting
  // Compare two element at lhs and rhs.
  // return negative value if at(lhs) < at(rhs)
  // return zero if at(lhs) == at(rhs)
  // return positive value if at(lhs) > at(rhs)
  virtual int Compare(size_t lhs, size_t rhs) const = 0;
  virtual int Compare(size_t pos, const std::pair<const void*, size_t>& rhs) const = 0;
  virtual int CompareHasNull(size_t lhs, size_t rhs) const;
  virtual int CompareHasNull(size_t lhs, size_t rhs, bool end_in_null) const;
  virtual int CompareHasNull(size_t pos, const std::pair<const void*, size_t>& rhs) const {
    auto is_null = pos >= size() || IsNull(pos);
    return rhs.first == nullptr ? (is_null ? 0 : 1) : (is_null ? -1 : Compare(pos, rhs));
  }

  virtual void ApplyPermutation(const std::vector<size_t>& permutation);
  virtual void ApplyPermutation(const std::vector<uint32_t>& permutation);

  virtual void ApplyFilter(const std::vector<bool>& to_keep);
  virtual std::shared_ptr<Partition> Filter(const std::vector<bool>& to_keep, size_t size) const = 0;

  virtual void AppendPartition(const std::shared_ptr<Partition>& rhs) = 0;
  virtual std::shared_ptr<PartitionIterator> Begin() const = 0;
  virtual std::shared_ptr<PartitionIterator> End() const = 0;

  virtual std::shared_ptr<Partition> Slice(size_t offset, size_t size) const = 0;

  virtual std::vector<uint32_t> GetSortedIndex() const = 0;

  void UpdateNotNull(size_t pos, size_t len, const char* not_null);
  void UpdateNotNull(size_t pos, const std::vector<bool>& not_null);
  void UpdateNotNull() {
    if (has_null_)
      not_null_.resize(size(), true);  // workaround for push back without updating null flags
  }

  void SetNull(const std::vector<uint32_t>& null_idx);

  bool IsNull(size_t pos) const { return !IsNotNull(pos); }
  bool IsNotNull(size_t pos) const {
    if (has_null_ && pos >= not_null_.size()) {
      throw std::runtime_error("Forgot to UpdateNotNull()? pos " + std::to_string(pos) + " not null " + std::to_string(not_null_.size()) + " size " +
                               std::to_string(size()));
    }
    return !has_null_ || not_null_[pos];
  }
  bool HasNull() const { return has_null_; }
  const auto& GetNotNull() const { return not_null_; }

  void AppendNull(const std::shared_ptr<Partition>& rhs);

 protected:
  bool has_null_ = false;
  std::vector<bool> not_null_;
};

}  // namespace common
}  // namespace axe
