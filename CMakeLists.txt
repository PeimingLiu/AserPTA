cmake_minimum_required(VERSION 3.4.3)
project(aser_pta)

if (EXISTS /llvm-project/build/lib/cmake/llvm/)
    # We are probably in the dev container
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        # here link to shared llvm
        set(LLVM_DIR /llvm-project/build-shared/lib/cmake/llvm/)
    else()
        set(LLVM_DIR /llvm-project/build/lib/cmake/llvm/)
    endif()
endif()

if(NOT ENABLE_TEST)
    set(ENABLE_TEST Off) # default disable testing
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_ACTIVE_LEVEL=2 -DRACE_DETECT_DEBUG")  # log level = debug
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPDLOG_ACTIVE_LEVEL=6")  # log level = warn
endif()

set(COMMON_CXX_FLAG "-fno-rtti -Wno-c++14-extensions -Wno-c++17-extensions")

# To support both in- and out-of-source builds,
# we check for the presence of the add_llvm_loadable_module command.
# - if this command is not present, we are building out-of-source
if(NOT COMMAND add_llvm_loadable_module)
    if (DEFINED LLVM_DIR)
        # We need to match the build environment for LLVM:
        # In particular, we need C++11 and the -fno-rtti flag
        set(CMAKE_CXX_STANDARD 11)
        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 ${COMMON_CXX_FLAG}")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static -O3 ${COMMON_CXX_FLAG}")
        endif()

        find_package(LLVM 9.0.0 REQUIRED CONFIG NO_DEFAULT_PATH)
        message("FIND LLVM IN ${LLVM_CMAKE_DIR}")

        list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
        include(AddLLVM)

        add_definitions(${LLVM_DEFINITIONS})
        include_directories(${LLVM_INCLUDE_DIRS})

    else()
        message(FATAL_ERROR "\
WARNING: The LLVM_DIR var was not set (required for an out-of-source build)!\n\
Please set this to environment variable to point to the LLVM build directory\
(e.g. on linux: export LLVM_DIR=/path/to/llvm/build/dir)")
    endif()
else()
        set(IN_SOURCE_BUILD 1)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    utils/jsoncons/include
    utils)

add_subdirectory(lib)
add_subdirectory(tools)

# unit tests
if (${ENABLE_TEST})
    enable_testing()

    add_subdirectory(utils/googletest)
    add_subdirectory(unittests)
endif()

