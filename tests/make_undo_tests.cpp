#include "make_undo_tests.h"

#include "position.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>


namespace
{
    struct PositionSnapshot
    {
        std::array<Piece, 64> board{};

        Bitboard occupancy = 0;

        std::array<Bitboard, 6> pieceTypes{};

        Bitboard whitePieces = 0;
        Bitboard blackPieces = 0;

        Color sideToMove = Color::White;

        StateInfo state{};
    };


    Move makeTestMove(
        Square from,
        Square to,
        MoveType type = MoveType::Normal,
        std::uint8_t data = 0
    )
    {
        return static_cast<Move>(
            static_cast<Move>(to) |
            (static_cast<Move>(from) << MOVE_FROM_SHIFT) |
            (
                static_cast<Move>(data)
                << MOVE_DATA_SHIFT
            ) |
            (
                static_cast<Move>(
                    static_cast<std::uint8_t>(type)
                )
                << MOVE_TYPE_SHIFT
            )
        );
    }


    void setupPosition(
        Position& position,
        StateInfo& state,
        Color sideToMove = Color::White,
        CastlingRights castlingRights = NO_CASTLING,
        Square enPassantSquare = NO_SQUARE,
        std::uint16_t halfmoveClock = 0
    )
    {
        position.clear();

        state = StateInfo{};

        state.castlingRights = castlingRights;
        state.enPassantSquare = enPassantSquare;
        state.halfmoveClock = halfmoveClock;
        state.capturedPiece = NO_PIECE;

        position.setState(state);
        position.setSideToMove(sideToMove);
    }


    PositionSnapshot takeSnapshot(Position& position)
    {
        PositionSnapshot snapshot;

        for (int i = 0; i < 64; i++)
        {
            snapshot.board[i] =
                position.pieceAt(
                    static_cast<Square>(i)
                );
        }

        snapshot.occupancy = position.pieces();

        for (int i = 1; i <= 6; i++)
        {
            snapshot.pieceTypes[i - 1] =
                position.pieces(
                    static_cast<PieceType>(i)
                );
        }

        snapshot.whitePieces =
            position.pieces(Color::White);

        snapshot.blackPieces =
            position.pieces(Color::Black);

        snapshot.sideToMove =
            position.getSideToMove();

        snapshot.state =
            position.getState();

        return snapshot;
    }


    void assertSamePosition(
        Position& position,
        const PositionSnapshot& snapshot
    )
    {
        assert(position.isValid());

        for (int i = 0; i < 64; i++)
        {
            assert(
                position.pieceAt(
                    static_cast<Square>(i)
                ) ==
                snapshot.board[i]
            );
        }

        assert(
            position.pieces() ==
            snapshot.occupancy
        );

        for (int i = 1; i <= 6; i++)
        {
            assert(
                position.pieces(
                    static_cast<PieceType>(i)
                ) ==
                snapshot.pieceTypes[i - 1]
            );
        }

        assert(
            position.pieces(Color::White) ==
            snapshot.whitePieces
        );

        assert(
            position.pieces(Color::Black) ==
            snapshot.blackPieces
        );

        assert(
            position.getSideToMove() ==
            snapshot.sideToMove
        );

        StateInfo& state =
            position.getState();

        assert(
            state.castlingRights ==
            snapshot.state.castlingRights
        );

        assert(
            state.enPassantSquare ==
            snapshot.state.enPassantSquare
        );

        assert(
            state.halfmoveClock ==
            snapshot.state.halfmoveClock
        );

        assert(
            state.capturedPiece ==
            snapshot.state.capturedPiece
        );
    }


