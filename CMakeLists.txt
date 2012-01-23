#
# Simple Common Gateway Interface protocol implementation in C.
#

cmake_minimum_required(VERSION 2.6)

project(cscgi)

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
  add_api_documentation(doc)

  # Resolve include directives as if installed on the system.
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code)

  # Build demo projects.
  add_subdirectory(demo)

  # Register unit tests.
  add_subdirectory(test)

endif()
