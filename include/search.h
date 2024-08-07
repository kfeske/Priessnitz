#include <time.h>
#include <psqt.h>
#include <evaluation.h>
#include <transposition_table.h>

struct Heuristics// : Noncopyable
{
	PSQT psqt {};
	Move killer_move[2][64];
	int history_move[16][64];
	//Move pv_table[64][64];
	//unsigned pv_lenght[64];
	//Move previous_pv_line[64];
	//Move pv_move = INVALID_MOVE;
};

#include <move_ordering.h>

struct Search
{
	Evaluation eval;
	Move best_move = INVALID_MOVE;
	Move best_move_this_iteration = INVALID_MOVE;
	unsigned max_depth = 63;
	double max_time = 99999;
	unsigned current_depth;
	int const mate_score = 31000;
	int const infinity = 32000;

	long nodes_searched = 0;
	double branching_factor = 0;
	long cutoffs = 0;
	long cutoffspv = 0;
	//long tt_cutoffs = 0;

	Heuristics heuristics {};

	//static TranspositionTable<(500 * 1024 * 1024) / sizeof(TTEntry)> tt;

	double time_start;
	bool abort_search;

	// Quiescence Search is a shallower search, generating only
	// captures and check evasions to avoid the horizon effect

	void reset()
	{
		best_move = INVALID_MOVE;
		max_depth = 63;
		max_time = 99999;
		nodes_searched = 0;
		branching_factor = 0;
		cutoffs = 0;
		cutoffspv = 0;
		//tt_cutoffs = 0;
		heuristics = {};
	}

	int qsearch(Board &board, int alpha, int beta, unsigned ply)
	{
		bool in_check = board.in_check() && ply <= 2;

		int evaluation = eval.evaluate(board, heuristics.psqt);

		if (!in_check && evaluation >= beta)
			return beta;

		if (evaluation > alpha)
			alpha = evaluation;

		MoveGenerator move_generator {};
		if (in_check)
			move_generator.generate_all_moves(board);
		else
			move_generator.generate_quiescence(board); // only captures

		rate_moves(board, heuristics, move_generator, best_move, true, ply);

		for (unsigned n = 0; n < move_generator.movelist.size(); n++) {
			Move move = next_move(move_generator, n);

			board.make_move(move);
	
			evaluation = -qsearch(board, -beta, -alpha, ply + 1);
	
			board.unmake_move(move);
	
			if (evaluation >= beta)
				return beta;
	
			if (evaluation > alpha)
				alpha = evaluation;
		}
		return alpha;
	}

	// function that finds the best move at a fixed depth (number of moves, it looks into the future)

