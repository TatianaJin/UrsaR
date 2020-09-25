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

#include <vector>

#include "base/bin_stream.h"
#include "common/constants.h"

namespace axe {
namespace common {

struct DataMemoryRecord {
  DataMemoryRecord() {}
  DataMemoryRecord(DataIdType _data_id, std::vector<double>&& _sizes) : data_id(_data_id), sizes(std::move(_sizes)) {}
  DataMemoryRecord(DataIdType _data_id, double size) : data_id(_data_id), sizes({size}) {}

  double GetSize() const {
    double size = 0;
    for (auto s : sizes) {
      size += s;
    }
    return size;
  }

  base::BinStream& serialize(base::BinStream& bin_stream) const {
    bin_stream << data_id << sizes;
    return bin_stream;
  }
  base::BinStream& deserialize(base::BinStream& bin_stream) {
    bin_stream >> data_id >> sizes;
    return bin_stream;
  }

  DataIdType data_id;
  std::vector<double> sizes;
};

using DataMemory = std::vector<DataMemoryRecord>;

}  // namespace common
}  // namespace axe
