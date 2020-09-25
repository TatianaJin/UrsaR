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
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "glog/logging.h"

#include "base/bin_stream.h"
#include "common/dataset/partition.h"
#include "common/has_method.h"

namespace std {
/** If print a class without the to_string overload, return dummy
 */
template <typename T>
std::string to_string(const T&) {
  // throw std::runtime_error("DatasetPartition: to_string not implemented " + std::string(typeid(T).name()));
  LOG(FATAL) << ("DatasetPartition: to_string not implemented " + std::string(typeid(T).name()));
  return "dummy";
}

inline std::string to_string(bool b) { return b ? "true" : "false"; }
inline std::string to_string(int8_t b) { return std::string(1, (char) b); }
inline std::string to_string(char b) { return std::string(1, b); }
}  // namespace std

namespace axe {
namespace common {

template <typename Val>
class DatasetPartitionIterator : public PartitionIterator {
 public:
  using value_type = Val;
  using size_type = uint32_t;

  explicit DatasetPartitionIterator(Val* data) : data_(data) {}

  DatasetPartitionIterator operator+(size_type count) { return DatasetPartitionIterator(data_ + count); }

  std::pair<const void*, size_t> GetPtr() const override { return {data_, sizeof(value_type)}; }

  value_type& operator*() { return *data_; }

  const value_type& operator*() const { return *data_; }

  void operator++() override { data_++; }
  bool operator==(const DatasetPartitionIterator& rhs) { return data_ == rhs.data_; }
  bool operator!=(const DatasetPartitionIterator& rhs) { return data_ != rhs.data_; }
  bool operator<(const DatasetPartitionIterator& rhs) { return data_ < rhs.data_; }

 private:
  Val* data_;
};

template <typename Val>
class ConstDatasetPartitionIterator : public PartitionIterator {
 public:
  using value_type = Val;
  using size_type = uint32_t;

  explicit ConstDatasetPartitionIterator(const Val* data) : data_(data) {}

  ConstDatasetPartitionIterator operator+(size_type count) { return ConstDatasetPartitionIterator(data_ + count); }

  std::pair<const void*, size_t> GetPtr() const override { return {data_, sizeof(value_type)}; }

  const value_type& operator*() const { return *data_; }

  void operator++() override { data_++; }
  bool operator==(const ConstDatasetPartitionIterator& rhs) { return data_ == rhs.data_; }
  bool operator!=(const ConstDatasetPartitionIterator& rhs) { return data_ != rhs.data_; }
  bool operator<(const ConstDatasetPartitionIterator& rhs) { return data_ < rhs.data_; }

 private:
  const Val* data_;
};

/** Dataset partition with std::vector like interface.
 *
 * Implicit type cast, and zero-copy construction.
 *
 * It provides similar functionalities comparing to std::vector,
 * including data(), size(), operator[], at(), resize(), clear(), insert().
 *
 * @tparam Val the value type of the dataset partition array. The type must have default constructor
 */
template <typename Val>
class DatasetPartition : public Partition {
 public:
  using value_type = Val;

  /** Empty constructor. **/
  DatasetPartition() {}

  /** Create a partition of length n with initialized value.
   * @param size the length.
   * @param val the initial value.
   */
  DatasetPartition(size_t size, Val val) { resize(size, val); }

  /** Create a partition of length n with default value 0
   * @param size the length.
   */
  explicit DatasetPartition(size_t size) { resize(size); }

  /** Construct from a shared pointer of std::vector.
   *
   * Zero-copy constructor, copying the pointer only.
   *
   * @param vec the shared pointer of the source vector.
   */
  explicit DatasetPartition(const std::shared_ptr<std::vector<Val>>& vec) {
    ptr_ = std::shared_ptr<Val>(vec, vec->data());
    size_ = vec->size();
    capacity_ = size_;
  }

  /* Copy */

  /** Copy from a std::vector. **/
  explicit DatasetPartition(const std::vector<Val>& vec) { *this = vec; }

  /** Copy from an initializer_list. **/
  template <typename OVal>
  DatasetPartition(const std::initializer_list<OVal>& list) {
    *this = list;
  }

  /** Copy from a std::vector */
  void operator=(const std::vector<Val>& vec) { CopyFrom(vec.data(), vec.size()); }

  /** Copy from an initializer_list */
  template <typename OVal>
  void operator=(const std::initializer_list<OVal>& list) {
    CopyFrom(list.begin(), list.end());
  }

  /** Make a copy of the DatasetPartition. **/
  DatasetPartition<Val> Copy() const {
    DatasetPartition<Val> ret;
    ret.CopyFrom(data(), size());
    return ret;
  }

