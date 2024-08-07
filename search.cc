#include <chrono>

#include "search.h"

#include "board.h"
#include "move_generator.h"
#include "transposition_table.h"
#include "see.h"
#include "move_ordering.h"

int const futility_margin[5] = {
	0, 100, 200, 300, 400
};

unsigned const lmp_margins[4] = {
	0, 9, 13, 17
};

bool mate(int score)
{
	return (abs(score) >= MATE_SCORE - 100);
}

double Search::time_elapsed()
{
	auto time_end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
}

// Quiescence Search is a shallower search, generating only
// captures and check evasions to avoid the horizon effect
int Search::quiescence_search(Board &board, int alpha, int beta, unsigned ply)
{
	bool in_check = board.in_check();

	int static_evaluation = eval.evaluate(board);

	// Captures are usually not forced. This will give the option to stop the capture sequence.
	if (!in_check && static_evaluation >= beta)
		return beta;

	// Again, we should at least be able to achieve the static evaluation.
	if (static_evaluation > alpha)
		alpha = static_evaluation;

	Move_orderer move_orderer { board, INVALID_MOVE, heuristics, 0 };
	move_orderer.stage = (in_check) ? GENERATE_IN_CHECKS : GENERATE_QUIESCENCES;

	Move move;
	while ((move = move_orderer.next_move(board)) != INVALID_MOVE) {
		if (!board.legal(move)) continue;

		// Delta Pruning
		// if this move is unlikely to be a good capture, because it will not improve alpha enough, it is pruned
		//int gain = abs(piece_value[board.board[move_to(move)]]);
		//if (static_evaluation + gain + 200 <= alpha && !in_check && !promotion(move))
		//	continue;

		// Prune moves that lose material. In this case, a quiet move is probably better.
		if (see(board, move) < 0) continue;

		statistics.quiescence_nodes++;

		board.make_move(move);

		int evaluation = -quiescence_search(board, -beta, -alpha, ply + 1);

		board.unmake_move(move);

		if (evaluation >= beta)
			return beta;

		if (evaluation > alpha)
			alpha = evaluation;
	}
	return alpha;
}

