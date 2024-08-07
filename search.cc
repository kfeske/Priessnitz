#include <chrono>

#include "search.h"

#include "board.h"
#include "move_generator.h"
#include "transposition_table.h"
#include "see.h"
#include "move_ordering.h"

bool mate(int score)
{
	return (abs(score) >= MATE_SCORE - 100);
}

void Search::reset()
{
	best_root_move = INVALID_MOVE;
	root_evaluation = 0;
	heuristics = {};
	statistics = {};
	age = 0;
	tt.resize(DEFAULT_TT_SIZE);
	abort_search = false;
}

double Search::time_elapsed()
{
	auto time_end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
}

bool Search::crossed_hard_time_limit()
{
	return (current_depth > 1 && (fixed_time || time_management) &&
		(statistics.search_nodes & 1024) == 0 && time_elapsed() >= hard_time_cap);
}

// Quiescence Search is a shallower search, generating only
// captures and check evasions to avoid the horizon effect
int Search::quiescence_search(Board &board, int alpha, int beta, unsigned ply)
{
	TT_entry &tt_entry = tt.probe(board.zobrist.key);
	if (tt.hit) {
		if (tt_entry.flag == EXACT) return tt_entry.evaluation;
		if (tt_entry.flag == LOWERBOUND && tt_entry.evaluation >= beta ) return beta;
		if (tt_entry.flag == UPPERBOUND && tt_entry.evaluation <= alpha) return alpha;
	}

	int static_evaluation = eval.evaluate(board);

	bool in_check = board.in_check();

	// Captures are usually not forced. This will give the option to stop the capture sequence.
	if (!in_check && static_evaluation >= beta)
		return beta;

	// Again, we should at least be able to achieve the static evaluation.
	if (static_evaluation > alpha)
		alpha = static_evaluation;

	Move best_move = INVALID_MOVE;
	TT_flag flag = UPPERBOUND;

	Move_orderer move_orderer { board, INVALID_MOVE, heuristics, 0 };
	move_orderer.stage = (in_check) ? GENERATE_IN_CHECKS : GENERATE_QUIESCENCES;

	Move move;
	while ((move = move_orderer.next_move(board, false)) != INVALID_MOVE) {
		if (!board.legal(move)) continue;

		// Prune moves that lose material. In this case, another quiet move is probably better.
		if (!in_check && see(board, move) < 0) continue;

		statistics.quiescence_nodes++;

		board.make_move(move);

		int evaluation = -quiescence_search(board, -beta, -alpha, ply + 1);

		board.unmake_move(move);


		if (evaluation > alpha) {
			alpha = evaluation;
			best_move = move;

			if (evaluation >= beta) {
				tt.store(board.zobrist.key, 0, beta, best_move, static_evaluation, LOWERBOUND, age);
				return beta;
			}

			flag = EXACT;
		}
	}

	tt.store(board.zobrist.key, 0, alpha, best_move, static_evaluation, flag, age);

	return alpha;
}

