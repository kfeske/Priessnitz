#include <time.h>
#include <psqt.h>
#include <evaluation.h>
#include <transposition_table.h>

struct Heuristics : Noncopyable
{
	PSQT psqt {};
	Move killer_move[2][64];
	int history_move[16][64];
	Move pv_table[64][64];
	unsigned pv_lenght[64];
	Move previous_pv_line[64];
	Move pv_move = INVALID_MOVE;
};

#include <move_ordering.h>

struct Perft {
	
	long nodes = 0;
	unsigned captures = 0;
	unsigned ep_captures = 0;
	unsigned max_depth;

	long perft(Board &board, unsigned depth)
	{
		if (depth == 0) {
			nodes++;
			return 1;
		}

		MoveGenerator movegenerator {};
		movegenerator.generate_all_moves(board, false);

		for (unsigned n = 0; n < movegenerator.movelist.size(); n++) {
			Move move = movegenerator.movelist.at(n);

			if (depth == 1) {
				switch(flags_of(move)) {
				case CAPTURE: case PC_KNIGHT: case PC_BISHOP: case PC_ROOK: case PC_QUEEN:
					captures++;
					break;
				case EP_CAPTURE:
					ep_captures++;
					captures++;
					break;
				default: break;
				}
			}
			board.make_move(move);

			perft(board, depth - 1);

			board.unmake_move(move);
		}
		return nodes;
	}

	long get_legal_moves_count(Board &board, unsigned depth)
	{
		long nds = 0;
		if (depth == 0) {
			return 1;
		}

		MoveGenerator movegenerator {};
		movegenerator.generate_all_moves(board, false);

		for (unsigned n = 0; n < movegenerator.movelist.size(); n++) {
			Move move = movegenerator.movelist.at(n);

			board.make_move(move);

			if (depth == max_depth) {
				print_move(move);
				nds += get_legal_moves_count(board, depth - 1);
				std::cerr << nds << "\n\n";
				nds = 0;
			}
			else
				nds += get_legal_moves_count(board, depth - 1);

			board.unmake_move(move);
		}
		return nds;
	}

	Perft(unsigned depth)
	:
		max_depth(depth)
	{}
};

void run_perft(Board &board, unsigned depth)
{
	auto lasttime = SDL_GetTicks();
	for (unsigned i = 1; i <= depth; i++)
	{
		Perft perft { i };
		perft.perft(board, i);
		std::cerr << "DEPTH: " << i << "  LEGAL_MOVES: " << perft.nodes << "  CAPTURES: " << perft.captures
			  << "  EN PASSANT: " << perft.ep_captures << "  TIME: " << SDL_GetTicks() - lasttime << "\n";
	}
}

void run_debug_perft(Board &board, unsigned depth)
{
	Perft perft { depth };
	perft.get_legal_moves_count(board, depth);

	perft.perft(board, depth);
	std::cerr << "DEPTH: " << depth << "  LEGAL_MOVES: " << perft.nodes << "  CAPTURES: " << perft.captures
		  << "  EN PASSANT: " << perft.ep_captures;;
}

void test_each_move(Renderer &renderer, EmptyTexture &board_texture, Board &board, unsigned depth)
{
	if (depth == 0)
		return;

	MoveGenerator movegenerator {};
	movegenerator.generate_all_moves(board, false);

	for (unsigned n = 0; n < movegenerator.movelist.size(); n++) {
		images_to_board_texture(board, renderer, board_texture);
		board_texture.render(renderer, 0, 0);
		SDL_RenderPresent(&renderer.renderer);
		SDL_Delay(100);

		Move move = movegenerator.movelist.at(n);
		board.make_move(move);

		images_to_board_texture(board, renderer, board_texture);
		board_texture.render(renderer, 0, 0);
		SDL_RenderPresent(&renderer.renderer);
		SDL_Delay(1000);

		test_each_move(renderer, board_texture, board, depth - 1);

		board.unmake_move(move);
	}
	return;
}

struct Search
{
	Evaluation eval {};
	Move best_move = INVALID_MOVE;
	Move best_move_this_iteration = INVALID_MOVE;
	unsigned max_depth = 1100;
	double max_time = 3000;
	unsigned current_depth;
	int const mate_score = 31000;
	int const infinity = 32000;

