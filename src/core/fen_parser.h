#pragma once

#include <string>

#include "position.h"

class FenParser
{
public:
    static bool parse(
        Position& position,
        StateInfo& state,
        const std::string& fen
    );

private:
    static void parseBoard(Position& position, const std::string& boardFen);
    static Piece parsePiece(char c);
    static Color parseSideToMove(const std::string& value);
    static CastlingRights parseCastlingRights(const std::string& value);
    static Square parseEnPassant(const std::string& value);
    // initialize cache positions into Position later
};