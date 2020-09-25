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

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "gflags/gflags.h"
#include "hdfs/hdfs.h"

#include "common/flags.h"
#include "common/io/hdfs_utils.h"
#include "common/io/input/input_block_info.h"

namespace axe {
namespace common {
namespace io {

class OrcInputBlockInfo : public AbstractInputBlockInfo {
 public:
  explicit OrcInputBlockInfo(const std::vector<std::string>& urls, const std::string& namenode = FLAGS_hdfs_namenode, int port = FLAGS_hdfs_port);
  virtual ~OrcInputBlockInfo() {}

  void FetchBlocksInfo() override;
  double GetBlockSize(const std::string& file, size_t offset) const override;

 private:
  void BrowseHdfs(const std::string& url);
  void BrowseLocalFile(const std::string& file_name);
  void BrowseLocalDir(const std::string& dir_name);

  io::HdfsFsPtr fs_ = nullptr;
  std::string namenode_;
  int port_;

  std::map<std::pair<std::string, size_t>, double> block_sizes_;
};

}  // namespace io
}  // namespace common
}  // namespace axe
