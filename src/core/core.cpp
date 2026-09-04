#include "core.h"
#include <string>

std::string getSquareName(Square sq)
{
    std::uint8_t rank = getRankOf(sq);
    std::uint8_t file = getFileOf(sq);

    char fileName = static_cast<char>('a' + file);
    char rankName = static_cast<char>('1' + rank);

    return std::string{fileName, rankName};
}

std::string getMoveToString(Move m)
{
    Square from = getFromSq(m);
    Square to = getToSq(m);

    std::string result = getSquareName(from) + getSquareName(to);

    MoveType type = getType(m);

    if (isPromotion(type))
    {
        std::uint8_t data = getData(m);
        char promotionPiece;

        switch (data)
        {
            case 0:
                promotionPiece = 'n';
                break;
            case 1:
                promotionPiece = 'b';
                break;
            case 2:
                promotionPiece = 'r';
                break;
            default:
                promotionPiece = 'q';
                break;
        }

        result.push_back(promotionPiece);
    }

    return result;
}