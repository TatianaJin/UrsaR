# Copyright 2020 HDL
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(ursa)
cmake_minimum_required(VERSION 3.0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### Options
option(ADDRESS_SANITIZER "Enable address sanitizer in debug mode" OFF)

### Build Config
if(CMAKE_COMPILER_IS_GNUCXX)
  if(ADDRESS_SANITIZER) # Check memory problems
    message(STATUS "Address sanitizer is enabled in debug mode")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -DURSA_DEBUG_MODE -fsanitize=address")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -DURSA_DEBUG_MODE")
  endif()
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DURSA_DEBUG_MODE")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -w -DNDEBUG")
endif()

### Dependencies
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(dep)

find_library(URSA_LIB NAMES axe HINTS lib lib/static)
find_library(URSA_LIB2 NAMES axe-core HINTS lib lib/static)
if (URSA_LIB)
  message(STATUS "Found Ursa library ${URSA_LIB}")
  if (URSA_LIB2)
    message(STATUS "Found Ursa library ${URSA_LIB2}")
  else()
    message(FATAL_ERROR "Cannot find ursa core library")
  endif()
else()
  message(FATAL_ERROR "Cannot find ursa library")
endif()
set(URSA_LIBS ${URSA_LIB} ${URSA_LIB2})

find_path(URSA_INCLUDE NAMES common/engine.h HINTS ursa)
if (URSA_INCLUDE)
  message(STATUS "Found Ursa includes ${URSA_INCLUDE}")
else()
  message(FATAL_ERROR "Cannot find ursa includes")
endif()
include_directories(${URSA_INCLUDE})

macro(add executable_prefix src_file)
  add_executable(${executable_prefix}JM ${src_file} ${ARGN})
  add_executable(${executable_prefix}JP ${src_file} ${ARGN})
  target_link_libraries(${executable_prefix}JM ${URSA_LIBS} ${AXE_EXTERNAL_LIB})
  target_link_libraries(${executable_prefix}JP ${URSA_LIBS} ${AXE_EXTERNAL_LIB})
  set_target_properties(${executable_prefix}JM PROPERTIES COMPILE_DEFINITIONS "IsJobManager")
  set_property(TARGET ${executable_prefix}JM PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set_property(TARGET ${executable_prefix}JP PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endmacro()

add(HelloWorld examples/hello_world.cc)
add(Read examples/read.cc)
add(ConnectedComponent examples/connected_component.cc)
add(PageRank examples/pr.cc)
add(WordCount examples/word_count.cc)
add(LogisticRegression examples/lr.cc)
add(KMeans examples/kmeans.cc)
