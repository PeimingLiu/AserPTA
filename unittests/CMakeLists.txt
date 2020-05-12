set(UnitTests
  Main.cpp
  RaceDetector/SVFTest.cpp
  RaceDetector/RDUtilTest.cpp)

  llvm_map_components_to_libnames(llvm_libs
  bitwriter
  core
  ipo
  irreader
  instcombine
  instrumentation
  target
  linker
  analysis
  scalaropts
  support
  transformutils
  # needed when link llvm as shared library
  AggressiveInstCombine
  demangle
  mc)


add_executable(cr_unittest ${UnitTests})

include_directories(
  ${CMAKE_SOURCE_DIR}/tools/RaceDetector/include
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/utils/jsoncons/include
  ${CMAKE_SOURCE_DIR}/utils/googletest/include
)

target_link_libraries(cr_unittest aser_gtest_main)
target_link_libraries(cr_unittest racedetect-lib)
target_link_libraries(cr_unittest aser)
target_link_libraries(cr_unittest ${llvm_libs})


add_test(
  NAME cr_test
  COMMAND ${CMAKE_BINARY_DIR}/unittests/cr_unittest
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/unittests
)

add_subdirectory(PointerAnalysis)