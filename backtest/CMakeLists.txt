add_executable(backtest_runner backtest_runner.cpp)
target_link_libraries(backtest_runner PRIVATE core models data features)
target_include_directories(backtest_runner PRIVATE ${CMAKE_SOURCE_DIR})
