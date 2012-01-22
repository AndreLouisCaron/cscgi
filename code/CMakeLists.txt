# netstring library.

set(scgi_headers
  scgi.h
)
set(scgi_sources
  scgi.c
)
add_library(scgi
  STATIC
  ${scgi_sources}
  ${scgi_headers}
)
