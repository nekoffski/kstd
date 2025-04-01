cmake_minimum_required(VERSION 3.10)
project(kstd CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(KSTD_INCLUDE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${fmt_INCLUDE_DIRS}
    ${spdlog_INCLUDE_DIRS}
    ${gtest_INCLUDE_DIRS}
    ${benchmark_INCLUDE_DIRS}
    ${boost_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
)

# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_definitions(-DKSTD_ENABLE_LOGGING=1)

set(GCC_COVERAGE_COMPILE_FLAGS 
    "--coverage -O0 -fprofile-arcs -ftest-coverage -fno-elide-constructors -fno-inline")
set(GCC_COVERAGE_LINK_FLAGS "-lgcov")
set(KSTD_COMPILER_FLAGS "")

find_package(spdlog)
find_package(fmt)
find_package(GTest)
find_package(benchmark)
find_package(Boost)
find_package(nlohmann_json)

set(KSTD_LIBS spdlog::spdlog fmt::fmt gtest::gtest benchmark::benchmark_main 
    boost::boost nlohmann_json::nlohmann_json)

option(KSTD_ENABLE_UNIT_TESTS "Build unit tests" OFF)
option(KSTD_ENABLE_BENCHMARKS "Build benchmakrs " OFF)
option(KSTD_ENABLE_COVERAGE "Enable code coverage" OFF)

if(KSTD_ENABLE_COVERAGE)
    message("-- Triggering build with coverage support")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
    string(REPLACE "-O3" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
endif()

set(KSTD_COMPILER_FLAGS -Wall -Wextra -Wpedantic -Werror)

if(KSTD_ENABLE_UNIT_TESTS)
    enable_testing()
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)

file(GLOB_RECURSE HEADERS "${KSTD_INCLUDE}/*.hh")

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/kstd)

install(TARGETS kstd)
install(FILES ${SRC}/Concepts.hh ${SRC}/Core.hh ${SRC}/Log.hh ${SRC}/Math.hh DESTINATION include/kstd)
