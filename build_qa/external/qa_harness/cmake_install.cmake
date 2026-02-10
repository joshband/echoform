# Install script for directory: /Users/artbox/Documents/Repos/echoform/external/qa_harness

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/artbox/Documents/Repos/echoform/build_qa/_deps/yaml-cpp-build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/libqa_core.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_core.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_core.a")
    execute_process(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_core.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/libqa_runners.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_runners.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_runners.a")
    execute_process(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_runners.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/libqa_scenario_engine.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_scenario_engine.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_scenario_engine.a")
    execute_process(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libqa_scenario_engine.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/qa_config.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/qa_all.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/core" TYPE FILE FILES
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/core/dsp_under_test.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/core/qa_runner.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/core/midi_events.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/core/effect_capabilities.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/core/allocation_tracker.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/core/performance_metrics.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/core/texture_qa" TYPE DIRECTORY FILES "/Users/artbox/Documents/Repos/echoform/external/qa_harness/core/texture_qa/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/analysis" TYPE FILE FILES
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/analysis/audio_metrics.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/analysis/envelope.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/analysis/csv_writer.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/analysis/spectral_analysis.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/runners" TYPE FILE FILES
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/runners/in_process_runner.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/runners/standalone_binary_runner.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/runners/binary_executor.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/runners/process_binary_executor.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/runners/performance_profiler.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/scenario_engine" TYPE FILE FILES
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/scenario_types.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/stimulus_injection_dut.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/stimulus_generators.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/scenario_loader.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/scenario_executor.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/invariant_evaluator.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/audio_analyzer.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/metric_evaluator.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/midi_pattern_validator.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/test_suite_types.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/test_suite_loader.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/test_suite_executor.h"
    "/Users/artbox/Documents/Repos/echoform/external/qa_harness/scenario_engine/preset_scanner.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/third_party" TYPE FILE FILES "/Users/artbox/Documents/Repos/echoform/external/qa_harness/third_party/json.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_dsp_qa_harness/audio_dsp_qa_harnessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_dsp_qa_harness/audio_dsp_qa_harnessTargets.cmake"
         "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/CMakeFiles/Export/98170f2475f56ff167fc4cfb3dda3c3b/audio_dsp_qa_harnessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_dsp_qa_harness/audio_dsp_qa_harnessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_dsp_qa_harness/audio_dsp_qa_harnessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_dsp_qa_harness" TYPE FILE FILES "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/CMakeFiles/Export/98170f2475f56ff167fc4cfb3dda3c3b/audio_dsp_qa_harnessTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_dsp_qa_harness" TYPE FILE FILES "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/CMakeFiles/Export/98170f2475f56ff167fc4cfb3dda3c3b/audio_dsp_qa_harnessTargets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_dsp_qa_harness" TYPE FILE FILES
    "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/audio_dsp_qa_harnessConfig.cmake"
    "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/audio_dsp_qa_harnessConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/audio_dsp_qa_harness.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/artbox/Documents/Repos/echoform/build_qa/external/qa_harness/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
