cmake_minimum_required(VERSION 4.3)

project(Mobius)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================================
# MOBIUS CORE LIBRARY
# =========================================================

add_library(mobius_core
        src/core/core.cpp
        src/core/position.cpp
        src/core/fen_parser.cpp
)

target_include_directories(mobius_core PUBLIC
        ${CMAKE_SOURCE_DIR}/src/core
)

# =========================================================
# MAIN ENGINE
# =========================================================

add_executable(mobius
        src/main.cpp
)

target_link_libraries(mobius PRIVATE mobius_core)

# =========================================================
# TESTS
# =========================================================

add_executable(mobius_tests
        tests/core_tests.cpp
)

target_link_libraries(mobius_tests PRIVATE mobius_core)