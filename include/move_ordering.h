#include <algorithm>

struct Heuristics
{
	Move killer_move[2][64];
	Move hash_move = INVALID_MOVE;
	int32_t history[16][64];
};

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

void rate_moves(Board &board, Heuristics &heuristics, MoveGenerator &move_generator, bool quiescence, unsigned ply)
{
	for (unsigned n = 0; n < move_generator.size; n++) {
		Scored_move &m = move_generator.move_list[n];
		MoveFlags flags = flags_of(m.move);

		// transposition table move
		if (m.move == heuristics.hash_move) {
			m.score += 30000;
			continue;
		}

		// MVV - LVA (most valuable victim, least valuable attacker)
		else if (flags == CAPTURE) {
			//if (see(board, m.move) >= 0)
			//	m.score += std::min(5000 + 10 * value(board.board[move_to(m.move)]) - value(board.board[move_from(m.move)]), 29999);
			//else
				m.score += std::min(10 * value(board.board[move_to(m.move)]) - value(board.board[move_from(m.move)]), 29999);
		}

		else if (!quiescence) {

			// primary killer move (non capture move that caused a beta cutoff)
			if (m.move == heuristics.killer_move[0][ply]) m.score += 80;

			// secondary killer move
			else if (m.move == heuristics.killer_move[1][ply]) m.score += 75;

			// if everything else fails, score history moves
			else m.score += std::min(-30000 + heuristics.history[board.board[move_from(m.move)]][move_to(m.move)], 74);
			//if (std::min(-30000 + heuristics.history[board.board[move_from(m.move)]][move_to(m.move)], 74) == 74)
			//	std::cerr << "history overflow\n";
		}
	}
}

Move next_move(MoveGenerator &move_generator, unsigned index)
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
}
