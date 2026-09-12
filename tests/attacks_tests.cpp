#include "attacks_tests.h"

#include "attacks.h"

#include <cassert>
#include <cstddef>
#include <iostream>


namespace
{
    constexpr std::size_t WHITE =
        static_cast<std::size_t>(Color::White);

    constexpr std::size_t BLACK =
        static_cast<std::size_t>(Color::Black);


    void testKnightAttackTable()
    {
        const Bitboard a1Expected =
            getBit(10) |
            getBit(17);

        assert(KNIGHT_ATTACKS[0] == a1Expected);

        const Bitboard d4Expected =
            getBit(10) |
            getBit(12) |
            getBit(17) |
            getBit(21) |
            getBit(33) |
            getBit(37) |
            getBit(42) |
            getBit(44);

        assert(KNIGHT_ATTACKS[27] == d4Expected);

        std::cout << "[PASS] Knight attack table\n";
    }


    void testKingAttackTable()
    {
        const Bitboard a1Expected =
            getBit(1) |
            getBit(8) |
            getBit(9);

        assert(KING_ATTACKS[0] == a1Expected);

        const Bitboard d4Expected =
            getBit(18) |
            getBit(19) |
            getBit(20) |
            getBit(26) |
            getBit(28) |
            getBit(34) |
            getBit(35) |
            getBit(36);

        assert(KING_ATTACKS[27] == d4Expected);

        std::cout << "[PASS] King attack table\n";
    }


    void testPawnAttackTable()
    {
        const Bitboard whiteE4Expected =
            getBit(35) |
            getBit(37);

        const Bitboard blackE4Expected =
            getBit(19) |
            getBit(21);

        assert(
            PAWN_ATTACKS[WHITE][28] ==
            whiteE4Expected
        );

        assert(
            PAWN_ATTACKS[BLACK][28] ==
            blackE4Expected
        );

        // White a2 -> b3
        assert(
            PAWN_ATTACKS[WHITE][8] ==
            getBit(17)
        );

        // Black h7 -> g6
        assert(
            PAWN_ATTACKS[BLACK][55] ==
            getBit(46)
        );

        // Pawns already on the final rank attack nowhere.
        assert(PAWN_ATTACKS[WHITE][56] == 0);
        assert(PAWN_ATTACKS[BLACK][0] == 0);

        std::cout << "[PASS] Pawn attack table\n";
    }


    void testStraightEmptyBoard()
    {
        // A rook always attacks 14 squares on an empty board.
        for (Square square = 0; square < 64; square++)
        {
            assert(
                popCount(
                    straightAttacksBasic(square, 0)
                ) == 14
            );
        }

        std::cout
            << "[PASS] Straight attacks - empty board\n";
    }


    void testDiagonalEmptyBoard()
    {
        // a1
        assert(
            popCount(
                diagonalAttacksBasic(0, 0)
            ) == 7
        );

        // d4
        assert(
            popCount(
                diagonalAttacksBasic(27, 0)
            ) == 13
        );

        // h8
        assert(
            popCount(
                diagonalAttacksBasic(63, 0)
            ) == 7
        );

        std::cout
            << "[PASS] Diagonal attacks - empty board\n";
    }


    void testStraightBlockers()
    {
        constexpr Square source = 27; // d4

        const Bitboard occupancy =
            getBit(source) |
            getBit(43) | // d6
            getBit(11) | // d2
            getBit(30) | // g4
            getBit(25);  // b4

        const Bitboard expected =
            getBit(35) | // d5
            getBit(43) | // d6 blocker

            getBit(19) | // d3
            getBit(11) | // d2 blocker

            getBit(28) | // e4
            getBit(29) | // f4
            getBit(30) | // g4 blocker

            getBit(26) | // c4
            getBit(25);  // b4 blocker

        const Bitboard attacks =
            straightAttacksBasic(
                source,
                occupancy
            );

        assert(attacks == expected);

        // Blocker itself must be included,
        // but squares behind it must not be.
        assert(isBitSet(attacks, 43));
        assert(!isBitSet(attacks, 51)); // d7

        std::cout
            << "[PASS] Straight attacks - blockers\n";
    }


    void testDiagonalBlockers()
    {
        constexpr Square source = 27; // d4

        const Bitboard occupancy =
            getBit(source) |
            getBit(45) | // f6
            getBit(41) | // b6
            getBit(13) | // f2
            getBit(9);   // b2

        const Bitboard expected =
            getBit(36) | // e5
            getBit(45) | // f6 blocker

            getBit(34) | // c5
            getBit(41) | // b6 blocker

            getBit(20) | // e3
            getBit(13) | // f2 blocker

            getBit(18) | // c3
            getBit(9);   // b2 blocker

        const Bitboard attacks =
            diagonalAttacksBasic(
                source,
                occupancy
            );

        assert(attacks == expected);

        assert(isBitSet(attacks, 45));

        assert(!isBitSet(attacks, 54)); // g7 behind f6
        assert(!isBitSet(attacks, 48)); // a7 behind b6
        assert(!isBitSet(attacks, 6));  // g1 behind f2
        assert(!isBitSet(attacks, 0));  // a1 behind b2

        std::cout
            << "[PASS] Diagonal attacks - blockers\n";
    }


    void testFullyOccupiedBoard()
    {
        constexpr Bitboard fullBoard =
            ~Bitboard{0};

        constexpr Square source = 27; // d4

        const Bitboard straightExpected =
            getBit(35) | // d5
            getBit(19) | // d3
            getBit(28) | // e4
            getBit(26);  // c4

        const Bitboard diagonalExpected =
            getBit(36) | // e5
            getBit(34) | // c5
            getBit(20) | // e3
            getBit(18);  // c3

        assert(
            straightAttacksBasic(
                source,
                fullBoard
            ) == straightExpected
        );

        assert(
            diagonalAttacksBasic(
                source,
                fullBoard
            ) == diagonalExpected
        );

        std::cout
            << "[PASS] Sliding attacks - full board\n";
    }
}


void runAttackTests()
{
    std::cout << "\n--- Attack tests ---\n";

    testKnightAttackTable();
    testKingAttackTable();
    testPawnAttackTable();

    testStraightEmptyBoard();
    testDiagonalEmptyBoard();
    testStraightBlockers();
    testDiagonalBlockers();
    testFullyOccupiedBoard();
}