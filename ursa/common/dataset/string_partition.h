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
#include <cstring>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "glog/logging.h"

#include "common/dataset/macro.h"
#include "common/dataset/partition.h"

namespace axe {
namespace common {

class StringPartition;

class StringPartitionIterator : public PartitionIterator {
 public:
  using value_type = string_view;
  using size_type = uint32_t;

  StringPartitionIterator(const size_type pos, const StringPartition* sp);

  std::pair<const void*, size_t> GetPtr() const override;

  value_type operator*();

  value_type operator*() const;

  void operator++() override;
  bool operator==(const StringPartitionIterator& rhs) { return pos_ == rhs.pos_; }
  bool operator!=(const StringPartitionIterator& rhs) { return pos_ != rhs.pos_; }
  bool operator<(const StringPartitionIterator& rhs) { return pos_ < rhs.pos_; }
  bool operator>(const StringPartitionIterator& rhs) { return pos_ > rhs.pos_; }

 private:
  size_type GetLength() const;
  const char* GetData() const;

  size_type pos_;
  const StringPartition* sp_;
};

class StringPartition : public Partition {
 public:
  using value_type = string_view;
  using size_type = uint32_t;
  using iterator = StringPartitionIterator;
  using const_iterator = StringPartitionIterator;

  friend class StringPartitionIterator;

  StringPartition() {
    size_ = 0;
    offset_.push_back(0);
  }

  explicit StringPartition(const std::vector<std::string>& vec) {
    size_ = 0;
    offset_.push_back(0);
    for (const auto& str : vec) {
      push_back(str);
    }
  }

  StringPartition(size_type size, const std::pair<const void*, size_t>& ptr) {
    size_ = size;
    offset_.resize(size + 1);
    data_.resize(size * ptr.second);
    for (int i = 0; i < size + 1; ++i)
      offset_[i] = i * ptr.second;
    for (int i = 0; i < size * ptr.second; ++i) {
      data_[i] = *((const char*) ptr.first + i % ptr.second);
    }
  }

  /* Vector-like APIs */

  void reserve(size_t size) override { offset_.reserve(size); }
  /** Resize the array to size elements.
   *
   * If size > size_, append size - size_ entries with empty string.
   *
   */
  void resize(size_t size, value_type val) {
    if (size < size_) {
      size_ = size;
      return;
    }
    AppendInner(size - size_, val);
  }

  /** Resize the array to size elements.
   *
   * If size > size_, append size - size_ entries with empty string.
   */
  void resize(size_t size) override {
    if (size < size_) {
      size_ = size;
      return;
    }
    AppendInner(size - size_, value_type());
  }

  /** Release the memory. **/
  void clear() override {
    Partition::clear();
    offset_.clear();
    data_.clear();
    offset_.shrink_to_fit();
    data_.shrink_to_fit();
    size_ = 0;
    offset_.push_back(0);
  }

  bool empty() const override { return size_ == 0; }
  size_t size() const override { return size_; }

  inline iterator begin() { return iterator(0, this); }
  inline const_iterator begin() const { return iterator(0, this); }
  inline iterator end() { return iterator(size_, this); }
  inline const_iterator end() const { return iterator(size_, this); }

  const value_type operator[](size_t i) const {
    size_type length = offset_[i + 1] - offset_[i];
    return value_type(data_.data() + offset_[i], length);
  }

  inline const value_type at(size_t i) const {
    if (i >= size_) {
      throw std::out_of_range("[StringPartition] Index out of range when accessing " + std::to_string(i) + "-th position. size is " +
                              std::to_string(size_));
    }
    return (*this)[i];
  }

  void push_back(const char* data, size_type length) override {
    size_type pre = offset_[size_];

    if (length == 0) {
      offset_.push_back(pre);
      size_++;
      return;
    }

    data_.resize(pre + length);
    memcpy(data_.data() + pre, data, length);
    offset_.push_back(pre + length);
    size_++;
  }

  inline void push_back(const std::string& val) { push_back(val.c_str(), val.length()); }

  inline void push_back(const value_type& val) { push_back(val.data(), val.length()); }

  inline void push_back(const char* data) { push_back(data, strlen(data)); }

  inline void pop_back() {
    size_--;
    data_.resize(offset_[size_]);
    offset_.pop_back();
  }

  void push_back(const std::pair<const void*, size_t>& val) override { push_back((const char*) val.first, val.second); }

  void shrink_to_fit() override {
    offset_.resize(size_ + 1);
    data_.resize(offset_[size_]);
    offset_.shrink_to_fit();
    data_.shrink_to_fit();
  }

  /* End of vector-like APIs */

  std::pair<const void*, size_t> At(size_t pos) override {
    size_type offset = offset_[pos];
    size_type length = offset_[pos + 1] - offset_[pos];
    return {data_.data() + offset, length};
  }

