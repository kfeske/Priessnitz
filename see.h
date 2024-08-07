#pragma once

#include "utility.h"
#include "board.h"
#include "pre_computed.h"

// The so called Static Exchange Evaluation (SEE) is used to estimate the gain of
// a capture (or a check) by looking at the series of exchanges on a single square.
// This can be used in move ordering of captures, check extensions and especially
// Quiescence Search.


// extracts the piece with the least value from a bitboard
// order is pawn, knight, bishop, rook, queen, king

struct Attacker
{
	Piece_type type;
	uint64_t square;
};

static inline Attacker lowest_piece(Board &board, uint64_t attackers, Color side)
{
	for (Piece_type type : { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING }) {
		uint64_t mask = board.pieces(side, type) & attackers;
		if (mask != 0ULL) return
		{
			.type	  = type,
			// abuse the two's complement, so only one bit is returned
		       	.square = mask & -mask
		};
	}
	return {};
}

static inline uint64_t revealed_attacks(Board &board, unsigned square, Piece_type pt, uint64_t occupied)
{
	uint64_t bishops = board.pieces(BISHOP);
	uint64_t rooks   = board.pieces(ROOK);
	uint64_t queens  = board.pieces(QUEEN);

	if (pt == BISHOP || pt == PAWN)
		return piece_attacks(BISHOP, square, occupied) & (bishops | queens);
	else if (pt == ROOK)
		return piece_attacks(ROOK, square, occupied) & (rooks | queens);
	else if (pt == QUEEN)
		return (piece_attacks(BISHOP, square, occupied) & (bishops | queens)) |
		       (piece_attacks(ROOK,   square, occupied) & (rooks | queens));
	return 0ULL;
}

static inline int see(Board &board, Move move)
{
	unsigned target_square = move_to(move);
	uint64_t occupied = board.occ;
	Color side = board.side_to_move;

	int gain[32];
	unsigned depth = 0;

	// the first exchange is obviously the move
	Attacker attacker { .type = type_of(board.board[move_from(move)]), .square = 1ULL << move_from(move) };
	gain[0] = piece_value[board.board[target_square]];
	//std::cerr << "depth " << depth << " gain " << gain[depth] << "\n";

	uint64_t attackers = board.square_attackers(target_square, occupied);

	// loop through the exchange sequence and see, who wins it
	while (true) {
		depth++;
		// execute the exchange with the next least valuable piece
		side = Color(!side);
		attackers &= ~attacker.square;
		occupied &= ~attacker.square;
		attackers |= revealed_attacks(board, target_square, attacker.type, occupied) & occupied;

		// no attackers left, we are done here
		if (!(attackers & board.pieces(side))) break;

		// ouch, the piece is defended, continue
		gain[depth] = piece_value[attacker.type] - gain[depth - 1];
		//std::cerr << "depth " << depth << " gain " << gain[depth] << "\n";

		// if the opponent were to stop the exchange and you still lose it,
		// there is no need to examine the sequence further
		if (std::max(-gain[depth - 1], gain[depth]) < 0) break;

		attacker = lowest_piece(board, attackers, side);
	}
	// We are not done yet, the sequence is not forced, a side could exit the exchange at any time.
	// The following lines evaluate the final material gain, if both sides always had the option to continue or to stop taking
	// by going backwards through the sequence (similar to minimax)
	for (unsigned d = depth - 1; d > 0; d--) {
		gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
		//std::cerr << "depth " << d - 1 << " gain " << gain[d - 1] << "\n";
	}
	return gain[0];
}
