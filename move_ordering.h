#pragma once

#include <algorithm>

#include "move_generator.h"
#include "see.h"

struct Heuristics
{
	Move killer_move[2][64];
	Move counter_move[16][64];
	int32_t history[16][64];

	Move counter(Board &board)
	{
		if (board.game_ply == 0) return INVALID_MOVE;
		unsigned to = move_to(board.history[board.game_ply - 1].move);
		return counter_move[board.board[to]][to];
	}
};

enum Stage
{
	// Normal ordering
	TRANSPOSITION,
	GENERATE_CAPTURES,
	GOOD_CAPTURES,
	FIRST_KILLER,
	SECOND_KILLER,
	COUNTER_MOVE,
	GENERATE_QUIETS,
	QUIETS,
	BAD_CAPTURES,
	// Ordering in check
	IN_CHECK_TRANSPOSITION,
	GENERATE_IN_CHECKS,
	IN_CHECKS,
	// Quiescence Search ordering
	QUIESCENCE_TRANSPOSITION,
	GENERATE_QUIESCENCES,
	QUIESCENCES
};

struct Move_orderer
{
	Move_list move_list {};    // careful, the array is uninitialized!
	Move_list bad_captures {}; // careful, the array is uninitialized!
	Stage stage = TRANSPOSITION;
	unsigned position = 0;
	Move hash_move;
	Move first_killer;
	Move second_killer;
	Move counter_move;
	Heuristics &heuristics;

	// Picks the next move from the movelist that has the highest score.
	Move next_best_move(Move_list &list)
	{
		unsigned best_index = position;
		int best_score = list.moves[position].score;
		for (unsigned i = position + 1; i < list.size; i++) {
			Scored_move &m = list.moves[i];
			if (m.score > best_score) {
				best_score = m.score;
				best_index = i;
			}
		}
		std::swap(list.moves[position], list.moves[best_index]);
		Move move = list.moves[position].move;
		position++;

		return move;
	}

	void score(Board &board, Gen_stage sort_type)
	{
		if (sort_type == CAPTURE_GEN) {
			for (unsigned n = position; n < move_list.size; n++) {
				Scored_move &scored_move = move_list.moves[n];
				Move move = scored_move.move;
				scored_move.score = 10 * piece_value[board.board[move_to(move)]] - piece_value[board.board[move_from(move)]];
			}
		}
		if (sort_type == QUIET_GEN) {
			for (unsigned n = position; n < move_list.size; n++) {
				Scored_move &scored_move = move_list.moves[n];
				Move move = scored_move.move;
				scored_move.score = std::min(30000000, heuristics.history[board.board[move_from(move)]][move_to(move)]) >> 3;
			}
		}
		if (sort_type == IN_CHECK_GEN) {
			for (unsigned n = position; n < move_list.size; n++) {
				Scored_move &scored_move = move_list.moves[n];
				scored_move.score = 0;
				Move move = scored_move.move;
				if (capture(move)) {
					scored_move.score = 10 * piece_value[board.board[move_to(move)]] - piece_value[board.board[move_from(move)]];
				}
			}
		}
	}

