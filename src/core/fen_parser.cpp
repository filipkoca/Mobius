#include "fen_parser.h"

#include <sstream>
#include <string>
#include <cctype>

bool FenParser::parse(
    Position &position,
    StateInfo &state,
    const std::string &fen
    )
{
    std::istringstream stream(fen);

    std::string boardPart;
    std::string sidePart;
    std::string castlingPart;
    std::string enPassantPart;
    std::string halfmovePart;
    std::string fullmovePart;

    stream  >> boardPart
            >> sidePart
            >> castlingPart
            >> enPassantPart
            >> halfmovePart
            >> fullmovePart;

    FenParser::parseBoard(position, boardPart);
    position.setSideToMove(FenParser::parseSideToMove(sidePart));
    state.castlingRights = FenParser::parseCastlingRights(castlingPart);
    state.enPassantSquare = FenParser::parseEnPassant(enPassantPart);
    state.halfmoveClock =
        static_cast<std::uint16_t>(std::stoi(halfmovePart));
    state.capturedPiece = NO_PIECE;

    position.setState(state);

    // discard full move number for now
    // int fullMoveNumber = std::stoi(fullmovePart);

    return true;
}

void FenParser::parseBoard(
    Position &position,
    const std::string& boardFen
    )
{
    position.clear();

    int rank = 7;
    int file = 0;

    for (char c: boardFen)
    {
        if (c == '/')
        {
            rank--;
            file = 0;
        }

        else if (std::isdigit(static_cast<unsigned char>(c)))
        {
            file += c - '0';
        }

        else
        {
            Piece piece = parsePiece(c);
            Square square = rank * 8 + file;
            position.setPiece(piece, square);
            file++;
        }
    }
}

Piece FenParser::parsePiece(char c)
{
    std::uint8_t pieceColor = std::isupper(static_cast<unsigned char>(c))
        ? 0b00000000
        : 0b00001000;

    char lower = static_cast<char>(
        std::tolower(static_cast<unsigned char>(c))
    );

    std::uint8_t typeMask;

    switch (lower)
    {
        case 'p': typeMask = 1; break;
        case 'n': typeMask = 2; break;
        case 'b': typeMask = 3; break;
        case 'r': typeMask = 4; break;
        case 'q': typeMask = 5; break;
        case 'k': typeMask = 6; break;
        default: return NO_PIECE;
    }

    return static_cast<Piece>(pieceColor | typeMask);
}

Color FenParser::parseSideToMove(const std::string& value)
{
    if (value == "w"){return Color::White;}
    return Color::Black;
}

CastlingRights FenParser::parseCastlingRights(const std::string& value)
{
    CastlingRights castlingRights = NO_CASTLING;

    for (char c: value)
    {
        switch (c)
        {
            case 'K': castlingRights |= WHITE_KING_SIDE; break;
            case 'Q': castlingRights |= WHITE_QUEEN_SIDE; break;
            case 'k': castlingRights |= BLACK_KING_SIDE; break;
            case 'q': castlingRights |= BLACK_QUEEN_SIDE; break;
        }
    }

    return castlingRights;
}

Square FenParser::parseEnPassant(const std::string& value)
{
    if (value == "-")
    {
        return NO_SQUARE;
    }

    int file = value[0] - 'a';
    int rank = value[1] - '1';

    Square square = rank * 8 + file;

    return square;
}

