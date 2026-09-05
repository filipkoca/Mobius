#include "position.h"

void Position::setPiece(Piece piece, Square square)
{
    PieceType pieceType = getPieceType(piece);
    Color pieceColor = getPieceColor(piece);
    Bitboard mask = getBit(square);

    typesBB[ALL_PIECES] |= mask;
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

        if (isNoPiece(piece))
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

void Position::makeMove(Move move, StateInfo& newState)
{
    // decode move
    Square from = getFromSq(move);
    Square to = getToSq(move);
    MoveType moveType = getType(move);

    Piece movingPiece = board[from];
    PieceType movingType = getPieceType(movingPiece);

    newState = *state;

    newState.capturedPiece = NO_PIECE;
    newState.enPassantSquare = NO_SQUARE;

    // ======================================
    //  Update physical board
    // ======================================

    switch (moveType)
    {
        case MoveType::Normal:
        {
            Piece capturedPiece = board[to];
            newState.capturedPiece = capturedPiece;

            if (capturedPiece != NO_PIECE)
            {
                removePiece(to);
            }

            movePiece(from, to);

            break;
        }

        case MoveType::Promotion:
        {
            PieceType promotionType = getPromotionType(move);
            Piece promotionPiece = makePiece(sideToMove, promotionType);

            if (!isEmpty(to))
            {
                newState.capturedPiece = board[to];
                removePiece(to);
            }

            removePiece(from);
            setPiece(promotionPiece, to);

            break;
        }

        case MoveType::EnPassant:
        {
            std::int8_t dir = sideToMove == Color::White ? -1 : 1;
            Square capturedPieceSquare = to + (8 * dir);

            newState.capturedPiece = board[capturedPieceSquare];

            removePiece(capturedPieceSquare);
            movePiece(from, to);

            break;
        }

        case MoveType::Castling:
        {
            Square oldRookSquare;
            Square newRookSquare;

            if (sideToMove == Color::White)
            {
                if (to == WHITE_KINGSIDE_CASTLE_TO)
                {
                    // K: 4 -> 6
                    // R: 7 -> 5
                    oldRookSquare = 7;
                    newRookSquare = 5;
                }

                else // to == WHITE_QUEENSIDE_CASTLE_TO
                {
                    // K: 4 -> 2
                    // R: 0 -> 3
                    oldRookSquare = 0;
                    newRookSquare = 3;
                }
            }
            else //sideToMove == Color::Black
            {
                if (to == BLACK_KINGSIDE_CASTLE_TO)
                {
                    // k: 60 -> 62
                    // r: 63 -> 61
                    oldRookSquare = 63;
                    newRookSquare = 61;
                }

                else // to == BLACK_QUEENSIDE_CASTLE_TO
                {
                    // k: 60 -> 58
                    // r: 56 -> 59
                    oldRookSquare = 56;
                    newRookSquare = 59;
                }
            }

            movePiece(from, to);
            movePiece(oldRookSquare, newRookSquare);

            break;
        }
    }

    // ======================================
    // Update castling rights
    // ======================================

    if (movingType == PieceType::King)
    {
        if (sideToMove == Color::White)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~WHITE_CASTLING);
        }
        else
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~BLACK_CASTLING);
        }
    }

    else if (movingType == PieceType::Rook)
    {
        if (from == 7)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~WHITE_KING_SIDE);
        }
        else if (from == 0)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~WHITE_QUEEN_SIDE);
        }
        else if (from == 63)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~BLACK_KING_SIDE);
        }
        else if (from == 56)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~BLACK_QUEEN_SIDE);
        }
    }

    if (
        newState.capturedPiece != NO_PIECE &&
        getPieceType(newState.capturedPiece) == PieceType::Rook
    )
    {
        if (to == 7)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~WHITE_KING_SIDE);
        }
        else if (to == 0)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~WHITE_QUEEN_SIDE);
        }
        else if (to == 63)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~BLACK_KING_SIDE);
        }
        else if (to == 56)
        {
            newState.castlingRights &=
                static_cast<CastlingRights>(~BLACK_QUEEN_SIDE);
        }
    }

    // --------------------------------------------------
    // Update en passant
    // --------------------------------------------------

    if (movingType == PieceType::Pawn)
    {
        if (
            sideToMove == Color::White &&
            to - from == 16
        )
        {
            newState.enPassantSquare = to - 8;
        }
        else if (
            sideToMove == Color::Black &&
            from - to == 16
        )
        {
            newState.enPassantSquare = to + 8;
        }
    }

    // --------------------------------------------------
    // Update halfmove clock
    // --------------------------------------------------

    if (
       movingType == PieceType::Pawn ||
       newState.capturedPiece != NO_PIECE
   )
    {
        newState.halfmoveClock = 0;
    }
    else
    {
        ++newState.halfmoveClock;
    }

    sideToMove = sideToMove == Color::White
        ? Color::Black
        : Color::White;

    state = &newState;
}