	int search(Board &board, unsigned depth, int alpha, int beta)
	{
		if ((nodes_searched & 2047) == 0 && SDL_GetTicks() - time_start > max_time && max_time != 0)
		{
			// Time's up! Use the best move from the previous iteration
			abort_search = true;
			std::cerr << "info abort search\n";
			return 0;
		}

		/*uint64_t z_key = 0ULL;
		for (unsigned square = 0; square <= 63; square++) {
			z_key ^= board.zobrist.piece_rand[board.board[square]][square];
		}
		if (board.side_to_move == BLACK)
			z_key ^= board.zobrist.side_rand;
		if (board.history[board.game_ply].ep_sq != SQ_NONE)
			z_key ^= board.zobrist.ep_rand[file(board.history[board.game_ply].ep_sq)];
		z_key ^= board.zobrist.castling_rand[board.history[board.game_ply].castling_rights];
		if (z_key != board.zobrist.key)
			tt_cutoffs++;*/

		Move best_move_this_node = INVALID_MOVE;
		int evaluation;
		bool in_check = board.in_check();

		// ply describes how far we are from the root of the search tree
		unsigned ply_from_root = current_depth - depth;
		//heuristics.pv_lenght[ply] = ply;
		//TTEntryFlag flag = UPPERBOUND;


		if (depth == 0) {
			nodes_searched++;
			// we reached a leaf node, start the Quiescence Search
			return qsearch(board, alpha, beta, 0);
		}

		// check for any transpositions
		/*if (tt.probe(board.zobrist.key, depth, alpha, beta))
			return tt.current_evaluation;
		heuristics.pv_move = tt.pv_move;
		*/
		// Null Move Pruning
		// if there is a beta cutoff even if we skip our turn (permitting the opponent to play two moves in a row),
		// the position is so terrible for the opponent that we can just prune the whole branch
		// may cause search instability, but it is worth the risk. If we want the speed, we have to live in fear

		// (should not be used in complicated endgames and zugzwang positions!!!)
		if (!in_check && ply_from_root > 0 && depth >= 3 &&
		    board.non_pawn_material[board.side_to_move]) {

			unsigned ep_square = board.make_null_move();

			evaluation = -search(board, depth - 3, -beta, -beta + 1);

			board.unmake_null_move(ep_square);

			if (evaluation >= beta && fabs(evaluation) < mate_score)
				return beta;
		}

		MoveGenerator move_generator {};
		move_generator.generate_all_moves(board);

		if (move_generator.movelist.size() == 0) {
			if (in_check) return -mate_score - depth;
			else return 0;
		}

		if (board.repetition || board.history[board.game_ply].rule_50 >= 100)
			return 0;

		rate_moves(board, heuristics, move_generator, best_move, false, ply_from_root);

		for (unsigned n = 0; n < move_generator.movelist.size(); n++) {
			Move move = next_move(move_generator, n);

			//std::cerr << "depth: " << depth;
			//std::cerr << " move: ";
			//print_move(move);
			//std::cerr << "\n";

			board.make_move(move);

			// Principle Variation Search

			/*if (n == 0)
				// search pv move with full alpha-beta window
				// it is very likely the best move
				evaluation = -search(board, depth - 1, -beta, -alpha);
			else {
				if (n >= 4 && depth >= 3 && !in_check && flags_of(move) != CAPTURE)
					// Late Move Reduction (assuming our move orderer is doing a good job, only the first
					// 4 moves are searched to the full depth
					evaluation = -search(board, depth - 2, -alpha - 1, -alpha);
				else
					// search remaining moves with null window to prove that they are worse than the pv move
					evaluation = -search(board, depth - 1, -alpha - 1, -alpha);

				if (evaluation > alpha && evaluation < beta)
					// if a move happens to be better, we need to re-search it with full window
					evaluation = -search(board, depth - 1, -beta, -alpha);
			}*/

			// temp search - delete later
			evaluation = -search(board, depth - 1, -beta, -alpha);
	
			board.unmake_move(move);

			if (abort_search) return 0;

			if (evaluation >= beta) {
				// Beta cutoff. There is a better line for the opponent
				// we know the opponent can get at least beta, so a branch that evaluates to more than beta
				// is irrelevant to search, since a better alternative for the opponent has alrady been found,
				// where he can get at least beta

				//flag = LOWERBOUND;

				if (flags_of(move) != CAPTURE) {
					// this is a killer move - Store it!
					heuristics.killer_move[1][ply_from_root] = heuristics.killer_move[0][ply_from_root];
					heuristics.killer_move[0][ply_from_root] = move;
				}
				// *snip*
				if (n == 0) cutoffspv++;
				cutoffs++;
				return beta;
			}

			if (evaluation > alpha) {
				// found a better move
				alpha = evaluation;
				best_move_this_node = move;
				//flag = EXACT;

				// remember principle variation (sequence of best moves)
				//heuristics.pv_table[ply][ply] = move;

				/*for (unsigned next_ply = ply + 1; next_ply < heuristics.pv_lenght[ply + 1]; next_ply++)
					heuristics.pv_table[ply][next_ply] = heuristics.pv_table[ply + 1][next_ply];
				heuristics.pv_lenght[ply] = heuristics.pv_lenght[ply + 1];
				*/

				//if (flags_of(move) != CAPTURE)
					// store history move
					// (similar to the killer move heuristic - keeps track of the history of best moves for move ordering purposes)
					//heuristics.history_move[board.board[move_from(move)]][move_to(move)] += depth;
			}
		}

		//tt.store(board.zobrist.key, depth, best_evaluation, best_move_this_node, flag);

		best_move_this_iteration = best_move_this_node;
		return alpha;
	}

	int start_search(Board &board)
	{
		int evaluation = 0;
		int final_evaluation = 0;
		abort_search = false;
		//memset(heuristics.killer_move, INVALID_MOVE, sizeof(heuristics.killer_move));
		//memset(heuristics.pv_table, INVALID_MOVE, sizeof(heuristics.pv_table));
		//tt = {};
		//tt_cutoffs = 0;

		time_start = SDL_GetTicks();
		int nodes_previous_iteration;
		long total_nodes = 0;

		//std::cerr << ".";
		for (current_depth = 1; current_depth <= max_depth; current_depth++) {

			nodes_previous_iteration = nodes_searched;
			nodes_searched = 0;

			evaluation = search(board, current_depth, -infinity, infinity);
			total_nodes += nodes_searched;

			if (abort_search) break;

			best_move = best_move_this_iteration;
			final_evaluation = evaluation;
			if (current_depth > 1) branching_factor = nodes_searched / double(nodes_previous_iteration) + 0.0001;
			//std::cerr << ".";
			/*for (unsigned j = 0; j < heuristics.pv_lenght[0]; j++) {
				heuristics.previous_pv_line[j] = heuristics.pv_table[0][j];
				print_move(heuristics.previous_pv_line[j]);
				std::cerr << "-> ";
			}
			std::cerr << "...\n";*/

			std::cout << "info depth " << current_depth << " score cp " << evaluation << " time " << SDL_GetTicks() - time_start;
			std::cout << " nodes " << nodes_searched << " branching " << branching_factor;
			std::cout << " pv " << move_string(best_move) << "\n";
		}

		std::cerr << "cut offs pv " << cutoffspv << " / " << cutoffs << "\n";
		std::cerr << "info total nodes " << total_nodes << "\n";
		return final_evaluation;
	}
};
