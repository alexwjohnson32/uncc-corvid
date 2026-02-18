# `gridpack`

This is the area for all common utils and GridPack only tools (not necessarily utilities). At the moment, its pretty barren but it does serve one good purpose.

## Linking to the library in CMake

NOTE: This is all done under the assumption of static linkage. If we have shared libraries, some of this guidance may change, or may require extra steps. As of the time of this writing though, we only build statically.

The `gridpack` directory builds a library that can be referenced throughout CMake by the variable `CORVID_GRIDPACK_LIB` (referenced in CMake like `${CORVID_GRIDPACK_LIB}`). This library publically links all of the libraries and headers in order to have your library compile with GridPack libraries. So no classes specifically exist here yet, but this is one library that you need in order to build GridPack components. It also links the `CORVID_UTILS_LIB` which exposes the utils and the Boost library, although you may still need to specify that you are linking against `CORVID_UTILS_LIB` as well within your CMake.

## Using the library in C++

Currently, there are no project defined classes/functions to use, but when they do exist in the future, they should follow these guidelines.

All classes and functions found within this library should exist under the `common::gridpack::` namespace.

When including these files in your file, be sure to give the fully qualified path (example, `#include "common/gridpack/tools.hpp"`).