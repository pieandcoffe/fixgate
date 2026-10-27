add_executable(fixgate_tests codec/test_tags.cpp)
target_link_libraries(fixgate_tests PRIVATE fixgate::codec fixgate_warnings GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(fixgate_tests)
