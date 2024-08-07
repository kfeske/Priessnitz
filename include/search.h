#include <time.h>
#include <evaluation.h>
#include <transposition_table.h>
#include <see.h>
#include <move_ordering.h>

struct Search : Noncopyable
{
	Evaluation eval;
	Move best_root_move = INVALID_MOVE;
	int16_t root_evaluation = 0;

	unsigned max_depth = 63;
	double max_time = 999999;
	unsigned current_depth;
	int const mate_score = 31000;
	int const infinity = 32000;

	long search_nodes = 0;
	long quiescence_nodes = 0;
	double branching_factor = 0;
	long cutoffs = 0;
	long cutoffspv = 0;
	long null_cuts = 0;
	//long tt_cutoffs = 0;

	Heuristics heuristics;

	TranspositionTable<(16 * 1024 * 1024) / sizeof(TTEntry)> tt;

	double time_start;
	bool abort_search;

	int futility_margin[5] = {
		0, 100, 200, 300, 400
	};

	unsigned lmp_margins[4] = {
		0, 8, 12, 16
	};

	void reset()
	{
		max_depth = 63;
		max_time = 999999;
		search_nodes = 0;
		quiescence_nodes = 0;
		branching_factor = 0;
		cutoffs = 0;
		cutoffspv = 0;
		null_cuts = 0;
		heuristics = {};
		for (TTEntry &entry : tt.entries) entry = {};
	}

	bool mate(int score)
	{
		return (abs(score) >= mate_score - 100);
	}

	// Quiescence Search is a shallower search, generating only
	// captures and check evasions to avoid the horizon effect
	int qsearch(Board &board, int alpha, int beta, unsigned ply)
	{
		bool in_check = board.in_check() && ply <= 2;

		int static_evaluation = eval.evaluate(board);

		if (!in_check && static_evaluation >= beta)
			return beta;


		if (static_evaluation > alpha)
			alpha = static_evaluation;

		MoveGenerator move_generator {};
		if (in_check)
			move_generator.generate_all_moves(board);
		else
			move_generator.generate_quiescence(board); // only captures

		rate_moves(board, heuristics, move_generator, true, ply);

		for (unsigned n = 0; n < move_generator.size; n++) {
			Move move = next_move(move_generator, n);

			// Delta Pruning
			// if this move is unlikely to be a good capture, because it will not improve alpha enough, it is pruned
			int gain = abs(piece_value(board.board[move_to(move)], MIDGAME));
			if (static_evaluation + gain + 200 <= alpha && !in_check && !promotion(move))
				continue;

			if (see(board, move) < 0) continue;

			quiescence_nodes++;


			board.make_move(move);
	
			int evaluation = -qsearch(board, -beta, -alpha, ply + 1);
	
			board.unmake_move(move);
	
			if (evaluation >= beta)
				return beta;
	
			if (evaluation > alpha)
				alpha = evaluation;
		}
		return alpha;
	}

