add_executable(scgi-demo demo.cpp)
target_link_libraries(scgi-demo ${cscgi_libraries})
add_dependencies(scgi-demo ${cscgi_libraries})
