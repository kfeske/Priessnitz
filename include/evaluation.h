
struct Evaluation
{
	Phase phase;
	int evaluate(Board &board, PSQT &psqt)
	{
		int value = 0;
		int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
		material = std::max(3915, std::min(material, 15258)); // endgame and midgame limit clamp
		phase = Phase(((material - 3915) * 128) / (15258 - 3915)); // 0(Endgame) - 128(Midgame) linear interpolation
	
		for (Piece p : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN }) {
			uint64_t pieces = board.pieces[p];
			while (pieces) {
				unsigned square = pop_lsb(pieces);
				//pop_lsb(pieces);
				//value += get_piece_value(p, MIDGAME);
				value += psqt.psqt[p][square];// * phase; important!!!
				//value += psqt.psqt[ENDGAME][p][square] * (128 - phase);
			}
		}
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
