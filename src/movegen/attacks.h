# pragma once

#include "bitboard.h"

#include <array>
#include <cstddef>
#include <immintrin.h>

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

inline Bitboard straightAttacksBasic(Square square, Bitboard occupancy) noexcept
{
    Bitboard attacks = 0;

    const int rank = getRankOf(square);
    const int file = getFileOf(square);

    for (int r = rank + 1; r < 8; r++)
    {
        const Square targetSquare =
            static_cast<Square>(r * 8 + file);
        const Bitboard targetBit = getBit(targetSquare);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    for (int r = rank - 1; r >= 0; --r)
    {
        const Square target =
            static_cast<Square>(r * 8 + file);

        const Bitboard targetBit = getBit(target);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    for (int f = file + 1; f < 8; ++f)
    {
        const Square target =
            static_cast<Square>(rank * 8 + f);

        const Bitboard targetBit = getBit(target);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    for (int f = file - 1; f >= 0; --f)
    {
        const Square target =
            static_cast<Square>(rank * 8 + f);

        const Bitboard targetBit = getBit(target);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    return attacks;
}

inline Bitboard diagonalAttacksBasic(Square square, Bitboard occupancy) noexcept
{
    Bitboard attacks = 0;

    const int rank = getRankOf(square);
    const int file = getFileOf(square);

    for (int r = rank + 1, f = file + 1;
         r < 8 && f < 8;
         ++r, ++f)
    {
        const Square target =
            static_cast<Square>(r * 8 + f);

        const Bitboard targetBit = getBit(target);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    for (int r = rank + 1, f = file - 1;
         r < 8 && f >= 0;
         ++r, --f)
    {
        const Square target =
            static_cast<Square>(r * 8 + f);

        const Bitboard targetBit = getBit(target);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    for (int r = rank - 1, f = file + 1;
         r >= 0 && f < 8;
         --r, ++f)
    {
        const Square target =
            static_cast<Square>(r * 8 + f);

        const Bitboard targetBit = getBit(target);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    for (int r = rank - 1, f = file - 1;
         r >= 0 && f >= 0;
         --r, --f)
    {
        const Square target =
            static_cast<Square>(r * 8 + f);

        const Bitboard targetBit = getBit(target);
        attacks |= targetBit;

        if (occupancy & targetBit)
        {
            break;
        }
    }

    return attacks;
}

constexpr std::size_t ROOK_ATTACK_TABLE_SIZE = 102400;
constexpr std::size_t BISHOP_ATTACK_TABLE_SIZE = 5248;

struct PextAttackTables
{
    std::array<Bitboard, SQUARE_COUNT> rookMasks{};
    std::array<Bitboard, SQUARE_COUNT> bishopMasks{};

    std::array<std::size_t, SQUARE_COUNT> rookOffsets{};
    std::array<std::size_t, SQUARE_COUNT> bishopOffsets{};

    std::array<Bitboard, ROOK_ATTACK_TABLE_SIZE> rookAttacks{};
    std::array<Bitboard, BISHOP_ATTACK_TABLE_SIZE> bishopAttacks{};
};


extern const PextAttackTables PEXT_ATTACKS;

inline Bitboard straightAttacksPext(Square square, Bitboard occupancy) noexcept
{
    const Bitboard mask  = PEXT_ATTACKS.rookMasks[square];
    const std::size_t index = static_cast<std::size_t>(_pext_u64(occupancy, mask));

    return PEXT_ATTACKS.rookAttacks[PEXT_ATTACKS.rookOffsets[square] + index];
}


inline Bitboard diagonalAttacksPext(Square square,Bitboard occupancy) noexcept
{
    const Bitboard mask =PEXT_ATTACKS.bishopMasks[square];
    const auto index = static_cast<std::size_t>(_pext_u64(occupancy, mask));

    return PEXT_ATTACKS.bishopAttacks[PEXT_ATTACKS.bishopOffsets[square] + index];
}


inline Bitboard queenAttacksPext(Square square, Bitboard occupancy) noexcept
{
    return
        straightAttacksPext(square, occupancy) |
        diagonalAttacksPext(square, occupancy);
}

inline Bitboard rookAttacks(Square square,Bitboard occupancy) noexcept
{
    return straightAttacksPext(square,occupancy);
}


inline Bitboard bishopAttacks(Square square,Bitboard occupancy) noexcept
{
    return diagonalAttacksPext(square,occupancy);
}


inline Bitboard queenAttacks(Square square,Bitboard occupancy) noexcept
{
    return
        rookAttacks(square, occupancy) |
        bishopAttacks(square, occupancy);
}