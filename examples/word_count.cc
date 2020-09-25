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

#include <cctype>
#include <memory>
#include <string>
#include <utility>

#include "glog/logging.h"

#include "base/tokenizer.h"
#include "common/engine.h"

using axe::base::Tokenizer;

void ParseLine(DatasetPartition<std::pair<std::string, int>>& collection, const std::string& line) {
  if (line.empty()) {
    return;
  }
  axe::base::WhiteSpaceTokenizer tokenizer(line);
  std::string tok;
  std::unordered_map<std::string, int> word_count;
  while (tokenizer.next(tok)) {
    word_count[std::move(tok)] += 1;
  }
  for (auto& pair : word_count) {
    collection.push_back(pair);
  }
}

class WordCountJob : public Job {
 public:
  void Run(TaskGraph* tg, const std::shared_ptr<Properties>& config) const override {
    TextSourceDataset(config->Get("input"), tg, std::stoi(config->Get("parallelism")))
        .FlatMap([](const std::string& line) {
          DatasetPartition<std::pair<std::string, int>> ret;
          ParseLine(ret, line);
          return ret;
        })
        .ReduceBy([](const std::pair<std::string, int>& ele) { return ele.first; },
                  [](std::pair<std::string, int>& agg, const std::pair<std::string, int>& update) { agg.second += update.second; })
        .MapPartition([](DatasetPartition<std::pair<std::string, int>> data) {
          DatasetPartition<int> count;
          count.push_back(data.size());
          LOG(INFO) << "Received " << data.size() << " distinct words";
          return count;
        })
        // Reduce to one worker to get the number of distinct words
        .ReduceBy([](int ele) { return 1; }, [](int& agg, int update) { agg += update; }, 1)
        .ApplyRead([](auto data) {
          LOG(INFO) << data.front();
          google::FlushLogFiles(google::INFO);
        });
  }
};

int main(int argc, char** argv) {
  axe::common::JobDriver::Run(argc, argv, WordCountJob());
  return 0;
}
