#include "position.h"
#include <array>

constexpr CastlingRights removeCastlingRights(CastlingRights rights)
{
    return static_cast<CastlingRights>(ALL_CASTLING & ~rights);
}

constexpr std::array<CastlingRights, 64> buildCastlingRightsMasks()
{
    std::array<CastlingRights, 64> masks = {};

    masks.fill(ALL_CASTLING);

    masks[0] = removeCastlingRights(WHITE_QUEEN_SIDE);
    masks[4] = removeCastlingRights(WHITE_CASTLING);
    masks[7] = removeCastlingRights(WHITE_KING_SIDE);

    masks[56] = removeCastlingRights(BLACK_QUEEN_SIDE);
    masks[60] = removeCastlingRights(BLACK_CASTLING);
    masks[63] = removeCastlingRights(BLACK_KING_SIDE);

    return masks;
}

constexpr std::array<CastlingRights, 64> CASTLING_RIGHTS_MASKS =
    buildCastlingRightsMasks();

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

    newState.castlingRights &= static_cast<CastlingRights>(
        CASTLING_RIGHTS_MASKS[from] &
        CASTLING_RIGHTS_MASKS[to]
    );

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

void Position::undoMove(Move move, StateInfo &previousState)
{
    // decode move
    Square from = getFromSq(move);
    Square to = getToSq(move);
    MoveType moveType = getType(move);

    Piece capturedPiece = state->capturedPiece;
    sideToMove = sideToMove == Color::White
        ? Color::Black
        : Color::White;

    switch (moveType)
    {
        case MoveType::Normal:
        {
            movePiece(to, from);
            if (capturedPiece != NO_PIECE)
            {
                setPiece(capturedPiece, to);
            }
            break;
        }

        case MoveType::Promotion:
        {
            removePiece(to);
            Piece pawn = makePiece(sideToMove, PieceType::Pawn);
            setPiece(pawn, from);

            if (capturedPiece != NO_PIECE)
            {
                setPiece(capturedPiece, to);
            }

            break;
        }

        case MoveType::EnPassant:
        {
            movePiece(to, from);

            std::int8_t dir = sideToMove == Color::White ? -1 : 1;
            Square capturedPieceSquare = to + (8 * dir);

            setPiece(capturedPiece, capturedPieceSquare);

            break;
        }


        case MoveType::Castling:
        {
            Square rookFrom;
            Square rookTo;

            if (sideToMove == Color::White)
            {
                if (to == WHITE_KINGSIDE_CASTLE_TO)
                {
                    // Undo:
                    // K: 6 -> 4
                    // R: 5 -> 7
                    rookFrom = 5;
                    rookTo = 7;
                }
                else
                {
                    // Undo:
                    // K: 2 -> 4
                    // R: 3 -> 0
                    rookFrom = 3;
                    rookTo = 0;
                }
            }
            else
            {
                if (to == BLACK_KINGSIDE_CASTLE_TO)
                {
                    // Undo:
                    // K: 62 -> 60
                    // R: 61 -> 63
                    rookFrom = 61;
                    rookTo = 63;
                }
                else
                {
                    // Undo:
                    // K: 58 -> 60
                    // R: 59 -> 56
                    rookFrom = 59;
                    rookTo = 56;
                }
            }

            movePiece(to, from);
            movePiece(rookFrom, rookTo);

            break;
        }
    }

    state = &previousState;
}