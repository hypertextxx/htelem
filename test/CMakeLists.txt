set(HTELEM_TEST_SOURCES test_runner.cpp simple_element_tests.cpp braid_tests.cpp braided_renderer_tests.cpp event_tests.cpp type_util_tests.cpp dynamically_many_tests.cpp)
add_executable(htelem_tests ${HTELEM_TEST_SOURCES})

target_compile_options(htelem_tests PUBLIC "-ftemplate-backtrace-limit=0")
target_link_libraries(htelem_tests PUBLIC htelem)
warn_format(htelem_tests ${HTELEM_TEST_SOURCES})

