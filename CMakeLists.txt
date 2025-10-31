cmake_minimum_required(VERSION 3.22)
project(pricer_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -O3 -march=native)
  # -ffast-math si tu sais ce que tu fais numériquement
endif()

# --- Dépendances ---
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(pybind11 REQUIRED)
find_package(range-v3 REQUIRED)
find_package(GTest REQUIRED)

# --- Sources de la lib (EXCLURE main.cpp) ---
file(GLOB_RECURSE CURVE_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER CURVE_SOURCES EXCLUDE REGEX ".*/src/main\\.cpp$")

add_library(curve ${CURVE_SOURCES})
target_include_directories(curve PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(curve PUBLIC fmt::fmt spdlog::spdlog Eigen3::Eigen range-v3::range-v3)

# --- Exécutable principal ---
add_executable(pricer_engine ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(pricer_engine PRIVATE curve fmt::fmt spdlog::spdlog Eigen3::Eigen range-v3::range-v3)

# --- Tests (GoogleTest découverte auto) ---
enable_testing()
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp")
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE curve GTest::gtest_main)

gtest_discover_tests(unit_tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DISCOVERY_TIMEOUT 30
  DISCOVERY_MODE POST_BUILD
)
add_executable(bench_curve tests/bench_curve.cpp)
target_link_libraries(bench_curve PRIVATE curve)