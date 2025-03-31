
set(KSTD_TARGET kstd)
file(GLOB_RECURSE KSTD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${KSTD_TARGET} STATIC ${KSTD_SRC})
target_include_directories(${KSTD_TARGET} PRIVATE ${KSTD_INCLUDE})
target_link_libraries(${KSTD_TARGET} PUBLIC ${KSTD_LIBS})
target_compile_options(${KSTD_TARGET} PRIVATE ${KSTD_COMPILER_FLAGS})
