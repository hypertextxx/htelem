cmake_minimum_required(VERSION 3.25)

project(htelem)

set(CMAKE_CXX_STANDARD 26)

set(HTELEM_HEADER_PATH "${CMAKE_SOURCE_DIR}/include/htelem")
file(GLOB_RECURSE HTELEM_SOURCES "${HTELEM_HEADER_PATH}/*.h")

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

if(WASI)
    target_compile_options(htelem INTERFACE "-fno-exceptions" "-fno-rtti")
endif()

target_include_directories(htelem INTERFACE ${CMAKE_SOURCE_DIR}/include)
add_dependencies(htelem ht-defs)
warn_format(htelem "${HTELEM_SOURCES}")

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_MULTILINE_CPP_IS_BRIEF 1)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/docs")
    set(DOXYGEN_TAGFILES "${CMAKE_SOURCE_DIR}/docs/cppreference-doxygen-web.tag.xml=https://en.cppreference.com/w/")
    doxygen_add_docs(docs "${HTELEM_SOURCES}")
endif()

add_definitions(-DHTELEM_VERBOSE_TESTS)
add_subdirectory(test/)

