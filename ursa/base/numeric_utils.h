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

#include <cinttypes>
#include <vector>

#include "glog/logging.h"

namespace axe {
namespace base {
namespace utils {

template <typename NumT>
std::enable_if_t<std::is_integral<NumT>::value || std::is_floating_point<NumT>::value, void> write_number(NumT num, std::vector<char>& bytes,
                                                                                                          size_t offset) {
  uint64_t size = sizeof(NumT) * 8;
  DCHECK_GT(bytes.size(), offset);
  DCHECK_GE(bytes.size(), size + offset);
  *reinterpret_cast<NumT*>(&(bytes[offset])) = num;
}

template <typename NumT>
std::enable_if_t<std::is_integral<NumT>::value || std::is_floating_point<NumT>::value, NumT> read_number(std::vector<char>& bytes, size_t offset) {
  uint64_t size = sizeof(NumT) * 8;
  DCHECK_GT(bytes.size(), offset);
  DCHECK_GE(bytes.size(), size + offset);
  return *reinterpret_cast<NumT*>(&(bytes[offset]));
}

}  // namespace utils
}  // namespace base
}  // namespace axe
