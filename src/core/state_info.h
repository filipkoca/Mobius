#pragma once
#include "core.h"

struct StateInfo
{
    CastlingRights castlingRights;
    Square enPassantSquare;
    std::uint16_t halfmoveClock;
    Piece capturedPiece;

    // later on hashId, cache info ...
};