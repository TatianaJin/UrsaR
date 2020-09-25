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

#include <cstdint>
#include <utility>
#include <vector>

namespace axe {
namespace base {

int64_t Now();

void GetSubgraphOrderHelper(const std::vector<std::vector<int>>& dep, std::vector<int>& order, std::vector<bool>& visited, int subgraph_id);

using pdd = std::pair<double, double>;

template <typename ValueT>
std::pair<ValueT, ValueT> operator+(const std::pair<ValueT, ValueT>& lhs, const std::pair<ValueT, ValueT>& rhs) {
  return {lhs.first + rhs.first, lhs.second + rhs.second};
}

template <typename ValueT>
void operator+=(std::pair<ValueT, ValueT>& lhs, const std::pair<ValueT, ValueT>& rhs) {
  lhs.first += rhs.first;
  lhs.second += rhs.second;
}

template <typename ValueT>
bool operator<(const std::pair<ValueT, ValueT>& lhs, const std::pair<ValueT, ValueT>& rhs) {
  return lhs.first + lhs.second < rhs.first + rhs.second;
}

template <typename ValueT>
std::pair<ValueT, ValueT> operator*(const std::pair<ValueT, ValueT>& lhs, double k) {
  return {lhs.first * k, lhs.second * k};
}

}  // namespace base
}  // namespace axe
