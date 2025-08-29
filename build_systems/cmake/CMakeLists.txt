# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

# This file is included from the root CMakeLists.txt

add_library(tincup INTERFACE)
add_library(tincup::tincup ALIAS tincup)

target_include_directories(tincup INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../../include")
target_compile_features(tincup INTERFACE cxx_std_20)

# --- Warnings policy (applies only to in-tree tests/examples) ---
option(TINCUP_ENABLE_WARNINGS "Enable compiler warnings for TInCuP tests/examples" ON)
option(TINCUP_WARNINGS_AS_ERRORS "Treat warnings as errors for TInCuP tests/examples" ON)

function(tincup_set_warnings target)
  if(NOT TINCUP_ENABLE_WARNINGS)
    return()
  endif()
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    if(TINCUP_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(TINCUP_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

# --- Testing & Examples ---
enable_testing()

# Only add the examples if they are enabled AND we are building the main project,
# not an external project (like the smoke test) that includes this one.
if(ENABLE_EXAMPLES AND (PROJECT_NAME STREQUAL "tincup"))
  add_subdirectory(${CMAKE_SOURCE_DIR}/examples
                   ${CMAKE_BINARY_DIR}/examples)
endif()

# --- Tests ---
# Build and register the static dispatch test only when this is the main project
if(PROJECT_NAME STREQUAL "tincup")
  add_executable(test_static_dispatch ${CMAKE_SOURCE_DIR}/tests/test_static_dispatch.cpp)
  target_link_libraries(test_static_dispatch PRIVATE tincup::tincup)
  tincup_set_warnings(test_static_dispatch)
  add_test(NAME test_static_dispatch COMMAND test_static_dispatch)
endif()