  /* End of copy */

  /* Vector-like APIs */

  /** Resize the array to size elements.
   *
   * If size <= size_, only change the size. Otherwise, append size -
   * size_ entries with values set to val.
   */
  void resize(size_t size, Val val) {
    if (size <= size_) {
      size_ = size;
      return;
    }
    size_t cur_n = size_;
    if (size > capacity_) {
      ResizeInner(std::max(size, size_ * 2));
    } else {
      ResizeInner(size);
    }
    size_ = size;
    SetValues(cur_n, size - cur_n, val);
  }

  /** Resize the array to size elements.
   *
   * If size <= size_, only change the size. Otherwise, append size -
   * size_ entries with 0.
   */
  void resize(size_t size) override {
    if (size <= size_) {
      size_ = size;
      return;
    }
    size_t cur_n = size_;
    if (size > capacity_) {
      ResizeInner(std::max(size, size_ * 2));
    } else {
      ResizeInner(size);
    }
    size_ = size;

    SetValues(cur_n, size - cur_n);
  }

  /** Requests that the capacity be at least enough to contain n elements.
   *
   * If capacity_ >= n, do nothing. Otherwise get an (n+5) sized array for storage.
   */
  void reserve(size_t n) override {
    if (capacity_ >= n) {
      return;
    }
    auto size = size_;
    ResizeInner(n);
    size_ = size;
  }

  /** Release the memory. **/
  inline void clear() override {
    Partition::clear();
    Reset(nullptr, 0, [](Val* data) {});
  }

  inline bool empty() const override { return size_ == 0; }
  inline size_t size() const override { return size_; }
  inline size_t capacity() const { return capacity_; }

  inline Val* begin() { return data(); }
  inline const Val* begin() const { return data(); }
  inline Val* end() { return data() + size(); }
  inline const Val* end() const { return data() + size(); }

  inline Val* data() const { return ptr_.get(); }

  inline Val& back() { return data()[size_ - 1]; }
  inline const Val& back() const { return data()[size_ - 1]; }
  inline Val& front() { return *data(); }
  inline const Val& front() const { return *data(); }
  inline Val& operator[](size_t i) { return data()[i]; }
  inline const Val& operator[](size_t i) const { return data()[i]; }

  inline Val& at(size_t i) {
    if (i >= size_) {
      throw std::out_of_range("[DatasetPartition] Index out of range when accessing " + std::to_string(i) + "-th position. size is " +
                              std::to_string(size_));
    }
    return data()[i];
  }
  inline const Val& at(size_t i) const {
    if (i >= size_) {
      throw std::out_of_range("[DatasetPartition] Index out of range when accessing " + std::to_string(i) + "-th position. size is " +
                              std::to_string(size_));
    }
    return data()[i];
  }

  /** Push back a new element.
   *
   * When the array is full, get a new array with (size_ * 2 + 5) capacity.
   */
  void push_back(const Val& val) {
    auto copy = val;  // copy before resize, in case val refers to some element in this instance
    if (size_ == capacity_) {
      reserve(size_ * 2 + 5);
    }
    data()[size_++] = std::move(copy);
    // data()[size_++] = val;
  }

  inline void push_back(Val&& val) {
    if (size_ == capacity_) {
      reserve(size_ * 2 + 5);
    }
    data()[size_++] = std::move(val);
  }

  void push_back(const char* data, uint32_t length) override { push_back(*reinterpret_cast<const Val*>(data)); }
  void push_back(const std::pair<const void*, size_t>& val) override { push_back(*(const Val*) val.first); }

  void shrink_to_fit() override {
    if (capacity_ == size_)
      return;
    GetNewBuffer(size_, data(), data() + size_);
  }

  inline void pop_back() {
    if (size_) {
      --size_;
    }
  }

  template <typename ForwardIt>
  void insert(size_t pos, const ForwardIt& first, const ForwardIt& last) {
    auto distance = static_cast<size_t>(std::distance(first, last));
    MoveInternal(pos, distance);
    for (auto it = first; it != last; ++it) {
      SetValueInternal(pos++, *it);
    }
  }

  /* End of vector-like APIs */

  /** Slice the partition into a subpartition, zero-copy
   *
   * @param offset The start index
   * @param size The size of the slice
   * @return The subpartition [offset, offset + size)
   */
  std::shared_ptr<Partition> Slice(size_t offset, size_t size) const override {
    auto ret = std::make_shared<DatasetPartition<Val>>();
    ret->ptr_ = std::shared_ptr<Val>(ptr_, data() + offset);
    ret->size_ = size;
    ret->capacity_ = size;
    if ((ret->has_null_ = has_null_)) {
      ret->not_null_.insert(ret->not_null_.end(), not_null_.begin() + offset, not_null_.begin() + offset + size);
    }
    return ret;
  }

