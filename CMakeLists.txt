# Copyright (c) 2011-2012, Andre Caron (andre.l.caron@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

#
# Simple Common Gateway Interface protocol implementation in C.
#

cmake_minimum_required(VERSION 2.6)

project(cscgi)

option(CSCGI_BUILD_CXX "Build C++ wrappers." ON)
option(CSCGI_BUILD_DEMOS "Build demo programs." ON)
option(CSCGI_BUILD_TESTS "Build test programs." ON)
option(CSCGI_SHARED_LIBS "Build cscgi shared library." OFF)

# Add targets for library referenced as Git submodule.
macro(add_submodule name)
  if(${ARGN1})
    set(${name}_DIR ${ARGN1})
  else()
    set(${name}_DIR ${CMAKE_SOURCE_DIR}/libs/${name})
  endif()
  find_package(${name})
endmacro()

# Compile API documentation from source code.
function(add_api_documentation target)
  if(DOXYGEN_EXECUTABLE)
    add_custom_target(
      ${target}
      COMMAND
        ${DOXYGEN_EXECUTABLE}
      WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT
        "Compiling documentation."
      VERBATIM
    )
  endif()
endfunction()

if(MSVC)
  # Ignore Microsoft's recommendations of
  # safety at the expense of portability.
  add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
    -D_CRT_NONSTDC_NO_WARNINGS
  )
endif()

enable_testing()

# Put all libraries and executables in the build folder root.
set(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# Build required libraries.
add_submodule(cnetstring)

# Build the primary target.
include_directories(
  ${cnetstring_include_dirs}
)
add_subdirectory(code)

# Optional targets (skip when building as a dependency).
if(${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})

  # Build API documentation.
  find_package(Doxygen QUIET)
  configure_file(
    ${CMAKE_SOURCE_DIR}/help/Doxyfile
    ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
  add_api_documentation(help)

  # Resolve include directives as if installed on the system.
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code)

  # Configure the library and its dependencies.
  set(cscgi_libraries
    scgi ${cnetstring_libraries}
  )

  # Build demo projects.
  if(CSCGI_BUILD_DEMOS)
    add_subdirectory(demo)
  endif()

  # Register unit tests.
  if(CSCGI_BUILD_TESTS)
    add_subdirectory(test)
  endif()

endif()