// Function that finds the best move at a fixed depth (number of moves, it looks into the future)
// Alpha is the best score, the side to move can guarantee in a sequence of moves
// Beta is the best score, the opponent can guarantee in the sequence
int Search::search(Board &board, int depth, int ply, int alpha, int beta, Move skip, bool allow_null_move)
{
	if ((fixed_time || time_management) && (statistics.search_nodes & 1024) == 0 && time_elapsed() >= hard_time_cap)
	{
		// Time's up!
		abort_search = true;
		return 0;
	}

	// check for draws by repetition or by 50 Move Rule
	if (board.immediate_draw(ply))
		return DRAW_SCORE;

	if (depth <= 0)
		// we reached a leaf node, start the Quiescence Search
		return quiescence_search(board, alpha, beta, 0);

	//if (board.game_ply >= 10 && ply % 2 == 0 && board.pieces[B_QUEEN]) return -9999;

	// Mate Distance Pruning
	// If a forced mate was found, we do not need to search deeper than to where it was found, because we
	// only care for the shortest mate.
	alpha = std::max(alpha, -MATE_SCORE + ply);
	beta  = std::min(beta, MATE_SCORE - ply);
	if (alpha >= beta) return alpha;
	

	unsigned move_count = 0;
	Move best_move = INVALID_MOVE;
	int evaluation;
	bool in_check = board.in_check();
	bool pv_node = beta - alpha != 1;

	// if no move exceeds alpha, we do not have an exact evaluation,
	// we only know that none of our moves can improve it. It can still be stored as an UPPERBOUND though!
	TT_flag flag = UPPERBOUND;

	// updated in the tt.probe() function
	tt.best_move = INVALID_MOVE;

	// Check for any transpositions at higher or equal depths
	bool usable = tt.probe(board.zobrist.key, depth, alpha, beta);
	if (usable && !pv_node && tt.best_move != skip)
		return tt.current_evaluation;



	// Static evaluation of the position
	int static_eval = eval.evaluate(board);

	// Razoring
	// Prune bad looking positions close to the horizon by dropping to Quiescence immediately.
	// Seems to have a negligible effect.
	/*if (!pv_node && depth <= 2 && !in_check && !mate(alpha) && static_eval + 200 * depth <= alpha) {
		if (quiescence_search(board, alpha, alpha + 1, 0) <= alpha)
			return alpha;
	}*/

	// Reverse Futility Pruning
	// The position is really bad for the opponent by a big margin, pruning this node is probably safe
	if (!pv_node && !in_check && depth < 10 && !mate(beta) && static_eval - 60 * depth >= beta)
		return beta;

	// Null Move Pruning
	// If there is a beta cutoff, even if we skip our turn (permitting the opponent to play two moves in a row),
	// the position is so terrible for the opponent that we can just prune the whole branch.
	// (should not be used in complicated endgames and zugzwang positions!!!)
	if (!pv_node && allow_null_move && !in_check && ply > 0 && depth >= 3 &&
	    board.non_pawn_material[board.side_to_move] && static_eval >= beta) {

		unsigned ep_square = board.make_null_move();

		unsigned reduction = 3;
		evaluation = -search(board, depth - 1 - reduction, ply + 1, -beta, -beta + 1, INVALID_MOVE, false);

		board.unmake_null_move(ep_square);

		if (evaluation >= beta && !mate(evaluation)) {
			statistics.null_cuts++;
			//tt.store(board.zobrist.key, depth, beta, INVALID_MOVE, LOWERBOUND);
			return beta;
		}
	}

	// Futility Pruning
	// Very close to the horizon of the search, where we are in a position that is much worse than alpha,
	// it is wise to skip moves that do not improve the situation.
	bool futile = false;
	if (!pv_node && depth <= 4 && !in_check && !mate(alpha) && !mate(beta))
		futile = static_eval + futility_margin[depth] <= alpha;

	// Internal Iterative Deepening
	/*if (pv_node && depth >= 6 && tt.best_move == INVALID_MOVE) {
		search(board, depth - 2, ply, alpha, beta, true);
		tt.probe(board.zobrist.key, depth, alpha, beta);
		heuristics.hash_move = tt.best_move;
	}*/

	Move_orderer move_orderer { board, tt.best_move, heuristics, ply };
	move_orderer.stage = (in_check) ? IN_CHECK_TRANSPOSITION : TRANSPOSITION;
	Move move;
	while ((move = move_orderer.next_move(board)) != INVALID_MOVE) {
		if (!board.legal(move) || move == skip) continue;

		move_count++;
		statistics.search_nodes++;


		board.make_move(move);

		bool gives_check = board.in_check();

		// Futility prune if the conditions are met
		if (futile && move_count > 1 && !gives_check && !capture(move) && !promotion(move)) {
			board.unmake_move(move);
			continue;
		}

		//if (depth <= 5 && (flags_of(move) == CAPTURE || gives_check) && see(board, move) <= -200) {
		//	board.unmake_move(move);
		//	continue;
		//}

		// Search extensions make the program spend more time in important positions
		unsigned extension = 0;

		if (gives_check && see(board, move) >= 0) extension = 1;

		bool late_move = (move_count >= 4 && !extension && !in_check && !gives_check && !mate(alpha) &&
		     		  !capture(move) && !promotion(move) && !board.passed_push(move));
		// Late Move Pruning
		//if (late_move && depth <= 3 && move_count >= lmp_margins[depth]) {
		//	board.unmake_move(move);
		//	continue;
		//}


		// Principle Variation Search
		// Search the best looking move with a full alpha-beta-window and prove that all other moves are worse
		// by searching them with a zero-width window centered around alpha, which is a lot faster.
		if (move_count == 1) {
			//if (move == tt.best_move && depth >= 8 && move != skip && extension == 0) {
			//      board.unmake_move(move);
			//      int singular_score = tt.current_evaluation - 130;

			//      evaluation = search(board, (depth - 1) / 2, ply + 1, singular_score, singular_score + 1, move, true);

			//      if (evaluation <= singular_score)
			//      	extension = 1;
			//      board.make_move(move);
			///

			evaluation = -search(board, depth - 1 + extension, ply + 1, -beta, -alpha, INVALID_MOVE, true);
		}
		else {
			// Late Move Reduction
			// Assuming our move ordering is doing a good job, only the first
			// moves are actually good and should thus be searched deeper than other moves.

			//evaluation = -search(board, depth - 1, ply + 1, -alpha - 1, -alpha, INVALID_MOVE, true);
			//if (evaluation > alpha && evaluation < beta)
			//	// If a move happens to be better, we need to re-search it with full window
			//	evaluation = -search(board, depth - 1, ply + 1, -beta, -alpha, INVALID_MOVE, true);

			unsigned reduction = 0;
			if (depth >= 3 && late_move) {
				reduction = 1;
				//reduction = std::min(2, int(depth / 4)) + unsigned(move_count / 12);
			}

			evaluation = -search(board, depth - reduction + extension - 1, ply + 1, -alpha - 1, -alpha, INVALID_MOVE, true);

			if (reduction && evaluation > alpha)
				// If the reduced search indicates an improvement, it needs a re-search to the full depth
				evaluation = -search(board, depth + extension - 1, ply + 1, -beta, -alpha, INVALID_MOVE, true);

			else if (evaluation > alpha && evaluation < beta)
				// If a move happens to be better, we need to re-search it with full window
				evaluation = -search(board, depth + extension - 1, ply + 1, -beta, -alpha, INVALID_MOVE, true);
		}

		board.unmake_move(move);

		if (abort_search) return 0;

		if (evaluation > alpha) {
			// Found a better move
			best_move = move;
			alpha = evaluation;

			if (evaluation >= beta) {
				// Beta cutoff. There is a better line for the opponent.
				// We know the opponent can get at least beta, so a branch that evaluates to more than beta
				// is irrelevant to search, since a better alternative for the opponent has alrady been found,
				// where he can get at least beta.

				// We have not looked at every move, since we pruned this node. That means, we do not have an exact evaluation,
				// we only know that it is good enough to cause a beta-cutoff. It can still be stored as a LOWERBOUND though!
				tt.store(board.zobrist.key, depth, beta, best_move, LOWERBOUND, age);

				if (!capture(move)) {
					// this is a killer move - Store it!
					if (move != heuristics.killer_move[0][ply]) {
						heuristics.killer_move[1][ply] = heuristics.killer_move[0][ply];
						heuristics.killer_move[0][ply] = move;
					}
					// The move can also be stored as a counter, because it refuted the previous move.
					if (board.game_ply > 0) {
						unsigned to = move_to(board.history[board.game_ply - 1].move);
						heuristics.counter_move[board.board[to]][to] = move;
					}

					// increment history score
					heuristics.history[board.board[move_from(move)]][move_to(move)] += depth * depth;
					/*if (heuristics.history[board.board[move_from(move)]][move_to(move)] > 100000) {
						std::cerr << "decay history\n";
						for (unsigned piece = 0; piece < 16; piece++) {
							for (unsigned square = 0; square < 64; square++) {
								heuristics.history[piece][square] >>= 6;
							}
						}
					}*/
				}
				if (move_count == 1) statistics.cutoffspv++;
				statistics.cutoffs++;

				// *snip* *snip*
				return beta;
			}

			// We have had an alpha improvement, we now know an exact score of the position.
			// This is valuable information for our hash table.
			flag = EXACT;

			if (ply == 0) {
				best_root_move = best_move;
				root_evaluation = evaluation;
			}
		}
	}

	if (move_count == 0) {
		if (in_check) return -MATE_SCORE + ply; // Checkmate
		else return 0; // Stalemate
	}
	
	// Store position in the hash table
	tt.store(board.zobrist.key, depth, alpha, best_move, flag, age);

	return alpha;
}

