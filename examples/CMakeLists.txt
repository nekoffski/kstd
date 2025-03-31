
file(GLOB EXAMPLES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(EXAMPLE ${EXAMPLES})
    message("Processing example binary" ${EXAMPLE})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WLE)

    add_executable(${EXAMPLE_NAME} ${EXAMPLE})
    target_include_directories(${EXAMPLE_NAME} PUBLIC ${KSTD_INCLUDE})
    target_link_libraries(${EXAMPLE_NAME} kstd)
endforeach()
