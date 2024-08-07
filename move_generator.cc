#include "move_generator.h"

#include "utility.h"
#include "pre_computed.h"
#include "board.h"

/*void Move_generator::generate_pawn_moves(Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq, uint64_t sliders)
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
		move_list[size++].move = create_move(QUIET, square - up, square);
	}
	while (double_pushes) {
		unsigned square = pop_lsb(double_pushes);
		move_list[size++].move = create_move(DOUBLE_PUSH, square - up - up, square);
	}

	uint64_t right_attacks = shift(pawns & ~FILE_H, up_right) & targets;
	uint64_t left_attacks  = shift(pawns & ~FILE_A, up_left ) & targets;

	while (right_attacks) {
		unsigned square = pop_lsb(right_attacks);
		move_list[size++].move = create_move(CAPTURE, square - up_right, square);
	}
	while (left_attacks) {
		unsigned square = pop_lsb(left_attacks);
		move_list[size++].move = create_move(CAPTURE, square - up_left,  square);
	}

	unsigned ep_square = board.history[board.game_ply].ep_sq;
	if (ep_square != NO_SQUARE) {

		// non pinned en passants

		uint64_t ep_candidates = pawns & pawn_attacks(swap(col), ep_square);
		while (ep_candidates) {
			unsigned attacker_square = pop_lsb(ep_candidates);

			// if an en passant cannot stop a check, it is illegal
			if (!((1ULL << ep_square) & quiets) && !((1ULL << (ep_square - up)) & targets)) continue;

			// avoid a horizontal check after an en passant, which can happen to unpinned pawns
			if ((piece_attacks(ROOK, ksq, board.occ & ~(1ULL << attacker_square)
			    & ~(1ULL << (ep_square - up))) & ray(ksq, attacker_square)
			    & sliders) == 0)
				move_list[size++].move = create_move(EP_CAPTURE, attacker_square, ep_square);
		}

		// pinned en passant


		// avoid diagonal checks after en passant, which can only happen to pinned pawns
		ep_candidates = pinned_pawns & pawn_attacks(swap(col), ep_square) & ray(ksq, ep_square);
		while (ep_candidates) {
			unsigned attacker_square = pop_lsb(ep_candidates);

			// if an en passant cannot stop a check, it is illegal
			if (((1ULL << ep_square) & quiets) || ((1ULL << (ep_square - up)) & targets))
				move_list[size++].move = create_move(EP_CAPTURE, attacker_square, ep_square);
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
			move_list[size++].move = create_move(PR_KNIGHT, square - up, square);
			move_list[size++].move = create_move(PR_BISHOP, square - up, square);
			move_list[size++].move = create_move(PR_ROOK,   square - up, square);
			move_list[size++].move = create_move(PR_QUEEN,  square - up, square);
		}

		// promotion captures
		right_attacks = shift(pawns & ~FILE_H, up_right) & targets;
		left_attacks  = shift(pawns & ~FILE_A, up_left ) & targets;

		while(right_attacks) {
			unsigned square = pop_lsb(right_attacks);
			move_list[size++].move = create_move(PC_KNIGHT, square - up_right, square);
			move_list[size++].move = create_move(PC_BISHOP, square - up_right, square);
			move_list[size++].move = create_move(PC_ROOK,   square - up_right, square);
			move_list[size++].move = create_move(PC_QUEEN,  square - up_right, square);
		}

		while(left_attacks) {
			unsigned square = pop_lsb(left_attacks);
			move_list[size++].move = create_move(PC_KNIGHT, square - up_left, square);
			move_list[size++].move = create_move(PC_BISHOP, square - up_left, square);
			move_list[size++].move = create_move(PC_ROOK,   square - up_left, square);
			move_list[size++].move = create_move(PC_QUEEN,  square - up_left, square);
		}
	}

	// for each pinned pawn

	uint64_t candidates = pinned_pawns;

	while (candidates) {
		unsigned square = pop_lsb(candidates);
		unsigned promotion_rank = (col == WHITE) ? 1: 6;
		uint64_t attacks = pawn_attacks(col, square) & targets & ray(ksq, square);

		// pinned promotion
		if (rank_num(square) == promotion_rank) {
			append_attacks(PC_KNIGHT, square, attacks);
			append_attacks(PC_BISHOP, square, attacks);
			append_attacks(PC_ROOK,   square, attacks);
			append_attacks(PC_QUEEN,  square, attacks);
		}
		else {
			append_attacks(CAPTURE, square, attacks);

			uint64_t single_push = shift(1ULL << square, up) & ~board.occ & ray(ksq, square);
			uint64_t double_push = shift(single_push & double_push_rank, up) & quiets & ray(ksq, square);
			single_push &= quiets;

			append_attacks(QUIET, square, single_push);
			append_attacks(DOUBLE_PUSH, square, double_push);

		}
	}
}


void Move_generator::generate_moves(Piece_type p, Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq)
{
	uint64_t pieces = board.pieces[piece_of(col, p)];
	uint64_t attacks;

	while(pieces) {
		unsigned from = pop_lsb(pieces);
		attacks = piece_attacks(p, from, board.occ) & quiets;
		if ((1ULL << from) & pinned) attacks &= ray(ksq, from);

		while(attacks) {
			unsigned to = pop_lsb(attacks);
			move_list[size++].move = create_move(QUIET, from, to);
		}

		attacks = piece_attacks(p, from, board.occ) & targets;
		if ((1ULL << from) & pinned) attacks &= ray(ksq, from);

		while(attacks) {
			unsigned to = pop_lsb(attacks);
			move_list[size++].move = create_move(CAPTURE, from, to);
		}
	}
}

void Move_generator::generate_all_moves(Board &board)
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

	danger |= piece_attacks(KING, lsb(board.pieces[piece_of(enemy, KING)]), 0ULL);

	uint64_t knights = board.pieces[piece_of(enemy, KNIGHT)];
	while(knights) danger |= piece_attacks(KNIGHT, pop_lsb(knights), 0ULL);

	uint64_t bishops_queens = enemy_diag_sliders;
	while(bishops_queens) danger |= piece_attacks(BISHOP, pop_lsb(bishops_queens), board.occ & ~(1ULL << ksq));

	uint64_t rook_queens = enemy_orth_sliders;
	while(rook_queens) danger |= piece_attacks(ROOK, pop_lsb(rook_queens), board.occ & ~(1ULL << ksq));


	uint64_t attacks = piece_attacks(KING, ksq, 0ULL) & ~(us_bb | danger);
	append_attacks(QUIET, ksq, attacks & ~them_bb);
	append_attacks(CAPTURE, ksq, attacks & them_bb);

	// leaper (knight, pawn) pieces, delivering check can be captured

	uint64_t checkers = 0ULL;

	checkers |= pawn_attacks(friendly, ksq) & board.pieces[piece_of(enemy, PAWN)];
	checkers |= piece_attacks(KNIGHT, ksq, 0ULL) & board.pieces[piece_of(enemy, KNIGHT)];

	// sliding pieces, delivering check can either be captured or blocked
	// or pin your pieces >:(

	uint64_t sliders = (piece_attacks(BISHOP, ksq, them_bb) & enemy_diag_sliders) |
			 (piece_attacks(ROOK, ksq, them_bb) & enemy_orth_sliders);

	uint64_t pinned = 0ULL;

	while (sliders) {
		unsigned square = pop_lsb(sliders);
		uint64_t pin_ray = ray_between(ksq, square) & us_bb;

		// no pieces block the ray - check detected
		if (pin_ray == 0) checkers |= 1ULL << square;

		// one of our pieces blocks the ray - register pin
		else if ((pin_ray & (pin_ray - 1)) == 0) pinned |= pin_ray;
	}

	uint64_t targets = 0ULL;
	uint64_t quiets = 0ULL;

	switch (pop_count(checkers)) {
	case 2: return;

	case 1:
		{
		targets = checkers;
		unsigned checker_square = lsb(checkers);
		Piece p = board.board[checker_square];
		if (p == piece_of(enemy, BISHOP) || p == piece_of(enemy, ROOK) || p == piece_of(enemy, QUEEN))
			quiets = ray_between(ksq, checker_square) & ~checkers;
		}
		break;
	default:
		targets = them_bb;
		quiets  = ~board.occ;

		// castling

		if (friendly == WHITE) {
			// kingside
			if ((board.history[board.game_ply].castling_rights & 0b0001) &&
			    !((danger | board.occ) & oo_blockers(friendly)))
				move_list[size++].move = create_move(OO, ksq, ksq + 2);

			// queenside
			if ((board.history[board.game_ply].castling_rights & 0b0010) &&
			    !(((danger & ~(1ULL << B1)) | board.occ) & ooo_blockers(friendly)))
				move_list[size++].move = create_move(OOO, ksq, ksq - 2);
		}
		else {
			// kingside
			if ((board.history[board.game_ply].castling_rights & 0b0100) &&
			    !((danger | board.occ) & oo_blockers(friendly)))
				move_list[size++].move = create_move(OO, ksq, ksq + 2);

			// queenside
			if ((board.history[board.game_ply].castling_rights & 0b1000) &&
			    !(((danger & ~(1ULL << B8)) | board.occ) & ooo_blockers(friendly)))
				move_list[size++].move = create_move(OOO, ksq, ksq - 2);
		}
	}

	generate_pawn_moves(board, friendly, targets, quiets, pinned, ksq, enemy_orth_sliders);
	generate_moves(KNIGHT, board, friendly, targets, quiets, pinned, ksq);
	generate_moves(BISHOP, board, friendly, targets, quiets, pinned, ksq);
	generate_moves(ROOK,   board, friendly, targets, quiets, pinned, ksq);
	generate_moves(QUEEN,  board, friendly, targets, quiets, pinned, ksq);
}

void Move_generator::generate_quiescence(Board &board)
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

	danger |= piece_attacks(KING,lsb(board.pieces[piece_of(enemy, KING)]), 0ULL);

	uint64_t knights = board.pieces[piece_of(enemy, KNIGHT)];
	while(knights) danger |= piece_attacks(KNIGHT, pop_lsb(knights), 0ULL);

	uint64_t bishops_queens = enemy_diag_sliders;
	while(bishops_queens) danger |= piece_attacks(BISHOP, pop_lsb(bishops_queens), board.occ & ~(1ULL << ksq));

	uint64_t rook_queens = enemy_orth_sliders;
	while(rook_queens) danger |= piece_attacks(ROOK, pop_lsb(rook_queens), board.occ & ~(1ULL << ksq));


	uint64_t attacks = piece_attacks(KING, ksq, 0ULL) & ~(us_bb | danger);
	append_attacks(CAPTURE, ksq, attacks & them_bb);

	// leaper (knight, pawn) pieces, delivering check can be captured

	uint64_t checkers = 0ULL;

	checkers |= pawn_attacks(friendly, ksq) & board.pieces[piece_of(enemy, PAWN)];
	checkers |= piece_attacks(KNIGHT, ksq, 0ULL) & board.pieces[piece_of(enemy, KNIGHT)];

	// sliding pieces, delivering check can either be captured or blocked
	// or pin your pieces >:(

	uint64_t sliders = (piece_attacks(BISHOP, ksq, them_bb) & enemy_diag_sliders) |
			   (piece_attacks(ROOK,   ksq, them_bb) & enemy_orth_sliders);

	uint64_t pinned = 0ULL;

	while (sliders) {
		unsigned square = pop_lsb(sliders);
		uint64_t pin_ray = ray_between(ksq, square) & us_bb;

		// no pieces block the ray - check detected
		if (pin_ray == 0) checkers |= 1ULL << square;

		// one of our pieces blocks the ray - register pin
		else if ((pin_ray & (pin_ray - 1)) == 0) pinned |= pin_ray;
	}

	uint64_t targets = 0ULL;
	uint64_t quiets = 0ULL;

	switch (pop_count(checkers)) {
	case 2: return;

	case 1:
		targets = checkers;
		break;
	default:
		targets = them_bb;
	}

	generate_pawn_moves(board, friendly, targets, quiets, pinned, ksq, enemy_orth_sliders);
	generate_moves(KNIGHT, board, friendly, targets, quiets, pinned, ksq);
	generate_moves(BISHOP, board, friendly, targets, quiets, pinned, ksq);
	generate_moves(ROOK,   board, friendly, targets, quiets, pinned, ksq);
	generate_moves(QUEEN,  board, friendly, targets, quiets, pinned, ksq);


	// generate quiet queen promotions too
	Direction up = (friendly == WHITE) ? UP : DOWN;
	pawns = board.pieces[piece_of(friendly, PAWN)] & ~pinned;
	pawns &= (friendly == WHITE) ? RANK_2 : RANK_7;
	if (pawns && !checkers) {

		uint64_t single_pushes = shift(pawns, up) & ~board.occ;
		while(single_pushes) {
			unsigned square = pop_lsb(single_pushes);
			move_list[size++].move = create_move(PR_QUEEN, square - up, square);
		}
	}
}*/

