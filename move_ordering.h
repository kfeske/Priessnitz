#pragma once

#include <algorithm>

#include "move_generator.h"

struct Heuristics
{
	Move killer_move[2][64];
	Move hash_move = INVALID_MOVE;
	int32_t history[16][64];
};

enum Stage
{
	// Normal ordering
	TRANSPOSITION,
	GENERATE_CAPTURES,
	CAPTURES,
	GENERATE_QUIETS,
	QUIETS,
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
	Move_list move_list {}; // careful, the array is uninitialized!
	Stage stage = TRANSPOSITION;
	Move tt_move = INVALID_MOVE;
	unsigned position = 0;

	// Picks the next move from the movelist that has the highest score.
	Move next_best_move()
	{
		unsigned best_index = position;
		int best_score = move_list.moves[position].score;
		for (unsigned i = position + 1; i < move_list.size; i++) {
			Scored_move &m = move_list.moves[i];
			if (m.score > best_score) {
				best_score = m.score;
				best_index = i;
			}
		}
		std::swap(move_list.moves[position], move_list.moves[best_index]);
		Move move = move_list.moves[position].move;
		position++;

		// The hash move is searched earlier and is therefore skipped here.
		//if (move == tt_move) return (position < move_list.size) ? next_best_move() : INVALID_MOVE;

		return move;
	}

	void score(Board &board, Stage sort_type)
	{
		if (sort_type == CAPTURES) {
			for (unsigned n = position; n < move_list.size; n++) {
				Scored_move &scored_move = move_list.moves[n];
				Move move = scored_move.move;
				scored_move.score = 10 * piece_value[board.board[move_to(move)]] - piece_value[board.board[move_from(move)]];
			}
		}
	}

	// For each call, the next most promising pseudo legal move is returned.
	Move next_move(Board &board)
	{
		if (stage == TRANSPOSITION) {
			stage = GENERATE_CAPTURES;
			if (board.pseudo_legal(tt_move)) return tt_move;
		}
		//if (stage == QUIESCENCE_TRANSPOSITION) {
		//	stage = GENERATE_QUIESCENCES;
		//	if (board.pseudo_legal(tt_move)) return tt_move;
		//}
		if (stage == IN_CHECK_TRANSPOSITION) {
			stage = GENERATE_IN_CHECKS;
			if (board.pseudo_legal(tt_move)) return tt_move;
		}
		if (stage == GENERATE_CAPTURES) {
			generate(board, move_list, CAPTURE_GEN);
			score(board, CAPTURES);
			stage = CAPTURES;
		}
		if (stage == CAPTURES) {
			while (position < move_list.size) {
				Move move = next_best_move();
				if (move == tt_move) continue;
				return move;
			}
			stage = QUIETS;
			generate(board, move_list, QUIET_GEN);
		}
		if (stage == GENERATE_QUIESCENCES) {
			generate(board, move_list, CAPTURE_GEN);
			score(board, CAPTURES);
			stage = QUIESCENCES;
		}
		if (stage == QUIESCENCES) {
			if (position < move_list.size)
				return next_best_move();
		}
		if (stage == GENERATE_IN_CHECKS) {
			generate(board, move_list, IN_CHECK_GEN);
			stage = IN_CHECKS;
		}
		if (stage == IN_CHECKS) {
			while (position < move_list.size) {
				Move move = next_best_move();
				if (move == tt_move) continue;
				return move;
			}
		}
		if (stage == QUIETS) {
			while (position < move_list.size) {
				Move move = next_best_move();
				if (move == tt_move) continue;
				return move;
			}
		}
		return INVALID_MOVE;
	}
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
