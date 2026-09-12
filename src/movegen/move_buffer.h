#pragma once

#include "core.h"

constexpr int MAX_MOVES = 256;

struct MoveBuffer
{
    Move moves[MAX_MOVES];
    int count = 0;

    inline void add(Move move) noexcept
    {
        moves[count++] = move;
    }

    inline void reset() noexcept
    {
        count = 0;
    }
};