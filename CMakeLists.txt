cmake_minimum_required(VERSION 3.25)

# this literally doesn't compile with gcc lmao
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

project(htelem)

set(CMAKE_CXX_STANDARD 26)

set(HTELEM_HEADER_PATH "${CMAKE_SOURCE_DIR}/include/htelem")
set(HTELEM_SOURCES "${HTELEM_HEADER_PATH}/attribute_list.h" "${HTELEM_HEADER_PATH}/braid.h" "${HTELEM_HEADER_PATH}/braided_renderer.h" "${HTELEM_HEADER_PATH}/element.h" "${HTELEM_HEADER_PATH}/html.h" "${HTELEM_HEADER_PATH}/interface_spec.h" "${HTELEM_HEADER_PATH}/render_common.h" "${HTELEM_HEADER_PATH}/static_string.h" "${HTELEM_HEADER_PATH}/util.h")

set(HTELEM_AUTOGEN_DIR "${CMAKE_SOURCE_DIR}/tools/htelem-autogen")

add_custom_command(
    OUTPUT "${HTELEM_AUTOGEN_DIR}/out/attribute_lists.def" "${HTELEM_AUTOGEN_DIR}/out/attribute_spec.def" "${HTELEM_AUTOGEN_DIR}/out/elements.def" "${HTELEM_AUTOGEN_DIR}/out/interface_spec.def"
    COMMAND "npm" ARGS "start"
    DEPENDS "${HTELEM_AUTOGEN_DIR}/index.ts"
    WORKING_DIRECTORY "${HTELEM_AUTOGEN_DIR}")

add_custom_target(ht-defs DEPENDS "${HTELEM_AUTOGEN_DIR}/out/attribute_lists.def" "${HTELEM_AUTOGEN_DIR}/out/attribute_spec.def" "${HTELEM_AUTOGEN_DIR}/out/elements.def" "${HTELEM_AUTOGEN_DIR}/out/interface_spec.def")

add_custom_target(autoformat)

function(warn_format TGT SRCS)
    add_custom_target("format-warn-${TGT}" COMMAND "clang-format" "--dry-run" "--ferror-limit=1" "${SRCS}" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMMAND_EXPAND_LISTS)
    add_dependencies(${TGT} "format-warn-${TGT}")
    add_custom_target("autoformat-${TGT}" COMMAND "clang-format" "-i" "${SRCS}" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" COMMAND_EXPAND_LISTS)
    add_dependencies(autoformat "autoformat-${TGT}")
endfunction()

add_library(htelem INTERFACE ${HTELEM_SOURCES})
target_include_directories(htelem INTERFACE ${CMAKE_SOURCE_DIR}/include)
add_dependencies(htelem ht-defs)
warn_format(htelem "${HTELEM_SOURCES}")

add_definitions(-DHTELEM_VERBOSE_TESTS)
add_subdirectory(test/)

