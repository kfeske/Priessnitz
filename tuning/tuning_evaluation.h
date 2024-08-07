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
	   42, 72, 31, 54, 32, 59, 0, -18,
	   -27, -16, 2, 2, 52, 73, 22, -10,
	   -37, -13, -12, 16, 13, 5, 0, -28,
	   -47, -31, -7, 10, 16, 0, -15, -38,
	   -37, -33, -10, -9, 8, 5, 10, -13,
	   -37, -22, -33, -19, -12, 29, 22, -19,
	   0, 0, 0, 0, 0, 0, 0, 0,
	   },
	   { // Knight
	   -148, -49, -36, -41, 6, -91, -13, -88,
	   -95, -55, 66, 10, 2, 32, -13, -49,
	   -55, 56, 30, 59, 75, 102, 75, 17,
	   -9, 28, 15, 61, 38, 71, 16, 10,
	   -12, 8, 26, 21, 43, 29, 20, -7,
	   -19, 0, 28, 26, 37, 35, 26, -13,
	   -22, -46, -6, 20, 15, 30, -8, -7,
	   -83, -4, -34, -21, 11, -12, 0, -32,
	   },
	   { // Bishop
	   -21, -10, -70, -36, -19, -35, -5, -3,
	   -33, -2, -37, -35, 4, 25, -8, -76,
	   -19, 19, 26, 10, 11, 18, 14, -8,
	   -7, 3, -5, 16, 11, 14, 5, -9,
	   7, 8, -8, 9, 14, -4, 4, 15,
	   10, 20, 5, -5, 11, 25, 20, 24,
	   16, 28, 12, 3, 9, 20, 44, 14,
	   -18, 13, 15, 4, 18, 10, -16, -3,
	   },
	   { // Rook
	   23, 30, 7, 47, 33, -14, -6, 11,
	   22, 20, 59, 52, 70, 50, 0, 18,
	   -5, 26, 15, 30, -2, 23, 47, 0,
	   -28, -14, 8, 10, 9, 21, -15, -19,
	   -48, -27, -14, -12, 0, -23, -3, -33,
	   -44, -23, -12, -13, 0, -8, -14, -35,
	   -36, -11, -14, -4, 10, 7, -6, -57,
	   -17, -16, -4, 1, 8, 0, -25, -1,
	   },
	   { // Queen
	   -1, -2, 4, 0, 45, 25, 24, 40,
	   -20, -50, -16, 11, -29, 11, -5, 33,
	   -1, -6, 15, -20, 13, 37, 17, 29,
	   -23, -21, -24, -37, -6, -9, -10, -9,
	   3, -29, -7, -8, -1, -6, 2, -1,
	   -13, 16, 0, 5, 8, 13, 22, 11,
	   -20, 7, 27, 23, 25, 28, 11, 21,
	   19, 0, 10, 33, 5, -6, -11, -32,
	   },
	   { // King
	   -8, 8, 13, 3, -11, -3, 1, 0,
	   11, 19, 10, 21, 11, 10, -4, -12,
	   10, 28, 36, 16, 17, 39, 39, -8,
	   1, 9, 15, 7, -4, -5, 5, -46,
	   -29, 30, 2, -45, -58, -42, -39, -77,
	   2, 13, -5, -35, -37, -26, 10, -30,
	   27, 40, -2, -58, -31, -8, 36, 26,
	   -15, 61, 32, -54, 19, -18, 52, 27,
	   }
	  },

	  { // Endgame
 	   { // Pawn
	   0, 0, 0, 0, 0, 0, 0, 0,
	   109, 101, 96, 75, 88, 78, 100, 116,
	   3, 0, 4, -5, 0, -21, 1, 2,
	   -6, -17, -17, -27, -23, -15, -19, -16,
	   -17, -16, -24, -25, -26, -30, -27, -25,
	   -27, -23, -22, -20, -13, -26, -35, -34,
	   -27, -27, -7, -18, -6, -23, -38, -36,
	   	   },
	   { // Knight
	   -66, -38, 5, -16, -6, -17, -50, -94,
	   -10, 8, -6, 25, 16, 0, -5, -36,
	   -9, -1, 35, 30, 20, 20, 0, -20,
	   -2, 22, 48, 43, 48, 33, 27, 2,
	   0, 12, 37, 46, 37, 33, 24, -2,
	   -7, 13, 19, 30, 26, 12, -4, -9,
	   -24, -4, 6, 6, 5, 0, -10, -36,
	   -28, -40, -12, 0, -13, -9, -42, -51,
	   },
	   { // Bishop
	   -7, -11, -5, -3, 5, -1, 0, -18,
	   6, 0, 13, -5, 3, -1, 0, 4,
	   11, -4, 2, -1, 0, 1, 5, 10,
	   7, 9, 10, 7, 4, 5, -1, 8,
	   5, 2, 11, 12, 0, 5, -2, -2,
	   -1, 3, 9, 8, 9, -6, 1, -4,
	   -6, -14, -4, 4, 6, -5, -10, -20,
	   -11, -1, -10, 2, -2, -7, 3, -6,
	   },
	   { // Rook
	   15, 9, 16, 11, 12, 16, 16, 10,
	   12, 16, 6, 7, -3, 6, 13, 8,
	   5, 6, 2, 6, 5, -2, -4, 0,
	   3, 1, 10, -2, 2, -1, 0, 6,
	   6, 8, 6, 1, -5, -9, -6, 0,
	   1, -1, -8, -4, -10, -16, -4, -9,
	   0, -4, -3, -3, -12, -11, -13, -1,
	   -10, -1, 1, 0, -6, -12, -3, -28,
	   },
	   { // Queen
	   -21, 25, 28, 26, 30, 27, 21, 35,
	   -14, 18, 31, 33, 47, 31, 35, 21,
	   -18, -10, -19, 43, 32, 16, 23, 28,
	   11, 17, 0, 17, 24, 21, 58, 46,
	   -13, 19, -10, 3, -4, 3, 20, 27,
	   -3, -52, -13, -25, -10, -13, -3, 14,
	   -7, -40, -49, -35, -29, -38, -49, -18,
	   -38, -36, -35, -60, -12, -31, -14, -34,
	   },
	   { // King
	   -67, -39, -22, -20, -16, 11, -3, -18,
	   -17, 10, 8, 14, 14, 35, 19, 2,
	   0, 16, 17, 10, 17, 46, 41, 6,
	   -15, 16, 26, 26, 25, 31, 25, 3,
	   -23, -8, 21, 32, 33, 26, 7, -6,
	   -22, -7, 9, 22, 26, 20, 6, -4,
	   -35, -16, 8, 13, 20, 7, -14, -26,
	   -64, -49, -27, -10, -33, -13, -42, -60,
	   }
	  }
	 };

	void prepare()
	{
		for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING }) {
			for (unsigned square = 0; square < 64; square++) {
				midgame[pc][square] = bonus[MIDGAME][type_of(pc)][square];
				endgame[pc][square] = bonus[ENDGAME][type_of(pc)][square];
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

	double mg_piece_value[6] = { 111, 414, 432, 529, 1195, 0 };
	double eg_piece_value[6] = { 112, 301, 320, 564, 1015, 0 };

	double attack_potency[6] = { 0, 11, 29, 36, 89, 0 };
	double king_danger_weight[8] = { 0, 13, 55, 94, 123, 39, 0, 0 };

	double average_mobility[6] = { 0, 14, 36, 17, 54, 0 };
	double mg_mobility_weight[6] = { 0, 32, 84, 77, 10, 0 };
	double eg_mobility_weight[6] = { 0, -90, 27, 28, 76, 0 };

	double mg_isolated_penalty = 22;
	double eg_isolated_penalty = 11;

	double tempo_bonus = 0;

	double mg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0,
	42, 72, 31, 54, 32, 59, 0, -18,
	65, 25, 29, 13, 3, 15, -17, -22,
	37, 13, 8, -6, 1, 29, -8, 0,
	25, -13, -20, -18, -21, -17, -13, 27,
	10, -6, -21, -37, -3, 24, 14, 29,
	9, 11, 17, -20, -9, 23, 22, 7,
	0, 0, 0, 0, 0, 0, 0, 0,
	};

	double eg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0,
	109, 101, 96, 75, 88, 78, 100, 116,
	149, 149, 114, 95, 72, 105, 125, 139,
	80, 74, 61, 52, 38, 42, 78, 71,
	45, 45, 35, 27, 25, 30, 50, 39,
	21, 20, 21, 19, 9, 5, 25, 16,
	17, 20, 0, 17, 19, 7, 17, 19,
	0, 0, 0, 0, 0, 0, 0, 0,
	};

	double taper_start = 6377;
	double taper_end = 321;

	// pawn structure evaluation
	void evaluate_pawn(Board &board, unsigned square, Color color)
	{
		uint64_t friendly_pawns = board.pieces[piece_of(color, PAWN)];
		uint64_t enemy_pawns = board.pieces[piece_of(!color, PAWN)];

		bool blocked = board.precomputed.forward_file_mask[color][square] & friendly_pawns;

		unsigned index = normalize[color][square];

		// reward passed pawns
		if (!(board.precomputed.passed_pawn_mask[color][square] & enemy_pawns) && !blocked) {
			mg_bonus[color] += mg_passed_bonus[index];
			eg_bonus[color] += eg_passed_bonus[index];
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
		return;
		// don't count mobility, when the squares are controlled by enemy pawns
		uint64_t enemy_pawns = board.pieces[piece_of(!friendly, PAWN)];
		uint64_t danger_squares = (friendly == WHITE) ? ((enemy_pawns & ~FILE_H) >> 7) | ((enemy_pawns >> 9) & ~FILE_A) :
								((enemy_pawns & ~FILE_H) << 9) | ((enemy_pawns << 7) & ~FILE_A);
		int safe_squares = pop_count(attacks & ~danger_squares);
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
			uint64_t attacks = board.precomputed.attacks_bb<KNIGHT>(square, 0ULL);
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
		material = std::max(int(taper_end), std::min(material, int(taper_start))); // endgame and midgame limit clamp
		double phase = ((material - taper_end) * 256) / (taper_start - taper_end); // 0(Endgame) - 256(Midgame) linear interpolation
		//std::cerr << phase;

		uint64_t pieces = board.occ;

		while (pieces) {
			unsigned square = pop_lsb(pieces);
			Piece p = board.board[square];
			Color color = color_of(p);
			PieceType type = type_of(p);

			// material and positional evaluation via Piece Square Tables
			mg_bonus[color] += psqt.midgame[type][normalize[color][square]] + mg_piece_value[type];
			eg_bonus[color] += psqt.endgame[type][normalize[color][square]] + eg_piece_value[type];

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
