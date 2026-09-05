#include <cassert>
#include <iostream>

#include "fen_parser.h"
#include "state_buffer.h"

void testStartingPositionFen()
{
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

int main()
{
    testStartingPositionFen();

    return 0;
}