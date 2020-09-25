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

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "hdfs/hdfs.h"

#include "common/flags.h"

namespace axe {
namespace common {
namespace io {

struct HdfsDisconnector {
  void operator()(hdfsFS fs) { hdfsDisconnect(fs); }
};

using HdfsFsPtr = std::unique_ptr<HdfsFileSystemInternalWrapper, HdfsDisconnector>;

struct HdfsUtils {
  static HdfsFsPtr ConnectToHDFS(const std::string& namenode_hostname = FLAGS_hdfs_namenode, int namenode_port = FLAGS_hdfs_port,
                                 int num_retries = 3) {
    hdfsFS fs = nullptr;
    while (num_retries--) {
      struct hdfsBuilder* builder = hdfsNewBuilder();
      hdfsBuilderSetNameNode(builder, namenode_hostname.c_str());
      hdfsBuilderSetNameNodePort(builder, namenode_port);
      fs = hdfsBuilderConnect(builder);
      hdfsFreeBuilder(builder);
      if (fs) {
        return HdfsFsPtr(fs, HdfsDisconnector());
      }
    }
    LOG(WARNING) << "Failed to connect to HDFS " << namenode_hostname << ":" << std::to_string(namenode_port);
    return HdfsFsPtr();
  }
};

}  // namespace io
}  // namespace common
}  // namespace axe
