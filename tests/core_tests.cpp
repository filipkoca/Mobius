#include <cassert>
#include <iostream>

#include "fen_parser.h"
#include "state_buffer.h"

void testStartingPositionFen()
{

    /*
     * FEN:
     * rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
     *
     *     a b c d e f g h
     *   +-----------------+
     * 8 | r n b q k b n r |
     * 7 | p p p p p p p p |
     * 6 | . . . . . . . . |
     * 5 | . . . . . . . . |
     * 4 | . . . . . . . . |
     * 3 | . . . . . . . . |
     * 2 | P P P P P P P P |
     * 1 | R N B Q K B N R |
     *   +-----------------+
     *
     * Side to move: White
     * Castling: KQkq
     * En passant: -
     * Halfmove clock: 0
     * Fullmove number: 1
     */

    const std::string fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Position position;
    StateBuffer stateBuffer;

    StateInfo& rootState = stateBuffer.current();

    bool parsed = FenParser::parse(position, rootState, fen);

    assert(parsed);
    assert(position.isValid());

    // General board state
    assert(popCount(position.pieces()) == 32);
    assert(popCount(position.pieces(Color::White)) == 16);
    assert(popCount(position.pieces(Color::Black)) == 16);

    // Side to move
    assert(position.getSideToMove() == Color::White);

    // StateInfo
    assert(position.getState().castlingRights == ALL_CASTLING);
    assert(position.getState().enPassantSquare == NO_SQUARE);
    assert(position.getState().halfmoveClock == 0);
    assert(position.getState().capturedPiece == NO_PIECE);

    // A few exact squares
    assert(getPieceType(position.pieceAt(0)) == PieceType::Rook);
    assert(getPieceColor(position.pieceAt(0)) == Color::White);

    assert(getPieceType(position.pieceAt(4)) == PieceType::King);
    assert(getPieceColor(position.pieceAt(4)) == Color::White);

    assert(getPieceType(position.pieceAt(60)) == PieceType::King);
    assert(getPieceColor(position.pieceAt(60)) == Color::Black);

    std::cout << "[PASS] Starting position FEN\n";
}

void testComplexFen()
{

    /*
     * FEN:
     * r3k2r/ppp2ppp/2n5/3pp3/3PP3/2N5/PPP2PPP/R3K2R b Kq e3 17 23
     *
     *     a b c d e f g h
     *   +-----------------+
     * 8 | r . . . k . . r |
     * 7 | p p p . . p p p |
     * 6 | . . n . . . . . |
     * 5 | . . . p p . . . |
     * 4 | . . . P P . . . |
     * 3 | . . N . . . . . |
     * 2 | P P P . . P P P |
     * 1 | R . . . K . . R |
     *   +-----------------+
     *
     * Side to move: Black
     * Castling: Kq
     * En passant: e3
     * Halfmove clock: 17
     * Fullmove number: 23
     */

    const std::string fen =
        "r3k2r/ppp2ppp/2n5/3pp3/3PP3/2N5/PPP2PPP/R3K2R b Kq e3 17 23";

    Position position;
    StateBuffer stateBuffer;

    StateInfo& rootState = stateBuffer.current();

    bool parsed = FenParser::parse(position, rootState, fen);

    assert(parsed);
    assert(position.isValid());

    // Occupancy
    assert(popCount(position.pieces()) == 24);
    assert(popCount(position.pieces(Color::White)) == 12);
    assert(popCount(position.pieces(Color::Black)) == 12);

    // Side to move
    assert(position.getSideToMove() == Color::Black);

    // StateInfo
    assert(position.getState().castlingRights ==
           (WHITE_KING_SIDE | BLACK_QUEEN_SIDE));

    assert(position.getState().enPassantSquare == 20); // e3
    assert(position.getState().halfmoveClock == 17);
    assert(position.getState().capturedPiece == NO_PIECE);

    // Exact pieces
    assert(getPieceType(position.pieceAt(4)) == PieceType::King);   // e1
    assert(getPieceColor(position.pieceAt(4)) == Color::White);

    assert(getPieceType(position.pieceAt(18)) == PieceType::Knight); // c3
    assert(getPieceColor(position.pieceAt(18)) == Color::White);

    assert(getPieceType(position.pieceAt(60)) == PieceType::King);  // e8
    assert(getPieceColor(position.pieceAt(60)) == Color::Black);

    assert(getPieceType(position.pieceAt(42)) == PieceType::Knight); // c6
    assert(getPieceColor(position.pieceAt(42)) == Color::Black);

    std::cout << "[PASS] Complex FEN\n";
}

int main()
{
    testStartingPositionFen();
    testComplexFen();

    return 0;
}