#pragma once

#include "utility.h"
#include "board.h"
#include "pre_computed.h"

// The so called Static Exchange Evaluation (SEE) is used to estimate the material gain of
// move by looking at the series of exchanges on the target square.
// This can be used in move ordering of captures and for pruning decisions.

static inline bool see(Board &board, Move move, int threshold)
{
	unsigned from_square = move_from(move);
	unsigned to_square   = move_to(move);

	// Best case: We capture the opponent piece and he does not take back. If the gained
	// material is not enough to beat the threshold, stop.
	int gain = piece_value[board.board[to_square]] - threshold;
	if (gain < 0) return false;

	// Worst case: The opponent captures the piece we moved + we do not take back. If the
	// exchange still beats the threshold, stop.
	gain -= piece_value[board.board[from_square]];
	if (gain >= 0) return true;
	
	uint64_t occupied = board.occ & ~((1ULL << from_square) | (1ULL << to_square));
	uint64_t attackers = board.square_attackers(to_square, occupied);

	uint64_t bishops = board.pieces(BISHOP) | board.pieces(QUEEN);
	uint64_t rooks   = board.pieces(ROOK)   | board.pieces(QUEEN);

	Color side = swap(board.side_to_move);

	while (true) {

		attackers &= occupied;

		uint64_t side_attackers = attackers & board.pieces(side);

		// The side out of attackers loses, because if it could get away without capturing, we
		// would have stopped the sequence sooner.
		if (!side_attackers)
			break;

		side = swap(side);

		int attacker_value;
		uint64_t attacker;

		// Remove the least valuable attacker from the board.
		// Make sure to add xray pieces that might have been blocked by that attacker. For example stacked rooks.
		if ((     attacker = side_attackers & board.pieces(PAWN))) {
			attacker_value = piece_value[PAWN];
			occupied &= ~(1ULL << lsb(attacker));
			attackers |= piece_attacks(BISHOP, to_square, occupied) & bishops;
		}
		else if ((attacker = side_attackers & board.pieces(KNIGHT))) {
			attacker_value = piece_value[KNIGHT];
			occupied &= ~(1ULL << lsb(attacker));
		}
		else if ((attacker = side_attackers & board.pieces(BISHOP))) {
			attacker_value = piece_value[BISHOP];
			occupied &= ~(1ULL << lsb(attacker));
			attackers |= piece_attacks(BISHOP, to_square, occupied) & bishops;
		}
		else if ((attacker = side_attackers & board.pieces(ROOK))) {
			attacker_value = piece_value[ROOK];
			occupied &= ~(1ULL << lsb(attacker));
			attackers |= piece_attacks(ROOK,   to_square, occupied) & rooks;
		}
		else if ((attacker = side_attackers & board.pieces(QUEEN))) {
			attacker_value = piece_value[QUEEN];
			occupied &= ~(1ULL << lsb(attacker));
			attackers |= piece_attacks(BISHOP, to_square, occupied) & bishops;
			attackers |= piece_attacks(ROOK,   to_square, occupied) & rooks;
		}
		else {
			attacker = side_attackers & board.pieces(KING);
			attacker_value = piece_value[KING];
			occupied &= ~(1ULL << lsb(attacker));
		}

		// Minimax our way through the capture sequence.
		gain = -gain - 1 - attacker_value;

		// Again, once we beat the threshold, even if we would not recapture, we win.
		if (gain >= 0)
			break;
	}
	return board.side_to_move != side;
}
