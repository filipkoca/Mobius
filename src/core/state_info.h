#pragma once
#include "core.h"

struct StateInfo
{
    CastlingRights castlingRights = NO_CASTLING;
    Square enPassantSquare = NO_SQUARE;
    std::uint16_t halfmoveClock = 0;
    Piece capturedPiece = NO_PIECE;

    // later on hashId, cache info ...
};