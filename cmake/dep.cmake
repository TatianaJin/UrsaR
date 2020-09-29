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

include(ExternalProject)

### pthread
find_package(Threads)
list(APPEND AXE_EXTERNAL_LIB ${CMAKE_THREAD_LIBS_INIT})

# test that filesystem header actually is there and works
try_compile(HAS_FS "${PROJECT_BINARY_DIR}/tmp"
  "${PROJECT_SOURCE_DIR}/test/has_filesystem.cc"
  CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
  LINK_LIBRARIES stdc++fs)
if(HAS_FS)
  message(STATUS "Compiler has filesystem support")
  list(APPEND AXE_EXTERNAL_LIB stdc++fs)
else()
	# Boost filesystem
	set(BoostFS_VERSION "1.74.0")
	# TODO(tatiana)
	#find_package(Boost ${BoostFS_VERSION} COMPONENTS filesystem)
  message(FATAL_ERROR "Compiler is missing filesystem capabilities")
endif(HAS_FS)

### gflags
find_package(gflags REQUIRED)
message(STATUS "Found gflags:")
message(STATUS " (Headers)      ${gflags_INCLUDE_DIR}")
message(STATUS " (Library)      ${gflags_LIBRARIES}")
list(APPEND AXE_EXTERNAL_INCLUDES ${gflags_INCLUDE_DIR})
list(APPEND AXE_EXTERNAL_LIB ${gflags_LIBRARIES})


### glog
find_path(glog_INCLUDE_DIR NAMES glog/logging.h)
find_library(glog_LIBRARY NAMES glog)
if(glog_INCLUDE_DIR AND glog_LIBRARY)
  message(STATUS "Found glog:")
  message(STATUS "  (Headers)       ${glog_INCLUDE_DIR}")
  message(STATUS "  (Library)       ${glog_LIBRARY}")
  list(APPEND AXE_EXTERNAL_INCLUDES ${glog_INCLUDE_DIR})
  list(APPEND AXE_EXTERNAL_LIB ${glog_LIBRARY})
else()
  message(ERROR "glog not found")
endif()


### zmq
find_path(ZMQ_INCLUDE_DIR NAMES zmq.hpp)
find_library(ZMQ_LIBRARY NAMES zmq)
if(ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY)
  message(STATUS "Found ZeroMQ:")
  message(STATUS "  (Headers)       ${ZMQ_INCLUDE_DIR}")
  message(STATUS "  (Library)       ${ZMQ_LIBRARY}")
  list(APPEND AXE_EXTERNAL_INCLUDES ${ZMQ_INCLUDE_DIR})
  list(APPEND AXE_EXTERNAL_LIB ${ZMQ_LIBRARY})
else()
  message(ERROR "ZeroMQ not found")
endif()


### libhdfs3
if (DEFINED ENV{LIBHDFS3_HOME})
  set(LIBHDFS3_HOME $ENV{LIBHDFS3_HOME})
  find_path(LIBHDFS3_INCLUDE_DIR NAMES hdfs/hdfs.h PATHS ${LIBHDFS3_HOME} PATH_SUFFIXES "include" NO_DEFAULT_PATH)
  find_library(LIBHDFS3_LIBRARY NAMES hdfs3 PATHS ${LIBHDFS3_HOME} PATH_SUFFIXES "lib" NO_DEFAULT_PATH)
else()
  find_path(LIBHDFS3_INCLUDE_DIR NAMES hdfs/hdfs.h PATHS ${LIBHDFS3_HOME} PATH_SUFFIXES "include")
  find_library(LIBHDFS3_LIBRARY NAMES hdfs3 PATHS ${LIBHDFS3_HOME} PATH_SUFFIXES "lib")
endif()
if(LIBHDFS3_INCLUDE_DIR AND LIBHDFS3_LIBRARY)
  set(LIBHDFS_FOUND true)
  message(STATUS "Found libhdfs3:")
  message(STATUS "  (Headers)       ${LIBHDFS3_INCLUDE_DIR}")
  message(STATUS "  (Library)       ${LIBHDFS3_LIBRARY}")
  list(APPEND AXE_EXTERNAL_INCLUDES ${LIBHDFS3_INCLUDE_DIR})
  list(APPEND AXE_EXTERNAL_LIB ${LIBHDFS3_LIBRARY})
else()
  message(WARNING "LibHDFS not found")
endif()

### ORC
if (USE_ORC)
  set(ORC_DEFINITION "-DWITH_ORC")
  find_package(ORC)
  if (ORC_FOUND)
    list(APPEND AXE_EXTERNAL_INCLUDES ${ORC_INCLUDE_DIR})
    list(APPEND AXE_EXTERNAL_LIB ${ORC_LIBRARIES})
    list(APPEND AXE_EXTERNAL_DEFINITION ${ORC_DEFINITION})
  else()
    message(STATUS "ORC not found")
  endif()
endif()

# Json
set(JSON_VERSION "3.4.0")
set(THIRDPARTY_DIR ${PROJECT_BINARY_DIR}/third_party)

if (NOT "${JSON_INCLUDE_DIR}" STREQUAL "")
  file(TO_CMAKE_PATH ${JSON_INCLUDE_DIR} _json_path)
elseif()
  file(TO_CMAKE_PATH ${THIRDPARTY_DIR} _json_path)
endif()

find_path (JSON_INCLUDE_DIR NAMES "nlohmann/json.hpp" HINTS ${_json_path})

if (JSON_INCLUDE_DIR)
  list(APPEND AXE_EXTERNAL_INCLUDES ${JSON_INCLUDE_DIR})
  message (STATUS "Found nlohmann/json.hpp:")
  message (STATUS " (Headers)   ${JSON_INCLUDE_DIR}")
else()
  ExternalProject_Add(json_ep
    PREFIX ${THIRDPARTY_DIR}/nlohmann
    DOWNLOAD_DIR ${THIRDPARTY_DIR}/nlohmann
    DOWNLOAD_NO_EXTRACT true
    SOURCE_DIR ""
    BINARY_DIR ""
    URL "https://github.com/nlohmann/json/releases/download/v${JSON_VERSION}/json.hpp"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    ${THIRDPARTI_LOG_OPTIONS})
  list(APPEND AXE_EXTERNAL_INCLUDES ${THIRDPARTY_DIR})
endif()


### gperf
set(gperf_DEFINITION "-DWITH_GPERF")
find_path(profiler_INCLUDE NAMES gperftools/profiler.h)
find_library(profiler_LIB NAMES profiler)
if(profiler_INCLUDE AND profiler_LIB)
  set(gperf_FOUND true)
  message(STATUS "Found gperf:")
  message(STATUS "  (Headers)       ${profiler_INCLUDE}")
  message(STATUS "  (Library)       ${profiler_LIB}")
  list(APPEND AXE_EXTERNAL_INCLUDES ${profiler_INCLUDE})
  list(APPEND AXE_EXTERNAL_LIB ${profiler_LIB})
  list(APPEND AXE_EXTERNAL_DEFINITION ${gperf_DEFINITION})
else()
  message(STATUS "gperf not found")
endif()


add_definitions(${AXE_EXTERNAL_DEFINITION})
include_directories(${AXE_EXTERNAL_INCLUDES})
