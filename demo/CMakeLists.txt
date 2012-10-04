add_executable(scgi-demo demo.cpp)
target_link_libraries(scgi-demo ${cscgi_libraries})
add_dependencies(scgi-demo ${cscgi_libraries})

# Example with popular evented I/O library for UNIX.
# TODO: conditionally include this if we know that
#       libevent is available; it isn't by default.
if (UNIX)
  add_subdirectory(libevent)
endif()
