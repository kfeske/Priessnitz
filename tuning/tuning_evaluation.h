struct PSQT : Noncopyable
{
	double midgame[16][64];
	double endgame[16][64];

	// credit to PeSTO's evaluation function for the psqt values

	// Phase (Endgame/Midgame), Piece, Square
	int bonus[2][6][64] = 
	 { 
	  { // Midgame
	   { // Pawn
	   0, 0, 0, 0, 0, 0, 0, 0, 
	   103, 148, 64, 142, 93, 131, 10, -18, 
	   4, 2, 44, 51, 83, 80, 36, -8, 
	   -9, 14, 14, 42, 40, 31, 27, -3, 
	   -22, -6, 18, 36, 42, 26, 11, -11, 
	   -14, -5, 15, 16, 35, 32, 38, 11, 
	   -11, 4, -6, 8, 14, 54, 49, 6, 
	   0, 0, 0, 0, 0, 0, 0, 0
	   },
	   { // Knight
	   -168, -74, -53, -64, 18, -124, -13, -115,
	   -84, -44, 78, 9, 8, 36, -4, -56,
	   -46, 68, 42, 68, 77, 113, 66, 25,
	   9, 46, 31, 73, 47, 80, 27, 24,
	   6, 27, 45, 40, 61, 48, 38, 11,
	   -1, 20, 47, 44, 57, 54, 46, 4,
	   -3, -31, 14, 41, 35, 49, 10, 11,
	   -96, 14, -13, -1, 32, 8, 17, -17
	   },
	   { // Bishop
	   -23, -6, -102, -60, -39, -52, -15, -5, 
	   -26, 5, -35, -52, -1, 23, -5, -75, 
	   -8, 27, 27, 8, 6, 21, 19, 1, 
	   2, 6, -1, 24, 16, 14, 11, 0, 
	   12, 13, 3, 19, 25, 2, 9, 24, 
	   17, 30, 17, 6, 23, 36, 28, 31, 
	   30, 38, 22, 14, 20, 31, 56, 22, 
	   -6, 26, 26, 16, 30, 20, -7, 11, 
	   },
	   { // Rook
	   10, 17, -13, 29, 22, -34, -20, -1, 
	   16, 11, 49, 40, 64, 45, -10, 15, 
	   -10, 17, 7, 22, -11, 24, 52, -6, 
	   -27, -17, 2, 4, 2, 21, -15, -19, 
	   -48, -31, -16, -16, -5, -25, -5, -34, 
	   -47, -28, -17, -18, -5, -10, -16, -38, 
	   -40, -16, -19, -10, 5, 3, -9, -60, 
	   -22, -21, -10, -3, 3, -4, -28, -5, 
	   },
	   { // Queen
	   10, -7, 4, -4, 69, 37, 38, 50, 
	   -7, -36, -11, 7, -43, 20, 3, 44, 
	   11, 0, 21, -18, 19, 55, 30, 47, 
	   -10, -12, -17, -28, 0, 0, -2, 4, 
	   15, -20, 1, -2, 5, 0, 11, 9, 
	   -1, 26, 8, 12, 13, 21, 32, 21, 
	   -6, 16, 36, 31, 34, 38, 25, 37, 
	   33, 13, 22, 44, 17, 6, 0, -16, 
	   },
	   { // King
	   -12, 24, 32, 11, -23, -10, 4, 2, 
	   27, 35, 24, 45, 23, 17, -10, -27, 
	   22, 50, 65, 37, 36, 73, 70, -12, 
	   2, 16, 21, 12, 0, -10, 2, -73, 
	   -50, 44, 3, -59, -68, -46, -50, -96, 
	   0, 11, -6, -38, -41, -29, 2, -37, 
	   19, 29, -10, -65, -37, -17, 27, 16, 
	   -25, 50, 21, -62, 9, -27, 41, 16, 
	   }
	  },

	  { // Endgame
 	   { // Pawn
	   0, 0, 0, 0, 0, 0, 0, 0, 
	   221, 199, 204, 161, 174, 161, 193, 221, 
	   85, 96, 92, 75, 68, 51, 73, 83, 
	   14, 4, 6, 0, -4, 4, 0, 2, 
	   -7, -7, -14, -13, -13, -16, -15, -14, 
	   -14, -11, -9, -5, 0, -13, -21, -20, 
	   -6, -10, 9, 3, 8, -8, -22, -19, 
	   0, 0, 0, 0, 0, 0, 0, 0, 
	   },
	   { // Knight
	   -13, 4, 45, 25, 27, 25, -20, -58, 
	   29, 44, 30, 60, 49, 35, 30, 3, 
	   26, 35, 71, 65, 55, 54, 36, 14, 
	   31, 56, 83, 78, 83, 68, 59, 35, 
	   32, 47, 71, 79, 70, 66, 56, 29, 
	   27, 48, 54, 64, 61, 46, 31, 25, 
	   7, 31, 40, 41, 40, 36, 26, -2, 
	   15, -3, 21, 33, 21, 25, -3, -23, 
	   },
	   { // Bishop
	   -8, -11, 1, -1, 5, 0, -1, -19, 
	   6, 1, 12, -4, 1, -2, -1, 4, 
	   9, -3, 1, -3, -2, 0, 3, 8, 
	   5, 5, 6, 4, 0, 2, -3, 6, 
	   1, -1, 8, 10, -2, 3, -4, -2, 
	   -4, 0, 7, 6, 7, -7, 0, -6, 
	   -10, -14, -4, 3, 5, -6, -9, -21, 
	   -12, -3, -10, 1, -2, -5, 1, -8, 
	   },
	   { // Rook
	   35, 29, 36, 30, 31, 37, 35, 29, 
	   34, 37, 27, 29, 16, 25, 33, 27, 
	   26, 28, 24, 29, 27, 16, 15, 21, 
	   22, 21, 33, 19, 24, 19, 19, 23, 
	   27, 29, 26, 23, 16, 10, 14, 18, 
	   22, 19, 12, 17, 10, 5, 15, 10, 
	   19, 17, 17, 18, 9, 10, 7, 18, 
	   12, 20, 23, 21, 14, 10, 17, -6, 
	   },
	   { // Queen
	   -29, 23, 21, 22, 14, 18, 14, 29, 
	   -18, 13, 30, 33, 57, 28, 35, 20, 
	   -20, -8, -21, 44, 31, 11, 22, 23, 
	   7, 11, -2, 18, 25, 21, 62, 45, 
	   -16, 16, -6, 7, -2, 8, 21, 27, 
	   -6, -52, -11, -19, -8, -9, -3, 12, 
	   -11, -40, -48, -33, -27, -37, -57, -31, 
	   -40, -39, -39, -61, -16, -31, -18, -41, 
	   },
	   { // King
	   -84, -40, -21, -19, -12, 12, 2, -16, 
	   -13, 11, 9, 13, 13, 36, 22, 6, 
	   2, 16, 17, 8, 15, 42, 39, 8, 
	   -16, 16, 26, 26, 24, 30, 24, 4, 
	   -22, -9, 21, 31, 32, 24, 7, -4, 
	   -22, -7, 8, 21, 25, 19, 7, -4, 
	   -36, -16, 7, 11, 18, 7, -14, -25, 
	   -62, -47, -27, -10, -32, -12, -39, -57, 
	   }
	  }
	 };

	void prepare()
	{
		for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING }) {
			for (unsigned square = 0; square < 64; square++) {
				midgame[pc][square] = bonus[MIDGAME][type_of(pc)][square];
				midgame[pc + 8][mirrored(square)] = -midgame[pc][square];

				endgame[pc][square] = bonus[ENDGAME][type_of(pc)][square];
				endgame[pc + 8][mirrored(square)] = -endgame[pc][square];
			}
		}
	}

	PSQT()
	{
		prepare();
	}
};

