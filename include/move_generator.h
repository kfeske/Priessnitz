#include <utility>
#include <vector>

struct Move_generator : Noncopyable
{
	uint8_t size = 0;
	Scored_move move_list[218]; // 218 is the maximum number of moves for a color in a legal position

	template <Move_flags mf>
	void append_attacks(unsigned from, uint64_t attacks)
	{
		while (attacks) move_list[size++].move = create_move<mf>(from, pop_lsb(attacks));
	}

	void generate_pawn_moves(Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq, uint64_t sliders)
	{
		Direction up = (col == WHITE) ? UP : DOWN;
		Direction up_right = (col == WHITE) ? UP_RIGHT : DOWN_RIGHT;
		Direction up_left  = (col == WHITE) ? UP_LEFT : DOWN_LEFT;
		uint64_t double_push_rank = (col == WHITE) ? RANK_6 : RANK_3;

		// pawns on the board (unpinned and pinned)
		uint64_t pawns = board.pieces[piece_of(col, PAWN)] & ~pinned;
		// special case if pawn is about to promote
		pawns &= (col == WHITE) ? ~RANK_2 : ~RANK_7;
		uint64_t pinned_pawns = board.pieces[piece_of(col, PAWN)] & pinned;

		uint64_t single_pushes = shift(pawns, up) & ~board.occ;
		uint64_t double_pushes = shift(single_pushes & double_push_rank, up) & quiets;
		single_pushes &= quiets;

		while (single_pushes) {
			unsigned square = pop_lsb(single_pushes);
			move_list[size++].move = create_move<QUIET>(square - up, square);
		}
		while (double_pushes) {
			unsigned square = pop_lsb(double_pushes);
			move_list[size++].move = create_move<DOUBLE_PUSH>(square - up - up, square);
		}

		uint64_t right_attacks = shift(pawns & ~FILE_H, up_right) & targets;
		uint64_t left_attacks  = shift(pawns & ~FILE_A, up_left ) & targets;

		while (right_attacks) {
			unsigned square = pop_lsb(right_attacks);
			move_list[size++].move = create_move<CAPTURE>(square - up_right, square);
		}
		while (left_attacks) {
			unsigned square = pop_lsb(left_attacks);
			move_list[size++].move = create_move<CAPTURE>(square - up_left,  square);
		}

		unsigned ep_square = board.history[board.game_ply].ep_sq;
		if (ep_square != SQ_NONE) {

			// non pinned en passants

			uint64_t ep_candidates = pawns & board.precomputed.pawn_attacks[!col][ep_square];
			while (ep_candidates) {
				unsigned attacker_square = pop_lsb(ep_candidates);

				// if an en passant cannot stop a check, it is illegal
				if (!((1ULL << ep_square) & quiets) && !((1ULL << (ep_square - up)) & targets)) continue;

				// avoid a horizontal check after an en passant, which can happen to unpinned pawns
				if ((board.precomputed.attacks_bb<ROOK>(ksq, board.occ & ~(1ULL << attacker_square)
				    & ~(1ULL << (ep_square - up))) & board.precomputed.line_bb[ksq][attacker_square]
				    & sliders) == 0)
					move_list[size++].move = create_move<EP_CAPTURE>(attacker_square, ep_square);
			}

			// pinned en passant


			// avoid diagonal checks after en passant, which can only happen to pinned pawns
			ep_candidates = pinned_pawns & board.precomputed.pawn_attacks[!col][ep_square] & board.precomputed.line_bb[ksq][ep_square];
			while (ep_candidates) {
				unsigned attacker_square = pop_lsb(ep_candidates);

				// if an en passant cannot stop a check, it is illegal
				if (((1ULL << ep_square) & quiets) || ((1ULL << (ep_square - up)) & targets))
					move_list[size++].move = create_move<EP_CAPTURE>(attacker_square, ep_square);
			}
		}

		// non pinned promotions

		pawns = board.pieces[piece_of(col, PAWN)] & ~pinned;
		pawns &= (col == WHITE) ? RANK_2 : RANK_7;
		if (pawns) {

			// quiet promotion
			single_pushes = shift(pawns, up) & quiets;
			while(single_pushes) {
				unsigned square = pop_lsb(single_pushes);
				move_list[size++].move = create_move<PR_KNIGHT>(square - up, square);
				move_list[size++].move = create_move<PR_BISHOP>(square - up, square);
				move_list[size++].move = create_move<PR_ROOK>(square - up, square);
				move_list[size++].move = create_move<PR_QUEEN>(square - up, square);
			}

			// promotion captures
			right_attacks = shift(pawns & ~FILE_H, up_right) & targets;
			left_attacks  = shift(pawns & ~FILE_A, up_left ) & targets;

			while(right_attacks) {
				unsigned square = pop_lsb(right_attacks);
				move_list[size++].move = create_move<PC_KNIGHT>(square - up_right, square);
				move_list[size++].move = create_move<PC_BISHOP>(square - up_right, square);
				move_list[size++].move = create_move<PC_ROOK>(square - up_right, square);
				move_list[size++].move = create_move<PC_QUEEN>(square - up_right, square);
			}

			while(left_attacks) {
				unsigned square = pop_lsb(left_attacks);
				move_list[size++].move = create_move<PC_KNIGHT>(square - up_left, square);
				move_list[size++].move = create_move<PC_BISHOP>(square - up_left, square);
				move_list[size++].move = create_move<PC_ROOK>(square - up_left, square);
				move_list[size++].move = create_move<PC_QUEEN>(square - up_left, square);
			}
		}

		// for each pinned pawn

		uint64_t candidates = pinned_pawns;

		while (candidates) {
			unsigned square = pop_lsb(candidates);
			unsigned promotion_rank = (col == WHITE) ? 1: 6;
			uint64_t attacks = board.precomputed.pawn_attacks[col][square] & targets & board.precomputed.line_bb[ksq][square];

			// pinned promotion
			if (rank_num(square) == promotion_rank) {
				append_attacks<PC_KNIGHT>(square, attacks);
				append_attacks<PC_BISHOP>(square, attacks);
				append_attacks<PC_ROOK>(square, attacks);
				append_attacks<PC_QUEEN>(square, attacks);
			}
			else {
				append_attacks<CAPTURE>(square, attacks);

				uint64_t single_push = shift(1ULL << square, up) & ~board.occ & board.precomputed.line_bb[ksq][square];
				uint64_t double_push = shift(single_push & double_push_rank, up) & quiets & board.precomputed.line_bb[ksq][square];
				single_push &= quiets;

				append_attacks<QUIET>(square, single_push);
				append_attacks<DOUBLE_PUSH>(square, double_push);

			}
		}
	}