    void testQuietMove()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White,
            NO_CASTLING,
            NO_SQUARE,
            7
        );

        Piece knight =
            makePiece(
                Color::White,
                PieceType::Knight
            );

        position.setPiece(knight, 6);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(6, 21);

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.isEmpty(6));
        assert(position.pieceAt(21) == knight);

        assert(
            position.getSideToMove() ==
            Color::Black
        );

        assert(
            position.getState().halfmoveClock ==
            8
        );

        assert(
            position.getState().capturedPiece ==
            NO_PIECE
        );

        assert(
            &position.getState() ==
            &next
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        assert(
            &position.getState() ==
            &root
        );

        std::cout << "[PASS] Quiet move\n";
    }


    void testCapture()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White,
            NO_CASTLING,
            NO_SQUARE,
            12
        );

        Piece bishop =
            makePiece(
                Color::White,
                PieceType::Bishop
            );

        Piece pawn =
            makePiece(
                Color::Black,
                PieceType::Pawn
            );

        position.setPiece(bishop, 26);
        position.setPiece(pawn, 53);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(26, 53);

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.isEmpty(26));
        assert(position.pieceAt(53) == bishop);

        assert(
            position.getState().capturedPiece ==
            pawn
        );

        assert(
            position.getState().halfmoveClock ==
            0
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Normal capture\n";
    }


    void testWhiteDoublePawnPush()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White
        );

        Piece pawn =
            makePiece(
                Color::White,
                PieceType::Pawn
            );

        position.setPiece(pawn, 12);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(12, 28);

        position.makeMove(move, next);

        assert(position.isValid());

        assert(
            position.pieceAt(28) ==
            pawn
        );

        // e3
        assert(
            position.getState().enPassantSquare ==
            20
        );

        assert(
            position.getState().halfmoveClock ==
            0
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] White double pawn push\n";
    }


    void testBlackDoublePawnPush()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::Black
        );

        Piece pawn =
            makePiece(
                Color::Black,
                PieceType::Pawn
            );

        position.setPiece(pawn, 51);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(51, 35);

        position.makeMove(move, next);

        assert(position.isValid());

        assert(
            position.pieceAt(35) ==
            pawn
        );

        // d6
        assert(
            position.getState().enPassantSquare ==
            43
        );

        assert(
            position.getState().halfmoveClock ==
            0
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Black double pawn push\n";
    }


    void testEnPassantExpires()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White,
            NO_CASTLING,
            20
        );

        Piece knight =
            makePiece(
                Color::White,
                PieceType::Knight
            );

        position.setPiece(knight, 6);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(6, 21);

        position.makeMove(move, next);

        assert(
            position.getState().enPassantSquare ==
            NO_SQUARE
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] En passant expiration\n";
    }


    void testWhitePromotions()
    {
        for (
            std::uint8_t data = 0;
            data < 4;
            data++
        )
        {
            Position position;

            StateInfo root;
            StateInfo next;

            setupPosition(
                position,
                root,
                Color::White
            );

            Piece pawn =
                makePiece(
                    Color::White,
                    PieceType::Pawn
                );

            position.setPiece(pawn, 48);

            PositionSnapshot before =
                takeSnapshot(position);

            Move move =
                makeTestMove(
                    48,
                    56,
                    MoveType::Promotion,
                    data
                );

            position.makeMove(
                move,
                next
            );

            PieceType expectedType =
                static_cast<PieceType>(
                    static_cast<std::uint8_t>(
                        PieceType::Knight
                    )
                    + data
                );

            Piece expectedPiece =
                makePiece(
                    Color::White,
                    expectedType
                );

            assert(position.isValid());

            assert(position.isEmpty(48));

            assert(
                position.pieceAt(56) ==
                expectedPiece
            );

            assert(
                position.getState().halfmoveClock ==
                0
            );

            position.undoMove(
                move,
                root
            );

            assertSamePosition(
                position,
                before
            );
        }

        std::cout << "[PASS] White promotions N/B/R/Q\n";
    }


    void testBlackPromotion()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::Black
        );

        Piece pawn =
            makePiece(
                Color::Black,
                PieceType::Pawn
            );

        position.setPiece(pawn, 8);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                8,
                0,
                MoveType::Promotion,
                3
            );

        position.makeMove(move, next);

        Piece queen =
            makePiece(
                Color::Black,
                PieceType::Queen
            );

        assert(position.isValid());
        assert(position.pieceAt(0) == queen);

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Black promotion\n";
    }


    void testPromotionCapture()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White,
            BLACK_KING_SIDE
        );

        Piece pawn =
            makePiece(
                Color::White,
                PieceType::Pawn
            );

        Piece rook =
            makePiece(
                Color::Black,
                PieceType::Rook
            );

        position.setPiece(pawn, 54);
        position.setPiece(rook, 63);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                54,
                63,
                MoveType::Promotion,
                3
            );

        position.makeMove(move, next);

        Piece queen =
            makePiece(
                Color::White,
                PieceType::Queen
            );

        assert(position.isValid());

        assert(
            position.pieceAt(63) ==
            queen
        );

        assert(
            position.getState().capturedPiece ==
            rook
        );

        assert(
            (
                position.getState().castlingRights &
                BLACK_KING_SIDE
            )
            == 0
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Promotion capture\n";
    }


    void testWhiteEnPassant()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White,
            NO_CASTLING,
            43
        );

        Piece whitePawn =
            makePiece(
                Color::White,
                PieceType::Pawn
            );

        Piece blackPawn =
            makePiece(
                Color::Black,
                PieceType::Pawn
            );

        position.setPiece(whitePawn, 36);
        position.setPiece(blackPawn, 35);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                36,
                43,
                MoveType::EnPassant
            );

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.isEmpty(36));
        assert(position.isEmpty(35));

        assert(
            position.pieceAt(43) ==
            whitePawn
        );

        assert(
            position.getState().capturedPiece ==
            blackPawn
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] White en passant\n";
    }


    void testBlackEnPassant()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::Black,
            NO_CASTLING,
            20
        );

        Piece blackPawn =
            makePiece(
                Color::Black,
                PieceType::Pawn
            );

        Piece whitePawn =
            makePiece(
                Color::White,
                PieceType::Pawn
            );

        position.setPiece(blackPawn, 27);
        position.setPiece(whitePawn, 28);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                27,
                20,
                MoveType::EnPassant
            );

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.isEmpty(27));
        assert(position.isEmpty(28));

        assert(
            position.pieceAt(20) ==
            blackPawn
        );

        assert(
            position.getState().capturedPiece ==
            whitePawn
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Black en passant\n";
    }


    void testWhiteKingsideCastle()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White,
            ALL_CASTLING
        );

        Piece king =
            makePiece(
                Color::White,
                PieceType::King
            );

        Piece rook =
            makePiece(
                Color::White,
                PieceType::Rook
            );

        position.setPiece(king, 4);
        position.setPiece(rook, 7);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                4,
                6,
                MoveType::Castling
            );

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.pieceAt(6) == king);
        assert(position.pieceAt(5) == rook);

        assert(
            (
                position.getState().castlingRights &
                WHITE_CASTLING
            )
            == 0
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] White kingside castle\n";
    }


    void testWhiteQueensideCastle()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::White,
            ALL_CASTLING
        );

        Piece king =
            makePiece(
                Color::White,
                PieceType::King
            );

        Piece rook =
            makePiece(
                Color::White,
                PieceType::Rook
            );

        position.setPiece(king, 4);
        position.setPiece(rook, 0);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                4,
                2,
                MoveType::Castling
            );

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.pieceAt(2) == king);
        assert(position.pieceAt(3) == rook);

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] White queenside castle\n";
    }


    void testBlackKingsideCastle()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::Black,
            ALL_CASTLING
        );

        Piece king =
            makePiece(
                Color::Black,
                PieceType::King
            );

        Piece rook =
            makePiece(
                Color::Black,
                PieceType::Rook
            );

        position.setPiece(king, 60);
        position.setPiece(rook, 63);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                60,
                62,
                MoveType::Castling
            );

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.pieceAt(62) == king);
        assert(position.pieceAt(61) == rook);

        assert(
            (
                position.getState().castlingRights &
                BLACK_CASTLING
            )
            == 0
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Black kingside castle\n";
    }


    void testBlackQueensideCastle()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::Black,
            ALL_CASTLING
        );

        Piece king =
            makePiece(
                Color::Black,
                PieceType::King
            );

        Piece rook =
            makePiece(
                Color::Black,
                PieceType::Rook
            );

        position.setPiece(king, 60);
        position.setPiece(rook, 56);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(
                60,
                58,
                MoveType::Castling
            );

        position.makeMove(move, next);

        assert(position.isValid());

        assert(position.pieceAt(58) == king);
        assert(position.pieceAt(59) == rook);

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Black queenside castle\n";
    }


    void runRookCastlingRightCase(
        Color color,
        Square from,
        Square to,
        CastlingRights right
    )
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            color,
            ALL_CASTLING
        );

        Piece rook =
            makePiece(
                color,
                PieceType::Rook
            );

        position.setPiece(rook, from);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(from, to);

        position.makeMove(move, next);

        CastlingRights expected =
            static_cast<CastlingRights>(
                ALL_CASTLING &
                static_cast<CastlingRights>(
                    ~right
                )
            );

        assert(
            position.getState().castlingRights ==
            expected
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);
    }


    void testRookCastlingRights()
    {
        runRookCastlingRightCase(
            Color::White,
            7,
            15,
            WHITE_KING_SIDE
        );

        runRookCastlingRightCase(
            Color::White,
            0,
            8,
            WHITE_QUEEN_SIDE
        );

        runRookCastlingRightCase(
            Color::Black,
            63,
            55,
            BLACK_KING_SIDE
        );

        runRookCastlingRightCase(
            Color::Black,
            56,
            48,
            BLACK_QUEEN_SIDE
        );

        std::cout << "[PASS] Rook castling rights\n";
    }


    void testCapturedRookCastlingRights()
    {
        Position position;

        StateInfo root;
        StateInfo next;

        setupPosition(
            position,
            root,
            Color::Black,
            ALL_CASTLING
        );

        Piece bishop =
            makePiece(
                Color::Black,
                PieceType::Bishop
            );

        Piece rook =
            makePiece(
                Color::White,
                PieceType::Rook
            );

        // Black bishop b2 -> a1
        position.setPiece(bishop, 9);
        position.setPiece(rook, 0);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move =
            makeTestMove(9, 0);

        position.makeMove(move, next);

        assert(
            (
                position.getState().castlingRights &
                WHITE_QUEEN_SIDE
            )
            == 0
        );

        position.undoMove(move, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Captured rook castling rights\n";
    }


    void testMultiplePlyRoundTrip()
    {
        Position position;

        StateInfo root;
        StateInfo state1;
        StateInfo state2;
        StateInfo state3;

        setupPosition(
            position,
            root,
            Color::White
        );

        Piece whitePawn =
            makePiece(
                Color::White,
                PieceType::Pawn
            );

        Piece blackPawn =
            makePiece(
                Color::Black,
                PieceType::Pawn
            );

        position.setPiece(whitePawn, 12);
        position.setPiece(blackPawn, 51);

        PositionSnapshot before =
            takeSnapshot(position);

        Move move1 =
            makeTestMove(12, 28);

        Move move2 =
            makeTestMove(51, 35);

        Move move3 =
            makeTestMove(28, 35);

        position.makeMove(move1, state1);
        assert(position.isValid());

        position.makeMove(move2, state2);
        assert(position.isValid());

        position.makeMove(move3, state3);
        assert(position.isValid());

        position.undoMove(move3, state2);
        assert(position.isValid());

        position.undoMove(move2, state1);
        assert(position.isValid());

        position.undoMove(move1, root);

        assertSamePosition(position, before);

        std::cout << "[PASS] Multi-ply round trip\n";
    }
}


void runMakeUndoTests()
{
    std::cout << "\n--- Make / Undo tests ---\n";

    testQuietMove();
    testCapture();

    testWhiteDoublePawnPush();
    testBlackDoublePawnPush();
    testEnPassantExpires();

    testWhitePromotions();
    testBlackPromotion();
    testPromotionCapture();

    testWhiteEnPassant();
    testBlackEnPassant();

    testWhiteKingsideCastle();
    testWhiteQueensideCastle();
    testBlackKingsideCastle();
    testBlackQueensideCastle();

    testRookCastlingRights();
    testCapturedRookCastlingRights();

    testMultiplePlyRoundTrip();

    std::cout << "\nMake/Undo suite passed.\n";
}