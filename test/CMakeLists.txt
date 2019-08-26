include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM sources "${PROJECT_SOURCE_DIR}/src/main.c")
file(GLOB tests "${PROJECT_SOURCE_DIR}/test/*.cpp")

add_executable(unit_tests
  ${sources}
  ${tests})
target_link_libraries(unit_tests gtest_main)
add_test(NAME unit COMMAND unit_Tests)
