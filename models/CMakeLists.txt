add_library(models INTERFACE)
target_link_libraries(models INTERFACE features data core)
target_include_directories(models INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
