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

#include "hdfs/hdfs.h"
#include "orc/OrcFile.hh"

#include "common/io/hdfs_utils.h"

namespace axe {
namespace common {
namespace io {

class OrcFileSplitter {
 public:
  static constexpr size_t DEFAULT_COLUMN_BATCH_SIZE = 1024;
  static std::unique_ptr<OrcFileSplitter> CreateOrcFileSplitter(const std::string& url);

  virtual ~OrcFileSplitter() {}

  inline void IncludeColumns(const std::list<uint64_t>& column_idx) { row_reader_options_.include(column_idx); }
  inline void SetColumnBatchSize(size_t size) { column_batch_size_ = size; }

  inline const auto& GetIncludedColumns() { return row_reader_options_.getInclude(); }
  inline size_t GetColumnBatchSize() const { return column_batch_size_; }
  inline const std::string& GetCurrentFile() const { return current_file_; }
  inline const orc::Type& GetSelectedType() const {
    CHECK(row_reader_ != nullptr) << "Must fetch new batch first";
    return row_reader_->getSelectedType();
  }

  /**
   * Fetch a orc::ColumnVectorBatch from the given stripe of the file.
   *
   * @param file_name input file name
   * @param stripe_id stripe id
   * @returns the pointer to orc::ColumnVectorBatch, nullptr if no more rows
   */
  orc::ColumnVectorBatch* FetchNewBatch(const std::string& file_name, uint64_t stripe_id);
  inline orc::ColumnVectorBatch* FetchNextBatch() { return (row_reader_->next(*column_batch_)) ? column_batch_.get() : nullptr; }

 protected:
  void InitReader(const std::string& file_name);
  virtual std::unique_ptr<orc::Reader> CreateReader(const std::string& file_name) const;

  uint64_t column_batch_size_ = DEFAULT_COLUMN_BATCH_SIZE;
  std::string current_file_ = "";
  std::unique_ptr<std::list<uint64_t>> included_columns_ = nullptr;

  std::unique_ptr<orc::ColumnVectorBatch> column_batch_;
  std::unique_ptr<orc::Reader> reader_;
  std::unique_ptr<orc::RowReader> row_reader_;
  orc::RowReaderOptions row_reader_options_;
  std::vector<std::pair<uint64_t, uint64_t>> stripe_info_;  // byte offset, length
};

class HdfsOrcFileSplitter : public OrcFileSplitter {
 public:
  HdfsOrcFileSplitter() : OrcFileSplitter(), fs_(HdfsUtils::ConnectToHDFS()) {}

  std::unique_ptr<orc::Reader> CreateReader(const std::string& file_name) const override;

 private:
  HdfsFsPtr fs_ = nullptr;
};

}  // namespace io
}  // namespace common
}  // namespace axe