// Function that finds the best move at a fixed depth (number of moves, it looks into the future)
// Alpha is the best score, the side to move can guarantee in a sequence of moves
// Beta is the best score, the opponent can guarantee in the sequence
int Search::search(Board &board, int depth, int ply, int alpha, int beta, Move skip, bool allow_null_move)
{
	if (crossed_hard_time_limit())
	{
		// Time's up! Abort the search immediately!
		abort_search = true;
		return 0;
	}

	// Check for draws by repetition, 50 Move Rule or insufficient material
	if (ply > 0 && board.immediate_draw(ply))
		return DRAW_SCORE;

	if (depth <= 0)
		// We reached a leaf node, start the Quiescence Search
		return quiescence_search(board, alpha, beta, 0);

	// Mate Distance Pruning
	// If a forced mate was found, we do not need to search deeper than to where it was found,
	// because we only care about the shortest mate.
	alpha = std::max(alpha, -MATE_SCORE + ply);
	beta  = std::min(beta,   MATE_SCORE - ply);
	if (alpha >= beta) return alpha;
	
	bool pv_node = beta - alpha != 1;

	// Check for any transpositions at higher or equal depths
	TT_entry &tt_entry = tt.probe(board.zobrist.key);
	Move hash_move = Move(tt_entry.best_move);
	if (tt.hit && !pv_node && tt_entry.depth >= depth && hash_move != skip) {
		// we have an exact score for that position. Great!
		// (that means, we searched all moves and received a new best move)
		if (tt_entry.flag == EXACT)
			return tt_entry.evaluation;

		// this value is too high for us to be concered about, it will cause a beta-cutoff
		if (tt_entry.flag == LOWERBOUND && tt_entry.evaluation >= beta)
			return beta;

		// this value is too low, we will not exceed alpha in the search
		if (tt_entry.flag == UPPERBOUND && tt_entry.evaluation <= alpha)
			return alpha;
	}

	// Static evaluation of the position
	int static_eval = (tt.hit) ? tt_entry.static_evaluation : eval.evaluate(board);

	int move_count = 0;
	Move best_move = INVALID_MOVE;
	int evaluation;
	bool in_check = board.in_check();
	bool skip_quiets = false;

	// If no move exceeds alpha, we do not have an exact evaluation,
	// we only know that none of our moves can improve it. It can still be stored as an UPPERBOUND though!
	TT_flag flag = UPPERBOUND;


	// Razoring
	// Prune bad looking positions close to the horizon by testing, if a Quiescence Search can improve them.
	int razor_alpha = alpha - search_constants.RAZOR_MARGIN;
	if (!pv_node && depth == 1 && static_eval < razor_alpha && !in_check && !mate(alpha)) {
		if (quiescence_search(board, razor_alpha, razor_alpha + 1, 0) <= razor_alpha)
			return alpha;
	}

	// Reverse Futility Pruning
	// The position is really bad for the opponent by a big margin, pruning this node is probably safe
	if (!pv_node && !in_check && depth < 10 && !mate(beta) && static_eval - search_constants.REVERSE_FUTILITY_MARGIN[depth] >= beta)
		return beta;

	// Null Move Pruning
	// If there is a beta cutoff, even if we skip our turn (permitting the opponent to play two moves in a row),
	// the position is so terrible for the opponent that we can just prune the whole branch.
	// (should not be used in complicated endgames and zugzwang positions!!!)
	if (!pv_node && allow_null_move && !in_check && ply > 0 && depth > 1 &&
	    board.non_pawn_material[board.side_to_move] && static_eval >= beta) {

		unsigned ep_square = board.make_null_move();

		unsigned reduction = 2 + depth / 3;
		evaluation = -search(board, depth - 1 - reduction, ply + 1, -beta, -beta + 1, INVALID_MOVE, false);

		board.unmake_null_move(ep_square);

		if (evaluation >= beta && !mate(evaluation)) {
			statistics.null_cuts++;
			//tt.store(board.zobrist.key, depth, beta, INVALID_MOVE, LOWERBOUND);
			return beta;
		}
	}

	bool futile = false;
	if (!pv_node && depth < 7 && !in_check)
		futile = static_eval + search_constants.FUTILITY_MARGIN[depth] <= alpha;

	// Internal Iterative Deepening
	if (pv_node && depth > 10 && hash_move == INVALID_MOVE) {
		search(board, depth - 10, ply, alpha, beta, INVALID_MOVE, true);
		TT_entry &tt_entry = tt.probe(board.zobrist.key);
		hash_move = Move(tt_entry.best_move);
	}

	// In the end, we decrement the history counters for bad quiet moves.
	Move_list bad_quiets_searched;

	Move_orderer move_orderer { board, hash_move, heuristics, ply };
	move_orderer.stage = (in_check) ? IN_CHECK_TRANSPOSITION : TRANSPOSITION;
	Move move;
	while ((move = move_orderer.next_move(board, skip_quiets)) != INVALID_MOVE) {
		if (!board.legal(move) || move == skip) continue;

		move_count++;
		statistics.search_nodes++;

		bool quiet_move = !capture(move) && !promotion(move);

		// Forward Pruning at low depths
		if (!in_check && move_count > 1 && !mate(alpha)) {

			// Late Move Pruning
			// Due to our move ordering, late moves are likely bad and not worth searching.
			if (depth < 4 && move_count >= search_constants.LMP_MARGIN[depth])
				skip_quiets = true;

			// Futility Pruning
			// Very close to the horizon of the search, where we are in a position that looks really bad,
			// it is wise to skip quiet moves that will likely not improve the situation.
			if (futile && quiet_move)
				continue;

			// SEE Pruning
			// Skip moves that lose material at low depths
			if (move_orderer.stage > GOOD_CAPTURES && depth < 6 && !promotion(move) && see(board, move) < -20 * depth)
				continue;
		}

		// Search extensions make the program spend more time in important positions
		unsigned extension = 0;

		// Singular Extensions
		// Extend, if the hash move is better than all the other moves
		if (ply > 0 && move == tt_entry.best_move && depth > 8 && move != skip && tt_entry.depth >= depth - 3 && tt_entry.flag == LOWERBOUND) {
			int singular_beta = tt_entry.evaluation - 2 * depth;

			evaluation = search(board, (depth - 1) / 2, ply + 1, singular_beta, singular_beta + 1, move, true);

			if (evaluation <= singular_beta)
			extension = 1;
		}
			

		board.make_move(move);

		bool gives_check = board.in_check();


		if (gives_check && see(board, move) >= 0) extension = 1;


		// Principle Variation Search
		// Search the best looking move with a full alpha-beta-window and prove that all other moves are worse
		// by searching them with a zero-width window centered around alpha, which is a lot faster.
		if (move_count == 1) {

			evaluation = -search(board, depth - 1 + extension, ply + 1, -beta, -alpha, INVALID_MOVE, true);
		}
		else {
			// Late Move Reduction
			// Assuming our move ordering is doing a good job, only the first
			// moves are actually good and should thus be searched deeper than other moves.

			int reduction = 0;
			if (quiet_move && depth > 1 && move_count > 3 && !in_check) {
				reduction = search_constants.LATE_MOVE_REDUCTION[std::min(63, depth)][std::min(63, move_count)];
				if (!pv_node) reduction++;
				reduction -= heuristics.history[board.board[move_from(move)]][move_to(move)] / 5000;
				reduction = std::min(depth - 1, std::max(reduction, 0));
			}

			evaluation = -search(board, depth - reduction + extension - 1, ply + 1, -alpha - 1, -alpha, INVALID_MOVE, true);

			// If the reduced search indicates an improvement, it needs a re-search to the full depth
			if (reduction && evaluation > alpha)
				evaluation = -search(board, depth + extension - 1, ply + 1, -beta, -alpha, INVALID_MOVE, true);

			// If a move happens to be better, we need to re-search it with full window
			// This is not done in non-PV nodes, because any alpha increase will cause a beta cutoff there.
			else if (pv_node && evaluation > alpha)
				evaluation = -search(board, depth + extension - 1, ply + 1, -beta, -alpha, INVALID_MOVE, true);
		}

		board.unmake_move(move);

		if (abort_search) return 0;

		// Improve alpha, if we have found a better move
		if (evaluation > alpha) {
			best_move = move;
			alpha = evaluation;

			// Beta cutoff.
			// We know the opponent can get at least beta, so a branch that evaluates to more than beta
			// is irrelevant to search, since a better alternative for the opponent has alrady been found,
			// where he can get at least beta.
			if (evaluation >= beta) {

				// We have not looked at every move, since we pruned this node. That means, we do not have an exact evaluation,
				// we only know that it is good enough to cause a beta-cutoff. It can still be stored as a LOWERBOUND though!
				tt.store(board.zobrist.key, depth, beta, best_move, static_eval, LOWERBOUND, age);

				// Update Killers, Counters, History
				update_heuristics(board, move, depth, ply, bad_quiets_searched);

				if (move_count == 1) statistics.cutoffspv++;
				statistics.cutoffs++;

				// *snip* *snip*
				return beta;
			}

			// We have had an alpha improvement, we now know an exact score of the position.
			// This is valuable information for our hash table.
			flag = EXACT;
		}
		else if (!capture(move)) bad_quiets_searched.add(move);
	}

	if (move_count == 0) {
		if (in_check) return -MATE_SCORE + ply; // Checkmate
		else return 0; // Stalemate
	}

	// Store position in the hash table
	tt.store(board.zobrist.key, depth, alpha, best_move, static_eval, flag, age);

	return alpha;
}