	// function that finds the best move at a fixed depth (number of moves, it looks into the future)
	// alpha is the best score, the side to move can guarantee in a sequence of moves
	// beta is the best score, the opponent can guarantee in the sequence
	int search(Board &board, int depth, unsigned ply, int alpha, int beta, bool allow_null_move)
	{
		if ((search_nodes & 2047) == 0 && SDL_GetTicks() - time_start > max_time && max_time != 0)
		{
			// Time's up!
			abort_search = true;
			return 0;
		}

		// check for draws by repetition or by 50 Move Rule
		if (board.immediate_draw(ply))
			return 0;

		if (depth <= 0)
			// we reached a leaf node, start the Quiescence Search
			return qsearch(board, alpha, beta, 0);

		Move best_move = INVALID_MOVE;
		int evaluation;
		bool in_check = board.in_check();
		bool pv_node = beta - alpha != 1;

		// if no move exceeds alpha, we do not have an exact evaluation,
		// we only know that none of our moves can improve it. It can still be stored as an UPPERBOUND though!
		TTEntryFlag flag = UPPERBOUND;

		// updated in the tt.probe() function
		tt.pv_move = INVALID_MOVE;

		// check for any transpositions at higher or equal depths
		bool usable = tt.probe(board.zobrist.key, depth, alpha, beta);
		if (usable && !pv_node)
			return tt.current_evaluation;

		// in case of a transposition at a lower depth, we can still use the best move in our move ordering
		heuristics.hash_move = tt.pv_move;


		// static evaluation of the position
		int static_eval = eval.evaluate(board);

		// Razoring
		// Prune bad looking positions close to the horizon by dropping to Quiescence immediately.
		// Seems to have a negligible effect.
		/*if (!pv_node && depth <= 2 && !in_check && !mate(alpha) && static_eval + 200 * depth <= alpha) {
			if (qsearch(board, alpha, alpha + 1, 0) <= alpha)
				return alpha;
		}*/

		// Reverse Futility Pruning
		// the position is really bad for the opponent by a big margin, pruning this node is probably safe
		if (!pv_node && !in_check && depth < 10 && !mate(beta) && static_eval - 60 * depth >= beta)
			return beta;

		// Null Move Pruning
		// if there is a beta cutoff, even if we skip our turn (permitting the opponent to play two moves in a row),
		// the position is so terrible for the opponent that we can just prune the whole branch
		// may cause search instability, but it is worth the risk. If we want the speed, we have to live in fear

		// (should not be used in complicated endgames and zugzwang positions!!!)
		if (!pv_node && allow_null_move && !in_check && ply > 0 && depth >= 3 &&
		    board.non_pawn_material[board.side_to_move] && static_eval >= beta) {

			unsigned ep_square = board.make_null_move();

			unsigned reduction = 3;
			evaluation = -search(board, depth - 1 - reduction, ply + 1, -beta, -beta + 1, false);

			board.unmake_null_move(ep_square);

			if (evaluation >= beta && !mate(evaluation)) {
				null_cuts++;
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
		/*if (pv_node && depth >= 6 && tt.pv_move == INVALID_MOVE) {
			search(board, depth - 2, ply, alpha, beta, true);
			tt.probe(board.zobrist.key, depth, alpha, beta);
			heuristics.hash_move = tt.pv_move;
		}*/

		MoveGenerator move_generator {};
		move_generator.generate_all_moves(board);

		if (move_generator.size == 0) {
			if (in_check) return -mate_score + ply; // Checkmate
			else return 0; // Stalemate
		}

		// score each move depending on how good it looks
		rate_moves(board, heuristics, move_generator, false, ply);

		for (unsigned n = 0; n < move_generator.size; n++) {
			search_nodes++;

			Move move = next_move(move_generator, n);

			board.make_move(move);

			bool gives_check = board.in_check();

			// Futility prune if the conditions are met
			if (futile && n > 0 && !gives_check && flags_of(move) != CAPTURE && !promotion(move)) {
				board.unmake_move(move);
				continue;
			}

			// Late Move Pruning
			if (!pv_node && depth <= 3 && !gives_check && !mate(alpha) && n >= lmp_margins[depth]
			    && flags_of(move) != CAPTURE && !promotion(move)) {
				board.unmake_move(move);
				continue;
			}

			// search extensions make the program spend more time in important positions
			unsigned extension = 0;

			if (gives_check && see(board, move) >= 0) extension = 1;

			// Principle Variation Search
			// Search the best looking move with a full alpha-beta-window and prove that all other moves are worse
			// by searching them with a zero-width window centered around alpha, which is a lot faster.
			if (n == 0) {
				evaluation = -search(board, depth - 1 + extension, ply + 1, -beta, -alpha, true);
			}
			else {
				// Late Move Reduction
				// Assuming our move ordering is doing a good job, only the first
				// moves are actually good and should thus be searched to full depth.

				unsigned reduction = 0;
				if (n >= 4 && depth >= 3 && !in_check && flags_of(move) != CAPTURE && !promotion(move) && !board.passed_push(move)) {
					reduction = 1;
					//reduction = std::min(2, int(depth / 4)) + unsigned(n / 12);
				}

				evaluation = -search(board, depth - reduction + extension - 1, ply + 1, -alpha - 1, -alpha, true);

				if (reduction && evaluation > alpha)
					// if the reduced search does not fail low, it needs a re-search to the full depth
					evaluation = -search(board, depth + extension - 1, ply + 1, -beta, -alpha, true);

				else if (evaluation > alpha && evaluation < beta)
					// if a move happens to be better, we need to re-search it with full window
					evaluation = -search(board, depth + extension - 1, ply + 1, -beta, -alpha, true);
			}

			board.unmake_move(move);

			if (abort_search) return 0;

			if (evaluation > alpha) {
				// found a better move
				best_move = move;
				alpha = evaluation;

				if (evaluation >= beta) {

					// Beta cutoff. There is a better line for the opponent.
					// We know the opponent can get at least beta, so a branch that evaluates to more than beta
					// is irrelevant to search, since a better alternative for the opponent has alrady been found,
					// where he can get at least beta.

					// we have not looked at every move, since we pruned this node. That means, we do not have an exact evaluation,
					// we only know that it is good enough to cause a beta-cutoff. It can still be stored as a LOWERBOUND though!
					tt.store(board.zobrist.key, depth, beta, best_move, LOWERBOUND);

					if (flags_of(move) != CAPTURE) {
						// this is a killer move - Store it!
						heuristics.killer_move[1][ply] = heuristics.killer_move[0][ply];
						heuristics.killer_move[0][ply] = move;

						// increment history score
						heuristics.history[board.board[move_from(move)]][move_to(move)] += depth * depth;
					}
					if (n == 0) cutoffspv++;
					cutoffs++;

					// *snip*
					return beta;
				}

				flag = EXACT;

				if (ply == 0) {
					best_root_move = best_move;
					root_evaluation = evaluation;
				}
			}
		}
		
		// store position in the hash table
		tt.store(board.zobrist.key, depth, alpha, best_move, flag);

		return alpha;
	}

	void start_search(Board &board)
	{
		abort_search = false;

		time_start = SDL_GetTicks();
		long nodes_previous_iteration;
		long total_nodes = 0;

		int alpha = -infinity;
		int beta = infinity;
		int window_size = 0;

		// Iterative Deepening
		// We want to be able to stop the search at any moment, so we start with a one depth search and
		// then increment the depth by one and search again for each iteration. We can stop the search
		// at any time and will be able to use the best move from the last fully searched iteration.
		// This process is unintuitively faster than a simple search to a fixed depth, because of dynamic
		// move ordering techniques.

		for (current_depth = 1; current_depth <= max_depth; current_depth++) {

			nodes_previous_iteration = search_nodes + quiescence_nodes;
			search_nodes = 0;
			quiescence_nodes = 0;

			// Aspiration Windows
			// inside Iterative Deepening, the evaluation of the previous iteration is often close
			// to the next evaluation. We can narrow the window around this guess to reduce the
			// tree size. However, if we are wrong, we need to re-search with a broader window.

			// set the alpha-beta-window close to where we expect the next evaluation
			if (current_depth >= 4) {
				window_size = 14;
				alpha = root_evaluation - window_size;
				beta = root_evaluation + window_size;
			}

			while (true) {
				int evaluation = search(board, current_depth, 0, alpha, beta, true);
				total_nodes += search_nodes + quiescence_nodes;

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


			if (current_depth > 1) branching_factor = (search_nodes + quiescence_nodes) / double(nodes_previous_iteration) + 0.0001;

			std::cout << "info depth " << current_depth << " score cp " << root_evaluation << " time " << SDL_GetTicks() - time_start;
			std::cout << " nodes " << search_nodes + quiescence_nodes << " snodes " << search_nodes << " qnodes " << quiescence_nodes;
			std::cout << " branching " << branching_factor;
			std::cout << " pv " << move_string(best_root_move) << "\n";

			if (abort_search) break;
		}

		std::cerr << "cut offs " << cutoffs << " pv " << double(cutoffspv) / double(cutoffs) << "\n";
		std::cerr << "null cuts " << null_cuts << "\n";
		std::cerr << "info total nodes " << total_nodes << "\n";
		std::cout << "bestmove " << move_string(best_root_move) << "\n";
		reset(); // make sure to clear all search data to avoid them affecting the next search
	}
};