// ******************************
// *** - New Move Generator - ***
// *************************+****

void generate_promotions(Move_list &move_list, unsigned from, unsigned to, Gen_stage gen)
{
	if (gen == QUIET_GEN) {
		move_list.add(create_move(from, to, PR_KNIGHT));
		move_list.add(create_move(from, to, PR_BISHOP));
		move_list.add(create_move(from, to, PR_ROOK  ));
		// Quiet queen promotions are generated in the CAPTURE_GEN.
	}

	if (gen == CAPTURE_GEN) {
		move_list.add(create_move(from, to, PC_KNIGHT));
		move_list.add(create_move(from, to, PC_BISHOP));
		move_list.add(create_move(from, to, PC_ROOK  ));
		move_list.add(create_move(from, to, PC_QUEEN ));
	}
}

void generate_pawn_moves(Board &board, Move_list &move_list, uint64_t targets, Color friendly, Gen_stage gen)
{
	Direction forward       = (friendly == WHITE) ? UP : DOWN;
	Direction forward_right = (friendly == WHITE) ? UP_RIGHT : DOWN_LEFT;
	Direction forward_left  = (friendly == WHITE) ? UP_LEFT  : DOWN_RIGHT;

	uint64_t double_push_rank = (friendly == WHITE) ? RANK_3 : RANK_6;
	uint64_t promotion_rank   = (friendly == WHITE) ? RANK_7 : RANK_2;

	uint64_t non_promotion_pawns = board.pieces[piece_of(friendly, PAWN)] & ~promotion_rank;
	uint64_t promotion_pawns     = board.pieces[piece_of(friendly, PAWN)] &  promotion_rank;
	
	if (gen == CAPTURE_GEN || gen == EVASION_GEN) {
		uint64_t enemy_targets = targets & board.enemy(friendly);
		uint64_t right_attacks = shift(non_promotion_pawns, forward_right) & enemy_targets;
		uint64_t left_attacks  = shift(non_promotion_pawns, forward_left ) & enemy_targets;

		while (right_attacks) {
			unsigned square = pop_lsb(right_attacks);
			move_list.add(create_move(square - forward_right, square, CAPTURE));
		}
		while (left_attacks) {
			unsigned square = pop_lsb(left_attacks);
			move_list.add(create_move(square - forward_left,  square, CAPTURE));
		}

		if (promotion_pawns) {
			// Queen promotions are generated with the captures, because they are not considered quiet.
			uint64_t promotion_pushes = shift(promotion_pawns, forward) & ~board.occ;
			if (gen == EVASION_GEN) promotion_pushes &= targets;
			while (promotion_pushes) {
				unsigned square = pop_lsb(promotion_pushes);
				move_list.add(create_move(square - forward, square, PR_QUEEN));
			}

			uint64_t right_attacks = shift(promotion_pawns, forward_right) & enemy_targets;
			while (right_attacks) {
				unsigned square = pop_lsb(right_attacks);
				generate_promotions(move_list, square - forward_right, square, CAPTURE_GEN);
			}

			uint64_t left_attacks  = shift(promotion_pawns, forward_left ) & enemy_targets;
			while (left_attacks) {
				unsigned square = pop_lsb(left_attacks);
				generate_promotions(move_list, square - forward_left,  square, CAPTURE_GEN);
			}
		}

		unsigned ep_square = board.history[board.game_ply].ep_sq;
		if (ep_square != NO_SQUARE) {
			uint64_t ep_candidates = non_promotion_pawns & pawn_attacks(swap(friendly), ep_square);
			while (ep_candidates) {
				unsigned attacker_square = pop_lsb(ep_candidates);
				move_list.add(create_move(attacker_square, ep_square, EP_CAPTURE));
			}
		}
	}

	if (gen == QUIET_GEN || gen == EVASION_GEN) {
		uint64_t single_pushes = shift(non_promotion_pawns, forward) & ~board.occ;
		uint64_t double_pushes = shift(single_pushes & double_push_rank, forward) & ~board.occ & targets;
		single_pushes &= targets; // Check evasions

		while (single_pushes) {
			unsigned square = pop_lsb(single_pushes);
			move_list.add(create_move(square - forward, square, QUIET));
		}
		while (double_pushes) {
			unsigned square = pop_lsb(double_pushes);
			move_list.add(create_move(square - forward - forward, square, DOUBLE_PUSH));
		}

		if (promotion_pawns) {
			uint64_t promotion_pushes = shift(promotion_pawns, forward) & targets;

			while (promotion_pushes) {
				unsigned square = pop_lsb(promotion_pushes);
				generate_promotions(move_list, square - forward, square, QUIET_GEN);
			}
		}
	}
}

