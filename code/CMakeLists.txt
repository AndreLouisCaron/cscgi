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

add_library(scgi
  STATIC
  ${scgi_sources}
  ${scgi_headers}
)
