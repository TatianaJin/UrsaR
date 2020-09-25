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

#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/dataset/dataset_partition.h"
#include "common/io/input/orc/orc_file_splitter.h"

namespace axe {
namespace common {
namespace io {

class RawOrcInputFormat {
 public:
  explicit RawOrcInputFormat(const std::string& url) : splitter_(OrcFileSplitter::CreateOrcFileSplitter(url)) {}

  inline void IncludeColumns(const std::list<size_t>& column_idx) const { splitter_->IncludeColumns(column_idx); }
  inline const orc::Type& GetSchema() const { return splitter_->GetSelectedType(); }

  /**
   * Read data as specified in block_descs and execute lambda on each column (orc::ColumnVectorBatch).
   *
   * @block_descs (input file path, stripe id) pairs.
   * @lambda      lambda that inputs ColumnVectorBatch* and outputs some result
   */
  template <typename Lambda>
  void ReadData(const std::vector<std::pair<std::string, size_t>>& block_descs, const Lambda& lambda) {
    if (block_descs.empty()) {
      return;
    }

    for (auto& blo : block_descs) {
      orc::ColumnVectorBatch* batch = splitter_->FetchNewBatch(blo.first, blo.second);
      while (batch != nullptr) {
        lambda(batch, blo.first);
        batch = splitter_->FetchNextBatch();
      }
    }
  }

 private:
  std::unique_ptr<OrcFileSplitter> splitter_;
};

}  // namespace io
}  // namespace common
}  // namespace axe
