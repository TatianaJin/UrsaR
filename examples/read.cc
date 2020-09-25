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

#include <vector>

#include "glog/logging.h"

#include "common/closure.h"
#include "common/dataset/dataset_partition.h"
#include "common/dataset/source_dataset.h"
#include "common/job_driver.h"
#include "common/task_graph.h"

using axe::base::Properties;
using axe::common::TextSourceDataset;
using axe::common::DatasetPartition;
using axe::common::Job;
using axe::common::TaskContext;
using axe::common::TaskGraph;

auto ParseLine(const std::string_view& line) {
  std::vector<std::pair<int64_t, int64_t>> ret;
  int ptr = 0;
  while (ptr < line.size()) {
    while (ptr < line.size() && !std::isdigit(line.at(ptr)))
      ++ptr;
    if (ptr == line.size()) {
      break;
    }
    int64_t tmp = 0;
    while (ptr < line.size() && std::isdigit(line.at(ptr))) {
      tmp = tmp * 10 + line.at(ptr) - 48;
      ++ptr;
    }
    ret.push_back({tmp, 1});
  }
  return ret;
}

class ReadJob : public Job {
 public:
  void Run(TaskGraph* tg, const std::shared_ptr<Properties>& config) const override {
    // load dataset A
    auto dataA =
        TextSourceDataset(config->Get("graph"), tg, std::stoi(config->GetOrSet("parallelism", "10"))).FlatMap([](const std::string_view& line) {
          DatasetPartition<std::pair<int64_t, int64_t>> ret(ParseLine(line));
          return ret;
        });
    auto key_selector = [](const std::pair<int64_t, int64_t>& pair) { return pair.first; };
    auto combiner = [](std::pair<int64_t, int64_t>& lhs, const std::pair<int64_t, int64_t>& rhs) { lhs.second += rhs.second; };
    auto dataB = dataA.ReduceBy(key_selector, combiner);
    auto dataC = dataB.MapPartition([](const DatasetPartition<std::pair<int64_t, int64_t>>& data) {
      DatasetPartition<std::pair<int64_t, int64_t>> ret;
      int64_t x = 0, y = 0;
      for (const auto& pair : data) {
        x ^= pair.first;
        y ^= pair.second;
      }
      ret.push_back({x, y});
      return ret;
    });
    auto dataD = dataC.ReduceBy([](const std::pair<int64_t, int64_t>& pair) { return 1; },
                                [](std::pair<int64_t, int64_t>& lhs, const std::pair<int64_t, int64_t>& rhs) {
                                  lhs.first ^= rhs.first;
                                  lhs.second ^= rhs.second;
                                },
                                1);
    dataD.ApplyRead([](const DatasetPartition<std::pair<int64_t, int64_t>>& data) {
      for (const auto& pair : data) {
        DLOG(INFO) << "x is " << pair.first << " y is " << pair.second;
      }
    });
    axe::common::JobDriver::ReversePrintTaskGraph(*tg);
  }
};

int main(int argc, char** argv) {
  axe::common::JobDriver::Run(argc, argv, ReadJob());
  return 0;
}