  /** Get the shared pointer. **/
  inline std::shared_ptr<Val>& GetPtr() { return ptr_; }
  /** Get the const shared pointer. **/
  inline const std::shared_ptr<Val>& GetPtr() const { return ptr_; }

  std::pair<const void*, size_t> At(size_t pos) override { return {data() + pos, sizeof(Val)}; }

  template <typename SizeT = size_t>
  std::string PrintInner(typename std::enable_if<!std::is_same<Val, std::string>::value, SizeT>::type pos) {
    return std::to_string(at(pos));
  }

  template <typename SizeT = size_t>
  std::string PrintInner(typename std::enable_if<std::is_same<Val, std::string>::value, SizeT>::type pos) {
    return at(pos);
  }

  std::string Print(size_t pos) override {
    if (IsNull(pos)) {
      return "NULL";
    }
    return PrintInner(pos);
  }

  int Compare(size_t lhs, size_t rhs) const override {
    if (at(lhs) == at(rhs))
      return 0;
    return at(lhs) < at(rhs) ? -1 : 1;
  }
  int Compare(size_t pos, const std::pair<const void*, size_t>& rhs) const override {
    if (at(pos) == value_type(*(const value_type*) rhs.first))
      return 0;
    return at(pos) < value_type(*(const value_type*) rhs.first) ? -1 : 1;
  }
  void ApplyPermutation(const std::vector<size_t>& permutation) override {
    CHECK(permutation.size() == size_) << "Size of permutation is not equals to size_";
    DatasetPartition<Val> res;
    res.reserve(capacity_);
    for (int i = 0; i < size_; ++i) {
      res.push_back(at(permutation[i]));
    }
    ptr_ = res.ptr_;
    Partition::ApplyPermutation(permutation);
  };

  void ApplyPermutation(const std::vector<uint32_t>& permutation) override {
    CHECK(permutation.size() == size_) << "Size of permutation is not equals to size_";
    DatasetPartition<Val> res;
    res.reserve(capacity_);
    for (int i = 0; i < size_; ++i) {
      res.push_back(at(permutation[i]));
    }
    ptr_ = res.ptr_;
    Partition::ApplyPermutation(permutation);
  };

  void ApplyFilter(const std::vector<bool>& to_keep) override {
    CHECK(to_keep.size() == size_) << "Apply filter vector of size " << to_keep.size() << " to dataset partition of size " << size_;
    size_t cur = 0;
    for (int i = 0; i < size_; ++i) {
      if (to_keep.at(i)) {
        at(cur++) = at(i);
      }
    }
    resize(cur);
    Partition::ApplyFilter(to_keep);
  }

  std::shared_ptr<Partition> Filter(const std::vector<bool>& to_keep, size_t size) const override {
    CHECK_EQ(to_keep.size(), size_);
    CHECK_LE(size, size_);
    auto ret = std::make_shared<DatasetPartition<Val>>();
    ret->reserve(size);
    ret->not_null_.reserve(size);
    if (has_null_) {
      ret->has_null_ = has_null_;
      for (int i = 0; i < size_; ++i) {
        if (to_keep.at(i)) {
          ret->push_back(at(i));
          ret->not_null_.push_back(not_null_[i]);
        }
      }
    } else {
      for (int i = 0; i < size_; ++i) {
        if (to_keep.at(i)) {
          ret->push_back(at(i));
        }
      }
    }
    CHECK_EQ(ret->size(), size);
    return ret;
  }

  void AppendPartition(const std::shared_ptr<Partition>& rhs) override {
    CHECK_EQ(rhs->size() * rhs->HasNull(), rhs->GetNotNull().size() * rhs->HasNull());
    CHECK_EQ(size() * HasNull(), GetNotNull().size() * HasNull());
    auto ds_ptr = std::dynamic_pointer_cast<DatasetPartition<Val>>(rhs);
    CHECK(ds_ptr != nullptr) << "Cannot cast Partition to Dataset Partition while appending";
    auto old_size = size_;
    resize(size_ + ds_ptr->size());
    memcpy(data() + old_size, ds_ptr->data(), ds_ptr->size() * sizeof(Val));
    AppendNull(rhs);
    if (has_null_) {
      CHECK_EQ(not_null_.size(), size()) << "old " << old_size << " " << ds_ptr->size() << " ";
    }
  }

