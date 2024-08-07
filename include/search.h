#include <time.h>
#include <evaluation.h>
#include <transposition_table.h>
#include <move_ordering.h>

struct Search : Noncopyable
{
	Evaluation eval;
	Move best_root_move = INVALID_MOVE;
	int16_t root_evaluation = 0;

	unsigned max_depth = 63;
	double max_time = 99999;
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

	void reset()
	{
		max_depth = 63;
		max_time = 99999;
		search_nodes = 0;
		quiescence_nodes = 0;
		branching_factor = 0;
		cutoffs = 0;
		cutoffspv = 0;
		null_cuts = 0;
		heuristics = {};
		for (TTEntry &entry : tt.entries) entry = {};
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
			// Time's up! Use the best move from the previous iteration
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

		// if no move exceeds alpha, we do not have an exact evaluation,
		// we only know that none of our moves can improve it. It can still be stored as an UPPERBOUND though!
		TTEntryFlag flag = UPPERBOUND;

		// updated in the tt.probe() function
		tt.pv_move = INVALID_MOVE;

		// check for any transpositions at higher or equal depths
		bool higher_depth = tt.probe(board.zobrist.key, depth, alpha, beta);
		if (higher_depth && ply > 0)
			return tt.current_evaluation;

		// in case of a transposition at a lower depth, we can still use the best move in our move ordering
		heuristics.hash_move = tt.pv_move;

		bool pv_node = beta - alpha != 1;

		// Null Move Pruning
		// if there is a beta cutoff even if we skip our turn (permitting the opponent to play two moves in a row),
		// the position is so terrible for the opponent that we can just prune the whole branch
		// may cause search instability, but it is worth the risk. If we want the speed, we have to live in fear

		// (should not be used in complicated endgames and zugzwang positions!!!)
		if (!pv_node && allow_null_move && !in_check && ply > 0 && depth >= 3 &&
		    board.non_pawn_material[board.side_to_move]) {

			unsigned ep_square = board.make_null_move();

			unsigned reduction = 3;
			evaluation = -search(board, depth - 1 - reduction, ply + 1, -beta, -beta + 1, false);

			board.unmake_null_move(ep_square);

			if (evaluation >= beta && fabs(evaluation) < mate_score - 100) {
				null_cuts++;
				return beta;
			}
		}

		// Futility Pruning
		// very close to the horizon of the search, where we are in a position that is much worse than alpha,
		// it is wise to prune this node completely and not waste our time on futile positions
		bool futile = false;
		if (!pv_node && depth <= 4 && !in_check && alpha < mate_score - 100 && beta < mate_score - 100) {
			int static_evaluation = eval.evaluate(board);
			futile = static_evaluation + futility_margin[depth] <= alpha;
		}
		
		//if (in_check) depth++;

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

			// Futility prune if the conditions are met
			if (futile && n > 0 && !board.in_check() && flags_of(move) != CAPTURE && !promotion(move)) {
				board.unmake_move(move);
				continue;
			}

			// Principle Variation Search

			/*if (n == 0)
				// search pv move with full alpha-beta window
				// it is very likely the best move
				evaluation = -search(board, depth - 1, ply + 1, -beta, -alpha, true);
			else {
				// search remaining moves with null window to prove that they are worse than the pv move
				evaluation = -search(board, depth - 1, ply + 1, -alpha - 1, -alpha, true);

				if (evaluation > alpha && evaluation < beta)
					// if a move happens to be better, we need to re-search with a full window
					evaluation = -search(board, depth - 1, ply + 1, -beta, -alpha, true);
			}*/

			//evaluation = -search(board, depth - 1, ply + 1, -beta, -alpha);

			if (n == 0) {
				// search pv move with full alpha-beta window
				// it is very likely the best move
				evaluation = -search(board, depth - 1, ply + 1, -beta, -alpha, true);
			}
			else {
				// search remaining moves with null window to prove that they are worse than the pv move

				// Late Move Reduction - assuming our move orderer is doing a good job, only the first
				// moves are actually good and thus should be searched to full depth
				unsigned reduction = 0;
				if (n >= 4 && depth >= 3 && !in_check && flags_of(move) != CAPTURE) {
					reduction = 1;
					//reduction = std::min(2, int(depth / 4)) + unsigned(n / 12);
				}

				evaluation = -search(board, depth - reduction - 1, ply + 1, -alpha - 1, -alpha, true);

				if (reduction && evaluation > alpha)
					// if the reduced search does not fail low, it needs a re-search to the full depth
					evaluation = -search(board, depth - 1, ply + 1, -beta, -alpha, true);

				else if (evaluation > alpha && evaluation < beta)
					// if a move happens to be better, we need to re-search it with full window
					evaluation = -search(board, depth - 1, ply + 1, -beta, -alpha, true);
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

		for (current_depth = 1; current_depth <= max_depth; current_depth++) {

			nodes_previous_iteration = search_nodes + quiescence_nodes;
			search_nodes = 0;
			quiescence_nodes = 0;

			search(board, current_depth, 0, -infinity, infinity, true);
			total_nodes += search_nodes + quiescence_nodes;


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
