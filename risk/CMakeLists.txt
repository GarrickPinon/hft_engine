add_library(risk INTERFACE)
target_link_libraries(risk INTERFACE core)
target_include_directories(risk INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
