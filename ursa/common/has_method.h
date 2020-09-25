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

#include <iostream>
#include <type_traits>

namespace axe {
namespace common {

template <typename C>
struct HasSize {
 private:
  template <typename T>
  static constexpr auto check(T*) -> typename std::is_arithmetic<decltype(std::declval<T>().size())>::type;

  template <typename>
  static constexpr std::false_type check(...);

  using type = decltype(check<C>(nullptr));

 public:
  static constexpr bool value = type::value;
};

template <typename C>
struct HasGetMemory {
 private:
  template <typename T>
  static constexpr auto check(T*) -> typename std::is_arithmetic<decltype(std::declval<T>().GetMemory())>::type;

  template <typename>
  static constexpr std::false_type check(...);

  using type = decltype(check<C>(nullptr));

 public:
  static constexpr bool value = type::value;
};

}  //  namespace common
}  // namespace axe
