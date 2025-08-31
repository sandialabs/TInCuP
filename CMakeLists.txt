# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

# Root CMakeLists.txt - Compatibility Shim
#
# This file exists for compatibility with projects that expect to find
# CMakeLists.txt at the repository root (e.g., when using FetchContent).
#
# The actual CMake configuration is in build_systems/cmake/CMakeLists.txt
# to keep the root directory clean and organized.

cmake_minimum_required(VERSION 3.20)

# Read version string from top-level VERSION file and use it for the project
file(READ "${CMAKE_SOURCE_DIR}/VERSION" TINCUP_VERSION_STR)
string(STRIP "${TINCUP_VERSION_STR}" TINCUP_VERSION_STR)
if(TINCUP_VERSION_STR STREQUAL "")
  set(TINCUP_VERSION_STR "0.0.0")
endif()

project(tincup VERSION ${TINCUP_VERSION_STR} LANGUAGES CXX)

# Extract license type from first line of LICENSE
file(STRINGS "${CMAKE_SOURCE_DIR}/LICENSE" TINCUP_LICENSE_LINE LIMIT_COUNT 1)
if(TINCUP_LICENSE_LINE STREQUAL "")
  set(TINCUP_LICENSE_LINE "BSD 3-Clause License")
endif()
set(TINCUP_LICENSE "${TINCUP_LICENSE_LINE}")

# Delegate to the actual CMake configuration
add_subdirectory(build_systems/cmake)