void generate_piece_moves(Board &board, Move_list &move_list, uint64_t targets, Color friendly, Piece_type type)
{
	uint64_t pieces = board.pieces[piece_of(friendly, type)];

	while (pieces) {
		unsigned from = pop_lsb(pieces);

		uint64_t attacks = piece_attacks(type, from, board.occ) & targets;

		while (attacks) {
			unsigned to = pop_lsb(attacks);
			Move_flags flag = (board.board[to] == NO_PIECE) ? QUIET : CAPTURE;
			move_list.add(create_move(from, to, flag));
		}
	}
}

void generate(Board &board, Move_list &move_list, Gen_stage gen)
{
	Color friendly = board.side_to_move;
	unsigned king_square = board.square(friendly, KING);

	uint64_t targets = 0ULL;
	switch (gen) {
		case CAPTURE_GEN: targets = board.enemy(friendly); break;
		case QUIET_GEN:	  targets = ~board.occ; break;
		case EVASION_GEN: targets = ray_between(king_square, lsb(board.history[board.game_ply].checkers)); break;
		default: break;
	}

	if (gen == EVASION_GEN) {
		generate_piece_moves(board, move_list, ~board.color[friendly], friendly, KING);
		// Only king moves are possible in case of a double check.
		if (pop_count(board.history[board.game_ply].checkers) >= 2) return;
	}
	else generate_piece_moves(board, move_list, targets, friendly, KING);

	generate_pawn_moves( board, move_list, targets, friendly, gen);
	generate_piece_moves(board, move_list, targets, friendly, KNIGHT);
	generate_piece_moves(board, move_list, targets, friendly, BISHOP);
	generate_piece_moves(board, move_list, targets, friendly, ROOK);
	generate_piece_moves(board, move_list, targets, friendly, QUEEN);

	if (gen == QUIET_GEN) {
		uint8_t castling_rights = board.history[board.game_ply].castling_rights;

		if ((castling_rights & king_side[friendly])  && !(oo_blockers(friendly)  & board.occ))
			move_list.add(create_move(king_square, king_square + 2, OO));

		if ((castling_rights & queen_side[friendly]) && !(ooo_blockers(friendly) & board.occ))
			move_list.add(create_move(king_square, king_square - 2, OOO));
	}
}

void generate_legal(Board &board, Move_list &move_list)
{
	Move_list temp_move_list {};
	if (board.history[board.game_ply].checkers != 0ULL) {
		generate(board, temp_move_list, EVASION_GEN);
	}
	else {
		generate(board, temp_move_list, CAPTURE_GEN);
		generate(board, temp_move_list, QUIET_GEN);
	}

	for (unsigned n = 0; n < temp_move_list.size; n++) {
		Move move = temp_move_list.moves[n].move;
		if (board.legal(move))
			move_list.add(move);
	}
};
