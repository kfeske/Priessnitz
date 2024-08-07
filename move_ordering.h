#pragma once

#include <algorithm>

#include "move_generator.h"

struct Heuristics
{
	Move killer_move[2][64];
	Move hash_move = INVALID_MOVE;
	int32_t history[16][64];
};

struct Move_orderer
{
	Move next_move()
	{
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
