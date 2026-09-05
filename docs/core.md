
<div align="center">
<img src="../assets/mobius_logo.png" alt="Mobius Logo" width="60%">
</div>

# Mobius Core Architecture

This document describes the current core architecture of the Mobius chess engine.

Mobius is built around compact binary protocols, redundant but efficient board representations, fixed-size state storage, and a strict separation between physical board state and reversible per-ply state.

## Table of Contents

1. [Core Design](#1-core-design)
2. [Square and Bitboard Protocol](#2-square-and-bitboard-protocol)
3. [Color and PieceType](#3-color-and-piecetype)
4. [Piece Protocol](#4-piece-protocol)
5. [Move Protocol](#5-move-protocol)
6. [Castling Rights](#6-castling-rights)
7. [Position Architecture](#7-position-architecture)
8. [Position Mutation](#8-position-mutation)
9. [StateInfo](#9-stateinfo)
10. [StateBuffer](#10-statebuffer)
11. [FEN Initialization](#11-fen-initialization)


---

## 1. Core Design

The Mobius core follows several principles:

- compact data representation
- predictable memory layout
- fixed-size storage
- no heap allocation in hot paths
- one mutable `Position`
- reversible state stored separately per ply
- fast bitboard operations
- strict testing of representation invariants

The complete chess state is represented as:

```text
Position + current StateInfo
```

`Position` stores the physical board state.

`StateInfo` stores reversible information that changes from ply to ply.

`StateBuffer` provides fixed reusable storage for `StateInfo` objects during DFS search.

---

## 2. Square and Bitboard Protocol

Squares use direct 0-63 indexing:

```text
A1 = 0
B1 = 1
...
H8 = 63
```

Board layout:

```text
     a   b   c   d   e   f   g   h

8   56  57  58  59  60  61  62  63
7   48  49  50  51  52  53  54  55
6   40  41  42  43  44  45  46  47
5   32  33  34  35  36  37  38  39
4   24  25  26  27  28  29  30  31
3   16  17  18  19  20  21  22  23
2    8   9  10  11  12  13  14  15
1    0   1   2   3   4   5   6   7
```

```cpp
using Square = std::uint8_t;
constexpr Square NO_SQUARE = 64;
```

Rank and file:

```text
rank = square >> 3
file = square & 7
```

A bitboard is:

```cpp
using Bitboard = std::uint64_t;
```

The bit index is identical to the square index:

```text
bit 0  <-> A1
bit 63 <-> H8
```

A square mask is therefore:

```text
1ULL << square
```

Important helpers include:

```text
getBit
isBitSet
setBit
clearBit
toggleBit
popCount
getLSBit
popLSBit
```

---

## 3. Color and PieceType

Color:

```cpp
enum class Color : std::uint8_t
{
    White = 0,
    Black = 1
};
```

This allows direct indexing into:

```text
colorsBB[0] -> White
colorsBB[1] -> Black
```

Piece type:

```cpp
enum class PieceType : std::uint8_t
{
    None   = 0,
    Pawn   = 1,
    Knight = 2,
    Bishop = 3,
    Rook   = 4,
    Queen  = 5,
    King   = 6
};
```

The values map directly to the type-bitboard indexes.

---

## 4. Piece Protocol

A `Piece` is stored in one byte:

```cpp
using Piece = std::uint8_t;
```

Bit layout:

```text
bit:    7 6 5 4 | 3 | 2 1 0
        --------+---+------
        unused  | C | TYPE
```

```text
+---------+---+-------+
| 7 6 5 4 | 3 | 2 1 0 |
+---------+---+-------+
| x x x x | C | T T T |
+---------+---+-------+
```

Type bits:

```text
000 = None
001 = Pawn
010 = Knight
011 = Bishop
100 = Rook
101 = Queen
110 = King
```

Color bit:

```text
0 = White
1 = Black
```

Examples:

```text
White Pawn = 0000 0001 = 1
White King = 0000 0110 = 6

Black Pawn = 0000 1001 = 9
Black King = 0000 1110 = 14
```

Empty square:

```cpp
constexpr Piece NO_PIECE = 0;
```

Extraction:

```text
type  = piece & 0b00000111
color = piece >> 3
```

---

## 5. Move Protocol

A move is stored in 16 bits:

```cpp
using Move = std::uint16_t;
```

Layout:

```text
15 14 | 13 12 | 11 10 9 8 7 6 | 5 4 3 2 1 0
------+-------+-----------------+--------------
 TYPE | DATA  |      FROM       |      TO
  2b  |  2b   |       6b        |      6b
```

Move types:

```text
00 = Normal
01 = Promotion
10 = En Passant
11 = Castling
```

Promotion data:

```text
00 = Knight
01 = Bishop
10 = Rook
11 = Queen
```

Mobius does not store the moving piece, captured piece, or capture flag inside `Move`.

Those values are inferred from `Position`.

---

## 6. Castling Rights

Castling rights use one byte:

```cpp
using CastlingRights = std::uint8_t;
```

Lower four bits:

```text
bit 3 | bit 2 | bit 1 | bit 0
------+-------+-------+------
  BQ  |  BK   |  WQ   |  WK
```

Examples:

```text
0000 = no castling rights
0001 = White king side
0010 = White queen side
0100 = Black king side
1000 = Black queen side
1111 = all castling rights
```

---

## 7. Position Architecture

Mobius uses one `Position` containing three synchronized board views:

```text
Position
+----------------------------------+
| board[64]                        |
| typesBB[8]                       |
| colorsBB[2]                      |
| sideToMove                       |
| StateInfo* state                 |
+----------------------------------+
```

### Mailbox

```cpp
std::array<Piece, 64> board{};
```

Answers:

```text
"What exact piece is on this square?"
```

### Type Bitboards

```cpp
std::array<Bitboard, 8> typesBB{};
```

Current layout:

```text
typesBB[0] = all pieces / occupancy
typesBB[1] = pawns
typesBB[2] = knights
typesBB[3] = bishops
typesBB[4] = rooks
typesBB[5] = queens
typesBB[6] = kings
typesBB[7] = reserved
```

### Color Bitboards

```cpp
std::array<Bitboard, 2> colorsBB{};
```

```text
colorsBB[0] = White pieces
colorsBB[1] = Black pieces
```

A color + type query is simply:

```text
typesBB[type] & colorsBB[color]
```

Example:

```text
White bishops =
typesBB[Bishop] & colorsBB[White]
```

The representation is redundant by design. The mailbox is best for exact square lookup, while bitboards are best for bulk piece queries.

---

## 8. Position Mutation

All three board representations must always remain synchronized.

### setPiece()

Adds a piece to an empty square:

```text
occupancy      |= mask
type bitboard  |= mask
color bitboard |= mask
board[square]   = piece
```

Expected preconditions:

```text
square is valid
square is empty
piece != NO_PIECE
```

### removePiece()

Removes an existing piece:

```text
mask = ~getBit(square)

occupancy      &= mask
type bitboard  &= mask
color bitboard &= mask
board[square]   = NO_PIECE
```

### movePiece()

Moves one piece from `from` to `to`:

```text
mask = getBit(from) | getBit(to)
```

The transition is:

```text
from: 1 -> 0
to:   0 -> 1
```

Therefore:

```text
occupancy      ^= mask
type bitboard  ^= mask
color bitboard ^= mask
```

Mailbox:

```text
board[from] = NO_PIECE
board[to]   = piece
```

Expected preconditions:

```text
from is occupied
to is empty
from != to
```

Captures are intentionally handled separately:

```text
removePiece(to)
movePiece(from, to)
```

---

## 9. StateInfo

Mobius avoids copying a complete `Position` at every node.

Instead:

```text
ONE mutable Position
+
small reversible StateInfo per ply
```

Current state:

```cpp
struct StateInfo
{
    CastlingRights castlingRights;
    Square enPassantSquare;
    std::uint16_t halfmoveClock;
    Piece capturedPiece;
};
```

Defaults:

```text
castlingRights  = NO_CASTLING
enPassantSquare = NO_SQUARE
halfmoveClock   = 0
capturedPiece   = NO_PIECE
```

Likely future additions include:

```text
Zobrist hash
cached tactical state
other incremental position data
```

---

## 10. StateBuffer

`StateBuffer` owns fixed reusable per-ply storage:

```cpp
std::array<StateInfo, MAX_STATES> states{};
std::size_t currentIndex = 0;
```

Logical structure:

```text
state[0]   root
state[1]   ply 1
state[2]   ply 2
state[3]   ply 3
...
```

Navigation:

```text
current()  -> current slot
next()     -> ++currentIndex
previous() -> --currentIndex
reset()    -> currentIndex = 0
```

DFS naturally behaves like a stack, so old slots are simply reused.

There is:

```text
no allocation per node
no deallocation per node
contiguous memory
predictable storage
```

`Position` stores only:

```cpp
StateInfo* state;
```

The pointer does not own the memory. `StateBuffer` owns the `StateInfo` objects.

---

## 11. FEN Initialization

The FEN parser is cold setup code.

It initializes:

```text
piece placement
side to move
castling rights
en-passant square
halfmove clock
```

The fullmove number is currently ignored because search does not need it.

Typical root setup:

```text
StateBuffer
    |
    v
state[0]

FenParser
    |
    +--> Position
    +--> state[0]

Position::state
    |
    v
state[0]
```

FEN begins at A8 while Mobius indexing begins at A1, so board parsing starts at:

```text
A8 = 56
```

and proceeds rank by rank toward A1.
