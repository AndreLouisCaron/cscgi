add_executable(demo demo.cpp)
target_link_libraries(demo scgi ${cnetstring_libraries})
add_dependencies(demo scgi ${cnetstring_libraries})
