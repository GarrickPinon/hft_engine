cmake_minimum_required(VERSION 3.20)
project(HighFrequencyTradingEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags for HFT
if(MSVC)
    add_compile_definitions(NOMINMAX)
    add_compile_options("$<$<CONFIG:Release>:/O2>" "$<$<CONFIG:Release>:/Oi>" "$<$<CONFIG:Release>:/Ot>" "$<$<CONFIG:Release>:/Oy>" "$<$<CONFIG:Release>:/GL>" /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -mtune=native -funroll-loops -fno-exceptions -fno-rtti -flto)
endif()

# Dependencies
include(FetchContent)

# Catch2 for testing
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Subdirectories
add_subdirectory(core)
add_subdirectory(data)
add_subdirectory(features)
add_subdirectory(models)
add_subdirectory(risk)
add_subdirectory(execution)
add_subdirectory(backtest)
add_subdirectory(tests)
add_subdirectory(tools)

# Main Executable
add_executable(hft_engine infra/main.cpp)
target_link_libraries(hft_engine PRIVATE core execution data risk models features)
