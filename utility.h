#pragma once
#include <iostream>

enum Constants {
	MAX_MOVES = 218,
};

enum Square {
	A8, B8, C8, D8, E8, F8, G8, H8,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A1, B1, C1, D1, E1, F1, G1, H1,
	NO_SQUARE
};

enum Rank : uint64_t {
	RANK_8 = 0xff,
	RANK_7 = RANK_8 << (8 * 1),
	RANK_6 = RANK_8 << (8 * 2),
	RANK_5 = RANK_8 << (8 * 3),
	RANK_4 = RANK_8 << (8 * 4),
	RANK_3 = RANK_8 << (8 * 5),
	RANK_2 = RANK_8 << (8 * 6),
	RANK_1 = RANK_8 << (8 * 7)
};

enum File : uint64_t {
	FILE_A = 0x0101010101010101,
	FILE_B = FILE_A << 1,
	FILE_C = FILE_A << 2,
	FILE_D = FILE_A << 3,
	FILE_E = FILE_A << 4,
	FILE_F = FILE_A << 5,
	FILE_G = FILE_A << 6,
	FILE_H = FILE_A << 7,
};

// mirror a square, useful for applying the evaluation tables for both white and black
uint8_t const normalize[2][64] =
{
	{
		A8, B8, C8, D8, E8, F8, G8, H8,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A1, B1, C1, D1, E1, F1, G1, H1,
	},
	{
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
	}
};

// this ugly looking gap from W_KING to B_PAWN can simplify some calculations
enum Piece {
	W_PAWN = 0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
	B_PAWN = 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
	NO_PIECE
};

Piece const ALL_PIECES[2][6] = { { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING },
				 { B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING } };

enum Color {
	WHITE, BLACK
};

// switches the color from WHITE to BLACK and back
static inline Color swap(Color color)
{
	return Color(!color);
}

static inline Color color_of(Piece piece)
{
	return Color(piece >> 3);
}

enum Piece_type {
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING
};

static inline Piece_type type_of(Piece piece)
{
	return Piece_type(piece & 0b111);
}

static inline Piece piece_of(Color color, Piece_type type)
{
	return Piece((color << 3) + type);
}

static inline unsigned rank_num(unsigned square)
{
	// equivalent to "square / 8".
	return square >> 3;
}

static inline unsigned file_num(unsigned square)
{
	// thanks to the 8 * 8 chess board, the last three bits denote the file.
	return square & 7;
}

static inline uint64_t rank(unsigned square)
{
	return RANK_8 << (rank_num(square) * 8);
}

static inline uint64_t file(unsigned square)
{
	return FILE_A << file_num(square);
}

// used in search and move ordering, evaluation has its own values
// Note: The weird zeros exist, because of the encoding of pieces.
// Element 15 is NO_PIECE.
int const piece_value[15] = { 100, 300, 320, 500, 900, 1000, 0, 0,
			      100, 300, 320, 500, 900, 1000, 0 };

int const non_pawn_value[15] = { 0, 300, 320, 500, 900, 0, 0, 0,
			         0, 300, 320, 500, 900, 0, 0 };

enum Score_type {
	DRAW_SCORE = 0,
	MATE_SCORE = 30000,
	INFINITY_SCORE = 31000
};

// directions are relative to whites' point of view
enum Direction {
	UP    = -8,
	DOWN  =  8,
	LEFT  = -1,
	RIGHT =  1,

	UP_LEFT    = UP + LEFT,
	UP_RIGHT   = UP + RIGHT,
	DOWN_LEFT  = DOWN + LEFT,
	DOWN_RIGHT = DOWN + RIGHT
};

// shifts every bit in a bitboard in a direction
static inline uint64_t shift(uint64_t bitboard, Direction direction)
{
	switch(direction) {
	case UP: 	 return bitboard >> 8;
	case DOWN: 	 return bitboard << 8;
	case LEFT:  	 return (bitboard & ~FILE_A) >> 1;
	case RIGHT:  	 return (bitboard & ~FILE_H) << 1;
	case UP_LEFT: 	 return (bitboard & ~FILE_A) >> 9;
	case UP_RIGHT: 	 return (bitboard & ~FILE_H) >> 7;
	case DOWN_LEFT:  return (bitboard & ~FILE_A) << 7;
	case DOWN_RIGHT: return (bitboard & ~FILE_H) << 9;
	default: return 0ULL;
	}
}

// Moves are encoded in a 16 bit integer.
// first     6 bits: from square
// next      6 bits: to square
// remaining 4 bits: move flag (Capture, Promotion, En Passant, ...)
enum Move : uint16_t {
	INVALID_MOVE
};

