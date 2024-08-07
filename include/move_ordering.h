#include <algorithm>

int value(Piece p)
{
	switch(p) {
	case W_PAWN:   case B_PAWN:   return PAWN_MG;
	case W_KNIGHT: case B_KNIGHT: return KNIGHT_MG;
	case W_BISHOP: case B_BISHOP: return BISHOP_MG;
	case W_ROOK:   case B_ROOK:   return ROOK_MG;
	case W_QUEEN:  case B_QUEEN:  return QUEEN_MG;
	default: return 0;
	}
}

void rate_moves(Board &board, Heuristics &, MoveGenerator &move_generator, Move pv_move, bool , unsigned ply_from_root)
{
	(void) ply_from_root;
	for (Scored_move &m : move_generator.movelist) {
		MoveFlags flags = flags_of(m.move);
		//PieceType pt = type_of(board.board[move_from(m.move)]);
		(void) board;
		(void) m;

		(void) pv_move;
		// same move as principle variation move of previous iteration (iterative deepening)
		if (m.move == pv_move) {
			m.score += 30000;
			//std::cerr << move_string(m.move) << " ";
			continue;
		}

		// transposition table move
		//if (move == heuristics.pv_move)
		//	return 10000;

		// MVV - LVA (most valuable victim, least valuable attacker)
		else if (flags == CAPTURE)
			m.score += 10 * value(board.board[move_to(m.move)]) - value(board.board[move_from(m.move)]);

		// a promotion is likely to be a good idea
		/*if (pt == PAWN) {
			switch(flags) {
			case PR_KNIGHT: case PC_KNIGHT: case PR_BISHOP: case PC_BISHOP:
				m.score += 300;
				break;
			case PR_ROOK: case PC_ROOK:
				m.score += 500;
				break;
			case PR_QUEEN: case PC_QUEEN:
				m.score += 900;
				break;
			default: break;
			}
		}*/

		//if (!quiescence) {

		//	// primary killer move (non capture move that caused a beta cutoff)
		//	if (move == heuristics.killer_move[0][ply]) score += 80;

		//	// secondary killer move
		//	else if (move == heuristics.killer_move[1][ply]) score += 75;

		//	// if everything else fails, score history moves
		//	//else score += heuristics.history_move[board.board[move_from(move)]][move_to(move)];
		//}
	}
}

Move next_move(MoveGenerator &move_generator, unsigned index)
{
	unsigned best_index = index;
	uint16_t best_score = move_generator.movelist.at(index).score;
	for (unsigned n = index + 1; n < move_generator.movelist.size(); n++) {
		Scored_move &m = move_generator.movelist.at(n);
		if (m.score > best_score) {
			best_score = m.score;
			best_index = n;
		}
	}
	std::swap(move_generator.movelist.at(index), move_generator.movelist.at(best_index));
	return move_generator.movelist.at(index).move;
}