	template <Piece_type p>
	void generate_moves(Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq)
	{
		uint64_t pieces = board.pieces[piece_of(col, p)];
		uint64_t attacks;

		while(pieces) {
			unsigned from = pop_lsb(pieces);
			attacks = board.precomputed.attacks_bb<p>(from, board.occ) & quiets;
			if ((1ULL << from) & pinned) attacks &= board.precomputed.line_bb[ksq][from];

			while(attacks) {
				unsigned to = pop_lsb(attacks);
				move_list[size++].move = create_move<QUIET>(from, to);
			}

			attacks = board.precomputed.attacks_bb<p>(from, board.occ) & targets;
			if ((1ULL << from) & pinned) attacks &= board.precomputed.line_bb[ksq][from];

			while(attacks) {
				unsigned to = pop_lsb(attacks);
				move_list[size++].move = create_move<CAPTURE>(from, to);
			}
		}
	}

	void generate_all_moves(Board &board)
	{
		Color friendly = board.side_to_move;
		
		Color enemy = swap(friendly);
		uint64_t us_bb = board.color[friendly];
		uint64_t them_bb = board.color[enemy];

		// king cannot move to danger squares

		unsigned ksq = lsb(board.pieces[piece_of(friendly, KING)]);
		uint64_t pawns = board.pieces[piece_of(enemy, PAWN)];
		uint64_t enemy_diag_sliders = board.pieces[piece_of(enemy, BISHOP)] | board.pieces[piece_of(enemy, QUEEN)];
		uint64_t enemy_orth_sliders = board.pieces[piece_of(enemy, ROOK)]   | board.pieces[piece_of(enemy, QUEEN)];
		Direction up_right = (enemy == WHITE) ? UP_RIGHT : DOWN_RIGHT;
		Direction up_left  = (enemy == WHITE) ? UP_LEFT : DOWN_LEFT;

		uint64_t danger = 0ULL;

		danger |= shift(pawns & ~FILE_H, up_right);
		danger |= shift(pawns & ~FILE_A, up_left );

		danger |= board.precomputed.attacks_bb<KING>(lsb(board.pieces[piece_of(enemy, KING)]), 0ULL);

		uint64_t knights = board.pieces[piece_of(enemy, KNIGHT)];
		while(knights) danger |= board.precomputed.attacks_bb<KNIGHT>(pop_lsb(knights), 0ULL);

		uint64_t bishops_queens = enemy_diag_sliders;
		while(bishops_queens) danger |= board.precomputed.attacks_bb<BISHOP>(pop_lsb(bishops_queens), board.occ & ~(1ULL << ksq));

		uint64_t rook_queens = enemy_orth_sliders;
		while(rook_queens) danger |= board.precomputed.attacks_bb<ROOK>(pop_lsb(rook_queens), board.occ & ~(1ULL << ksq));


		uint64_t attacks = board.precomputed.attacks_bb<KING>(ksq, 0ULL) & ~(us_bb | danger);
		append_attacks<QUIET>(ksq, attacks & ~them_bb);
		append_attacks<CAPTURE>(ksq, attacks & them_bb);

		// leaper (knight, pawn) pieces, delivering check can be captured

		uint64_t checkers = 0ULL;

		checkers |= board.precomputed.pawn_attacks[friendly][ksq] & board.pieces[piece_of(enemy, PAWN)];
		checkers |= board.precomputed.attacks_bb<KNIGHT>(ksq, 0ULL) & board.pieces[piece_of(enemy, KNIGHT)];

		// sliding pieces, delivering check can either be captured or blocked
		// or pin your pieces >:(

		uint64_t sliders = (board.precomputed.attacks_bb<BISHOP>(ksq, them_bb) & enemy_diag_sliders) |
				 (board.precomputed.attacks_bb<ROOK  >(ksq, them_bb) & enemy_orth_sliders);

		uint64_t pinned = 0ULL;

		while (sliders) {
			unsigned square = pop_lsb(sliders);
			uint64_t pin_ray = board.precomputed.between_bb[ksq][square] & us_bb;

			// no pieces block the ray - check detected
			if (pin_ray == 0) checkers |= 1ULL << square;

			// one of our pieces blocks the ray - register pin
			else if ((pin_ray & (pin_ray - 1)) == 0) pinned |= pin_ray;
		}

		uint64_t targets = 0ULL;
		uint64_t quiets = 0ULL;

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
			quiets  = ~board.occ;

			// castling

			if (friendly == WHITE) {
				// kingside
				if ((board.history[board.game_ply].castling_rights & 0b0001) &&
				    !((danger | board.occ) & board.precomputed.oo_blockers[friendly]))
					move_list[size++].move = create_move<OO>(ksq, ksq + 2);

				// queenside
				if ((board.history[board.game_ply].castling_rights & 0b0010) &&
				    !(((danger & ~(1ULL << B1)) | board.occ) & board.precomputed.ooo_blockers[friendly]))
					move_list[size++].move = create_move<OOO>(ksq, ksq - 2);
			}
			else {
				// kingside
				if ((board.history[board.game_ply].castling_rights & 0b0100) &&
				    !((danger | board.occ) & board.precomputed.oo_blockers[friendly]))
					move_list[size++].move = create_move<OO>(ksq, ksq + 2);

				// queenside
				if ((board.history[board.game_ply].castling_rights & 0b1000) &&
				    !(((danger & ~(1ULL << B8)) | board.occ) & board.precomputed.ooo_blockers[friendly]))
					move_list[size++].move = create_move<OOO>(ksq, ksq - 2);
			}
		}