void Search::start_search(Board &board)
{
	abort_search = false;
	age++;
	age %= 64; // fits in 6 bits (important for the hash table)

	time_start = std::chrono::high_resolution_clock::now();

	statistics = {};
	heuristics = {};
	long nodes_previous_iteration;
	long total_nodes = 0;

	int alpha = -INFINITY;
	int beta = INFINITY;
	int window_size = 0;

	// Iterative Deepening
	// We want to be able to stop the search at any moment, so we start with a one depth search and
	// then increment the depth by one and search again for each iteration. We can stop the search
	// at any time and will be able to use the best move from the last fully searched iteration.
	// This process is unintuitively faster than a simple search to a fixed depth, because of dynamic
	// move ordering techniques.

	for (current_depth = 1; current_depth <= max_depth; current_depth++) {

		nodes_previous_iteration = statistics.search_nodes + statistics.quiescence_nodes;
		statistics.search_nodes = 0;
		statistics.quiescence_nodes = 0;

		// Aspiration Windows
		// Inside Iterative Deepening, the evaluation of the previous iteration is often close
		// to the next evaluation. We can narrow the window around this guess to reduce the
		// tree size. However, if we are wrong, we need to re-search with a broader window.

		// Set the alpha-beta-window close to where we expect the next evaluation
		if (current_depth >= 4) {
			window_size = 14;
			alpha = root_evaluation - window_size;
			beta = root_evaluation + window_size;
		}

		while (true) {
			int evaluation = search(board, current_depth, 0, alpha, beta, INVALID_MOVE, true);
			total_nodes += statistics.search_nodes + statistics.quiescence_nodes;

			if (time_management) {
				if (evaluation <= alpha && time_elapsed() >= soft_time_cap)
					soft_time_cap = time_elapsed() + search_time_increment;
			}
			if (time_management && time_elapsed() >= soft_time_cap) abort_search = true;
			if (abort_search) break;

			// The returned evaluation was not inside the windows :/
			// A costly re-search has to be done with a wider window.
			if (evaluation <= alpha) {
				window_size += window_size;
				alpha -= window_size / 2;
			}
			else if (evaluation >= beta) {
				window_size += window_size;
				beta += window_size / 2;
			}
			else break;
		}


		plot_info(board, nodes_previous_iteration);

		if (time_management && mate(root_evaluation)) break;

		if (abort_search) break;
	}

	plot_final_info(total_nodes);
}

