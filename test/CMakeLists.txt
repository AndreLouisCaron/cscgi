# sha1 library demo projects.

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
