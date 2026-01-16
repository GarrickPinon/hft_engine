add_executable(benchmark_runner benchmark_runner.cpp)
target_link_libraries(benchmark_runner PRIVATE core models data features)
target_include_directories(benchmark_runner PRIVATE ${CMAKE_SOURCE_DIR})
