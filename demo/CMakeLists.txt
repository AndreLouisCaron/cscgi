add_executable(demo demo.cpp)
target_link_libraries(demo ${cscgi_libraries})
add_dependencies(demo ${cscgi_libraries})
