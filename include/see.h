
// The so called Static Exchange Evaluation (SEE) is used to estimate the gain of
// a capture (or a check) by looking at the series of exchanges on a single square.
// This can be used in move ordering of captures, check extensions and especially
// Quiescence Search.


// extracts the piece with the least value from a bitboard
// order is pawn, knight, bishop, rook, queen, king

int const see_value[15] = { 100, 325, 325, 500, 1000, 0, 0, 0,
			    100, 325, 325, 500, 1000, 0, 0 };

struct Attacker
{
	Piece piece;
	uint64_t square;
};

uint64_t attacks_to(Board &board, unsigned square, uint64_t occupied)
{
	return 	(board.precomputed.pawn_attacks[WHITE][square] & board.pieces[B_PAWN]) |
		(board.precomputed.pawn_attacks[BLACK][square] & board.pieces[W_PAWN]) |
		(board.precomputed.attacks_bb<KNIGHT>(square, 0ULL) & (board.pieces[W_KNIGHT] | board.pieces[B_KNIGHT])) |
		(board.precomputed.attacks_bb<BISHOP>(square, occupied) &
		(board.pieces[W_BISHOP] | board.pieces[B_BISHOP] | board.pieces[W_QUEEN] | board.pieces[B_QUEEN])) |
		(board.precomputed.attacks_bb<ROOK>(square, occupied) &
		(board.pieces[W_ROOK] | board.pieces[B_ROOK] | board.pieces[W_QUEEN] | board.pieces[B_QUEEN])) |
		(board.precomputed.attacks_bb<KING>(square, 0ULL) & (board.pieces[W_KING] | board.pieces[B_KING]));
}

Attacker lowest_piece(Board &board, uint64_t attackers, Color side)
{
	for (Piece pc : ALL_PIECES[side]) {
		uint64_t mask = board.pieces[pc] & attackers;
		if (mask != 0ULL) return
		{
			.piece	  = pc,
			// abuse the two's complement, so only one bit is returned
		       	.square = mask & -mask
		};
	}
	return {};
}

/*uint64_t lowest_piece(Board &board, uint64_t attackers, Color side, Piece &p)
{
	for (Piece pc : ALL_PIECES[side]) {
		p = pc;
		uint64_t mask = board.pieces[p] & attackers;
		// abuse the two's complement, so only one bit is returned
		if (mask != 0ULL) return mask & -mask;
	}
	return 0ULL;
}*/

uint64_t revealed_attacks(Board &board, unsigned square, PieceType pt, uint64_t occupied)
{
	uint64_t bishops = board.pieces[W_BISHOP] | board.pieces[B_BISHOP];
	uint64_t rooks   = board.pieces[W_ROOK]   | board.pieces[B_ROOK];
	uint64_t queens  = board.pieces[W_QUEEN]  | board.pieces[B_QUEEN];

	if (pt == BISHOP || pt == PAWN)
		return board.precomputed.attacks_bb<BISHOP>(square, occupied) & (bishops | queens);
	else if (pt == ROOK)
		return board.precomputed.attacks_bb<ROOK>(square, occupied) & (rooks | queens);
	else if (pt == QUEEN)
		return (board.precomputed.attacks_bb<BISHOP>(square, occupied) & (bishops | queens)) |
		       (board.precomputed.attacks_bb<ROOK>(square, occupied) & (rooks | queens));
	return 0ULL;
}

int see(Board &board, Move move)
{
	unsigned target_square = move_to(move);
	uint64_t occupied = board.occ;
	Color side = board.side_to_move;

	int gain[32];
	unsigned depth = 0;

	// the first exchange is obviously the move
	Attacker attacker { .piece = board.board[move_from(move)], .square = 1ULL << move_from(move) };
	gain[0] = see_value[board.board[target_square]];
	//std::cerr << "depth " << depth << " gain " << gain[depth] << "\n";

	uint64_t attackers = attacks_to(board, target_square, occupied);

	// loop through the exchange sequence and see, who wins it
	while (true) {
		depth++;
		// execute the exchange with the next least valuable piece
		side = Color(!side);
		attackers &= ~attacker.square;
		occupied &= ~attacker.square;
		attackers |= revealed_attacks(board, target_square, type_of(attacker.piece), occupied) & occupied;

		// no attackers left, we are done here
		if (!(attackers & board.color[side])) break;

		// ouch, the piece is defended, continue
		gain[depth] = see_value[attacker.piece] - gain[depth - 1];
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