		generate_pawn_moves(board, friendly, targets, quiets, pinned, ksq, enemy_orth_sliders);
		generate_moves<KNIGHT>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<BISHOP>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<ROOK  >(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<QUEEN >(board, friendly, targets, quiets, pinned, ksq);
	}

	void generate_pawn_attacks(Board &board, Color friendly, uint64_t targets)
	{
		Direction up_right = (friendly == WHITE) ? UP_RIGHT : DOWN_RIGHT;
		Direction up_left  = (friendly == WHITE) ? UP_LEFT : DOWN_LEFT;
		uint64_t pawns = board.pieces[piece_of(friendly, PAWN)];
		uint64_t right_attacks = shift(pawns & ~FILE_H, up_right) & targets;
		uint64_t left_attacks  = shift(pawns & ~FILE_A, up_left ) & targets;

		while (right_attacks) {
			unsigned square = pop_lsb(right_attacks);
			move_list[size++].move = create_move<CAPTURE>(square - up_right, square);
		}
		while (left_attacks) {
			unsigned square = pop_lsb(left_attacks);
			move_list[size++].move = create_move<CAPTURE>(square - up_left,  square);
		}

		unsigned ep_square = board.history[board.game_ply].ep_sq;
		if (ep_square != SQ_NONE) {

			uint64_t ep_candidates = pawns & board.precomputed.pawn_attacks[!friendly][ep_square];
			while (ep_candidates) {
				unsigned attacker_square = pop_lsb(ep_candidates);
				move_list[size++].move = create_move<EP_CAPTURE>(attacker_square, ep_square);
			}
		}
	}

