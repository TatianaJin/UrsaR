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

# Short command for caching cmake variable
# Usage:
#   cache_variable(<var> <value>:[arg1, arg2, ...])
function(cache_variable var)
    set(__list "")
    foreach(value ${ARGN})
        list(APPEND __list ${value})
    endforeach()
    set(${var} ${__list} CACHE INTERNAL "")
endfunction()

macro(add executable_prefix src_file)
  add_executable(${executable_prefix}JM ${src_file} ${ARGN} $<TARGET_OBJECTS:axe-jm-obj> $<TARGET_OBJECTS:axe-input-obj> ${PROJECT_SOURCE_DIR}/src/common/job_driver.cc)
  add_executable(${executable_prefix}JP ${src_file} $<TARGET_OBJECTS:axe-jp-obj> $<TARGET_OBJECTS:axe-input-obj> ${ARGN} ${PROJECT_SOURCE_DIR}/src/common/job_driver.cc)
  target_link_libraries(${executable_prefix}JM axe-core ${AXE_EXTERNAL_LIB})
  target_link_libraries(${executable_prefix}JP axe-core ${AXE_EXTERNAL_LIB})
  set_target_properties(${executable_prefix}JM PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD} COMPILE_DEFINITIONS "IsJobManager")
  set_target_properties(${executable_prefix}JP PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})
  set_property(TARGET ${executable_prefix}JM PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set_property(TARGET ${executable_prefix}JP PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endmacro()

macro(add_simulation executable_prefix src_file)
  add_executable(${executable_prefix}Simulation ${src_file} ${ARGN}
    $<TARGET_OBJECTS:axe-jm-obj> $<TARGET_OBJECTS:axe-jp-obj> $<TARGET_OBJECTS:axe-input-obj>
  )
  target_link_libraries(${executable_prefix}Simulation axe-core ${AXE_EXTERNAL_LIB})
  if (gperf_FOUND)
    target_link_libraries(${executable_prefix}Simulation profiler)
  endif()
  set_target_properties(${executable_prefix}Simulation PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})
  set_property(TARGET ${executable_prefix}Simulation PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endmacro()

macro(app executable_prefix src_file)
  add_executable(${executable_prefix}JM ${src_file} ${ARGN} $<TARGET_OBJECTS:axe-jm-obj> $<TARGET_OBJECTS:axe-input-obj> ${PROJECT_SOURCE_DIR}/src/common/job_driver.cc)
  add_executable(${executable_prefix}JP ${src_file} $<TARGET_OBJECTS:axe-jp-obj> $<TARGET_OBJECTS:axe-input-obj> ${ARGN} ${PROJECT_SOURCE_DIR}/src/common/job_driver.cc)
  target_link_libraries(${executable_prefix}JM axe-core ${AXE_EXTERNAL_LIB})
  target_link_libraries(${executable_prefix}JP axe-core ${AXE_EXTERNAL_LIB})
  set_target_properties(${executable_prefix}JM PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD} COMPILE_DEFINITIONS "IsJobManager")
  set_target_properties(${executable_prefix}JP PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})
  set_property(TARGET ${executable_prefix}JM PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/apps)
  set_property(TARGET ${executable_prefix}JP PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/apps)
endmacro()
