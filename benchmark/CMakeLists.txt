
set(BENCHMARK_LINK gtest::gtest)
set(UTILS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/utils)
set(LIBS kstd benchmark::benchmark_main)

file (GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/Bench*.cpp)
add_executable(kstd.benchmark ${SRC})
target_link_libraries(kstd.benchmark PUBLIC ${LIBS})
target_include_directories(kstd.benchmark PUBLIC ${UTILS_INCLUDE} ${KSTD_INCLUDE})
