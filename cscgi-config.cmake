# Library configuration file used by dependent projects
# via find_package() built-in directive in "config" mode.

if(NOT DEFINED cscgi_FOUND)

  # Locate library headers.
  find_path(cscgi_include_dirs
    NAMES scgi.h
    PATHS ${cscgi_DIR}/code
  )

  # Export library targets.
  set(cscgi_libraries
    scgi ${cnetstring_libraries}
    CACHE INTERNAL "cscgi library" FORCE
  )

  # Usual "required" et. al. directive logic.
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    cscgi DEFAULT_MSG
    cscgi_include_dirs
    cscgi_libraries
  )

  # Register library targets when found as part of a dependent project.
  # Since this project uses a find_package() directive to include this
  # file, don't recurse back into the CMakeLists file.
  if(NOT ${PROJECT_NAME} STREQUAL cscgi)
    add_subdirectory(
      ${cscgi_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/cscgi
    )
  endif()
endif()
