#include <psqt.h>

struct Evaluation : Noncopyable
{
	PSQT psqt;
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_zone[2];
	int king_danger[2];
	int king_attackers[2];

	int attack_potency[6] = { 0, 20, 20, 40, 80, 0 };
	int king_danger_weight[8] = { 0, 0, 50, 75, 88, 94, 97, 99 };

	int average_mobility[6] = { 0, 30, 40, 50, 0, 0 };
	int mg_mobility_weight[6] = { 0, 40, 50, 20, 10, 0 };
	int eg_mobility_weight[6] = { 0, 40, 50, 40, 20, 0 };

	int mg_isolated_penalty = 15;
	int eg_isolated_penalty = 5;

	int mg_passed_bonus[64] =
	{
	  /*0,  0,  0,  0,  0,  0,  0,  0,
	 40, 40, 40, 40, 40, 40, 40, 40,
	 30, 30, 30, 30, 30, 30, 30, 30,
	 15, 15, 15, 15, 15, 15, 15, 15,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  0,  0,  0,  0,  0,  0,  0,  0,*/

	 0, 0, 0, 0, 0, 0, 0, 0, 
	 1, 18, 54, 37, 50, -4, 62, 29, 
	 20, 9, 7, -4, -1, 38, 50, 43, 
	 16, 0, -2, -28, -8, 49, 30, 17, 
	 39, -15, -7, -4, 14, 32, 28, 27, 
	 30, 16, 4, -4, -30, 0, 18, 23, 
	 22, 1, 34, 26, 26, 41, 24, 28, 
	 0, 0, 0, 0, 0, 0, 0, 0, 
	};

	/*int mg_passed_bonus[64] = 
	{
-26, -26, -26, -26, -10, -26, -26, -26,
-36, -36, -36, -36, -36, -36, -36, -36,
-15, -25, -21, -29, -25, -45, -37, -15,
17, -5, -1, -7, 3, 11, 11, 7,
23, 17, 11, 3, 9, 43, 17, 17,
-3, -1, 11, 27, -11, 33, -1, 5,
0, 0, 0, 0, 0, 0, 0, 0,
	};*/

	int eg_passed_bonus[64] =
	{
	  /*0,  0,  0,  0,  0,  0,  0,  0,
	 65, 65, 65, 65, 65, 65, 65, 65,
	 65, 65, 65, 65, 65, 65, 65, 65,
	 45, 45, 45, 45, 45, 45, 45, 45,
	 25, 25, 25, 25, 25, 25, 25, 25,
	  7,  7,  7,  7,  7,  7,  7,  7,
	  3,  3,  3,  3,  3,  3,  3,  3,
	  0,  0,  0,  0,  0,  0,  0,  0,*/

	 0, 0, 0, 0, 0, 0, 0, 0, 
	 6, -4, -7, 7, 47, -6, -14, -5, 
	 28, 24, 18, 28, -4, 32, 16, 24, 
	 48, 34, 30, 26, 34, 24, 42, 50, 
	 50, 38, 38, 24, 28, 26, 34, 52, 
	 38, 28, 14, 14, 12, 12, 22, 22, 
	 10, -6, 3, 17, 22, 8, 5, 10, 
	 0, 0, 0, 0, 0, 0, 0, 0, 
	};