	// For each call, the next most promising pseudo legal move is returned.
	Move next_move(Board &board)
	{
		// The hash move is probably the best move, because we retrieved it by searching this position earlier.
		if (stage == TRANSPOSITION) {
			stage = GENERATE_CAPTURES;
			if (board.pseudo_legal(hash_move)) return hash_move;
		}
		if (stage == IN_CHECK_TRANSPOSITION) {
			stage = GENERATE_IN_CHECKS;
			if (board.pseudo_legal(hash_move)) return hash_move;
		}

		// Generate captures and queen promotions
		if (stage == GENERATE_CAPTURES) {
			generate(board, move_list, CAPTURE_GEN);
			score(board, CAPTURE_GEN);
			stage = GOOD_CAPTURES;
		}
		if (stage == GOOD_CAPTURES) {
			while (position < move_list.size) {
				Move move = next_best_move(move_list);
				if (move == hash_move) continue;
				// Queen promotions can be killer and counter moves.
				if (move == first_killer)  first_killer  = INVALID_MOVE;
				if (move == second_killer) second_killer = INVALID_MOVE;
				if (move == counter_move)  counter_move  = INVALID_MOVE;

				// Only captures with a positive see score are "Good captures".
				if (see(board, move) < 0) {
					bad_captures.add(move);
					continue;
				}
				return move;
			}
			stage = FIRST_KILLER;
		}

		// Generate captures and queen promotions
		if (stage == GENERATE_QUIESCENCES) {
			generate(board, move_list, CAPTURE_GEN);
			score(board, CAPTURE_GEN);
			stage = QUIESCENCES;
		}
		if (stage == QUIESCENCES) {
			if (position < move_list.size)
				return next_best_move(move_list);
		}

		// We can still try the killer and counter moves before generating the quiet moves.
		if (stage == FIRST_KILLER) {
			stage = SECOND_KILLER;
			if (first_killer  != hash_move && board.pseudo_legal( first_killer)) return first_killer;
		}
		if (stage == SECOND_KILLER) {
			stage = COUNTER_MOVE;
			if (second_killer != hash_move && board.pseudo_legal(second_killer)) return second_killer;
		}
		if (stage == COUNTER_MOVE) {
			stage = GENERATE_QUIETS;
			if (counter_move != hash_move && counter_move != first_killer && counter_move != second_killer &&
			    board.pseudo_legal(counter_move)) return counter_move;
		}

		// Generate check evasions.
		if (stage == GENERATE_IN_CHECKS) {
			generate(board, move_list, IN_CHECK_GEN);
			score(board, IN_CHECK_GEN);
			stage = IN_CHECKS;
		}
		if (stage == IN_CHECKS) {
			while (position < move_list.size) {
				Move move = next_best_move(move_list);
				if (move == hash_move) continue;
				return move;
			}
		}

		// Now, all quiet moves are generated.
		if (stage == GENERATE_QUIETS) {
			generate(board, move_list, QUIET_GEN);
			score(board, QUIET_GEN);
			stage = QUIETS;
		}
		if (stage == QUIETS) {
			while (position < move_list.size) {
				Move move = next_best_move(move_list);
				if (move == hash_move || move == first_killer || move == second_killer || move == counter_move) continue;
				return move;
			}
			stage = BAD_CAPTURES;
			position = 0; // We have a seperate list for bad captures.
		}

		// Finally, loop over the losing captures, they might still be some kind of sacrifice.
		if (stage == BAD_CAPTURES) {
			while (position < bad_captures.size) {
				Move move = next_best_move(bad_captures);
				if (move == hash_move || move == first_killer || move == second_killer || move == counter_move) continue;
				return move;
			}
		}
		return INVALID_MOVE;
	}

	Move_orderer(Board &board, Move hash_move, Heuristics &heuristics, int ply)
	:
		hash_move(hash_move),
		first_killer( heuristics.killer_move[0][ply]),
		second_killer(heuristics.killer_move[1][ply]),
		counter_move(heuristics.counter(board)),
		heuristics(heuristics)
	{}
};

/*static inline void rate_moves(Board &board, Heuristics &heuristics, Move_generator &move_generator, bool quiescence, unsigned ply)
{
	for (unsigned n = 0; n < move_generator.size; n++) {
		Scored_move &m = move_generator.move_list[n];

		// transposition table move
		if (m.move == heuristics.hash_move) {
			m.score += 30000;
			continue;
		}

		// MVV - LVA (most valuable victim, least valuable attacker)
		else if (capture(m.move))
			m.score += std::min(10 * piece_value[board.board[move_to(m.move)]] - piece_value[board.board[move_from(m.move)]], 29999);

		else if (promotion(m.move)) m.score += 100;

		else if (!quiescence) {

			// primary killer move (non capture move that caused a beta cutoff)
			if (m.move == heuristics.killer_move[0][ply]) m.score += 80;

			// secondary killer move
			else if (m.move == heuristics.killer_move[1][ply]) m.score += 75;

			// if everything else fails, score history moves
			else m.score += std::min((-30000000 + heuristics.history[board.board[move_from(m.move)]][move_to(m.move)]) >> 3, 74);
			//if (std::min((-30000000 + heuristics.history[board.board[move_from(m.move)]][move_to(m.move)]) >> 3, 74) == 74)
			//	std::cerr << "history overflow\n";
		}
	}
}

static inline Move next_move(Move_generator &move_generator, unsigned index)
{
	unsigned best_index = index;
	int16_t best_score = move_generator.move_list[index].score;
	for (unsigned n = index + 1; n < move_generator.size; n++) {
		Scored_move &m = move_generator.move_list[n];
		if (m.score > best_score) {
			best_score = m.score;
			best_index = n;
		}
	}
	std::swap(move_generator.move_list[index], move_generator.move_list[best_index]);
	return move_generator.move_list[index].move;
}*/
