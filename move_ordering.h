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

enum class Main_stage
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
};

struct Main_move_orderer
{
	Move_list move_list {};    // careful, the array is uninitialized!
	Move_list bad_captures {}; // careful, the array is uninitialized!
	Main_stage stage = Main_stage::TRANSPOSITION;
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
				scored_move.score = heuristics.history[board.board[move_from(move)]][move_to(move)];
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
	Move next_move(Board &board, bool skip_quiets)
	{
		switch (stage) {
		// The hash move is probably the best move, because we retrieved it by searching this position earlier.
		case Main_stage::TRANSPOSITION:
			stage = Main_stage::GENERATE_CAPTURES;
			if (board.pseudo_legal(hash_move)) return hash_move;
			[[fallthrough]];

		// Generate captures and queen promotions
		case Main_stage::GENERATE_CAPTURES:
			generate(board, move_list, CAPTURE_GEN);
			score(board, CAPTURE_GEN);
			stage = Main_stage::GOOD_CAPTURES;
			[[fallthrough]];

		case Main_stage::GOOD_CAPTURES:
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
			stage = Main_stage::FIRST_KILLER;
			[[fallthrough]];

		// We can still try the killer and counter moves before generating the quiet moves.
		case Main_stage::FIRST_KILLER:
			stage = Main_stage::SECOND_KILLER;
			if (!skip_quiets && first_killer  != hash_move && board.pseudo_legal( first_killer)) return first_killer;
			[[fallthrough]];

		case Main_stage::SECOND_KILLER:
			stage = Main_stage::COUNTER_MOVE;
			if (!skip_quiets && second_killer != hash_move && board.pseudo_legal(second_killer)) return second_killer;
			[[fallthrough]];

		case Main_stage::COUNTER_MOVE:
			stage = Main_stage::GENERATE_QUIETS;
			if (!skip_quiets && counter_move != hash_move && counter_move != first_killer && counter_move != second_killer &&
			    board.pseudo_legal(counter_move)) return counter_move;
			[[fallthrough]];

		// Now, all quiet moves are generated.
		case Main_stage::GENERATE_QUIETS:
			if (!skip_quiets) {
				generate(board, move_list, QUIET_GEN);
				score(board, QUIET_GEN);
			}
			stage = Main_stage::QUIETS;
			[[fallthrough]];

		case Main_stage::QUIETS:
			while (!skip_quiets && position < move_list.size) {
				Move move = next_best_move(move_list);
				if (move == hash_move || move == first_killer || move == second_killer || move == counter_move) continue;
				return move;
			}
			stage = Main_stage::BAD_CAPTURES;
			position = 0; // We have a seperate list for bad captures.
			[[fallthrough]];

		// Finally, loop over the losing captures, they might still be some kind of sacrifice.
		case Main_stage::BAD_CAPTURES:
			while (position < bad_captures.size) {
				Move move = next_best_move(bad_captures);
				if (move == hash_move || move == first_killer || move == second_killer || move == counter_move) continue;
				return move;
			}
			return INVALID_MOVE;

		// Generate check evasions.
		case Main_stage::IN_CHECK_TRANSPOSITION:
			stage = Main_stage::GENERATE_IN_CHECKS;
			if (board.pseudo_legal(hash_move)) return hash_move;
			[[fallthrough]];

		case Main_stage::GENERATE_IN_CHECKS:
			generate(board, move_list, IN_CHECK_GEN);
			score(board, IN_CHECK_GEN);
			stage = Main_stage::IN_CHECKS;
			[[fallthrough]];

		case Main_stage::IN_CHECKS:
			while (position < move_list.size) {
				Move move = next_best_move(move_list);
				if (move == hash_move) continue;
				return move;
			}
		}

		return INVALID_MOVE;
	}

	Main_move_orderer(Board &board, Move hash_move, Heuristics &heuristics, int ply)
	:
		hash_move(hash_move),
		first_killer( heuristics.killer_move[0][ply]),
		second_killer(heuristics.killer_move[1][ply]),
		counter_move(heuristics.counter(board)),
		heuristics(heuristics)
	{}
};

enum class Quiescence_stage
{
	GENERATE_CAPTURES,
	GOOD_CAPTURES,
	GENERATE_IN_CHECKS,
	IN_CHECKS
};

struct Quiescence_move_orderer
{
	Move_list move_list {}; // careful, the array is uninitialized!
	Quiescence_stage stage = Quiescence_stage::GENERATE_CAPTURES;
	unsigned position = 0;

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
	Move next_move(Board &board, int threshold)
	{
		switch (stage) {
		// Generate captures and queen promotions
		case Quiescence_stage::GENERATE_CAPTURES:
			generate(board, move_list, CAPTURE_GEN);
			score(board, CAPTURE_GEN);
			stage = Quiescence_stage::GOOD_CAPTURES;
			[[fallthrough]];

		case Quiescence_stage::GOOD_CAPTURES:
			while (position < move_list.size) {
				Move move = next_best_move(move_list);
				if (see(board, move) >= threshold || promotion(move)) return move;
			}
			return INVALID_MOVE;

		// Generate check evasions.
		case Quiescence_stage::GENERATE_IN_CHECKS:
			generate(board, move_list, IN_CHECK_GEN);
			score(board, IN_CHECK_GEN);
			stage = Quiescence_stage::IN_CHECKS;
			[[fallthrough]];

		case Quiescence_stage::IN_CHECKS:
			while (position < move_list.size) {
				Move move = next_best_move(move_list);
				return move;
			}
		}
		return INVALID_MOVE;
	}
};
