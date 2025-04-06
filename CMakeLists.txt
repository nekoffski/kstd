cmake_minimum_required(VERSION 3.10)
project(kstd CXX)

SET(KSTD_ENABLE_COVERAGE OFF CACHE BOOL "Coverage option")

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

add_definitions(-DKSTD_ENABLE_INTERNAL_LOGGING=1)

set(GCC_COVERAGE_COMPILE_FLAGS 
    "--coverage -O0 -fprofile-arcs -ftest-coverage -fno-elide-constructors -fno-inline")
set(GCC_COVERAGE_LINK_FLAGS "-lgcov")
set(KSTD_COMPILER_FLAGS "")

find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(KSTD_LIBS spdlog::spdlog fmt::fmt gtest::gtest benchmark::benchmark_main 
        boost::boost nlohmann_json::nlohmann_json)

if(KSTD_ENABLE_COVERAGE)
    message("-- Triggering build with coverage support")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
    string(REPLACE "-O3" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else()
    message ("-- Coverage disabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
endif()

set(KSTD_COMPILER_FLAGS -Wall -Wextra -Wpedantic -Werror)

add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)

file(GLOB_RECURSE HEADERS "${KSTD_INCLUDE}/*.hh")

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/kstd)

install(TARGETS kstd)
install(
    FILES 
        ${SRC}/Concepts.hh 
        ${SRC}/Core.hh 
        ${SRC}/Log.hh 
        ${SRC}/Math.hh 
        ${SRC}/Functional.hh 
        ${SRC}/Macros.hh 
        ${SRC}/Scope.hh 
        ${SRC}/memory/AlignedAllocator.hh
        ${SRC}/memory/Allocator.hh
        ${SRC}/memory/Mallocator.hh
        ${SRC}/memory/PoolAllocator.hh
        ${SRC}/memory/UniquePtr.hh
        ${SRC}/memory/SharedPtr.hh
    DESTINATION include/kstd)
