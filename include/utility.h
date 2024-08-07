#pragma once

enum Square {
	A8, B8, C8, D8, E8, F8, G8, H8,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A1, B1, C1, D1, E1, F1, G1, H1,
	SQ_NONE
};

const std::string square_string[65] = {
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

enum Color {
	WHITE, BLACK
};

enum State {
	ONGOING, W_CHECKMATE, B_CHECKMATE, STALEMATE, MOVE_50, REPETITION
};

enum Piece {
	W_PAWN = 0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
	B_PAWN = 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
	NO_PIECE
};

Piece piece_of(unsigned c, unsigned p)
{
	return Piece((c << 3) + p);
}

Color color_of(Piece p)
{
	return Color(p >> 3);
}

enum PieceType {
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING
};

PieceType type_of(Piece p)
{
	return PieceType(p & 0b111);
}

enum Phase {
	MIDGAME, ENDGAME
};

enum Value : int {
	PAWN_MG   = 126,   PAWN_EG   = 208,
	KNIGHT_MG = 781,   KNIGHT_EG = 854,
	BISHOP_MG = 825,   BISHOP_EG = 915,
	ROOK_MG   = 1276,  ROOK_EG   = 1380,
	QUEEN_MG  = 2538,  QUEEN_EG  = 2682
};

Value get_piece_value(Piece pc, Phase ph)
{
	switch(pc) {
	case W_PAWN:   return (ph == MIDGAME) ? Value( PAWN_MG)   : Value( PAWN_EG);
	case B_PAWN:   return (ph == MIDGAME) ? Value(-PAWN_MG)   : Value(-PAWN_EG);
	case W_KNIGHT: return (ph == MIDGAME) ? Value( KNIGHT_MG) : Value( KNIGHT_EG);
	case B_KNIGHT: return (ph == MIDGAME) ? Value(-KNIGHT_MG) : Value(-KNIGHT_EG);
	case W_BISHOP: return (ph == MIDGAME) ? Value( BISHOP_MG) : Value( BISHOP_EG);
	case B_BISHOP: return (ph == MIDGAME) ? Value(-BISHOP_MG) : Value(-BISHOP_EG);
	case W_ROOK:   return (ph == MIDGAME) ? Value( ROOK_MG)   : Value( ROOK_EG);
	case B_ROOK:   return (ph == MIDGAME) ? Value(-ROOK_MG)   : Value(-ROOK_EG);
	case W_QUEEN:  return (ph == MIDGAME) ? Value( QUEEN_MG)  : Value( QUEEN_EG);
	case B_QUEEN:  return (ph == MIDGAME) ? Value(-QUEEN_MG)  : Value(-QUEEN_EG);
	default: return Value(0);
	}
}

Value non_pawn_value(Piece pc)
{
	switch(type_of(pc)) {
	case KNIGHT: return KNIGHT_MG;
	case BISHOP: return BISHOP_MG;
	case ROOK:   return ROOK_MG;
	case QUEEN:  return QUEEN_MG;
	default: return Value(0);
	}
}

enum Direction {
	NORTH = -8,
	SOUTH = -NORTH,
	EAST = 1,
	WEST = -1,

	NORTH_EAST = NORTH + EAST,
	NORTH_WEST = NORTH + WEST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST
};

uint64_t shift(uint64_t b, Direction d)
{
	switch(d) {
	case NORTH: return b >> 8;
	case SOUTH: return b << 8;
	case EAST:  return b << 1;
	case WEST:  return b >> 1;
	case NORTH_EAST: return b >> 7;
	case NORTH_WEST: return b >> 9;
	case SOUTH_EAST: return b << 9;
	case SOUTH_WEST: return b << 7;
	default: return 0ULL;
	}
}

enum Move : uint16_t {
	INVALID_MOVE
};

struct Scored_move
{
	Move move;
	int16_t score;

	Scored_move(Move move)
	:
		move(move), score(0)
	{}
};

enum MoveFlags {
	QUIET = 0b0000,
	DOUBLE_PUSH = 0b0001,
	OO = 0b0010, OOO = 0b0011,							// kingside, queenside
	CAPTURE = 0b0100, EP_CAPTURE = 0b0101,						// captures
	PR_KNIGHT = 0b1000, PR_BISHOP = 0b1001, PR_ROOK = 0b1010, PR_QUEEN = 0b1011,	// quiet promotion
	PC_KNIGHT = 0b1100, PC_BISHOP = 0b1101, PC_ROOK = 0b1110, PC_QUEEN = 0b1111	// promotion capture
};

unsigned move_from(Move move)
{
	return (move >> 6) & 0x3F;
}

unsigned move_to(Move move)
{
	return move & 0x3F;
}

MoveFlags flags_of(Move move)
{
	return MoveFlags((move >> 12) & 0xF);
}

Move create_move(unsigned from, unsigned to)
{
	return Move((from << 6) + to);
}

Move create_move(unsigned from, unsigned to, MoveFlags mf)
{
	return Move((mf << 12) | (from << 6) | to);
}

template<MoveFlags mf>
Scored_move new_move(unsigned from, unsigned to)
{
	return Scored_move { create_move(from, to, mf) };
}

unsigned rank(unsigned square)
{
	return square >> 3;
}

unsigned file(unsigned square)
{
	return square & 7;
}

unsigned mirrored(unsigned square)
{
	unsigned y = 7 - rank(square);
	unsigned x = file(square);
	return y * 8 + x;
}

bool is_edge(unsigned square, Direction d)
{
	switch(d) {
	case NORTH: return (rank(square) == 0);
	case SOUTH: return (rank(square) == 7);
	case EAST:  return (file(square) == 7);
	case WEST:  return (file(square) == 0);
	case NORTH_EAST: return (rank(square) == 0 || file(square) == 7);
	case SOUTH_EAST: return (rank(square) == 7 || file(square) == 7);
	case SOUTH_WEST: return (rank(square) == 7 || file(square) == 0);
	case NORTH_WEST: return (rank(square) == 0 || file(square) == 0);
	default: return false;
	}
}

enum CastlingRights {
	NO_RIGHTS,
	WHITE_OO  = 0b0001,
	WHITE_OOO = 0b0010,
	BLACK_OO  = 0b0100,
	BLACK_OOO = 0b1000
};

enum TTEntryFlag {
	NO_FLAG,
	UPPERBOUND,
	LOWERBOUND,
	EXACT
};

uint64_t random_64()
{
	uint64_t ran = rand();
	return ran << 32 | rand();
}

struct Magic
{
	uint64_t magic;
	unsigned shift;
};

// bit stuff

bool get_bit(uint64_t b, unsigned square)
{
	return (b & (1ULL << square));
}

void set_bit(uint64_t &b, unsigned square)
{
	b |= (1ULL << square);
}

void pop_bit(uint64_t &b, unsigned square)
{
	b &= ~(1ULL << square);
}

unsigned pop_count(uint64_t b)
{
	return __builtin_popcountll(b);
}

unsigned lsb(uint64_t b)
{
	return __builtin_ctzll(b);
}

unsigned pop_lsb(uint64_t &b)
{
	unsigned square = lsb(b);
	pop_bit(b, square);
	return square;
}

// print

void print_bitboard(uint64_t b)
{
	std::string str = "    +---+---+---+---+---+---+---+---+\n";

	for (unsigned rank = 0; rank < 8; rank++) { 
		str += "  " + std::to_string(8 - rank) + " ";
		for (unsigned file = 0; file < 8; file++) { 
			unsigned square = rank * 8 + file;
			str += get_bit(b, square) ? "| X " : "|   ";

		}
		str += "|\n    +---+---+---+---+---+---+---+---+\n";
	}
	str += "      A   B   C   D   E   F   G   H\n\n";
	std::cerr << str;
}

std::string flag_string(MoveFlags flag)
{
	switch(flag) {
	case PR_KNIGHT: case PC_KNIGHT: return "n";
	case PR_BISHOP: case PC_BISHOP: return "b";
	case PR_ROOK: case PC_ROOK: return "r";
	case PR_QUEEN: case PC_QUEEN: return "q";
	default: return "";
	}
}

std::string move_string(Move move)
{
	unsigned from = move_from(move);
	unsigned to   = move_to(move);
	return square_string[from] + square_string[to] + flag_string(flags_of(move));
}

void print_move(Move move)
{
	std::cerr << move_string(move);
}
