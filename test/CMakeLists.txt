
set(UTILS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/utils)
set(LIBS kstd gtest::gtest)

function(add_test_file TEST_FILE)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WLE)
    set(TEST_EXE_NAME kstd.${TEST_NAME})
    add_executable(${TEST_EXE_NAME} ${TEST_FILE})

    target_link_libraries(${TEST_EXE_NAME} PUBLIC ${LIBS})
    target_include_directories(${TEST_EXE_NAME} PUBLIC ${KSTD_INCLUDE} ${UTILS_INCLUDE})
    add_test(NAME kstd.${TEST_NAME} COMMAND ${TEST_EXE_NAME})
endfunction()

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Tests*.cpp)
foreach(TEST_FILE ${TEST_FILES})
    add_test_file(${TEST_FILE} ${MODULE_NAME})
endforeach()
