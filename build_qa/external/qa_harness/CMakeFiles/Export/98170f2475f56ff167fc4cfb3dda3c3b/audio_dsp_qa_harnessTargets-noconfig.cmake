#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "qa::qa_core" for configuration ""
set_property(TARGET qa::qa_core APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(qa::qa_core PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libqa_core.a"
  )

list(APPEND _cmake_import_check_targets qa::qa_core )
list(APPEND _cmake_import_check_files_for_qa::qa_core "${_IMPORT_PREFIX}/lib/libqa_core.a" )

# Import target "qa::qa_runners" for configuration ""
set_property(TARGET qa::qa_runners APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(qa::qa_runners PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libqa_runners.a"
  )

list(APPEND _cmake_import_check_targets qa::qa_runners )
list(APPEND _cmake_import_check_files_for_qa::qa_runners "${_IMPORT_PREFIX}/lib/libqa_runners.a" )

# Import target "qa::qa_scenario_engine" for configuration ""
set_property(TARGET qa::qa_scenario_engine APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(qa::qa_scenario_engine PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libqa_scenario_engine.a"
  )

list(APPEND _cmake_import_check_targets qa::qa_scenario_engine )
list(APPEND _cmake_import_check_files_for_qa::qa_scenario_engine "${_IMPORT_PREFIX}/lib/libqa_scenario_engine.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
