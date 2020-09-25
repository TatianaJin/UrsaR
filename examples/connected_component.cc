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

#include <memory>
#include <numeric>
#include <queue>
#include <vector>

#include "glog/logging.h"

#include "common/engine.h"

using AnsType = std::pair<int, std::pair<int, bool>>;

class Vertex {
 public:
  Vertex() : adj_(std::make_shared<std::vector<int>>()) {}
  Vertex(int id, const std::shared_ptr<std::vector<int>>& adj) : id_(id), adj_(adj) {}

  int GetId() const { return id_; }
  const std::shared_ptr<std::vector<int>>& GetAdjList() const { return adj_; }

  bool operator<(const Vertex& other) const { return id_ < other.id_; }
  bool operator==(const Vertex& other) const { return id_ == other.id_; }

  friend void operator<<(axe::base::BinStream& bin_stream, const Vertex& v) { bin_stream << v.id_ << *(v.adj_); }
  friend void operator>>(axe::base::BinStream& bin_stream, Vertex& v) { bin_stream >> v.id_ >> *(v.adj_); }

 private:
  int id_;
  std::shared_ptr<std::vector<int>> adj_;
};

int ReadInt(const std::string& line, size_t& ptr) {
  int ret = 0;
  while (ptr < line.size() && !isdigit(line.at(ptr)))
    ++ptr;
  CHECK(ptr < line.size()) << "Invalid Input";
  while (ptr < line.size() && isdigit(line.at(ptr))) {
    ret = ret * 10 + line.at(ptr) - '0';
    ++ptr;
  }
  return ret;
}

DatasetPartition<Vertex> ParseLine(const std::string& line) {
  size_t ptr = 0;
  int id, k;
  auto adj = std::make_shared<std::vector<int>>();
  id = ReadInt(line, ptr);
  k = ReadInt(line, ptr);
  adj->reserve(k);
  for (int i = 0; i < k; ++i) {
    adj->push_back(ReadInt(line, ptr));
  }
  DatasetPartition<Vertex> ret;
  ret.push_back(Vertex(id, adj));
  return ret;
}

class ConnectedComponent : public Job {
 public:
  void Run(TaskGraph* tg, const std::shared_ptr<Properties>& config) const override {
    auto input = config->GetOrSet("graph", "/datasets/graph/google-adj");
    int n_partitions = std::stoi(config->GetOrSet("parallelism", "20"));
    int n_iters = std::stoi(config->GetOrSet("n_iters", "5"));

    // Load data, partition by id, and sort within partition
    auto graph = TextSourceDataset(input, tg, n_partitions)
                     .FlatMap([](const std::string& line) { return ParseLine(line); })
                     .PartitionBy([](const Vertex& v) { return v.GetId(); }, n_partitions);
    graph.UpdatePartition([](DatasetPartition<Vertex>& data) {
      std::sort(data.begin(), data.end(), [](const Vertex& a, const Vertex& b) { return a.GetId() < b.GetId(); });
    });

    // initialize answers
    auto answer = std::make_shared<axe::common::Dataset<AnsType>>(graph.MapPartition([](const DatasetPartition<Vertex>& data) {
      DatasetPartition<AnsType> ret;
      ret.reserve(data.size());
      for (auto& v : data) {
        ret.push_back(std::make_pair(v.GetId(), std::make_pair(v.GetId(), 1)));
      }
      return ret;
    }));

    int iter = 0;
    auto send_updates = [iter](const DatasetPartition<Vertex>& data, const DatasetPartition<AnsType>& answer) {
      DatasetPartition<std::pair<int, int>> updates;
      int local_id = 0;
      for (const Vertex& v : data) {
        // simply ignore those with zero out degree
        while (local_id < answer.size() && answer.at(local_id).first < v.GetId()) {
          ++local_id;
        }
        if (answer.at(local_id).second.second == 0)
          continue;
        int neighbor_id = 0;
        int v_answer = answer.at(local_id).second.first;
        for (auto neighbor : *v.GetAdjList()) {
          if (v.GetId() < neighbor) {
            updates.push_back(std::make_pair(neighbor, v_answer));
          }
        }
        ++local_id;
      }
      LOG(INFO) << " iter : " << iter << " update size : " << updates.size();
      return updates;
    };

    // main loop
    for (iter = 0; iter < n_iters; ++iter) {
      // TODO (lby) : consider an updata-based version
      answer = std::make_shared<axe::common::Dataset<AnsType>>(answer->SharedDataMapPartitionWith(
          std::make_shared<axe::common::Dataset<std::pair<int, int>>>(
              graph.SharedDataMapPartitionWith(answer.get(), send_updates)
                  .ReduceBy([](const std::pair<int, int>& msg) { return msg.first; },
                            [](std::pair<int, int>& agg, const std::pair<int, int>& update) { agg.second = std::min(agg.second, update.second); },
                            n_partitions))
              .get(),
          [](const DatasetPartition<AnsType>& old, const DatasetPartition<std::pair<int, int>>& updates) {
            DatasetPartition<AnsType> ret;
            int idx = 0;
            for (const auto& field : old) {
              while (idx < updates.size() && updates.at(idx).first < field.first)
                idx++;
              if (idx < updates.size() && updates.at(idx).second < field.second.first)
                ret.push_back(std::make_pair(field.first, std::make_pair(updates.at(idx).second, 1)));
              else
                ret.push_back(std::make_pair(field.first, std::make_pair(field.second.first, 0)));
            }
            return ret;
          }));
    }

    axe::common::JobDriver::ReversePrintTaskGraph(*tg);
  }
};

int main(int argc, char** argv) {
  axe::common::JobDriver::Run(argc, argv, ConnectedComponent());
  return 0;
}
