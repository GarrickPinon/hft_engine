add_executable(unit_tests test_core.cpp)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain core)
