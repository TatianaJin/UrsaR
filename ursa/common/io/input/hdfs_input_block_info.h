// Copyright 2020 HDL
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

#include <string>
#include <vector>

#include "common/io/hdfs_utils.h"
#include "common/io/input/input_block_info.h"

namespace axe {
namespace common {

class HdfsInputBlockInfo : public AbstractInputBlockInfo {
 public:
  explicit HdfsInputBlockInfo(const std::string& url);
  virtual ~HdfsInputBlockInfo() {}

  void FetchBlocksInfo() override;
  double GetBlockSize(const std::string& file, size_t offset) const override { return block_size_; }

 private:
  int num_blocks_ = 0;
  size_t block_size_;
  io::HdfsFsPtr fs_ptr_ = nullptr;
};

}  // namespace common
}  // namespace axe
