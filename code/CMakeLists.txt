# Copyright (c) 2011-2012, Andre Caron (andre.l.caron@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

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
