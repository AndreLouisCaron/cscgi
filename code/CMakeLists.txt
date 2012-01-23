# netstring library.

set(scgi_headers
  scgi.h
  scgi.hpp
)
set(scgi_sources
  scgi.c
  scgi.cpp
)
add_library(scgi
  STATIC
  ${scgi_sources}
  ${scgi_headers}
)
