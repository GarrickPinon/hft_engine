add_library(features INTERFACE)
target_link_libraries(features INTERFACE core)
target_include_directories(features INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
