#pragma once
#include "core.h"
#include <bit>

// BITBOARD operation helpers

constexpr Bitboard getBit(Square square)
{
    return Bitboard{1} << square;
}

constexpr bool isBitSet(Bitboard bitboard, Square square)
{
    return (bitboard & getBit(square)) != 0;
}

constexpr void setBit(Bitboard& bitboard, Square square)
{
    bitboard |= getBit(square);
}

constexpr void clearBit(Bitboard& bitboard, Square square)
{
    bitboard &= ~getBit(square);
}

constexpr void toggleBit(Bitboard& bitboard, Square square)
{
    bitboard ^= getBit(square);
}

constexpr int popCount(Bitboard bitboard)
{
    return std::popcount(bitboard);
}

constexpr Square getLSBit(Bitboard bitboard)
{
    return static_cast<Square>(std::countr_zero(bitboard));
}

constexpr Square popLSBit(Bitboard& bitboard)
{
    Square square = getLSBit(bitboard);
    bitboard &= bitboard - 1;
    return square;
}