	void generate_quiescence(Board &board)
	{
		Color friendly = board.side_to_move;

		Color enemy = swap(friendly);
		uint64_t us_bb = board.color[friendly];
		uint64_t them_bb = board.color[enemy];

		// king cannot move to danger squares

		unsigned ksq = lsb(board.pieces[piece_of(friendly, KING)]);
		uint64_t pawns = board.pieces[piece_of(enemy, PAWN)];
		uint64_t enemy_diag_sliders = board.pieces[piece_of(enemy, BISHOP)] | board.pieces[piece_of(enemy, QUEEN)];
		uint64_t enemy_orth_sliders = board.pieces[piece_of(enemy, ROOK)]   | board.pieces[piece_of(enemy, QUEEN)];
		Direction up_right = (enemy == WHITE) ? UP_RIGHT : DOWN_RIGHT;
		Direction up_left  = (enemy == WHITE) ? UP_LEFT : DOWN_LEFT;

		uint64_t danger = 0ULL;

		danger |= shift(pawns & ~FILE_H, up_right);
		danger |= shift(pawns & ~FILE_A, up_left );

		danger |= board.precomputed.attacks_bb<KING>(lsb(board.pieces[piece_of(enemy, KING)]), 0ULL);

		uint64_t knights = board.pieces[piece_of(enemy, KNIGHT)];
		while(knights) danger |= board.precomputed.attacks_bb<KNIGHT>(pop_lsb(knights), 0ULL);

		uint64_t bishops_queens = enemy_diag_sliders;
		while(bishops_queens) danger |= board.precomputed.attacks_bb<BISHOP>(pop_lsb(bishops_queens), board.occ & ~(1ULL << ksq));

		uint64_t rook_queens = enemy_orth_sliders;
		while(rook_queens) danger |= board.precomputed.attacks_bb<ROOK>(pop_lsb(rook_queens), board.occ & ~(1ULL << ksq));


		uint64_t attacks = board.precomputed.attacks_bb<KING>(ksq, 0ULL) & ~(us_bb | danger);
		append_attacks<CAPTURE>(ksq, attacks & them_bb);

		// leaper (knight, pawn) pieces, delivering check can be captured

		uint64_t checkers = 0ULL;

		checkers |= board.precomputed.pawn_attacks[friendly][ksq] & board.pieces[piece_of(enemy, PAWN)];
		checkers |= board.precomputed.attacks_bb<KNIGHT>(ksq, 0ULL) & board.pieces[piece_of(enemy, KNIGHT)];

		// sliding pieces, delivering check can either be captured or blocked
		// or pin your pieces >:(

		uint64_t sliders = (board.precomputed.attacks_bb<BISHOP>(ksq, them_bb) & enemy_diag_sliders) |
				 (board.precomputed.attacks_bb<ROOK  >(ksq, them_bb) & enemy_orth_sliders);

		uint64_t pinned = 0ULL;

		while (sliders) {
			unsigned square = pop_lsb(sliders);
			uint64_t pin_ray = board.precomputed.between_bb[ksq][square] & us_bb;

			// no pieces block the ray - check detected
			if (pin_ray == 0) checkers |= 1ULL << square;

			// one of our pieces blocks the ray - register pin
			else if ((pin_ray & (pin_ray - 1)) == 0) pinned |= pin_ray;
		}

		uint64_t targets = 0ULL;
		uint64_t quiets = 0ULL;

		switch(pop_count(checkers)) {
		case 2: return;

		case 1:
			targets = checkers;
			break;
		default:
			targets = them_bb;
		}

		generate_pawn_moves(board, friendly, targets, quiets, pinned, ksq, enemy_orth_sliders);
		generate_moves<KNIGHT>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<BISHOP>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<ROOK  >(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<QUEEN >(board, friendly, targets, quiets, pinned, ksq);


		// generate quiet queen promotions too
		Direction up = (friendly == WHITE) ? UP : DOWN;
		pawns = board.pieces[piece_of(friendly, PAWN)] & ~pinned;
		pawns &= (friendly == WHITE) ? RANK_2 : RANK_7;
		if (pawns && !checkers) {

			uint64_t single_pushes = shift(pawns, up) & ~board.occ;
			while(single_pushes) {
				unsigned square = pop_lsb(single_pushes);
				move_list[size++].move = create_move<PR_QUEEN>(square - up, square);
			}
		}
	}
	
	/*void generate_quiescence(Board &board)
	{
		Color friendly = board.side_to_move;
		Color enemy = !friendly;

		unsigned ksq = 0;
		uint64_t pinned = 0ULL;
		uint64_t quiets = 0ULL;
		uint64_t targets = board.color[enemy] & ~board.pieces[piece_of(enemy, KING)];

		generate_pawn_attacks(board, friendly, targets);
		generate_moves<KNIGHT>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<BISHOP>(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<ROOK  >(board, friendly, targets, quiets, pinned, ksq);
		generate_moves<QUEEN >(board, friendly, targets, quiets, pinned, ksq);
	}*/
};

enum Gen_stage {
	CAPTURE_GEN, QUIET_GEN, EVASION_GEN
};