  std::string Print(size_t pos) override {
    if (IsNull(pos)) {
      return "NULL";
    }
    auto view = at(pos);
    return std::string(view.data(), view.length());
  }

  int Compare(size_t lhs, size_t rhs) const override { return at(lhs).compare(at(rhs)); }

  int Compare(size_t pos, const std::pair<const void*, size_t>& rhs) const override {
    return at(pos).compare(value_type((const char*) rhs.first, rhs.second));
  }

  void ApplyPermutation(const std::vector<size_t>& permutation) override {
    CHECK(permutation.size() == size_) << "Size of permutation is not equals to size_";
    StringPartition res;
    for (int i = 0; i < size_; ++i) {
      res.push_back(at(permutation[i]));
    }
    offset_ = std::move(res.offset_);
    data_ = std::move(res.data_);
    Partition::ApplyPermutation(permutation);
  }

  void ApplyPermutation(const std::vector<uint32_t>& permutation) override {
    CHECK(permutation.size() == size_) << "Size of permutation is not equals to size_";
    StringPartition res;
    for (int i = 0; i < size_; ++i) {
      res.push_back(at(permutation[i]));
    }
    offset_ = std::move(res.offset_);
    data_ = std::move(res.data_);
    Partition::ApplyPermutation(permutation);
  }

  std::shared_ptr<Partition> Filter(const std::vector<bool>& to_keep, size_t size) const override {
    auto ret = std::make_shared<StringPartition>(*this);
    ret->ApplyFilter(to_keep);
    return ret;
  }

  void ApplyFilter(const std::vector<bool>& to_keep) override {
    CHECK_EQ(to_keep.size(), size_) << "Apply filter vector of size " << to_keep.size() << " to string partition of size " << size_;

    size_type cur = 0;
    size_type cur_ptr = 0;
    for (int i = 0; i < size_; ++i) {
      if (to_keep.at(i)) {
        size_type length = offset_.at(i + 1) - offset_.at(i);
        if (cur_ptr != offset_.at(i)) {
          memmove(data_.data() + cur_ptr, data_.data() + offset_.at(i), length);
        }
        offset_[cur++] = cur_ptr;
        cur_ptr += length;
      }
    }
    size_ = cur;
    offset_[cur++] = cur_ptr;
    offset_.resize(size_ + 1);
    Partition::ApplyFilter(to_keep);
  }

  void AppendPartition(const std::shared_ptr<Partition>& rhs) override {
    auto sp_ptr = std::dynamic_pointer_cast<StringPartition>(rhs);
    if (sp_ptr == nullptr) {
      throw std::runtime_error("Cannot cast Partition to String Partition while appending");
    }

    for (auto str : *sp_ptr) {
      push_back(str);
    }
    AppendNull(rhs);
  }

  std::shared_ptr<PartitionIterator> Begin() const override { return std::make_shared<StringPartitionIterator>(0, this); }
  std::shared_ptr<PartitionIterator> End() const override { return std::make_shared<StringPartitionIterator>(size_, this); }

  // TODO(tatiana): zero copy slice
  std::shared_ptr<Partition> Slice(size_t offset, size_t size) const override {
    CHECK_LT(offset, size_);
    CHECK_LE(offset + size, size_);

    auto ret = std::make_shared<StringPartition>();
    ret->size_ = size;
    // copy data
    auto n_bytes = offset_[offset + size] - offset_[offset];
    ret->data_.resize(n_bytes);
    std::memcpy(ret->data_.data(), data_.data() + offset_[offset], n_bytes);
    // copy offsets
    ret->offset_.reserve(size + 1);
    auto begin = offset_.begin() + offset;
    auto end = begin + size + 1;
    for (auto iter = begin + 1; iter != end; ++iter) {
      ret->offset_.push_back(*iter - *begin);
    }

    if ((ret->has_null_ = has_null_)) {
      ret->not_null_.insert(ret->not_null_.end(), not_null_.begin() + offset, not_null_.begin() + offset + size);
    }
    return ret;
  }

  std::vector<uint32_t> GetSortedIndex() const override {
    std::vector<uint32_t> ret(size_);
    std::iota(ret.begin(), ret.end(), 0);
    sort(ret.begin(), ret.end(), [this](const auto& l, const auto& r) { return at(l) < at(r); });
    return ret;
  }

  /** Memory Usage in bytes */
  double GetMemory() const override { return (data_.size() + offset_.size() * sizeof(size_type)); }

 private:
  void AppendInner(size_type length, const value_type& val) {
    for (size_type i = 0; i < length; ++i)
      push_back(val);
  }

  size_type size_ = 0;
  std::vector<size_type> offset_;
  std::vector<char> data_;
};

}  // namespace common
}  // namespace axe
