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

#include "hdfs/hdfs.h"
#include "orc/OrcFile.hh"

#include "common/io/hdfs_utils.h"

namespace axe {
namespace common {
namespace io {

class HdfsFileInputStream : public orc::InputStream {
 public:
  HdfsFileInputStream(const std::string& url, const HdfsFsPtr& fs) : fs_(fs), filename_(url) {
    CHECK(fs_ != nullptr);
    file_ = hdfsOpenFile(fs_.get(), filename_.c_str(), O_RDONLY, 0, 0, 0);
    CHECK(file_ != nullptr) << "Cannot open HDFS file " << filename_;

    hdfsFileInfo* file_info = hdfsGetPathInfo(fs_.get(), filename_.c_str());
    CHECK(file_info != nullptr) << "Cannot get path info " << filename_;
    CHECK(file_info->mKind == kObjectKindFile) << "The given path is not a file: " << filename_;
    file_length_ = file_info->mSize;
    hdfsFreeFileInfo(file_info, 1);
  }

  ~HdfsFileInputStream() override {
    if (file_) {
      hdfsCloseFile(fs_.get(), file_);
    }
  }

  inline uint64_t getLength() const override { return file_length_; }
  inline uint64_t getNaturalReadSize() const override { return READ_SIZE; }
  inline const std::string& getName() const override { return filename_; }

  void read(void* buf, uint64_t length, uint64_t offset) override {
    CHECK(buf) << "Buffer is null";

    int n_bytes_read = 0;
    int current_read;
    CHECK_EQ(0, hdfsSeek(fs_.get(), file_, offset)) << "Seek error. offset = " << offset;
    while (n_bytes_read < length) {
      current_read = hdfsRead(fs_.get(), file_, (char*) buf + n_bytes_read, length - n_bytes_read);
      n_bytes_read += current_read;
      DLOG(INFO) << " have read " << n_bytes_read << " bytes";
      google::FlushLogFiles(google::INFO);
      if (current_read <= 0) {
        LOG(WARNING) << "Could not read full length " << length << ". Read " << n_bytes_read;
        break;
      }
    }
  }

 private:
  const uint64_t READ_SIZE = 1024 * 1024;  // 1 MB

  std::string filename_;

  const HdfsFsPtr& fs_;
  hdfsFile file_ = nullptr;
  uint64_t file_length_;
};

}  // namespace io
}  // namespace common
}  // namespace axe
