# netstring library.

set(scgi_headers
  scgi.h
)
set(scgi_sources
  scgi.c
)

if(CSCGI_BUILD_CXX)
  set(scgi_headers
    ${scgi_headers}
    scgi.hpp
  )
  set(scgi_sources
    ${scgi_sources}
    scgi.cpp
  )
endif()

if(CSCGI_SHARED_LIBS)
  add_library(scgi
    SHARED
    ${scgi_sources}
    ${scgi_headers}
  )
  target_link_libraries(scgi ${cnetstring_libraries})
else()
  add_library(scgi
    STATIC
    ${scgi_sources}
    ${scgi_headers}
  )
endif()
