#include <utility>
#include <vector>

template <MoveFlags mf>
void append_to_movelist(std::vector<Move> &movelist, unsigned from, Uint64 attacks)
{
	while(attacks) movelist.emplace_back(create_move(from, pop_lsb(attacks), mf));
}

struct MoveGenerator : Noncopyable
{
	std::vector<Move> movelist = {};

	void generate_pawn_moves(Board &board, Color col, Uint64 targets, Uint64 quiets, Uint64 pinned, unsigned ksq, Uint64 sliders)
	{
		Direction up = (col == WHITE) ? NORTH : SOUTH;
		Direction up_right = (col == WHITE) ? NORTH_EAST : SOUTH_EAST;
		Direction up_left  = (col == WHITE) ? NORTH_WEST : SOUTH_WEST;
		Uint64 double_push_rank = (col == WHITE) ? board.precomputed.rank_6 : board.precomputed.rank_3;

		// pawns on the board (unpinned and pinned)
		Uint64 pawns = board.pieces[piece_of(col, PAWN)] & ~pinned;
		pawns &= (col == WHITE) ? ~board.precomputed.rank_2 : ~board.precomputed.rank_7;
		Uint64 pinned_pawns = board.pieces[piece_of(col, PAWN)] & pinned;

		Uint64 single_pushes = shift(pawns, up) & ~board.occ;
		Uint64 double_pushes = shift(single_pushes & double_push_rank, up) & quiets;
		single_pushes &= quiets;

		while (single_pushes) {
			unsigned square = pop_lsb(single_pushes);
			movelist.emplace_back(create_move(square - up, square, QUIET));
		}
		while (double_pushes) {
			unsigned square = pop_lsb(double_pushes);
			movelist.emplace_back(create_move(square - up - up, square, DOUBLE_PUSH));
		}

		Uint64 right_attacks = shift(pawns & ~board.precomputed.file_H, up_right) & targets;
		Uint64 left_attacks  = shift(pawns & ~board.precomputed.file_A, up_left ) & targets;

		while (right_attacks) {
			unsigned square = pop_lsb(right_attacks);
			movelist.emplace_back(create_move(square - up_right, square, CAPTURE));
		}
		while (left_attacks) {
			unsigned square = pop_lsb(left_attacks);
			movelist.emplace_back(create_move(square - up_left,  square, CAPTURE));
		}


		unsigned ep_square = board.history[board.game_ply].ep_sq;
		if (ep_square != SQ_NONE) {

			// non pinned en passants

			Uint64 ep_candidates = pawns & board.precomputed.pawn_attacks[!col][ep_square];
			while (ep_candidates) {
				unsigned attacker_square = pop_lsb(ep_candidates);

				// if an en passant cannot stop a check, it is illegal
				if (!((1ULL << ep_square) & quiets) && !((1ULL << (ep_square - up)) & targets)) continue;

				// avoid a horizontal check after an en passant, which can happen to unpinned pawns
				if ((board.precomputed.attacks_bb<ROOK>(ksq, board.occ & ~(1ULL << attacker_square)
				    & ~(1ULL << (ep_square - up))) & board.precomputed.line_bb[ksq][attacker_square]
				    & sliders) == 0)
					movelist.emplace_back(create_move(attacker_square, ep_square, EP_CAPTURE));
			}

			// pinned en passant


			// avoid diagonal checks after en passant, which can only happen to pinned pawns
			ep_candidates = pinned_pawns & board.precomputed.pawn_attacks[!col][ep_square] & board.precomputed.line_bb[ksq][ep_square];
			while (ep_candidates) {
				unsigned attacker_square = pop_lsb(ep_candidates);

				// if an en passant cannot stop a check, it is illegal
				if (((1ULL << ep_square) & quiets) || ((1ULL << (ep_square - up)) & targets))
					movelist.emplace_back(create_move(attacker_square, ep_square, EP_CAPTURE));
			}
		}

		// non pinned promotions

		pawns = board.pieces[piece_of(col, PAWN)] & ~pinned;
		pawns &= (col == WHITE) ? board.precomputed.rank_2 : board.precomputed.rank_7;
		if (pawns) {

			// quiet promotion
			single_pushes = shift(pawns, up) & quiets;
			while(single_pushes) {
				unsigned square = pop_lsb(single_pushes);
				movelist.emplace_back(create_move(square - up, square, PR_KNIGHT));
				movelist.emplace_back(create_move(square - up, square, PR_BISHOP));
				movelist.emplace_back(create_move(square - up, square, PR_ROOK));
				movelist.emplace_back(create_move(square - up, square, PR_QUEEN));
			}

			// promotion captures
			right_attacks = shift(pawns & ~board.precomputed.file_H, up_right) & targets;
			left_attacks  = shift(pawns & ~board.precomputed.file_A, up_left ) & targets;

			while(right_attacks) {
				unsigned square = pop_lsb(right_attacks);
				movelist.emplace_back(create_move(square - up_right, square, PC_KNIGHT));
				movelist.emplace_back(create_move(square - up_right, square, PC_BISHOP));
				movelist.emplace_back(create_move(square - up_right, square, PC_ROOK));
				movelist.emplace_back(create_move(square - up_right, square, PC_QUEEN));
			}

			while(left_attacks) {
				unsigned square = pop_lsb(left_attacks);
				movelist.emplace_back(create_move(square - up_left, square, PC_KNIGHT));
				movelist.emplace_back(create_move(square - up_left, square, PC_BISHOP));
				movelist.emplace_back(create_move(square - up_left, square, PC_ROOK));
				movelist.emplace_back(create_move(square - up_left, square, PC_QUEEN));
			}
		}

		// for each pinned pawn

		Uint64 candidates = pinned_pawns;

		while (candidates) {
			unsigned square = pop_lsb(candidates);
			unsigned promotion_rank = (col == WHITE) ? 1: 6;
			Uint64 attacks = board.precomputed.pawn_attacks[col][square] & targets & board.precomputed.line_bb[ksq][square];

			// pinned promotion
			if (rank(square) == promotion_rank) {
				append_to_movelist<PC_KNIGHT>(movelist, square, attacks);
				append_to_movelist<PC_BISHOP>(movelist, square, attacks);
				append_to_movelist<PC_ROOK>(movelist, square, attacks);
				append_to_movelist<PC_QUEEN>(movelist, square, attacks);
			}
			else {
				append_to_movelist<CAPTURE>(movelist, square, attacks);

				Uint64 single_push = shift(1ULL << square, up) & ~board.occ & board.precomputed.line_bb[ksq][square];
				Uint64 double_push = shift(single_push & double_push_rank, up) & quiets & board.precomputed.line_bb[ksq][square];
				single_push &= quiets;

				append_to_movelist<QUIET>(movelist, square, single_push);
				append_to_movelist<DOUBLE_PUSH>(movelist, square, double_push);

			}
		}
	}


