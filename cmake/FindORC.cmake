# Copyright 2018 H-AXE
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
#
# The following may be set: (No default or system path used as search path)
# (CMake variable)
#   ORC_HOME
# (Environment variable)
#   ORC_HOME
#
# The following will be set:
#   ORC_FOUND        Package found flag
#   ORC_INCLUDE_DIR  Headers
#   ORC_LIBRARIES    Static Libraries

if("${ORC_HOME}" STREQUAL "" AND DEFINED ENV{ORC_HOME})
  set(ORC_HOME "$ENV{ORC_HOME}")
endif()
if(NOT "${ORC_HOME}" STREQUAL "")
  file(TO_CMAKE_PATH ${ORC_HOME} _orc_path)
endif()
message(STATUS "ORC_HOME:  ${ORC_HOME}")

set(FIND_PATH_CONF NO_DEFAULT_PATH NO_SYSTEM_ENVIRONMENT_PATH)


### Includes
find_path(ORC_INCLUDE_DIR NAMES orc/OrcFile.hh HINTS ${_orc_path} ${FIND_PATH_CONF} PATH_SUFFIXES "include")


### Library
# Protobuf
find_library(ORC_L0 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}protobuf${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS ${_orc_path} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
if (NOT ORC_L0 AND DEFINED ENV{PROTOBUF_HOME})
  find_library(ORC_L0 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}protobuf${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS $ENV{PROTOBUF_HOME} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
endif()
# Zlib
find_library(ORC_L1 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS ${_orc_path} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
if (NOT ORC_L1 AND DEFINED ENV{ZLIB_HOME})
  find_library(ORC_L1 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS $ENV{ZLIB_HOME} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
endif()
# LZ4
find_library(ORC_L2 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS ${_orc_path} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
if (NOT ORC_L2 AND DEFINED ENV{LZ4_HOME})
  find_library(ORC_L2 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}lz4${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS $ENV{LZ4_HOME} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
endif()
# Snappy
find_library(ORC_L3 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}snappy${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS ${_orc_path} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
if (NOT ORC_L3 AND DEFINED ENV{SNAPPY_HOME})
  find_library(ORC_L3 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}snappy${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS $ENV{SNAPPY_HOME} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})
endif()
# ORC
find_library(ORC_L4 NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}orc${CMAKE_STATIC_LIBRARY_SUFFIX} HINTS ${_orc_path} PATH_SUFFIXES "lib" ${FIND_PATH_CONF})


if(ORC_INCLUDE_DIR AND ORC_L4 AND ORC_L0 AND ORC_L0 AND ORC_L2 AND ORC_L3)
  set(ORC_FOUND true)
else()
  set(ORC_FOUND false)
endif()
if(ORC_FOUND)
  set(ORC_LIBRARIES ${ORC_L4} ${ORC_L3} ${ORC_L2} ${ORC_L1} ${ORC_L0})
  message(STATUS "Found ORC:")
  message(STATUS "  (Headers)       ${ORC_INCLUDE_DIR}")
  message(STATUS "  (Library)       ${ORC_L4}")
  message(STATUS "  (Library)       ${ORC_L3}")
  message(STATUS "  (Library)       ${ORC_L2}")
  message(STATUS "  (Library)       ${ORC_L1}")
  message(STATUS "  (Library)       ${ORC_L0}")
endif()