  std::shared_ptr<PartitionIterator> Begin() const override { return std::make_shared<ConstDatasetPartitionIterator<Val>>(data()); }
  std::shared_ptr<PartitionIterator> End() const override { return std::make_shared<ConstDatasetPartitionIterator<Val>>(data() + size_); }

  std::vector<uint32_t> GetSortedIndex() const override {
    std::vector<uint32_t> ret(size_);
    std::iota(ret.begin(), ret.end(), 0);
    sort(ret.begin(), ret.end(), [this](const auto& l, const auto& r) { return at(l) < at(r); });
    return ret;
  }

  /** Memory Usage in KBs */
  double GetMemory() const override { return GetMemoryInternal<Val>(0); }

 protected:
  /** Reset the current data pointer with a deleter. **/
  template <typename Deleter>
  void Reset(Val* ptr, size_t size, Deleter del) {
    size_ = size;
    capacity_ = size;
    ptr_.reset(ptr, del);
  }

  /** Copy from an iterator. **/
  template <typename ForwardIt>
  void CopyFrom(const ForwardIt& first, const ForwardIt& last) {
    GetNewBuffer(static_cast<size_t>(std::distance(first, last)), first, last);
  }

  /** Copy from a c-array **/
  void CopyFrom(const Val* ptr, size_t size) { GetNewBuffer(size, ptr, ptr + size); }

  void ResizeInner(size_t size) {
    if (capacity_ >= size) {
      size_ = size;
      return;
    }

    GetNewBuffer(size + 5, data(), data() + size_);
    size_ = size;
  }

  template <typename ForwardIt>
  void GetNewBuffer(size_t size, const ForwardIt& first, const ForwardIt& last) {
    Val* new_data = new Val[size];
    std::move(first, last, new_data);
    Reset(new_data, size, [](Val* data) { delete[] data; });
  }

  template <typename SizeT = size_t>
  inline void SetValues(std::enable_if_t<std::is_copy_assignable<Val>::value, SizeT> index, size_t length, const Val& val) {
    std::fill(data() + index, data() + index + length, val);
  }

  template <typename SizeT = size_t>
  inline void SetValues(typename std::enable_if<!std::is_copy_assignable<Val>::value, SizeT>::type index, size_t length, const Val& val) {
    for (auto iter = data() + index; iter != data() + index + length; ++iter) {
      *iter = Val(val);
    }
  }

  template <typename SizeT = size_t>
  inline void SetValues(typename std::enable_if<!std::is_fundamental<Val>::value, SizeT>::type index, size_t length) {}

  template <typename SizeT = size_t>
  inline void SetValues(std::enable_if_t<std::is_fundamental<Val>::value, SizeT> index, size_t length) {
    std::fill(data() + index, data() + index + length, Val());
  }

  void MoveInternal(size_t pos, size_t distance) {
    auto original_size = size_;
    if (size_ + distance > capacity_)
      reserve((size_ + distance) * 2 + 5);
    resize(size_ + distance);
    for (auto move = original_size; move > pos; --move) {
      data()[move + distance - 1] = data()[move - 1];
    }
  }
  void SetValueInternal(size_t pos, const Val& val) { data()[pos] = val; }

  template <typename T>
  double GetMemoryInternal(typename std::enable_if<HasGetMemory<T>::value, double>::type) const {
    double total = 0;
    for (auto& val : *this) {
      total += val.GetMemory();
    }
    return total + capacity_ * sizeof(Val);
  }

  template <typename T>
  double GetMemoryInternal(typename std::enable_if<std::is_same<T, std::shared_ptr<base::BinStream>>::value, double>::type) const {
    double total = 0;
    for (auto& val : *this) {
      total += val->get_total_size();
    }
    return total;
  }

  template <typename T>
  double GetMemoryInternal(
      typename std::enable_if<(!HasGetMemory<T>::value) && (!std::is_same<T, std::shared_ptr<base::BinStream>>::value) && HasSize<Val>::value,
                              double>::type) const {
    double total = 0;
    for (auto& val : *this) {
      total += val.size();
    }
    return total + capacity_ * sizeof(Val);
  }

  template <typename T>
  double GetMemoryInternal(
      typename std::enable_if<!(HasGetMemory<T>::value || HasSize<Val>::value || std::is_same<T, std::shared_ptr<base::BinStream>>::value),
                              double>::type) const {
    return (double) (capacity_ * sizeof(Val));
  }

 private:
  size_t size_ = 0;
  size_t capacity_ = 0;
  std::shared_ptr<Val> ptr_ = nullptr;
};

}  // namespace common
}  // namespace axe
