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
#include <string>
#include <utility>
#include <vector>

#include "glog/logging.h"

#include "common/dataset/dataset_partition.h"
#include "common/io/input/hdfs_file_splitter.h"
#include "common/io/input/inputformat_helper.h"

namespace axe {
namespace common {

// TODO(tatiana): doc
class LineInputFormat {
 public:
  explicit LineInputFormat(const std::string& url, FS protocol = HDFS) { SetSplitter(url, protocol); }

  template <typename String = std::string, typename Lambda>
  auto ReadData(const std::vector<std::pair<std::string, size_t>>& block_descs, const Lambda& executor) {
    auto ret = DatasetPartition<typename decltype(executor(std::string()))::value_type>();
    for (auto& blo : block_descs) {
      // blo : (url, offset)
      ClearBuffer();
      bool success = FetchNewBlock(blo.first, blo.second);
      if (success == false) {
        continue;
      }

      if (blo.second == 0) {
        r = -1;
      } else {
        r = FindNext(buffer_, 0, '\n');
        if (r == std::string::npos) {
          continue;
        }
      }

      while (true) {
        // last charater in block
        if (r == buffer_.size() - 1) {
          // fetch next block
          buffer_ = splitter_->FetchBlockView(blo.first, blo.second, true);
          if (!buffer_.empty()) {
            // directly process the remaing
            last_part_ = "";
            HandleNextBlock(blo.first, blo.second);
            auto res = executor(last_part_);
            ret.insert(ret.size(), res.begin(), res.end());
          }
          break;
        }

        l = r + 1;
        r = FindNext(buffer_, l, '\n');

        // if the right end does not exist in current block
        if (r == std::string::npos) {
          last_part_ = buffer_.substr(l);
          // fetch next subBlock
          buffer_ = splitter_->FetchBlockView(blo.first, blo.second, true);
          HandleNextBlock(blo.first, blo.second);
          auto result = executor(last_part_);
          ret.insert(ret.size(), result.begin(), result.end());
          break;
        } else {
          auto result = executor(String(buffer_.substr(l, r - l)));
          if (result.size() != 0) {
            ret.insert(ret.size(), result.begin(), result.end());
          }
        }
      }
    }
    return ret;
  }

  auto& GetSplitter() const { return splitter_; }

 protected:
  void SetSplitter(const std::string& url, FS protocol);
  void HandleNextBlock(const std::string& url, size_t offset);
  bool FetchNewBlock(const std::string& url, size_t offset);
  void ClearBuffer();

  std::unique_ptr<FileSplitter> splitter_;
  int l = 0;
  int r = 0;
  std::string last_part_;
  std::string url_;
  std::string_view buffer_;
};

}  // namespace common
}  // namespace axe
