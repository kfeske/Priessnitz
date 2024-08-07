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
	   229, 229, 192, 244, 212, 252, 131, 80, 
	   -10, 16, 67, 38, 91, 116, 65, -36, 
	   -17, -4, -5, 30, 30, 23, 16, -24, 
	   -36, -13, -4, 15, 20, 3, -11, -38, 
	   -29, -23, -3, -11, 10, -8, 8, -15, 
	   -30, -18, -27, -20, -4, 15, 17, -17, 
	   0, 0, 0, 0, 0, 0, 0, 0, 
	   },
	   { // Knight
	   -79, -47, 28, 27, 106, -75, -13, -63, 
	   -86, 12, 127, 92, 69, 91, 22, -10, 
	   -11, 72, 61, 95, 148, 169, 108, 76, 
	   14, 29, 26, 73, 45, 82, 24, 36, 
	   -8, 45, 22, 17, 58, 27, 53, 0, 
	   -19, 9, 28, 18, 40, 29, 23, -22, 
	   -19, -43, -1, 19, 7, 34, -2, 22, 
	   -127, 1, -81, -35, 3, -32, -7, -42, 
	   },
	   { // Bishop
	   58, 53, -48, 49, 23, -21, 13, 77, 
	   -11, 32, 9, 52, 96, 97, 57, -58, 
	   3, 56, 68, 59, 89, 79, 43, -7, 
	   18, -3, 20, 42, 33, 71, 3, -6, 
	   3, 0, -7, 14, 30, -14, 11, 17, 
	   -4, 32, 11, 5, 16, 29, 19, 20, 
	   13, 25, 26, 0, 9, 15, 37, 18, 
	   -48, 0, 4, -4, 8, 6, -45, -2, 
	   },
	   { // Rook
	   56, 78, 50, 111, 93, 29, 84, 65, 
	   50, 22, 82, 85, 117, 114, 54, 74, 
	   0, 17, 54, 86, 74, 70, 109, 24, 
	   -17, 35, 9, 37, 30, 62, -9, 1, 
	   -30, -14, -29, -2, 14, 8, 30, -22, 
	   -60, -12, -30, -13, 12, -13, -1, -42, 
	   -44, -16, -20, -13, 0, 17, -13, -66, 
	   -25, -35, -15, -7, 10, -3, -23, -10, 
	   },
	   { // Queen
	   36, 71, 61, 69, 117, 130, 147, 173, 
	   1, -16, 39, 68, 34, 111, 88, 94, 
	   21, -5, 52, 11, 55, 117, 75, 96, 
	   -16, -25, -8, -11, 14, 27, 17, -4, 
	   -3, -34, -4, -16, -2, 4, -9, -5, 
	   -28, 12, -23, 6, 14, 14, 9, -3, 
	   -39, -7, 23, 12, 18, 41, -14, 19, 
	   3, -4, 6, 22, 7, -22, -44, -51, 
	   },
	   { // King
	   -68, 29, 23, -2, -39, -29, 9, 27, 
	   30, 8, -9, 18, 3, 1, -24, -19, 
	   7, 43, 39, -25, 45, 64, 59, -7, 
	   -18, 29, 37, 16, -13, 7, -9, -54, 
	   -42, 10, -33, -28, -71, -7, -64, -65, 
	   -45, 12, -4, -33, -48, -54, -21, -55, 
	   -31, 8, -31, -70, -45, -36, 7, -6, 
	   -50, 32, 2, -69, -10, -46, 22, 6,
	   }
	  },

	  { // Endgame
 	   { // Pawn
	   0, 0, 0, 0, 0, 0, 0, 0, 
	   199, 182, 184, 129, 126, 137, 187, 208, 
	   87, 85, 72, 68, 65, 38, 65, 73, 
	   11, -3, 0, -10, -7, -7, -12, 0, 
	   -10, -14, -20, -20, -12, -17, -22, -18, 
	   -15, -18, -15, -10, -9, -10, -28, -23, 
	   -2, -11, 7, 5, 12, -13, -23, -24, 
	   0, 0, 0, 0, 0, 0, 0, 0,
	   },
	   { // Knight
	   1, 4, 35, -5, -7, 3, -63, -71, 
	   7, 14, -14, 12, 10, 18, 8, -48, 
	   -5, 3, 29, 15, 7, 12, -4, -14, 
	   3, 33, 60, 44, 52, 23, 26, 4, 
	   21, 12, 46, 55, 32, 23, 8, 17, 
	   6, 21, 31, 43, 26, 29, 12, -9, 
	   -33, 8, 6, 9, 20, 8, 1, -46, 
	   -38, -19, 0, 7, 11, 10, -30, -65,
	   },
	   { // Bishop
	   29, -2, 14, -7, 14, 9, -12, -11, 
	   20, -1, 0, -9, -11, -8, 7, -6, 
	   16, -8, 0, -9, 3, -23, -6, 28, 
	   11, 5, -12, -11, -20, -10, 7, 9, 
	   17, -7, 23, 5, -3, -10, -15, 6, 
	   6, -9, 8, 6, 11, -5, 17, 0, 
	   4, -2, -7, 1, 10, -1, -13, -16, 
	   -7, 3, 3, 15, 1, 0, 0, 13,
	   },
	   { // Rook
	   13, -6, 10, -7, 4, 22, 11, 11, 
	   17, 17, -9, 5, -11, 3, 6, 1, 
	   1, 11, -11, 17, -16, -1, -5, 1, 
	   4, -13, 19, -11, 4, -4, 23, 6, 
	   3, 19, 6, 6, -7, -22, -8, 1, 
	   0, -10, -11, -11, -11, -27, -9, -11, 
	   -6, -6, -6, -6, -9, -19, -10, -7, 
	   -1, 2, 3, -7, -11, -27, -6, -20,
	   },
	   { // Queen
	   37, 67, 83, 89, 86, 136, 142, 133, 
	   41, 71, 94, 100, 105, 93, 90, 65, 
	   40, 71, 43, 95, 120, 98, 94, 78, 
	   63, 74, 77, 93, 77, 88, 127, 79, 
	   33, 72, 47, 71, 60, 78, 85, 81, 
	   50, -10, 54, 18, 32, 31, 56, 75, 
	   25, 17, -12, 11, 13, 18, 4, 32, 
	   11, -38, -15, -5, 14, -12, 30, 8,
	   },
	   { // King
	   -70, -15, -10, 8, -4, 40, 29, 16, 
	   14, 22, 13, 3, 3, 33, 28, 16, 
	   20, 23, 30, 5, 20, 43, 26, 15, 
	   -16, 19, 24, 29, 20, 25, 32, -1, 
	   -26, -12, 27, 22, 27, 19, 11, -5, 
	   -8, -15, -1, 19, 29, 22, 7, -5, 
	   -33, -19, 10, 9, 14, 2, -13, -27, 
	   -63, -48, -33, -15, -34, -20, -46, -63,
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
	int king_danger[2];
	int king_attackers[2];

	double attack_potency[6] = { 0, 8, 36, 22, 60, 0 };
	double king_danger_weight[8] = { 0, 14, 60, 125, 237, 103, 97, 99 };

	double average_mobility[6] = { 0, 30, 51, 39, 36, 0 };
	double mg_mobility_weight[6] = { 0, 40, 75, 58, 0, 0 };
	double eg_mobility_weight[6] = { 0, 40, 42, 36, 56, 0 };

	double mg_isolated_penalty = 21;
	double eg_isolated_penalty = 13;

	double mg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 
	8, -6, 37, 53, 59, 21, 75, 41, 
	36, 13, -27, -26, 30, 24, 61, 42, 
	38, 9, 10, -20, 7, 45, -1, 25, 
	29, 3, -15, 6, 13, 16, 54, 34, 
	17, 45, 3, 8, -22, 35, 26, 1, 
	14, 27, 33, 35, 53, 56, 55, 39, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	};

	double eg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0,
	-4, 14, -8, -4, 44, 8, 0, 15,
	42, 50, 32, 26, 8, 32, 44, 46,
	54, 58, 46, 40, 26, 30, 44, 52,
	58, 62, 54, 34, 38, 36, 56, 50,
	46, 38, 28, 22, 20, 26, 38, 36,
	-8, 12, 2, 3, 19, 6, 2, 10,
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

		mg_value += mg_bonus[WHITE] - mg_bonus[BLACK];
		eg_value += eg_bonus[WHITE] - eg_bonus[BLACK];

		// Tapered Eval
		// interpolation between midgame and endgame to create a smooth transition
		double value = (mg_value * phase + eg_value * (256 - phase)) / 256;
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
