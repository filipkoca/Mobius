#pragma once
#include "core.h"
#include <array>
#include <cstddef>

#include "bitboard.h"

/*
 * POSITION BOARD REPRESENTATION
 *
 * board[64]     -> Square -> exact Piece
 * typesBB[8]   -> PieceType -> occupied squares
 * colorsBB[2]  -> Color -> occupied squares
 *
 */

constexpr std::size_t ALL_PIECES = 0;

class Position
{
private:
    std::array<Piece, 64> board{};      // 64B
    std::array<Bitboard, 8> typesBB{};  // 64B
    std::array<Bitboard, 2> colorsBB{}; // 16B
    Color sideToMove = Color::White;    // 1B
                                        // = 145B -> 152B with padding

public:
    Piece pieceAt(Square square) const
    {
        return board[square];
    }

    Bitboard pieces() const
    {
        return typesBB[0];
    }

    Bitboard pieces(Color color) const
    {
        return colorsBB[static_cast<std::size_t>(color)];
    }

    Bitboard pieces(PieceType pieceType) const
    {
        return typesBB[static_cast<std::size_t>(pieceType)];
    }

    Bitboard pieces(Color color, PieceType pieceType) const
    {
        return pieces(color) & pieces(pieceType);
    }

    bool isOccupied(Square square) const
    {
        return isBitSet(typesBB[0], square);
    }
    bool isEmpty(Square square) const
    {
        return !isBitSet(typesBB[0], square);
    }

    Color getSideToMove() const
    {
        return sideToMove;
    }

    void setPiece(Piece piece, Square square);
    void removePiece(Square square);
    void movePiece(Square from, Square to);

    void clear();
    bool isValid() const;
};

