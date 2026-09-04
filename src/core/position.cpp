#include "position.h"

void Position::setPiece(Piece piece, Square square)
{
    PieceType pieceType = getPieceType(piece);
    Color pieceColor = getPieceColor(piece);
    Bitboard mask = getBit(square);

    typesBB[0] |= mask;
    typesBB[static_cast<std::size_t>(pieceType)] |= mask;
    colorsBB[static_cast<std::size_t>(pieceColor)] |= mask;
    board[square] = piece;
}


void Position::removePiece(Square square)
{
    Piece piece = board[square];
    PieceType pieceType = getPieceType(piece);
    Color pieceColor = getPieceColor(piece);
    Bitboard mask = ~getBit(square);

    typesBB[ALL_PIECES] &= mask;
    typesBB[static_cast<std::size_t>(pieceType)] &= mask;
    colorsBB[static_cast<std::size_t>(pieceColor)] &= mask;
    board[square] = NO_PIECE;
}

void Position::movePiece(Square from, Square to)
{
    Piece piece = board[from];
    PieceType pieceType = getPieceType(piece);
    Color pieceColor = getPieceColor(piece);
    Bitboard mask = getBit(from) | getBit(to);

    board[from] = NO_PIECE;
    board[to] = piece;
    typesBB[ALL_PIECES] ^= mask;
    typesBB[static_cast<std::size_t>(pieceType)] ^= mask;
    colorsBB[static_cast<std::size_t>(pieceColor)] ^= mask;
}

void Position::clear()
{
    board.fill(NO_PIECE);
    typesBB.fill(0);
    colorsBB.fill(0);
    sideToMove = Color::White;
}

bool Position::isValid() const
{
    Bitboard typeOccupancy = 0;

    for (std::size_t i = 1; i <= 6; i++)
    {
        typeOccupancy |= typesBB[i];
    }

    if (typeOccupancy != typesBB[ALL_PIECES])
    {
        return false;
    }

    if ((colorsBB[0] | colorsBB[1]) != typesBB[ALL_PIECES])
    {
        return false;
    }

    if ((colorsBB[0] & colorsBB[1]) != 0)
    {
        return false;
    }

    for (Square square = 0; square < 64; square++)
    {
        Piece piece = board[square];

        if (isEmpty(piece))
        {
            if (isBitSet(typesBB[ALL_PIECES], square))
            {
                return false;
            }

            continue;
        }

        PieceType pieceType = getPieceType(piece);
        Color pieceColor = getPieceColor(piece);

        if (!isBitSet(typesBB[ALL_PIECES], square))
        {
            return false;
        }

        if (!isBitSet(typesBB[static_cast<std::size_t>(pieceType)], square))
        {
            return false;
        }

        if (!isBitSet(colorsBB[static_cast<std::size_t>(pieceColor)], square))
        {
            return false;
        }
    }

    return true;
}
