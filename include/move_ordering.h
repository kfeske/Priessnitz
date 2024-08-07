#include <algorithm>

int value(Piece p)
{
	switch(p) {
	case W_PAWN:   case B_PAWN:   return 10;
	case W_KNIGHT: case B_KNIGHT: return 30;
	case W_BISHOP: case B_BISHOP: return 30;
	case W_ROOK:   case B_ROOK:   return 50;
	case W_QUEEN:  case B_QUEEN:  return 90;
	default: return 0;
	}
}

struct MoveOrderer : Noncopyable
{
	int rate_move(Board &board, Heuristics &heuristics, Move move, bool quiescence, unsigned ply)
	{
		int score = 0;
		MoveFlags flags = flags_of(move);
		PieceType pt = type_of(board.board[move_from(move)]);

		// same move at same depth in principle variation of previous iteration (iterative deepening)
		if (move == heuristics.pv_table[ply][0])
			return 30000;

		// transposition table move
		if (move == heuristics.pv_move)
			return 10000;

		// MVV - LVA (most valuable victim, least valuable attacker)
		if (flags == CAPTURE)
			score += 10 * value(board.board[move_to(move)]) - value(board.board[move_from(move)]);

		// a promotion is likely to be a good idea
		if (pt == PAWN) {
			switch(flags) {
			case PR_KNIGHT: case PC_KNIGHT: case PR_BISHOP: case PC_BISHOP:
				score += 300;
				break;
			case PR_ROOK: case PC_ROOK:
				score += 500;
				break;
			case PR_QUEEN: case PC_QUEEN:
				score += 900;
				break;
			default: break;
			}
		}

		if (!quiescence) {

			// primary killer move (non capture move that caused a beta cutoff)
			if (move == heuristics.killer_move[0][ply]) score += 80;

			// secondary killer move
			else if (move == heuristics.killer_move[1][ply]) score += 75;

			// if everything else fails, score history moves
			//else score += heuristics.history_move[board.board[move_from(move)]][move_to(move)];
		}	
		return score; 
	}

	void sort_moves(Board &board, Heuristics &heuristics, std::vector<Move> &unsorted, bool quiescence, unsigned ply)
	{
		unsigned n = unsorted.size();
		std::pair<int, Move> pair[n];

		for (unsigned i = 0; i < n; i++) {
			Move move = unsorted.at(i);
			pair[i].first  = -rate_move(board, heuristics, move, quiescence, ply); // negative to simplify sorting
			pair[i].second = move;
		}
		std::sort(pair, pair + n);

		for (unsigned i = 0; i < n; i++)
			unsorted[i] = pair[i].second;
		
	}

	MoveOrderer(Board &board, Heuristics &heuristics, std::vector<Move> &unsorted, unsigned ply)
	{
		sort_moves(board, heuristics, unsorted, false, ply);
	}

	// seperate move orderer for quiescence search
	MoveOrderer(Board &board, Heuristics &heuristics, std::vector<Move> &unsorted)
	{
		sort_moves(board, heuristics, unsorted, true, 0);
	}
};

