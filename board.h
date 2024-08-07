#pragma once

#include "utility.h"
#include "pre_computed.h"

struct Zobrist
{
	uint64_t piece_side_key = 0ULL; // here, we keep track of the piece positions and side to move
	uint64_t key = 0ULL; // final zobrist key  (we add the castling rights and en passant file, only if needed)
	uint64_t pawn_key = 0ULL; // Key for the pawn hash table

	uint64_t piece_rand[16][64] {};
	uint64_t ep_rand[8] {};
	uint64_t castling_rand[16] {};
	uint64_t side_rand = 0ULL;

	void populate();

	Zobrist()
	{
		populate();
	}
};

// used in unmake_move()
// stores essential position data that cannot be restored
struct Undo_info {
	Piece captured = NO_PIECE;
	uint8_t ep_sq = NO_SQUARE; // save en passant square in case the last move was a double pawn push
	uint8_t castling_rights = NO_RIGHTS;
	unsigned rule_50 = 0;
	uint64_t checkers = 0ULL;
	uint64_t pinned = 0ULL;
	Move move;
};

struct Board_state
{
	Piece board[64] {};
	uint64_t pieces_by_type[6] {};

	uint64_t pieces_by_color[2] {};
	uint64_t occ = 0ULL;

	Color side_to_move = WHITE;

	unsigned game_ply = 0;

	Undo_info history[1024] {};
	uint64_t position_history[1024] {};
	bool repetition = false;

	int non_pawn_material[2] {};
};

struct Board : Board_state
{
	Zobrist zobrist {};

	unsigned square(Color friendly, Piece_type type)
	{
		return lsb(pieces(friendly, type));
	}

	// resets information neccessary to start a new game
	void reset()
	{
		static_cast<Board_state&>(*this) = {};
		for (unsigned square = 0; square < 64; square++)
			board[square] = NO_PIECE;
	}

	uint64_t pieces(Color friendly, Piece_type type)
	{
		return pieces_by_color[friendly] & pieces_by_type[type];
	}

	uint64_t pieces(Piece_type type)
	{
		return pieces_by_type[type];
	}

	uint64_t pieces(Color friendly)
	{
		return pieces_by_color[friendly];
	}

	void add_piece(unsigned square, Piece piece)
	{
		Color friendly = color_of(piece);
	
		set_bit(pieces_by_type[type_of(piece)], square);
		set_bit(pieces_by_color[friendly], square);
		occ = pieces_by_color[WHITE] | pieces_by_color[BLACK];
		board[square] = piece;
		non_pawn_material[friendly] += non_pawn_value[piece];
		zobrist.piece_side_key ^= zobrist.piece_rand[piece][square];
		if (type_of(piece) == PAWN) zobrist.pawn_key ^= zobrist.piece_rand[piece][square];
	}
	
	void remove_piece(unsigned square)
	{
		Piece piece = board[square];
		Color friendly = color_of(piece);
	
		pop_bit(pieces_by_type[type_of(piece)], square);
		pop_bit(pieces_by_color[friendly], square);
		occ = pieces_by_color[WHITE] | pieces_by_color[BLACK];
		board[square] = NO_PIECE;
		non_pawn_material[friendly] -= non_pawn_value[piece];
		zobrist.piece_side_key ^= zobrist.piece_rand[piece][square];
		if (type_of(piece) == PAWN) zobrist.pawn_key ^= zobrist.piece_rand[piece][square];
	}
	
	void push_piece_quiet(unsigned from, unsigned to)
	{
		Piece piece = board[from];
	
		uint64_t mask = 1ULL << from | 1ULL << to;
		pieces_by_type[type_of(piece)] ^= mask;
		pieces_by_color[color_of(piece)] ^= mask;
		occ = pieces_by_color[WHITE] | pieces_by_color[BLACK];
	
		board[to]   = piece;
		board[from] = NO_PIECE;
		zobrist.piece_side_key ^= zobrist.piece_rand[piece][from];
		zobrist.piece_side_key ^= zobrist.piece_rand[piece][to];
		if (type_of(piece) == PAWN) {
			zobrist.pawn_key ^= zobrist.piece_rand[piece][from];
			zobrist.pawn_key ^= zobrist.piece_rand[piece][to];
		}
	}

	void make_move(Move move);

	void unmake_move(Move move);

	unsigned make_null_move();

	void unmake_null_move(unsigned ep);

	bool in_check()
	{
		return (history[game_ply].checkers != 0ULL);
	}

	void update_checkers_and_pinners();

	uint64_t square_attackers(unsigned square, uint64_t occupied)
	{
		return 	(pawn_attacks(WHITE, square) & pieces(BLACK, PAWN)) |
			(pawn_attacks(BLACK, square) & pieces(WHITE, PAWN)) |
			(piece_attacks(KNIGHT, square, 0ULL) & pieces(KNIGHT)) |
			(piece_attacks(BISHOP, square, occupied) & (pieces(BISHOP) | pieces(QUEEN))) |
			(piece_attacks(ROOK, square, occupied)   & (pieces(ROOK)   | pieces(QUEEN))) |
			(piece_attacks(KING, square, 0ULL) & pieces(KING));
	}

	bool legal(Move move);

	bool pseudo_legal(Move move);

	uint64_t all_pawn_attacks(Color friendly);

	bool insufficient_material();

	bool immediate_draw(unsigned ply)
	{
		return history[game_ply].rule_50 >= 100 || (ply > 1 && (repetition || insufficient_material()));
	}

	void set_fenpos(std::string fen);

	void set_startpos();

	Board();
};

std::string piece_string(Board &board, unsigned square);

void print_board(Board &board);