void Search::start_search(Board &board)
{
	time_start = std::chrono::high_resolution_clock::now();

	abort_search = false;
	age++;
	age %= 64; // fits in 6 bits (important for the hash table)

	statistics = {};
	long nodes_previous_iteration;
	long total_nodes = 0;

	int evaluation = 0;
	int alpha = -INFINITY_SCORE;
	int beta = INFINITY_SCORE;
	int window_size = 0;

	heuristics = {};
	best_root_move = INVALID_MOVE;
	Move last_best_move = INVALID_MOVE;
	int last_best_move_depth = 0;

	// Iterative Deepening
	// We want to be able to stop the search at any moment, so we start with a one depth search and
	// then increment the depth by one and search again for each iteration. We can stop the search
	// at any time and will be able to use the best move from the last fully searched iteration.
	// This process is unintuitively faster than a simple search to a fixed depth, because of dynamic
	// move ordering techniques.

	for (current_depth = 1; current_depth <= max_depth; current_depth++) {

		last_best_move = best_root_move;
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
			beta  = root_evaluation + window_size;
		}

		while (true) {
			evaluation = search(board, current_depth, 0, alpha, beta, INVALID_MOVE, true);
			total_nodes += statistics.search_nodes + statistics.quiescence_nodes;

			// Stop here, if the allocated time for the move runs out.
			// We continue searching, if we failed low.
			if (time_management && current_depth > 1) {
				double stable_best_move = (last_best_move_depth + 7 < current_depth) ? 0.9 : 1.2;
				if (time_elapsed() >= soft_time_cap * stable_best_move) abort_search = true;
			}
			if (abort_search) break;

			// The returned evaluation was not inside the windows :/
			// A costly re-search has to be done with a wider window.
			if (evaluation <= alpha) {
				alpha -= window_size;
				window_size += window_size / 2;
			}
			else if (evaluation >= beta) {
				beta += window_size;
				window_size += window_size / 2;
			}
			else break;
		}

		if (best_root_move != last_best_move) last_best_move_depth = current_depth;

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
		unsigned expected_time_left = time_left - move_overhead + increment * remaining_moves;

		// The search can abort after an iteration, if the soft time cap has been crossed.
		soft_time_cap = std::max(1U, expected_time_left / remaining_moves);
		soft_time_cap = std::min(soft_time_cap, time_left - move_overhead);

		// The hard time cap aborts the search and is the absolute maximum time, the engine can search.
		hard_time_cap = soft_time_cap * 5;
		hard_time_cap = std::min(hard_time_cap, time_left - move_overhead);

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

void Search::update_heuristics(Board &board, Move best_move, int depth, int ply, Move_list &bad_quiets_searched)
{
	if (!capture(best_move)) {
		// This is a killer move - Store it!
		if (best_move != heuristics.killer_move[0][ply]) {
			heuristics.killer_move[1][ply] = heuristics.killer_move[0][ply];
			heuristics.killer_move[0][ply] = best_move;
		}
		// The move can also be stored as a counter, because it refuted the previous move.
		if (board.game_ply > 0) {
			unsigned to = move_to(board.history[board.game_ply - 1].move);
			heuristics.counter_move[board.board[to]][to] = best_move;
		}

		// Increment history score. Moves closer to the root have a bigger impact (depth * depth).
		increment_history(board, best_move, depth * depth);

	}
	// Decrement history score for all previously searched quiet moves that did not improve alpha.
	for (unsigned n = 0; n < bad_quiets_searched.size; n++)
		increment_history(board, bad_quiets_searched.moves[n].move, -(depth * depth));
}

void Search::increment_history(Board &board, Move move, int bonus)
{
	int32_t &history_score = heuristics.history[board.board[move_from(move)]][move_to(move)];
	// Saturate the counter, so that 16000 is not exceeded.
	history_score += bonus - history_score * bonus / 16000;
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
std::string Search::extract_pv_line(Board &board)
{
	std::string pv_line {};
	Board temp_board = board;
	for (int depth = 0; depth < current_depth; depth++) {
		TT_entry &tt_entry = tt.probe(temp_board.zobrist.key);
		Move tt_move = Move(tt_entry.best_move);
		if (tt_move == INVALID_MOVE) break;
		if (depth == 0) {
			best_root_move = tt_move;
			root_evaluation = tt_entry.evaluation;
		}
		pv_line += move_string(tt_move) + " ";
		temp_board.make_move(tt_move);
	}
	return pv_line;
}

void Search::plot_info(Board &board, unsigned nodes_previous_iteration)
{
	unsigned nodes = statistics.search_nodes + statistics.quiescence_nodes;
	if (current_depth > 1) statistics.branching_factor = nodes / double(nodes_previous_iteration) + 0.0001;
	std::string pv_line = extract_pv_line(board);

	std::cout << "info depth " << current_depth << " score cp " << root_evaluation << " time " << time_elapsed()
		  << " nodes " << nodes << " hashfull " << statistics.hash_full(tt)
	//std::cout << " snodes " << statistics.search_nodes << " qnodes " << statistics.quiescence_nodes;
	//std::cout << " branching " << statistics.branching_factor;
		  << " pv " << pv_line << "\n";
}

void Search::plot_final_info(unsigned total_nodes)
{
	std::cerr << "cut offs " << statistics.cutoffs << " pv " << double(statistics.cutoffspv) / double(statistics.cutoffs) << "\n";
	std::cerr << "null cuts " << statistics.null_cuts << "\n";
	std::cerr << "info total nodes " << total_nodes << "\n";
	std::cout << "bestmove " << move_string(best_root_move) << "\n";
}

