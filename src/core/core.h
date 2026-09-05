#pragma once
#include <cstdint>
#include <string>

// BITBOARD
using Bitboard = std::uint64_t;

// SQUARE
using Square = std::uint8_t;
constexpr Square NO_SQUARE = 64;

constexpr std::uint8_t getRankOf(Square square)
{
    return square >> 3;
}

constexpr std::uint8_t getFileOf(Square square)
{
    return square & 7;
}

// COLOR
enum class Color : std::uint8_t
{
    White = 0,
    Black = 1
};

// PIECE TYPE
enum class PieceType : std::uint8_t
{
    None = 0,
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    King = 6
};

/* PIECE uint8
 * +---------+---+-------+
 * | 7 6 5 4 | 3 | 2 1 0 |
 * +---------+---+-------+
 * | x x x x | C | T T T |
 * +---------+---+-------+
*/
using Piece = std::uint8_t;

constexpr Piece NO_PIECE = 0;
constexpr std::uint8_t PIECE_TYPE_MASK = 0b00000111;
constexpr std::uint8_t PIECE_COLOR_MASK = 0b00001000;

constexpr bool isNoPiece(Piece piece)
{
    return piece == NO_PIECE;
}

constexpr PieceType getPieceType(Piece piece)
{
    return static_cast<PieceType>(piece & PIECE_TYPE_MASK);
}

constexpr Color getPieceColor(Piece piece)
{
    return static_cast<Color>(piece >> 3);
}


/* MOVE uint16
 * +-----------------------------------------------------+
 * | 15 14 | 13 12 | 11 10  9  8  7  6 | 5  4  3  2  1  0|
 * +-------+-------+-------------------+-----------------+
 * | MT MT | DA DA |        From       |       To        |
 * +-----------------------------------------------------+
 */
using Move = std::uint16_t;

enum class MoveType : std::uint8_t
{
    Normal = 0,
    Promotion = 1,
    EnPassant = 2,
    Castling = 3
};

constexpr bool isNormal(MoveType moveType)
{
    return moveType == MoveType::Normal;
}

constexpr bool isPromotion(MoveType moveType)
{
    return moveType == MoveType::Promotion;
}

constexpr bool isEnPassant(MoveType moveType)
{
    return moveType == MoveType::EnPassant;
}

constexpr bool isCastling(MoveType moveType)
{
    return moveType == MoveType::Castling;
}


constexpr std::uint16_t MOVE_TO_MASK   = 0b0000000000111111;
constexpr std::uint16_t MOVE_FROM_MASK = 0b0000111111000000;
constexpr std::uint16_t MOVE_DATA_MASK = 0b0011000000000000;
constexpr std::uint16_t MOVE_TYPE_MASK = 0b1100000000000000;

constexpr std::uint8_t MOVE_FROM_SHIFT = 6;
constexpr std::uint8_t MOVE_DATA_SHIFT = 12;
constexpr std::uint8_t MOVE_TYPE_SHIFT = 14;

constexpr std::uint8_t SIX_LS_BITS_MASK = 0b00111111;
constexpr std::uint8_t TWO_LS_BITS_MASK = 0b00000011;

constexpr Square getToSq(Move move)
{
    return static_cast<Square>(move & MOVE_TO_MASK);
}

constexpr Square getFromSq(Move move)
{
    return static_cast<Square>(move >> MOVE_FROM_SHIFT) & SIX_LS_BITS_MASK;
}

constexpr std::uint8_t getData(Move move)
{
    return static_cast<std::uint8_t>(move >> MOVE_DATA_SHIFT) & TWO_LS_BITS_MASK;
}

constexpr MoveType getType(Move move)
{
    return static_cast<MoveType>(move >> MOVE_TYPE_SHIFT);
}

// CASTLING RIGHTS
using CastlingRights = std::uint8_t;

constexpr CastlingRights NO_CASTLING     = 0b0000;

constexpr CastlingRights WHITE_KING_SIDE  = 0b0001;
constexpr CastlingRights WHITE_QUEEN_SIDE = 0b0010;
constexpr CastlingRights BLACK_KING_SIDE  = 0b0100;
constexpr CastlingRights BLACK_QUEEN_SIDE = 0b1000;

constexpr CastlingRights WHITE_CASTLING  = 0b0011;
constexpr CastlingRights BLACK_CASTLING  = 0b1100;

constexpr CastlingRights ALL_CASTLING    = 0b1111;

//DECLARATIONS
std::string getSquareName(Square square);
std::string getMoveToString(Move move);

// check if compiler understands that i aint playin ...
static_assert(sizeof(Bitboard) == 8);
static_assert(sizeof(Square) == 1);
static_assert(sizeof(Color) == 1);
static_assert(sizeof(PieceType) == 1);
static_assert(sizeof(Piece) == 1);
static_assert(sizeof(Move) == 2);
static_assert(sizeof(MoveType) == 1);
static_assert(sizeof(CastlingRights) == 1);