// generated moves are later scored based on how good they look to make the search more efficient.
struct Scored_move
{
	Move move;
	int16_t score;
};

// Chess has loads of special moves that have to be treated differently.
// These can be neatly encoded in 4 bits.
enum Move_flags : uint8_t {
	QUIET = 0b0000,
	DOUBLE_PUSH = 0b0001,
	OO = 0b0010, OOO = 0b0011,							// kingside, queenside
	CAPTURE = 0b0100, EP_CAPTURE = 0b0101,						// captures
	PR_KNIGHT = 0b1000, PR_BISHOP = 0b1001, PR_ROOK = 0b1010, PR_QUEEN = 0b1011,	// quiet promotion
	PC_KNIGHT = 0b1100, PC_BISHOP = 0b1101, PC_ROOK = 0b1110, PC_QUEEN = 0b1111	// promotion capture
};

// source square
static inline unsigned move_from(Move move)
{
	return (move >> 6) & 0x3F;
}

// target square
static inline unsigned move_to(Move move)
{
	return move & 0x3F;
}

// special flag
static inline Move_flags flags_of(Move move)
{
	return Move_flags((move >> 12) & 0xF);
}

static inline Move create_move(unsigned from, unsigned to)
{
	return Move((from << 6) + to);
}

static inline Move create_move(unsigned from, unsigned to, Move_flags flag)
{
	return Move((flag << 12) | (from << 6) | to);
}

static inline bool promotion(Move move)
{
	return (flags_of(move) & 0b1000);
}

static inline bool capture(Move move)
{
	return (flags_of(move) & 0b0100);
}

// the castling rights for both sides fit in 4 bits
enum Castling_rights : uint8_t {
	NO_RIGHTS,
	WHITE_OO  = 0b0001,
	WHITE_OOO = 0b0010,
	BLACK_OO  = 0b0100,
	BLACK_OOO = 0b1000
};

Castling_rights const king_side[2]  = { WHITE_OO,  BLACK_OO  };
Castling_rights const queen_side[2] = { WHITE_OOO, BLACK_OOO };

// there are different types of hash entries, since we do not always have exact information about positions
enum TT_flag {
	NO_FLAG,
	UPPERBOUND,
	LOWERBOUND,
	EXACT
};

static inline uint64_t random_64()
{
	uint64_t ran = rand();
	return ran << 32 | rand();
}

// bit stuff

static inline bool get_bit(uint64_t bitboard, unsigned square)
{
	return (bitboard & (1ULL << square));
}

static inline void set_bit(uint64_t &bitboard, unsigned square)
{
	bitboard |= (1ULL << square);
}

static inline void pop_bit(uint64_t &bitboard, unsigned square)
{
	bitboard &= ~(1ULL << square);
}

static inline unsigned pop_count(uint64_t bitboard)
{
	return __builtin_popcountll(bitboard);
}

static inline unsigned lsb(uint64_t bitboard)
{
	return __builtin_ctzll(bitboard);
}

static inline unsigned pop_lsb(uint64_t &bitboard)
{
	unsigned square = lsb(bitboard);
	bitboard &= (bitboard - 1);
	return square;
}

// print

static inline void print_bitboard(uint64_t bitboard)
{
	std::string str = "    +---+---+---+---+---+---+---+---+\n";

	for (unsigned rank = 0; rank < 8; rank++) { 
		str += "  " + std::to_string(8 - rank) + " ";
		for (unsigned file = 0; file < 8; file++) { 
			unsigned square = rank * 8 + file;
			str += get_bit(bitboard, square) ? "| X " : "|   ";

		}
		str += "|\n    +---+---+---+---+---+---+---+---+\n";
	}
	str += "      A   B   C   D   E   F   G   H\n\n";
	std::cerr << str;
}

std::string const square_string[65] = {
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"None"
};

static inline std::string flag_string(Move_flags flag)
{
	switch(flag) {
	case PR_KNIGHT: case PC_KNIGHT: return "n";
	case PR_BISHOP: case PC_BISHOP: return "b";
	case PR_ROOK: case PC_ROOK: return "r";
	case PR_QUEEN: case PC_QUEEN: return "q";
	default: return "";
	}
}

static inline std::string move_string(Move move)
{
	unsigned from = move_from(move);
	unsigned to   = move_to(move);
	return square_string[from] + square_string[to] + flag_string(flags_of(move));
}

static inline void print_move(Move move)
{
	std::cerr << move_string(move);
}

struct Noncopyable
{
	public:
		Noncopyable() {}
	private:
		Noncopyable(Noncopyable const &);
		Noncopyable &operator = (Noncopyable const &);
};
