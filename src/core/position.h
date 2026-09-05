#pragma once
#include "core.h"
#include "bitboard.h"
#include "state_info.h"

#include <array>
#include <cstddef>


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
    std::array<Piece, 64> board{};
    std::array<Bitboard, 8> typesBB{};
    std::array<Bitboard, 2> colorsBB{};

    Color sideToMove = Color::White;

    StateInfo* state = nullptr;

public:

    Piece pieceAt(Square square) const
    {
        return board[square];
    }

    Bitboard pieces() const
    {
        return typesBB[ALL_PIECES];
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
        return isBitSet(typesBB[ALL_PIECES], square);
    }
    bool isEmpty(Square square) const
    {
        return !isBitSet(typesBB[ALL_PIECES], square);
    }

    Color getSideToMove() const
    {
        return sideToMove;
    }
    void setSideToMove(Color color)
    {
        sideToMove = color;
    }

    StateInfo& getState()
    {
        return *state;
    }
    void setState(StateInfo& newState)
    {
        state = &newState;
    }

    void setPiece(Piece piece, Square square);
    void removePiece(Square square);
    void movePiece(Square from, Square to);

    constexpr PieceType getPromotionType(Move move)
    {
        return static_cast<PieceType>(
            getData(move) + static_cast<std::uint8_t>(PieceType::Knight)
        );
    }

    void clear();
    bool isValid() const;

    void makeMove(Move move, StateInfo& newState);
    void undoMove(Move move, StateInfo& previousState);
};

constexpr Square WHITE_KINGSIDE_CASTLE_TO = 6;
constexpr Square WHITE_QUEENSIDE_CASTLE_TO = 2;
constexpr Square BLACK_KINGSIDE_CASTLE_TO = 62;
constexpr Square BLACK_QUEENSIDE_CASTLE_TO = 58;


