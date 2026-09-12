cmake_minimum_required(VERSION 4.3)

project(Mobius)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)


# =========================================================
# MOBIUS CORE LIBRARY
# =========================================================

add_library(mobius_core
        src/core/core.cpp
        src/core/position.cpp
        src/core/fen_parser.cpp
        src/movegen/attacks.cpp
)

target_include_directories(mobius_core PUBLIC
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/movegen
)


# =========================================================
# MAIN ENGINE
# =========================================================

add_executable(mobius
        src/main.cpp
        src/movegen/move_buffer.h
)

target_link_libraries(mobius PRIVATE mobius_core)


# =========================================================
# TESTS
# =========================================================

enable_testing()

add_executable(mobius_tests
        tests/test_main.cpp
        tests/core_tests.cpp
        tests/make_undo_tests.cpp
        tests/attacks_tests.cpp
        tests/attack_detector_tests.cpp
)

target_link_libraries(mobius_tests PRIVATE mobius_core)

add_test(
        NAME mobius_tests
        COMMAND mobius_tests
)

set_tests_properties(
        mobius_tests
        PROPERTIES TIMEOUT 10
)


# =========================================================
# BENCHMARKS
# =========================================================

add_executable(mobius_benchmarks
        benchmarks/benchmark_main.cpp
        benchmarks/make_undo_benchmark.cpp
        benchmarks/sliding_attacks_benchmark.cpp
        benchmarks/attack_detector_benchmark.cpp
)

target_link_libraries(mobius_benchmarks PRIVATE mobius_core)


# =========================================================
# NATIVE RELEASE OPTIMIZATION
# =========================================================

# PEXT is used from inline functions in attacks.h, so every target that
# includes those functions should receive -march=native in Release mode.
if (
        CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"
)
    target_compile_options(mobius_core PRIVATE
            $<$<CONFIG:Release>:-march=native>
    )

    target_compile_options(mobius PRIVATE
            $<$<CONFIG:Release>:-march=native>
    )

    target_compile_options(mobius_tests PRIVATE
            $<$<CONFIG:Release>:-march=native>
    )

    target_compile_options(mobius_benchmarks PRIVATE
            $<$<CONFIG:Release>:-march=native>
    )
endif()
