#include <psqt.h>

struct Evaluation : Noncopyable
{
	PSQT psqt;
	int mg_value;
	int eg_value;

	int mg_passed_bonus[64] =
	{
	  0,  0,  0,  0,  0,  0,  0,  0,
	 40, 40, 40, 40, 40, 40, 40, 40,
	 30, 30, 30, 30, 30, 30, 30, 30,
	 15, 15, 15, 15, 15, 15, 15, 15,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int eg_passed_bonus[64] =
	{
	  0,  0,  0,  0,  0,  0,  0,  0,
	 65, 65, 65, 65, 65, 65, 65, 65,
	 65, 65, 65, 65, 65, 65, 65, 65,
	 45, 45, 45, 45, 45, 45, 45, 45,
	 25, 25, 25, 25, 25, 25, 25, 25,
	  7,  7,  7,  7,  7,  7,  7,  7,
	  3,  3,  3,  3,  3,  3,  3,  3,
	  0,  0,  0,  0,  0,  0,  0,  0,
	};

	/*int mg_passed_bonus[64] =
	{
	 0, 0, 0, 0, 0, 0, 0, 0,
	 45, 52, 42, 43, 28, 34, 19, 9,
	 48, 43, 43, 30, 24, 31, 12, 2,
	 28, 17, 13, 10, 10, 19, 6, 1,
	 14, 0, -9, -7, -13, -7, 9, 16,
	 5, 3, -3, -14, -3, 10, 13, 19,
	 8, 9, 2, -8, -3, 8, 16, 9,
	 0, 0, 0, 0, 0, 0, 0, 0,
	};

	int eg_passed_bonus[64] =
	{
	 0, 0, 0, 0, 0, 0, 0, 0,
	 77, 74, 63, 53, 59, 60, 72, 77,
	 91, 83, 66, 40, 30, 61, 67, 84,
	 55, 52, 42, 35, 30, 34, 56, 52,
	 29, 26, 21, 18, 17, 19, 34, 30,
	 8, 6, 5, 1, 1, -1, 14, 7,
	 2, 3, -4, 0, -2, -1, 7, 6,
	 0, 0, 0, 0, 0, 0, 0, 0,
	};*/



	void evaluate_pawn(Board &board, unsigned square, Color color)
	{
		int mg_pawn_value = 0;
		int eg_pawn_value = 0;
		uint64_t friendly_pawns = board.pieces[piece_of(color, PAWN)];
		uint64_t enemy_pawns = board.pieces[piece_of(!color, PAWN)];

		bool blocked = board.precomputed.forward_file_mask[color][square] & friendly_pawns;

		// reward passed pawns
		if (!(board.precomputed.passed_pawn_mask[color][square] & enemy_pawns) && !blocked) {
			mg_pawn_value += mg_passed_bonus[square];
			eg_pawn_value += eg_passed_bonus[square];
		}

		// punish isolated pawns
		if (!(board.precomputed.isolated_pawn_mask[file(square)] & friendly_pawns)) {
			mg_pawn_value -= 15;
			eg_pawn_value -= 5;
		}

		mg_value += (color == WHITE) ? mg_pawn_value : -mg_pawn_value;
		eg_value += (color == WHITE) ? eg_pawn_value : -eg_pawn_value;
	}

	int evaluate(Board &board)
	{
		mg_value = 0;
		eg_value = 0;
		int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];

		material = std::max(int(MIN_MATERIAL), std::min(material, int(MAX_MATERIAL))); // endgame and midgame limit clamp
		int phase = ((material - MIN_MATERIAL) * 256) / (MAX_MATERIAL - MIN_MATERIAL); // 0(Endgame) - 256(Midgame) linear interpolation
	
		for (Piece p : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING }) {
			uint64_t pieces = board.pieces[p];
			while (pieces) {
				unsigned square = pop_lsb(pieces);

				// material and positional evaluation via Piece Square Tables
				mg_value += psqt.midgame[p][square];
				eg_value += psqt.endgame[p][square];

				// pawn structure evaluation
				//if (type_of(p) == PAWN)
				//	evaluate_pawn(board, square, color_of(p));
			}
		}

		// Tapered Eval
		// interpolation between midgame and endgame to create a smooth transition
		int value = (mg_value * phase + eg_value * (256 - phase)) / 256;
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
