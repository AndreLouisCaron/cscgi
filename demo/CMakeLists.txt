set(headers
  scgi.hpp
)
set(sources
  scgi.cpp
  demo.cpp
)
add_executable(
  demo
  ${sources}
  ${headers}
)
target_link_libraries(demo scgi ${cnetstring_libraries})
add_dependencies(demo scgi ${cnetstring_libraries})
