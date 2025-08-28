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

project(tincup LANGUAGES CXX)

# Delegate to the actual CMake configuration
add_subdirectory(build_systems/cmake)
