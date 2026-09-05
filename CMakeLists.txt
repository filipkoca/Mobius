cmake_minimum_required(VERSION 4.3)

project(Mobius)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mobius
        src/main.cpp
        src/core/core.h
        src/core/core.cpp
        src/core/bitboard.h
        src/core/position.h
        src/core/position.cpp
        src/core/state_buffer.h
        src/core/state_info.h
        src/core/fen_parser.cpp
        src/core/fen_parser.h
)