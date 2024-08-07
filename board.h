#pragma once

#include "utility.h"
#include "pre_computed.h"

struct Zobrist
{
	uint64_t piece_side_key = 0; // here, we keep track of the piece positions and side to move
	uint64_t key = 0; // final zobrist key  (we add the castling rights and en passant file, only if needed)

	uint64_t piece_rand[16][64] {};
	uint64_t ep_rand[8] {};
	uint64_t castling_rand[16] {};
	uint64_t side_rand = 0;

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
	uint64_t pieces[15] {};

	uint64_t color[2] {};
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

	uint64_t enemy(Color friendly) { return color[swap(friendly)]; }
	uint64_t enemy_or_empty(Color friendly) { return ~color[friendly]; }

	unsigned square(Color friendly, Piece_type type)
	{
		return lsb(pieces[piece_of(friendly, type)]);
	}

	// resets information neccessary to start a new game
	void reset()
	{
		static_cast<Board_state&>(*this) = {};
	}


	void add_piece(unsigned square, Piece p)
	{
		Color c = color_of(p);
	
		set_bit(pieces[p], square);
		set_bit(color[c], square);
		occ = color[WHITE] | color[BLACK];
		board[square] = p;
		non_pawn_material[c] += non_pawn_value[p];
		zobrist.piece_side_key ^= zobrist.piece_rand[p][square];
	}
	
	void remove_piece(unsigned square)
	{
		Piece p = board[square];
		Color c = color_of(p);
	
		pop_bit(pieces[p], square);
		pop_bit(color[c], square);
		occ = color[WHITE] | color[BLACK];
		board[square] = NO_PIECE;
		non_pawn_material[c] -= non_pawn_value[p];
		zobrist.piece_side_key ^= zobrist.piece_rand[p][square];
	}
	
	void push_piece_quiet(unsigned from, unsigned to)
	{
		Piece p = board[from];
		Color c = color_of(p);
	
		uint64_t mask = 1ULL << from | 1ULL << to;
		pieces[p] ^= mask;
		color[c] ^= mask;
		occ = color[WHITE] | color[BLACK];
	
		board[to]   = p;
		board[from] = NO_PIECE;
		zobrist.piece_side_key ^= zobrist.piece_rand[p][from];
		zobrist.piece_side_key ^= zobrist.piece_rand[p][to];
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
		return 	(pawn_attacks(WHITE, square) & pieces[B_PAWN]) |
			(pawn_attacks(BLACK, square) & pieces[W_PAWN]) |
			(piece_attacks(KNIGHT, square, 0ULL) & (pieces[W_KNIGHT] | pieces[B_KNIGHT])) |
			(piece_attacks(BISHOP, square, occupied) &
			(pieces[W_BISHOP] | pieces[B_BISHOP] | pieces[W_QUEEN] | pieces[B_QUEEN])) |
			(piece_attacks(ROOK, square, occupied) &
			(pieces[W_ROOK] | pieces[B_ROOK] | pieces[W_QUEEN] | pieces[B_QUEEN])) |
			(piece_attacks(KING, square, 0ULL) & (pieces[W_KING] | pieces[B_KING]));
	}


	bool legal(Move move);

	bool pseudo_legal(Move move);

	uint64_t all_pawn_attacks(Color friendly);

	bool insufficient_material();

	bool immediate_draw(unsigned ply)
	{
		return ply > 1 && (repetition || history[game_ply].rule_50 >= 100 || insufficient_material());
	}

	void set_fenpos(std::string fen);

	void set_startpos();

	Board();
};

std::string piece_string(Board &board, unsigned square);

void print_board(Board &board);
