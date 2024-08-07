#include "move_generator.h"

#include "utility.h"
#include "pre_computed.h"
#include "board.h"

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

	uint64_t non_promotion_pawns = board.pieces(friendly, PAWN) & ~promotion_rank;
	uint64_t promotion_pawns     = board.pieces(friendly, PAWN) &  promotion_rank;
	
	if (gen == CAPTURE_GEN || gen == IN_CHECK_GEN) {
		uint64_t enemy_targets = targets & board.pieces(swap(friendly));
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
			if (gen == IN_CHECK_GEN) promotion_pushes &= targets;
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
		if (ep_square != NO_SQUARE && !(gen == IN_CHECK_GEN &&
		    !(((1ULL << ep_square) | (1ULL << (ep_square - forward)))  & targets))) {
			uint64_t ep_candidates = non_promotion_pawns & pawn_attacks(swap(friendly), ep_square);
			while (ep_candidates) {
				unsigned attacker_square = pop_lsb(ep_candidates);
				move_list.add(create_move(attacker_square, ep_square, EP_CAPTURE));
			}
		}
	}

	if (gen == QUIET_GEN || gen == IN_CHECK_GEN) {
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
			uint64_t promotion_pushes = shift(promotion_pawns, forward) & targets & ~board.occ;

			while (promotion_pushes) {
				unsigned square = pop_lsb(promotion_pushes);
				generate_promotions(move_list, square - forward, square, QUIET_GEN);
			}
		}
	}
}

void generate_piece_moves(Board &board, Move_list &move_list, uint64_t targets, Color friendly, Piece_type type)
{
	uint64_t pieces = board.pieces(friendly, type);

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
		case CAPTURE_GEN: targets = board.pieces(swap(friendly)); break;
		case QUIET_GEN:	  targets = ~board.occ; break;
		case IN_CHECK_GEN: targets = ray_between(king_square, lsb(board.history[board.game_ply].checkers)); break;
		default: break;
	}

	if (gen == IN_CHECK_GEN) {
		generate_piece_moves(board, move_list, ~board.pieces(friendly), friendly, KING);
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
	if (board.in_check()) {
		generate(board, temp_move_list, IN_CHECK_GEN);
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
