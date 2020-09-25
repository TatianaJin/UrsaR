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
#include <vector>

#include "orc/OrcFile.hh"

#include "common/dataset/partition.h"

namespace axe {
namespace common {
namespace io {

struct AbstractOrcColumnEncoder {
  virtual void EncodeByColumn(const orc::ColumnVectorBatch& batch, std::vector<std::shared_ptr<Partition>>* buffer, uint32_t column_idx) = 0;
  virtual ~AbstractOrcColumnEncoder() {}
};

std::unique_ptr<AbstractOrcColumnEncoder> CreateOrcColumnEncoder(const orc::Type& schema);

}  // namespace io
}  // namespace common
}  // namespace axe
