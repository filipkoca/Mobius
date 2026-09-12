#include "attack_detector_tests.h"

#include "attack_detector.h"
#include "bitboard.h"
#include "position.h"

#include <cassert>
#include <iostream>


namespace
{
    constexpr Square E4 = 28;


    void testNoAttackers()
    {
        Position position;
        position.clear();

        const Bitboard occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == 0
        );

        assert(
            !isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        std::cout << "[PASS] No attackers\n";
    }


    void testPawnAttackers()
    {
        Position position;

        // Black pawn on d5 attacks e4.
        position.clear();
        position.setPiece(
            makePiece(Color::Black, PieceType::Pawn),
            35
        );

        Bitboard occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == getBit(35)
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        assert(
            !isSquareAttacked(
                position,
                E4,
                Color::White,
                occupancy
            )
        );

        // White pawn on d3 attacks e4.
        position.clear();
        position.setPiece(
            makePiece(Color::White, PieceType::Pawn),
            19
        );

        occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::White,
                occupancy
            ) == getBit(19)
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::White,
                occupancy
            )
        );

        std::cout << "[PASS] Pawn attackers\n";
    }


    void testKnightAttacker()
    {
        Position position;
        position.clear();

        // Black knight on f6 attacks e4.
        position.setPiece(
            makePiece(Color::Black, PieceType::Knight),
            45
        );

        const Bitboard occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == getBit(45)
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        std::cout << "[PASS] Knight attacker\n";
    }


    void testKingAttacker()
    {
        Position position;
        position.clear();

        // Black king on f4 attacks e4.
        position.setPiece(
            makePiece(Color::Black, PieceType::King),
            29
        );

        const Bitboard occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == getBit(29)
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        std::cout << "[PASS] King attacker\n";
    }


    void testBishopAttackerAndBlocker()
    {
        Position position;
        position.clear();

        // h7 -> g6 -> f5 -> e4
        position.setPiece(
            makePiece(Color::Black, PieceType::Bishop),
            55
        );

        Bitboard occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == getBit(55)
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        // Put a blocker on f5.
        position.setPiece(
            makePiece(Color::White, PieceType::Pawn),
            37
        );

        occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == 0
        );

        assert(
            !isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        std::cout << "[PASS] Bishop attacker and blocker\n";
    }


    void testRookAttackerAndBlocker()
    {
        Position position;
        position.clear();

        // e8 -> e4
        position.setPiece(
            makePiece(Color::Black, PieceType::Rook),
            60
        );

        Bitboard occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == getBit(60)
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        // Put a blocker on e6.
        position.setPiece(
            makePiece(Color::White, PieceType::Pawn),
            44
        );

        occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == 0
        );

        assert(
            !isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        std::cout << "[PASS] Rook attacker and blocker\n";
    }


    void testQueenAttacker()
    {
        Position position;
        position.clear();

        // Black queen on e1 attacks e4 vertically.
        position.setPiece(
            makePiece(Color::Black, PieceType::Queen),
            4
        );

        const Bitboard occupancy = position.pieces();

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == getBit(4)
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        std::cout << "[PASS] Queen attacker\n";
    }


    void testMultipleAttackers()
    {
        Position position;
        position.clear();

        // All six black piece types attack e4 at the same time.
        position.setPiece(
            makePiece(Color::Black, PieceType::Pawn),
            35              // d5
        );

        position.setPiece(
            makePiece(Color::Black, PieceType::Knight),
            45              // f6
        );

        position.setPiece(
            makePiece(Color::Black, PieceType::Bishop),
            55              // h7
        );

        position.setPiece(
            makePiece(Color::Black, PieceType::Rook),
            60              // e8
        );

        position.setPiece(
            makePiece(Color::Black, PieceType::Queen),
            4               // e1
        );

        position.setPiece(
            makePiece(Color::Black, PieceType::King),
            29              // f4
        );

        const Bitboard occupancy = position.pieces();

        const Bitboard expected =
            getBit(35) |
            getBit(45) |
            getBit(55) |
            getBit(60) |
            getBit(4) |
            getBit(29);

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                occupancy
            ) == expected
        );

        assert(
            isSquareAttacked(
                position,
                E4,
                Color::Black,
                occupancy
            )
        );

        std::cout << "[PASS] Multiple attackers\n";
    }


    void testCustomOccupancy()
    {
        Position position;
        position.clear();

        position.setPiece(
            makePiece(Color::Black, PieceType::Rook),
            60              // e8
        );

        // The Position itself has no blocker, but the occupancy supplied
        // to the detector pretends that e6 is occupied.
        const Bitboard customOccupancy =
            position.pieces() | getBit(44);

        assert(
            attackersTo(
                position,
                E4,
                Color::Black,
                customOccupancy
            ) == 0
        );

        assert(
            !isSquareAttacked(
                position,
                E4,
                Color::Black,
                customOccupancy
            )
        );

        std::cout << "[PASS] Custom occupancy\n";
    }
}


void runAttackDetectorTests()
{
    std::cout << "\n--- Attack detector tests ---\n";

    testNoAttackers();
    testPawnAttackers();
    testKnightAttacker();
    testKingAttacker();
    testBishopAttackerAndBlocker();
    testRookAttackerAndBlocker();
    testQueenAttacker();
    testMultipleAttackers();
    testCustomOccupancy();
}