	template <PieceType p>
	void generate_moves(Board &board, Color col, Uint64 targets, Uint64 quiets, Uint64 pinned, unsigned ksq)
	{
		Uint64 pieces = board.pieces[piece_of(col, p)];
		Uint64 attacks;

		while(pieces) {
			unsigned from = pop_lsb(pieces);
			attacks = board.precomputed.attacks_bb<p>(from, board.occ) & quiets;
			if ((1ULL << from) & pinned) attacks &= board.precomputed.line_bb[ksq][from];

			while(attacks) {
				unsigned to = pop_lsb(attacks);
				movelist.emplace_back(create_move(from, to, QUIET));
			}

			attacks = board.precomputed.attacks_bb<p>(from, board.occ) & targets;
			if ((1ULL << from) & pinned) attacks &= board.precomputed.line_bb[ksq][from];

			while(attacks) {
				unsigned to = pop_lsb(attacks);
				movelist.emplace_back(create_move(from, to, CAPTURE));
			}
		}
	}

	void generate_all_moves(Board &board, bool quiescence)
	{
		//movelist = {};
		Color friendly = Color(board.side_to_move);
		
		Color enemy = Color(!friendly);
		Uint64 us_bb = board.color[friendly];
		Uint64 them_bb = board.color[enemy];

		// king cannot move to danger squares

		unsigned ksq = lsb(board.pieces[piece_of(friendly, KING)]);
		Uint64 pawns = board.pieces[piece_of(enemy, PAWN)];
		Uint64 enemy_diag_sliders = board.pieces[piece_of(enemy, BISHOP)] | board.pieces[piece_of(enemy, QUEEN)];
		Uint64 enemy_orth_sliders = board.pieces[piece_of(enemy, ROOK)]   | board.pieces[piece_of(enemy, QUEEN)];
		Direction up_right = (enemy == WHITE) ? NORTH_EAST : SOUTH_EAST;
		Direction up_left  = (enemy == WHITE) ? NORTH_WEST : SOUTH_WEST;

		Uint64 danger = 0ULL;

		danger |= shift(pawns & ~board.precomputed.file_H, up_right);
		danger |= shift(pawns & ~board.precomputed.file_A, up_left );

		danger |= board.precomputed.attacks_bb<KING>(lsb(board.pieces[piece_of(enemy, KING)]), 0ULL);

		Uint64 knights = board.pieces[piece_of(enemy, KNIGHT)];
		while(knights) danger |= board.precomputed.attacks_bb<KNIGHT>(pop_lsb(knights), 0ULL);

		Uint64 bishops_queens = enemy_diag_sliders;
		while(bishops_queens) danger |= board.precomputed.attacks_bb<BISHOP>(pop_lsb(bishops_queens), board.occ & ~(1ULL << ksq));

		Uint64 rook_queens = enemy_orth_sliders;
		while(rook_queens) danger |= board.precomputed.attacks_bb<ROOK>(pop_lsb(rook_queens), board.occ & ~(1ULL << ksq));


		Uint64 attacks = board.precomputed.attacks_bb<KING>(ksq, 0ULL) & ~(us_bb | danger);
		if ((quiescence && danger & (1ULL << ksq)) || !quiescence)
			append_to_movelist<QUIET>(movelist, ksq, attacks & ~them_bb);
		append_to_movelist<CAPTURE>(movelist, ksq, attacks & them_bb);

		// leaper (knight, pawn) pieces, delivering check can be captured

		Uint64 checkers = 0ULL;

		checkers |= board.precomputed.pawn_attacks[friendly][ksq] & board.pieces[piece_of(enemy, PAWN)];
		checkers |= board.precomputed.attacks_bb<KNIGHT>(ksq, 0ULL) & board.pieces[piece_of(enemy, KNIGHT)];

		// sliding pieces, delivering check can either be captured or blocked
		// or pin your pieces >:(

		Uint64 sliders = (board.precomputed.attacks_bb<BISHOP>(ksq, them_bb) & enemy_diag_sliders) |
				 (board.precomputed.attacks_bb<ROOK  >(ksq, them_bb) & enemy_orth_sliders);

		Uint64 pinned = 0ULL;

		while (sliders) {
			unsigned square = pop_lsb(sliders);
			Uint64 pin_ray = board.precomputed.between_bb[ksq][square] & us_bb;

			// no pieces block the ray - check detected
			if (pin_ray == 0) checkers |= 1ULL << square;

			// one of our pieces blocks the ray - register pin
			else if ((pin_ray & (pin_ray - 1)) == 0) pinned |= pin_ray;
		}

		Uint64 targets = 0ULL;
		Uint64 quiets = 0ULL;

		switch(pop_count(checkers)) {
		case 2: return;

		case 1:
			{
			targets = checkers;
			unsigned checker_square = lsb(checkers);
			Piece p = board.board[checker_square];
			if (p == piece_of(enemy, BISHOP) || p == piece_of(enemy, ROOK) || p == piece_of(enemy, QUEEN))
				quiets = board.precomputed.between_bb[ksq][checker_square] & ~checkers;
			}
			break;
		default:
			targets = them_bb;
			if (!quiescence)
				quiets  = ~board.occ;

			// castling

			// kingside

			if (!quiescence) {
				if (friendly == WHITE) {
					// kingside
					if ((board.history[board.game_ply].castling_rights & 0b0001) &&
					    !((danger | board.occ) & board.precomputed.oo_blockers[friendly]))
						movelist.emplace_back(create_move(ksq, ksq + 2, OO));

					// queenside
					if ((board.history[board.game_ply].castling_rights & 0b0010) &&
					    !(((danger & ~board.precomputed.oooo) | board.occ) & board.precomputed.ooo_blockers[friendly]))
						movelist.emplace_back(create_move(ksq, ksq - 2, OOO));
				}
				else {
					// kingside
					if ((board.history[board.game_ply].castling_rights & 0b0100) &&
					    !((danger | board.occ) & board.precomputed.oo_blockers[friendly]))
						movelist.emplace_back(create_move(ksq, ksq + 2, OO));

					// queenside
					if ((board.history[board.game_ply].castling_rights & 0b1000) &&
					    !(((danger & ~board.precomputed.oooo) | board.occ) & board.precomputed.ooo_blockers[friendly]))
						movelist.emplace_back(create_move(ksq, ksq - 2, OOO));
				}
			}
		}


		generate_pawn_moves(board, friendly, targets, quiets, pinned, ksq, enemy_orth_sliders);
		generate_moves<KNIGHT>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<BISHOP>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<ROOK  >(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<QUEEN >(board, friendly, targets, quiets, pinned, ksq);

	}
	MoveGenerator()
	{
		movelist.reserve(60);
	}
};