	long nodes_searched = 0;
	//long tt_cutoffs = 0;

	Heuristics heuristics {};

	//static TranspositionTable<(500 * 1024 * 1024) / sizeof(TTEntry)> tt;

	double time_start;
	bool abort_search;

	// Quiescence Search is a shallower search, generating only
	// captures and check evasions to avoid the horizon effect

	int qsearch(Board &board, int alpha, int beta)
	{
		int evaluation = 0;

		evaluation = eval.evaluate(board, heuristics.psqt);

		if (!board.in_check()) {
			if (evaluation >= beta)
				return beta;
		}

		if (evaluation > alpha)
			alpha = evaluation;

		MoveGenerator movegenerator {};
		movegenerator.generate_all_moves(board, true); // only captures and check evasions

		//MoveOrderer { board, heuristics, movegenerator.movelist };

		for (unsigned n = 0; n < movegenerator.movelist.size(); n++) {

			Move move = movegenerator.movelist.at(n);
			board.make_move(move);
	
			evaluation = -qsearch(board, -beta, -alpha);
	
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
		if ((nodes_searched & 255) == 0 && SDL_GetTicks() - time_start > max_time && max_time != 0)
		{
			// Time's up! Use the best move from the previous iteration
			abort_search = true;
			return 0;
		}

		/*Uint64 z_key = 0ULL;
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
		int evaluation = 0;
		int best_evaluation = -infinity;
		bool in_check = board.in_check();
		//unsigned ply = current_depth - depth;
		//heuristics.pv_lenght[ply] = ply;
		//TTEntryFlag flag = UPPERBOUND;


		if (depth == 0) {
			nodes_searched++;
			// we reached a leaf node, start the Quiescence Search
			//return qsearch(board, alpha, beta);
			return eval.evaluate(board, heuristics.psqt);
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
		/*if (!in_check && ply > 0 && depth >= 3 &&
		    board.non_pawn_material[board.side_to_move]) {

			unsigned ep_square = board.make_null_move();

			evaluation = -search(board, depth - 3, -beta, -beta + 1);

			board.unmake_null_move(ep_square);

			if (evaluation >= beta && fabs(evaluation) < mate_score)
				return beta;
		}*/

		MoveGenerator movegenerator {};
		movegenerator.generate_all_moves(board, false);
	
		if (movegenerator.movelist.size() == 0) {
			if (in_check) return -mate_score - depth;
			else return 0;
		}

		if (board.repetition || board.history[board.game_ply].rule_50 >= 100)
			return 0;

		//MoveOrderer { board, heuristics, movegenerator.movelist, ply };

		for (unsigned n = 0; n < movegenerator.movelist.size(); n++) {
			Move move = movegenerator.movelist.at(n);

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


			if (evaluation > best_evaluation) {
				best_evaluation = evaluation;
				best_move_this_node = move;

				/*if (evaluation >= beta) {
					// Beta cutoff. Move is too good, so the opponent will never pick it,
					// since he found a better move in another better branch before
					flag = LOWERBOUND;

					if (flags_of(move) != CAPTURE) {
						// this is a killer move - Store it!
						heuristics.killer_move[1][ply] = heuristics.killer_move[0][ply];
						heuristics.killer_move[0][ply] = move;
					}
					// *snip*
					break;
				}*/

				if (evaluation > alpha) {
					// found a better move
					alpha = evaluation;
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
		}

		//tt.store(board.zobrist.key, depth, best_evaluation, best_move_this_node, flag);

		best_move_this_iteration = best_move_this_node;
		return best_evaluation;
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

		for (current_depth = 1; current_depth <= max_depth; current_depth++) {

			evaluation = search(board, current_depth, -infinity, infinity);

			if (abort_search) break;

			best_move = best_move_this_iteration;
			final_evaluation = evaluation;
			/*std::cerr << ".";
			for (unsigned j = 0; j < heuristics.pv_lenght[0]; j++) {
				heuristics.previous_pv_line[j] = heuristics.pv_table[0][j];
				print_move(heuristics.previous_pv_line[j]);
				std::cerr << "-> ";
			}
			std::cerr << "...\n";*/
		}
		std::cerr << "\n";
		return final_evaluation;
	}
};
