#pragma once

#include "attacks.h"
#include "position.h"

#include <cstddef>

inline Bitboard attackersTo(
    const Position& position,
    Square target,
    Color attackerColor,
    Bitboard occupancy
) noexcept
{
    Color reversePawnColor = attackerColor == Color::White ?
        Color::Black : Color::White;

    const auto pawnIndex =
        static_cast<std::size_t>(reversePawnColor);

    const Bitboard pawns = position.pieces(attackerColor, PieceType::Pawn);
    const Bitboard knights = position.pieces(attackerColor, PieceType::Knight);
    const Bitboard bishops = position.pieces(attackerColor, PieceType::Bishop);
    const Bitboard rooks = position.pieces(attackerColor, PieceType::Rook);
    const Bitboard queens = position.pieces(attackerColor, PieceType::Queen);
    const Bitboard king = position.pieces(attackerColor, PieceType::King);

    Bitboard attackers = 0;

    attackers |= PAWN_ATTACKS[pawnIndex][target] & pawns;
    attackers |= KNIGHT_ATTACKS[target] & knights;
    attackers |= KING_ATTACKS[target] & king;

    attackers |= bishopAttacks(target, occupancy) & (bishops | queens);
    attackers |= rookAttacks(target, occupancy) & (rooks | queens);

    return attackers;
}

inline bool isSquareAttacked(
    const Position& position,
    Square target,
    Color attackerColor,
    Bitboard occupancy
) noexcept
{
    Color reversePawnColor = attackerColor == Color::White ?
        Color::Black : Color::White;

    const auto pawnIndex =
        static_cast<std::size_t>(reversePawnColor);

    if (
        (PAWN_ATTACKS[pawnIndex][target] &
        position.pieces(attackerColor, PieceType::Pawn))
        |
        (KNIGHT_ATTACKS[target] &
        position.pieces(attackerColor, PieceType::Knight))
        |
        (KING_ATTACKS[target] &
        position.pieces(attackerColor, PieceType::King))
        )
    {
        return true;
    }

    const Bitboard queens = position.pieces(attackerColor, PieceType::Queen);

    if (
        bishopAttacks(target, occupancy) &
        (position.pieces(attackerColor, PieceType::Bishop) | queens)
    )
    {
        return true;
    }

    if (
        rookAttacks(target, occupancy) &
        (position.pieces(attackerColor, PieceType::Rook) | queens)
    )
    {
        return true;
    }

    return false;

}