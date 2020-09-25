// Copyright 2020 H-AXE
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

#include <iostream>
#include <string>

#include "base/bin_stream.h"

namespace axe {
namespace common {

using base::BinStream;

struct DateType {
  DateType() = default;
  DateType(int64_t _days_since_epoch) : days_since_epoch(_days_since_epoch) {}
  explicit DateType(const DateType& copy) : days_since_epoch(copy.days_since_epoch) {}

  friend std::ostream& operator<<(std::ostream& out, const DateType& t) {
    out << t.days_since_epoch;
    return out;
  }

  friend std::istream& operator>>(std::istream& in, DateType& t) {
    in >> t.days_since_epoch;
    return in;
  }

  explicit operator int64_t() const { return days_since_epoch; }
  explicit operator double() const { return days_since_epoch; }
  // operator int() const { return days_since_epoch; }
  // operator double() const { return days_since_epoch; }
  // operator float() const { return days_since_epoch; }
  bool operator<(const DateType& another) const { return days_since_epoch < another.days_since_epoch; }
  bool operator>(const DateType& another) const { return days_since_epoch > another.days_since_epoch; }
  bool operator>=(const DateType& another) const { return days_since_epoch >= another.days_since_epoch; }
  bool operator==(const DateType& another) const { return days_since_epoch == another.days_since_epoch; }
  void operator+=(const DateType& another) { days_since_epoch += another.days_since_epoch; }
  void operator-=(const DateType& another) { days_since_epoch -= another.days_since_epoch; }
  void operator*=(const DateType& another) { days_since_epoch *= another.days_since_epoch; }
  void operator/=(const DateType& another) { days_since_epoch /= another.days_since_epoch; }

  BinStream& serialize(BinStream& bin_stream) const { return bin_stream << days_since_epoch; }
  base::BinStream& deserialize(base::BinStream& bin_stream) { return bin_stream >> days_since_epoch; }

  int64_t days_since_epoch = 0;
};

}  // namespace common
}  // namespace axe
