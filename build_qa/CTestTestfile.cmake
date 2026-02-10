# CMake generated Testfile for 
# Source directory: /Users/artbox/Documents/Repos/echoform
# Build directory: /Users/artbox/Documents/Repos/echoform/build_qa
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(MemoryDelayEngineTests "/Users/artbox/Documents/Repos/echoform/build_qa/MemoryDelayEngineTests_artefacts/MemoryDelayEngineTests")
set_tests_properties(MemoryDelayEngineTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/artbox/Documents/Repos/echoform/CMakeLists.txt;80;add_test;/Users/artbox/Documents/Repos/echoform/CMakeLists.txt;0;")
add_test(echoform_qa "/Users/artbox/Documents/Repos/echoform/build_qa/echoform_qa_artefacts/echoform_qa")
set_tests_properties(echoform_qa PROPERTIES  _BACKTRACE_TRIPLES "/Users/artbox/Documents/Repos/echoform/CMakeLists.txt;117;add_test;/Users/artbox/Documents/Repos/echoform/CMakeLists.txt;0;")
subdirs("_deps/juce-build")
subdirs("external/qa_harness")
