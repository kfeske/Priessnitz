#pragma once

#include "utility.h"

struct Zobrist
{
	uint64_t piece_side_key = 0; // here, we keep track of the piece positions and side to move
	uint64_t key = 0; // final zobrist key  (we add the castling rights and en passant file, only if needed)

	uint64_t piece_rand[16][64] {};
	uint64_t ep_rand[8] {};
	uint64_t castling_rand[16] {};
	uint64_t side_rand = 0;

	void populate();

	Zobrist();
};

// used in unmake_move()
// stores essential position data that cannot be restored
struct UndoInfo {
	Piece captured = NO_PIECE;
	uint8_t ep_sq = NO_SQUARE; // save en passant square in case the last move was a double pawn push
	uint8_t castling_rights = NO_RIGHTS;
	unsigned rule_50 = 0;
};

struct Board_state
{
	Piece board[64] {};
	uint64_t pieces[15] {};

	uint64_t color[2] {};
	uint64_t occ = 0ULL;

	Color side_to_move = WHITE;

	unsigned game_ply = 0;

	UndoInfo history[1024] {};
	uint64_t position_history[1024] {};
	bool repetition = false;

	int non_pawn_material[2] {};
};

struct Board : Board_state
{
	Zobrist zobrist {};

	uint64_t enemy(Color friendly);
	uint64_t enemy_or_empty(Color friendly);

	void reset();

	void add_piece(unsigned square, Piece p);

	void remove_piece(unsigned square);

	void push_piece_quiet(unsigned from, unsigned to);

	void make_move(Move move);

	void unmake_move(Move move);

	unsigned make_null_move();

	void unmake_null_move(unsigned ep);

	bool in_check();

	uint64_t all_pawn_attacks(Color friendly);

	bool passed_push(Move move);

	bool immediate_draw(unsigned ply_from_root);

	void set_fenpos(std::string fen);

	void set_startpos();

	Board();
};

std::string piece_string(Board &board, unsigned square);

void print_board(Board &board);