void Search::think(Board &board, unsigned move_time, unsigned w_time, unsigned b_time, unsigned w_inc, unsigned b_inc)
{
	unsigned time_left = board.side_to_move == WHITE ? w_time : b_time;

	if (time_management) {

		Move_list move_list;
		generate_legal(board, move_list);

		unsigned increment = board.side_to_move == WHITE ? w_inc  : b_inc;

		unsigned move_overhead = 10;

		// Assume the game will last another n moves on average. Of course, the game will likely last for more than
		// these n moves, but this will make the engine spend more time in the deciding early stages of the game.
		unsigned remaining_moves = 40;

		// Add the total time for increments and make sure, we have a
		// safety move overhead buffer in case of delays.
		time_left = time_left - move_overhead + increment * remaining_moves;

		soft_time_cap = std::max(1U, unsigned(time_left / remaining_moves * 0.6));
		hard_time_cap = std::min(soft_time_cap * 5, time_left / 4);
		search_time_increment = (hard_time_cap - soft_time_cap) / 40;

		// Respond instantly in case of a single legal move.
		if (move_list.size == 1)
			hard_time_cap = 1;

		else if (fixed_time)
			hard_time_cap = std::min(hard_time_cap, move_time);
	}
	else if (fixed_time)
		hard_time_cap = move_time;

	start_search(board);
}

unsigned Statistics::hash_full(Transposition_table &tt)
{
	unsigned hits = 0;
	for (unsigned index = 0; index < 1000; index++) {
		TT_bucket &bucket = tt.buckets[index];
		if (bucket.entries[0].key != 0ULL) hits++;
	}
	return hits;
}

// The best sequence of moves can be extracted from the hash table.
void Search::extract_pv_line(Board &board)
{
	Board temp_board = board;
	temp_board.make_move(best_root_move);
	std::cout << move_string(best_root_move) << " ";
	for (unsigned depth = current_depth - 1; depth > 0; depth--) {
		bool hit = tt.probe(temp_board.zobrist.key, depth, -INFINITY, INFINITY);
		if (!hit) return;
		std::cout << move_string(tt.best_move) << " ";
		temp_board.make_move(tt.best_move);
	}
}

void Search::plot_info(Board &board, unsigned nodes_previous_iteration)
{
	unsigned nodes = statistics.search_nodes + statistics.quiescence_nodes;
	if (current_depth > 1) statistics.branching_factor = nodes / double(nodes_previous_iteration) + 0.0001;

	std::cout << "info depth " << current_depth << " score cp " << root_evaluation << " time " << time_elapsed();
	std::cout << " nodes " << nodes << " snodes " << statistics.search_nodes << " qnodes " << statistics.quiescence_nodes;
	std::cout << " branching " << statistics.branching_factor << " hashfull " << statistics.hash_full(tt);
	std::cout << " pv ";
	extract_pv_line(board);
	std::cout << "\n";
}

void Search::plot_final_info(unsigned total_nodes)
{
	std::cerr << "cut offs " << statistics.cutoffs << " pv " << double(statistics.cutoffspv) / double(statistics.cutoffs) << "\n";
	std::cerr << "null cuts " << statistics.null_cuts << "\n";
	std::cerr << "info total nodes " << total_nodes << "\n";
	std::cout << "bestmove " << move_string(best_root_move) << "\n";
}