struct Evaluation : Noncopyable
{
	PSQT psqt;
	double mg_bonus[2];
	double eg_bonus[2];
	uint64_t king_zone[2];
	double king_danger[2];
	int king_attackers[2];

	double attack_potency[6] = { 0, 20, 38, 28, 73, 0 };
	double king_danger_weight[8] = { 0, 13, 58, 93, 126, 59, 0, 0 };

	double average_mobility[6] = { 0, -26, -14, -40, -109, 0 };
	double mg_mobility_weight[6] = { 0, 159, 88, 80, 23, 0 };
	double eg_mobility_weight[6] = { 0, -65, 28, 26, 68, 0 };

	double mg_isolated_penalty = 21;
	double eg_isolated_penalty = 11;

	double tempo_bonus = 0;

	double mg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 
	7, 19, 20, -23, 47, 26, 41, 3, 
	6, 4, -9, -23, -6, 11, 7, 21, 
	28, 12, 1, -12, -3, -5, -9, 21, 
	32, -8, -7, -11, -6, 9, -7, 7, 
	13, 3, -6, -14, -4, 35, -23, 3, 
	8, 6, 12, -3, -47, 21, 1, 5, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	};

	double eg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 
	8, 9, -2, 2, 5, 3, 8, 12, 
	54, 40, 24, 24, 12, 29, 41, 36, 
	59, 54, 41, 31, 28, 36, 59, 51, 
	56, 58, 48, 37, 33, 30, 63, 51, 
	48, 43, 31, 25, 15, 17, 47, 33, 
	7, 15, -1, 2, 21, 9, 20, 18, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	};

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
	double evaluate(Board &board)
	{
		double mg_value = 0;
		double eg_value = 0;
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
		double phase = ((material - MIN_MATERIAL) * 256) / (MAX_MATERIAL - MIN_MATERIAL); // 0(Endgame) - 256(Midgame) linear interpolation

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

		mg_bonus[board.side_to_move] += tempo_bonus;

		mg_value += mg_bonus[WHITE] - mg_bonus[BLACK];
		eg_value += eg_bonus[WHITE] - eg_bonus[BLACK];

		// Tapered Eval
		// interpolation between midgame and endgame to create a smooth transition
		double value = (mg_value * phase + eg_value * (256 - phase)) / 256;
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
