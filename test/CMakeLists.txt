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

macro(add_test_program name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} scgi ${cnetstring_libraries})
  add_dependencies(${name} scgi ${cnetstring_libraries})
endmacro()

add_test_program(scgi-get-head)
add_test_program(scgi-get-body)

set(get-head ${PROJECT_BINARY_DIR}/scgi-get-head)
set(get-body ${PROJECT_BINARY_DIR}/scgi-get-body)
set(test-data ${CMAKE_CURRENT_SOURCE_DIR}/data)

add_test(request-001-head
  "${get-head}" "${test-data}/request-001.txt")
set_tests_properties(request-001-head
  PROPERTIES
  PASS_REGULAR_EXPRESSION "CONTENT_LENGTH=27"
  PASS_REGULAR_EXPRESSION "SCGI=1"
  PASS_REGULAR_EXPRESSION "REQUEST_METHOD=POST"
  PASS_REGULAR_EXPRESSION "REQUEST_URI=/deepthought"
)

add_test(request-001-body
  "${get-body}" "${test-data}/request-001.txt")
set_tests_properties(request-001-body
  PROPERTIES
  PASS_REGULAR_EXPRESSION "What is the answer to life\\?"
)
