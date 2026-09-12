# pragma once

#include "bitboard.h"

#include <array>
#include <cstddef>

constexpr std::size_t COLOR_COUNT = 2;
constexpr std::size_t SQUARE_COUNT = 64;

constexpr bool isOnBoard(int rank, int file)
{
    return
        rank >= 0 && rank < 8 &&
        file >= 0 && file < 8;
}

constexpr Bitboard generateKnightAttacks(Square square)
{
    Bitboard attacks = 0;

    const int rank = getRankOf(square);
    const int file = getFileOf(square);

    constexpr int offsets[8][2] =
    {
        {1, 2},
        {2, 1},
        {2, -1},
        {1, -2},
        {-1, -2},
        {-2, -1},
        {-2, 1},
        {-1, 2}
    };

    for (const auto& offset : offsets)
    {
        const int targetRank = rank + offset[0];
        const int targetFile = file + offset[1];

        if (isOnBoard(targetRank, targetFile))
        {
            const Square targetSquare =
                static_cast<Square>(targetRank * 8 + targetFile);
            attacks |= getBit(targetSquare);
        }
    }

    return attacks;
}

constexpr Bitboard generateKingAttacks(Square square)
{
    Bitboard attacks = 0;

    const int rank = getRankOf(square);
    const int file = getFileOf(square);

    constexpr int offsets[8][2] =
    {
        {-1, 1},
        {0, 1},
        {1, 1},
        {-1, 0},
        {1, 0},
        {-1, -1},
        {0, -1},
        {1, -1}
    };

    for (const auto& offset : offsets)
    {
        const int targetRank = rank + offset[0];
        const int targetFile = file + offset[1];

        if (isOnBoard(targetRank, targetFile))
        {
            const Square targetSquare =
                static_cast<Square>(targetRank * 8 + targetFile);
            attacks |= getBit(targetSquare);
        }
    }

    return attacks;
}

constexpr Bitboard generatePawnAttacks(Square square, Color color)
{
    Bitboard attacks = 0;

    const int rank = getRankOf(square);
    const int file = getFileOf(square);

    const int dir = color == Color::White ? 1 : -1;

    const int targetRank = rank + dir;

    if (isOnBoard(targetRank, file - 1))
    {
        const Square targetSquare =
            static_cast<Square>(targetRank * 8 + file - 1);

        attacks |= getBit(targetSquare);
    }

    if (isOnBoard(targetRank, file + 1))
    {
        const Square targetSquare =
            static_cast<Square>(targetRank * 8 + file + 1);

        attacks |= getBit(targetSquare);
    }

    return attacks;
}

constexpr std::array<Bitboard, SQUARE_COUNT> generateKnightAttackTable()
{
    std::array<Bitboard, SQUARE_COUNT> table{};

    for (Square square = 0; square < SQUARE_COUNT; square++)
    {
        table[square] = generateKnightAttacks(square);
    }

    return table;
}

constexpr std::array<Bitboard, SQUARE_COUNT> generateKingAttackTable()
{
    std::array<Bitboard, SQUARE_COUNT> table{};

    for (Square square = 0; square < SQUARE_COUNT; square++)
    {
        table[square] = generateKingAttacks(square);
    }

    return table;
}

constexpr std::array<
        std::array<Bitboard, SQUARE_COUNT>, COLOR_COUNT
    > generatePawnAttackTable()
{
    std::array<
        std::array<Bitboard, SQUARE_COUNT>, COLOR_COUNT
        > table{};

    for (std::size_t color = 0; color < COLOR_COUNT; color++)
    {
        for (Square square = 0; square < SQUARE_COUNT; square++)
        {
            table[color][square] =
                generatePawnAttacks(square, static_cast<Color>(color));
        }
    }

    return table;
}

inline constexpr auto KNIGHT_ATTACKS =
    generateKnightAttackTable();
inline constexpr auto KING_ATTACKS =
    generateKingAttackTable();
inline constexpr auto PAWN_ATTACKS =
    generatePawnAttackTable();
