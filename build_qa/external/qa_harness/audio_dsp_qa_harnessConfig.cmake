
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was audio_dsp_qa_harnessConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

# yaml-cpp is a private dependency of qa_core (used internally by SpecLoader).
# It must be available at link time for static library consumers.
# Only required when the harness was built with YAML support.
set(_qa_yaml_support ON)
if(_qa_yaml_support)
    find_dependency(yaml-cpp)
endif()
unset(_qa_yaml_support)

# nlohmann_json is bundled as a header-only INTERFACE library in the export set.
# No find_dependency needed — it's included in the exported targets.

include("${CMAKE_CURRENT_LIST_DIR}/audio_dsp_qa_harnessTargets.cmake")

check_required_components(audio_dsp_qa_harness)
