#include "attacks.h"


namespace
{
    Bitboard generateRookOccupancyMask(Square square)
    {
        Bitboard mask = 0;

        const int rank = getRankOf(square);
        const int file = getFileOf(square);

        // up
        for (int r = rank + 1; r <= 6; ++r)
        {
            mask |= getBit(static_cast<Square>(r * 8 + file));
        }

        // down
        for (int r = rank - 1; r >= 1; --r)
        {
            mask |= getBit(static_cast<Square>(r * 8 + file));
        }

        // right
        for (int f = file + 1; f <= 6; ++f)
        {
            mask |= getBit(static_cast<Square>(rank * 8 + f));
        }

        // left
        for (int f = file - 1; f >= 1; --f)
        {
            mask |= getBit(static_cast<Square>(rank * 8 + f));
        }

        return mask;
    }


    Bitboard generateBishopOccupancyMask(Square square)
    {
        Bitboard mask = 0;

        const int rank = getRankOf(square);
        const int file = getFileOf(square);

        // (1, 1)
        for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; ++r, ++f)
        {
            mask |= getBit(static_cast<Square>(r * 8 + f));
        }

        // (1, -1)
        for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; ++r, --f)
        {
            mask |= getBit(static_cast<Square>(r * 8 + f));
        }

        // (-1, 1)
        for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; --r, ++f)
        {
            mask |= getBit(static_cast<Square>(r * 8 + f));
        }

        // (-1, -1)
        for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; --r, --f)
        {
            mask |= getBit(static_cast<Square>(r * 8 + f));
        }

        return mask;
    }


    Bitboard occupancyFromIndex(
        std::size_t index,
        Bitboard mask
    )
    {
        Bitboard occupancy = 0;

        std::size_t indexBit = 0;

        while (mask != 0)
        {
            // Extract the lowest 1-bit from the mask.
            const Bitboard maskBit =
                mask & (~mask + 1);

            if (
                index &
                (std::size_t{1} << indexBit)
            )
            {
                occupancy |= maskBit;
            }

            // Remove lowest 1-bit.
            mask &= mask - 1;

            ++indexBit;
        }

        return occupancy;
    }


    PextAttackTables generatePextAttackTables()
    {
        PextAttackTables tables{};

        std::size_t rookOffset = 0;
        std::size_t bishopOffset = 0;


        for (int squareIndex = 0;
             squareIndex < 64;
             ++squareIndex)
        {
            const Square square =
                static_cast<Square>(squareIndex);


            // =============================================
            // ROOK
            // =============================================

            const Bitboard rookMask =
                generateRookOccupancyMask(square);

            tables.rookMasks[square] =
                rookMask;

            tables.rookOffsets[square] =
                rookOffset;

            const int rookRelevantBits =
                popCount(rookMask);

            const std::size_t rookConfigurations =
                std::size_t{1} << rookRelevantBits;


            for (
                std::size_t index = 0;
                index < rookConfigurations;
                ++index
            )
            {
                const Bitboard blockers =
                    occupancyFromIndex(index,rookMask);

                tables.rookAttacks[rookOffset + index] =
                    straightAttacksBasic(square,blockers);
            }

            rookOffset += rookConfigurations;


            // =============================================
            // BISHOP
            // =============================================

            const Bitboard bishopMask =
                generateBishopOccupancyMask(square);

            tables.bishopMasks[square] =
                bishopMask;

            tables.bishopOffsets[square] =
                bishopOffset;

            const int bishopRelevantBits =
                popCount(bishopMask);

            const std::size_t bishopConfigurations =
                std::size_t{1} << bishopRelevantBits;


            for (
                std::size_t index = 0;
                index < bishopConfigurations;
                ++index
            )
            {
                const Bitboard blockers =
                    occupancyFromIndex(index,bishopMask);

                tables.bishopAttacks[bishopOffset + index] =
                    diagonalAttacksBasic(square,blockers);
            }

            bishopOffset += bishopConfigurations;
        }

        return tables;
    }
}

const PextAttackTables PEXT_ATTACKS =generatePextAttackTables();