
struct Evaluation
{
	int evaluate(Board &board, PSQT &psqt)
	{
		int value = 0;
		int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];

		material = std::max(int(MIN_MATERIAL), std::min(material, int(MAX_MATERIAL))); // endgame and midgame limit clamp
		int phase = ((material - MIN_MATERIAL) * 255) / (MAX_MATERIAL - MIN_MATERIAL); // 0(Endgame) - 256(Midgame) linear interpolation
		(void) phase;
	
		for (Piece p : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN }) {
			uint64_t pieces = board.pieces[p];
			while (pieces) {
				unsigned square = pop_lsb(pieces);

				// Tapered Eval
				value += psqt.midgame[p][square];
				//value += (psqt.midgame[p][square] * phase) / 255;
				//value += (psqt.endgame[p][square] * (255 - phase)) / 255;
			}
		}
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
