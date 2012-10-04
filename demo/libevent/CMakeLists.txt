add_executable(scgi-libevent scgi-libevent.c)
target_link_libraries(scgi-libevent ${cscgi_libraries} event)
add_dependencies(scgi-libevent ${cscgi_libraries})