	/*int eg_passed_bonus[64] =
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

	// pawn structure evaluation
	void evaluate_pawn(Board &board, unsigned square, Color color)
	{
		uint64_t friendly_pawns = board.pieces[piece_of(color, PAWN)];
		uint64_t enemy_pawns = board.pieces[piece_of(!color, PAWN)];

		bool blocked = board.precomputed.forward_file_mask[color][square] & friendly_pawns;

		// reward passed pawns
		if (!(board.precomputed.passed_pawn_mask[color][square] & enemy_pawns) && !blocked) {
			mg_bonus[color] += mg_passed_bonus[square];
			eg_bonus[color] += eg_passed_bonus[square];
		}

		// punish isolated pawns
		if (!(board.precomputed.isolated_pawn_mask[file(square)] & friendly_pawns)) {
			mg_bonus[color] -= mg_isolated_penalty;
			eg_bonus[color] -= eg_isolated_penalty;
		}
	}

	void evaluate_kings()
	{
		mg_bonus[WHITE] -= king_danger[WHITE] * king_danger_weight[std::min(king_attackers[WHITE], 7)] / 100;
		mg_bonus[BLACK] -= king_danger[BLACK] * king_danger_weight[std::min(king_attackers[BLACK], 7)] / 100;
	}

	template <PieceType pt>
	void note_king_attacks(uint64_t attacks, Color friendly)
	{
		if (attacks & king_zone[!friendly]) {
			king_danger[!friendly] += attack_potency[pt] * pop_count(attacks & king_zone[!friendly]);
			king_attackers[!friendly]++;
		}
	}

	template <PieceType pt>
	void evaluate_mobility(Board &board, uint64_t attacks, Color friendly)
	{
		int safe_squares = pop_count(attacks & ~board.pieces[piece_of(!friendly, PAWN)]);
		mg_bonus[friendly] += (10 * safe_squares - average_mobility[pt]) * mg_mobility_weight[pt] / 100;
		eg_bonus[friendly] += (10 * safe_squares - average_mobility[pt]) * eg_mobility_weight[pt] / 100;
	}

	void evaluate_piece(Board &board, Piece p, unsigned square)
	{
		Color friendly = color_of(p);

		switch (type_of(p)) {
		case PAWN:
			evaluate_pawn(board, square, color_of(p));
			return;

		case KNIGHT:
			{
			uint64_t attacks = board.precomputed.attacks_bb<KNIGHT>(square, 0ULL) & king_zone[!friendly];
			note_king_attacks<KNIGHT>(attacks, friendly);
			evaluate_mobility<KNIGHT>(board, attacks, friendly);
			return;
			}
		case BISHOP:
			{
			// queen is not counted as a blocker, because a bishop behind a queen makes the attack more potent
			uint64_t ray_blockers = board.occ & ~board.pieces[piece_of(friendly, QUEEN)];
			uint64_t attacks = board.precomputed.attacks_bb<BISHOP>(square, ray_blockers);
			note_king_attacks<BISHOP>(attacks, friendly);
			evaluate_mobility<BISHOP>(board, attacks, friendly);
			return;
			}
		case ROOK:
			{
			// queen and rooks are not counted as a blockers, because they increase the pressure on a king square when cooperating
			uint64_t ray_blockers = board.occ & ~(board.pieces[piece_of(friendly, QUEEN)] | board.pieces[piece_of(friendly, ROOK)]);
			uint64_t attacks = board.precomputed.attacks_bb<ROOK>(square, ray_blockers);
			note_king_attacks<ROOK>(attacks, friendly);
			evaluate_mobility<ROOK>(board, attacks, friendly);
			return;
			}
		case QUEEN:
			{
			uint64_t attacks = board.precomputed.attacks_bb<QUEEN>(square, board.occ);
			note_king_attacks<QUEEN>(attacks, friendly);
			evaluate_mobility<QUEEN>(board, attacks, friendly);
			return;
			}
		case KING:
			return;
		}
	}

	// main evaluation function
	int evaluate(Board &board)
	{
		int mg_value = 0;
		int eg_value = 0;
		mg_bonus[WHITE] = 0;
		mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = 0;
		eg_bonus[BLACK] = 0;
		king_danger[WHITE] = 0;
		king_danger[BLACK] = 0;
		king_attackers[WHITE] = 0;
		king_attackers[BLACK] = 0;
		king_zone[WHITE] = board.precomputed.attacks_bb<KING>(lsb(board.pieces[piece_of(WHITE, KING)]), 0ULL);
		king_zone[BLACK] = board.precomputed.attacks_bb<KING>(lsb(board.pieces[piece_of(BLACK, KING)]), 0ULL);

		int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
		material = std::max(int(MIN_MATERIAL), std::min(material, int(MAX_MATERIAL))); // endgame and midgame limit clamp
		int phase = ((material - MIN_MATERIAL) * 256) / (MAX_MATERIAL - MIN_MATERIAL); // 0(Endgame) - 256(Midgame) linear interpolation

		uint64_t pieces = board.occ;

		while (pieces) {
			unsigned square = pop_lsb(pieces);
			Piece p = board.board[square];

			// material and positional evaluation via Piece Square Tables
			if (color_of(p) == WHITE) {
				mg_value += psqt.midgame[p][square] + piece_value(p, MIDGAME);
				eg_value += psqt.endgame[p][square] + piece_value(p, ENDGAME);
			}
			else
			{
				mg_value += -psqt.midgame[p - 8][mirrored(square)] + piece_value(p, MIDGAME);
				eg_value += -psqt.endgame[p - 8][mirrored(square)] + piece_value(p, ENDGAME);
			}

			evaluate_piece(board, p, square);
		}

		evaluate_kings();

		mg_value += mg_bonus[WHITE] - mg_bonus[BLACK];
		eg_value += eg_bonus[WHITE] - eg_bonus[BLACK];

		// Tapered Eval
		// interpolation between midgame and endgame to create a smooth transition
		int value = (mg_value * phase + eg_value * (256 - phase)) / 256;
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
