set(HTELEM_TEST_SOURCES test_runner.cpp simple_element_tests.cpp braid_tests.cpp braided_renderer_tests.cpp type_util_tests.cpp)
add_executable(htelem_tests ${HTELEM_TEST_SOURCES})

target_link_libraries(htelem_tests PUBLIC htelem)
warn_format(htelem_tests ${HTELEM_TEST_SOURCES})

