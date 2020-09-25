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

#include <string>
#include <utility>
#include <vector>

#include "common/constants.h"
#include "common/dataset/dataset.h"
#include "common/dataset/dataset_partition.h"
#include "common/io/input/hdfs_file_splitter.h"
#include "common/io/input/hdfs_input_block_info.h"
#include "common/io/input/line_inputformat.h"
#include "common/io/input/nfs_file_splitter.h"
#include "common/io/input/nfs_input_block_info.h"
#include "common/source_data.h"

namespace axe {
namespace common {

class SourceDataset : public Dataset<std::string> {
 public:
  explicit SourceDataset(TaskGraph* task_graph) : Dataset<std::string>(task_graph) {}
};

class TextSourceDataset : public SourceDataset {
 public:
  /** Source dataset that read from text file
   */
  TextSourceDataset(const std::string& url, TaskGraph* task_graph, int parallelism) : url_(url), SourceDataset(task_graph) {
    {  // set protocol
      size_t pos = 0;
      if ((pos = url_.find("://")) != url_.npos) {
        protocol_ = (url_.substr(0, pos).compare("hdfs") == 0) ? HDFS : NFS;
        url_ = url_.substr(pos + 3);
      }
    }
    SetParallelism(parallelism);
  }

  template <typename Lambda>
  inline auto FlatMap(Lambda lambda) {
    auto task = CreateTask("FlatMap");
    return FlatMapInner(task, lambda);
  }

  template <typename Lambda, typename MemoryEstimateLambda>
  inline auto FlatMap(Lambda lambda, MemoryEstimateLambda memory_estimate_lambda) {
    auto task = CreateTask("FlatMap");
    task->SetResourcePredictor({memory_estimate_lambda});
    return FlatMapInner(task, lambda);
  }

 private:
  template <typename Lambda>
  auto FlatMapInner(const std::shared_ptr<Task>& task, Lambda lambda) {
    auto ret = Dataset<typename decltype(lambda(std::string()))::value_type>::Create(task, task_graph_, GetParallelism());
    LOG(INFO) << "protocol " << (protocol_ == HDFS ? "hdfs" : "nfs");
    RegisterClosure(task->GetId(), [ lambda, url = url_, protocol = protocol_, ret = ret.GetId() ](TaskContext * tc) {
      LOG(INFO) << "flatmap =================";
      google::FlushLogFiles(google::INFO);
      axe::common::LineInputFormat input(url, protocol);
      auto task_desc = tc->GetTaskDesc();
      auto block_desc = SourceData::GetBlockDesc(task_desc);
      using ret_type = typename decltype(lambda(std::string()))::value_type;
      if (block_desc.size() == 0) {
        tc->InsertDatasetPartition(ret, std::make_shared<DatasetPartition<ret_type>>());
        tc->InjectWatermark();
        return;
      }
      auto data = std::make_shared<DatasetPartition<ret_type>>(input.ReadData(block_desc, lambda));
      tc->InsertDatasetPartition(ret, data);
      tc->InjectWatermark();
    });
    task_graph_->AddSourceData(SourceData(InputBlockInfo::Create(url_, protocol_), task));
    return ret;
  }

  std::string url_;
  FS protocol_ = HDFS;
};

// TODO(tatiana)
/** @deprecated replaced by {@link TextSourceDataset}*/
class HdfsSourceDataset : public SourceDataset {
 public:
  HdfsSourceDataset(const std::string& url, TaskGraph* task_graph, int parallelism) : url_(url), SourceDataset(task_graph) {
    SetParallelism(parallelism);
  }

  template <typename Lambda>
  auto Map(Lambda lambda) {
    auto task = CreateTask("Hdfs Map");
    return SetMap(task, lambda);
  }

  template <typename Lambda, typename MemoryEstimateLambda>
  auto Map(Lambda lambda, MemoryEstimateLambda memory_estimate_lambda) {
    auto task = CreateTask("Hdfs Map");
    task->SetResourcePredictor({memory_estimate_lambda});
    return SetMap(task, lambda);
  }

 private:
  template <typename Lambda>
  auto SetMap(const std::shared_ptr<Task>& task, Lambda lambda) {
    auto ret = Dataset<typename decltype(lambda(std::string()))::value_type>::Create(task, task_graph_, GetParallelism());
    RegisterClosure(task->GetId(), [ lambda, url = url_, ret = ret.GetId() ](TaskContext * tc) {
      axe::common::LineInputFormat input(url);
      auto task_desc = tc->GetTaskDesc();
      auto block_desc = SourceData::GetBlockDesc(task_desc);
      using ret_type = typename decltype(lambda(std::string()))::value_type;
      if (block_desc.size() == 0) {
        tc->InsertDatasetPartition(ret, std::make_shared<DatasetPartition<ret_type>>());
        tc->InjectWatermark();
        return;
      }
      auto data = std::make_shared<DatasetPartition<ret_type>>(input.ReadData(block_desc, lambda));
      tc->InsertDatasetPartition(ret, data);
      tc->InjectWatermark();
    });
    task_graph_->AddSourceData(SourceData(InputBlockInfo::Create(url_, HDFS), task));
    return ret;
  }
  std::string url_;
};

}  // namespace common
}  // namespace axe
