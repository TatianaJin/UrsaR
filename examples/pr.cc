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

class Vertex {
 public:
  Vertex() : adj_(std::make_shared<std::vector<int>>()) {}
  Vertex(int id, const std::shared_ptr<std::vector<int>>& adj) : id_(id), adj_(adj) {}

  int GetId() const { return id_; }
  const std::shared_ptr<std::vector<int>>& GetAdjList() const { return adj_; }

  double GetMemory() const {
    double ret = sizeof(int) + adj_->size() * sizeof(int);
    return ret;
  }
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

class PageRank : public Job {
 public:
  void Run(TaskGraph* tg, const std::shared_ptr<Properties>& config) const override {
    auto input = config->GetOrSet("graph", "/graph/google-adj");
    int n_partitions = std::stoi(config->GetOrSet("parallelism", "20"));
    int n_iters = std::stoi(config->GetOrSet("n_iters", "5"));

    // Load data, partition by id, and sort within partition
    auto graph = TextSourceDataset(input, tg, n_partitions)
                     .FlatMap([](const std::string& line) { return ParseLine(line); },
                              [](const std::vector<double>& input) {
                                double ret = 0;
                                for (double x : input) {
                                  ret += x;
                                }
                                return ret * 2;
                              })
                     .PartitionBy([](const Vertex& v) { return v.GetId(); }, n_partitions);
    graph.UpdatePartition([](DatasetPartition<Vertex>& data) {
      std::sort(data.begin(), data.end(), [](const Vertex& a, const Vertex& b) { return a.GetId() < b.GetId(); });
    });

    // initialize ranks
    auto rank_ptr = std::make_shared<axe::common::Dataset<std::pair<int, double>>>(graph.MapPartition([](const DatasetPartition<Vertex>& data) {
      DatasetPartition<std::pair<int, double>> ret;
      ret.reserve(data.size());
      for (auto& v : data) {
        ret.push_back(std::make_pair(v.GetId(), 100.0));
      }
      return ret;
    }));

    auto send_updates = [](const DatasetPartition<Vertex>& data, const DatasetPartition<std::pair<int, double>>& rank) {
      DatasetPartition<std::pair<int, double>> updates;
      int local_id = 0;
      for (const Vertex& v : data) {
        // simply ignore those with zero out degree
        while (local_id < rank.size() && rank.at(local_id).first < v.GetId()) {
          ++local_id;
        }
        DCHECK_EQ(rank.at(local_id).first, v.GetId());
        double distribute = rank.at(local_id).second / v.GetAdjList()->size() * 0.8;
        updates.push_back(std::make_pair(v.GetId(), 0.2));
        for (auto neighbor : *v.GetAdjList()) {
          updates.push_back(std::make_pair(neighbor, distribute));
        }
        ++local_id;
      }
      return updates;
    };

    // main loop
    for (int iter = 0; iter < n_iters; ++iter) {
      rank_ptr = std::make_shared<axe::common::Dataset<std::pair<int, double>>>(
          graph.SharedDataMapPartitionWith(rank_ptr.get(), send_updates)
              .ReduceBy([](const std::pair<int, double>& id_rank) { return id_rank.first; },
                        [](std::pair<int, double>& agg, const std::pair<int, double>& update) { agg.second += update.second; }, n_partitions));
    }

    // select top 10
    auto select_top10 = [](const DatasetPartition<std::pair<int, double>>& data) {
      DatasetPartition<std::pair<int, double>> ret;
      auto greater = [&data](int a, int b) { return data.at(a).second > data.at(b).second; };
      std::priority_queue<int, std::vector<int>, decltype(greater)> top10(greater);
      int idx = 0;
      for (auto& rank : data) {
        if (top10.size() < 10) {
          top10.push(idx);
        } else if (rank.second > data.at(top10.top()).second) {
          top10.pop();
          top10.push(idx);
        }
        ++idx;
      }
      while (!top10.empty()) {
        ret.push_back(data.at(top10.top()));
        top10.pop();
      }
      return ret;
    };
    rank_ptr->MapPartition(select_top10)
        .PartitionBy([](const std::pair<int, double>&) { return 0; }, 1)
        .MapPartition(select_top10)
        .ApplyRead([](const DatasetPartition<std::pair<int, double>>& data) {
          for (auto& rank : data) {
            LOG(INFO) << "id: " << rank.first << ", rank: " << rank.second;
          }
          google::FlushLogFiles(google::INFO);
        });
    axe::common::JobDriver::ReversePrintTaskGraph(*tg);
  }
};

int main(int argc, char** argv) {
  axe::common::JobDriver::Run(argc, argv, PageRank());
  return 0;
}
