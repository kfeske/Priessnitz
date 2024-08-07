#include <psqt.h>

struct Evaluation : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	uint64_t king_zone[2];
	int ring_pressure[2];
	int zone_pressure[2];
	int ring_attackers[2];
	int zone_attackers[2];
	
	int mg_piece_value[6] = { 98, 347, 365, 501, 1178, 0, };
	int eg_piece_value[6] = { 108, 292, 306, 550, 951, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		26, 68, 19, 45, 30, 51, -7, -37,
		-11, -7, 0, 3, 56, 79, 27, 0,
		-34, -18, -9, 15, 11, 7, -8, -23,
		-38, -32, -5, 7, 15, 8, -11, -28,
		-37, -34, -16, -9, 3, 1, 14, -16,
		-27, -19, -19, -4, -6, 39, 34, -6,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-112, -44, -28, -29, 16, -76, -10, -63,
		-73, -47, 76, 18, 12, 31, -8, -35,
		-58, 43, 10, 34, 50, 82, 61, 40,
		1, 16, -2, 51, 30, 50, 14, 29,
		-3, 7, 2, 6, 22, 18, 20, 12,
		-11, -17, -4, -11, 14, 2, 20, -7,
		-1, -35, -23, 12, 4, 21, -12, 7,
		-58, 11, -24, -21, 18, -9, 12, -5,
	};
	int mg_bishop_psqt[64] = {
		-36, -18, -79, -49, -12, -20, -14, -9,
		-43, -3, -54, -43, 0, 22, -18, -85,
		-23, 25, 21, -5, -3, 26, -1, -19,
		-5, 15, 4, 26, 10, 9, 10, -18,
		9, 8, 6, 16, 22, -1, 4, 23,
		13, 20, 14, 13, 12, 32, 8, 19,
		21, 30, 19, 9, 18, 22, 53, 22,
		-22, 19, 23, 8, 13, 14, -21, 0,
	};
	int mg_rook_psqt[64] = {
		13, 28, -15, 40, 26, -32, 1, 5,
		25, 20, 61, 49, 65, 41, -5, 14,
		-7, 33, 23, 45, -11, 28, 46, 1,
		-27, -5, 15, 15, 23, 16, -16, -26,
		-55, -21, -14, -12, 3, -27, -2, -42,
		-45, -24, -11, -14, 7, -6, -15, -39,
		-43, -11, -14, 0, 13, 2, 0, -61,
		-20, -16, -5, 6, 11, 0, -28, -5,
	};
	int mg_queen_psqt[64] = {
		8, -9, 7, -6, 44, 25, 24, 38,
		-11, -46, -9, 12, -29, 10, 1, 40,
		6, 1, 15, -27, 8, 34, 11, 38,
		-23, -14, -23, -43, -23, -23, -16, -12,
		14, -32, -1, -14, -12, -14, -5, -3,
		-4, 32, 5, 9, -3, 4, 20, 6,
		-2, 19, 38, 26, 32, 33, 18, 20,
		28, 10, 24, 44, 14, 4, -6, -34,
	};
	int mg_king_psqt[64] = {
		-10, 4, 15, 3, -13, 2, 3, -1,
		15, 14, 12, 25, 14, 13, -7, -13,
		9, 27, 36, 16, 16, 48, 49, -8,
		-4, 6, 21, 7, -5, -9, 15, -37,
		-16, 35, -2, -57, -61, -40, -29, -67,
		3, 17, -5, -28, -32, -28, 5, -38,
		26, 32, -7, -62, -36, -8, 34, 27,
		-20, 59, 31, -66, 15, -20, 52, 23,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		117, 103, 98, 79, 93, 84, 105, 121,
		7, 9, 13, 0, 9, -12, 6, 2,
		-8, -13, -16, -29, -23, -22, -18, -16,
		-15, -16, -24, -25, -25, -28, -29, -26,
		-27, -23, -26, -20, -19, -24, -38, -32,
		-20, -27, -11, -21, -8, -24, -36, -37,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-67, -32, 4, -14, -8, -18, -47, -84,
		-9, 9, -15, 17, 6, -8, -2, -33,
		-3, -3, 25, 25, 12, 13, -2, -20,
		0, 19, 42, 33, 34, 24, 23, 3,
		1, 10, 29, 43, 30, 29, 22, 1,
		-5, 16, 13, 30, 24, 11, -2, -4,
		-21, 0, 10, 5, 10, -3, -4, -31,
		-17, -32, -11, 6, -11, -7, -37, -41,
	};
	int eg_bishop_psqt[64] = {
		-6, -12, -6, -2, -4, -8, 6, -17,
		4, 0, 14, -8, 3, 0, 2, 5,
		9, -4, 0, 2, -3, 0, 6, 12,
		3, 7, 7, 5, 7, 6, -4, 12,
		-1, 3, 12, 12, 0, 7, -5, -5,
		-5, 2, 9, 11, 15, -2, 3, -5,
		-5, -14, -3, 2, 3, -6, -13, -18,
		-10, 2, -14, -1, -4, -7, 5, -7,
	};
	int eg_rook_psqt[64] = {
		10, 7, 18, 7, 12, 15, 14, 9,
		4, 9, 3, 7, -9, 2, 11, 8,
		8, 5, 4, 0, 8, -4, -5, 0,
		11, 2, 10, 0, 1, 6, 2, 8,
		13, 7, 11, 3, -4, -3, -7, 2,
		3, 2, -6, -4, -11, -13, -1, -7,
		0, -3, -3, -4, -15, -10, -14, -2,
		-8, 1, 0, -4, -9, -10, -2, -31,
	};
	int eg_queen_psqt[64] = {
		-26, 18, 21, 17, 25, 20, 20, 34,
		-21, 5, 11, 23, 37, 23, 22, 24,
		-17, -13, -21, 42, 37, 23, 28, 26,
		15, 13, 2, 21, 33, 25, 63, 51,
		-19, 19, -8, 7, 4, 14, 30, 27,
		-3, -53, -13, -18, -1, -6, 3, 22,
		-22, -42, -50, -29, -29, -37, -45, -24,
		-33, -29, -37, -63, -11, -26, -21, -37,
	};
	int eg_king_psqt[64] = {
		-66, -49, -18, -19, -17, 5, -5, -16,
		-16, 14, 7, 15, 17, 34, 17, 4,
		3, 15, 17, 12, 15, 46, 37, 5,
		-15, 18, 22, 27, 27, 35, 24, 1,
		-25, -9, 22, 31, 35, 29, 9, -9,
		-26, -7, 14, 23, 26, 21, 5, -8,
		-39, -15, 8, 17, 17, 8, -9, -26,
		-64, -46, -27, -8, -28, -14, -40, -61,
	};
	
	int ring_attack_potency[6] = { 0, 3, 32, 39, 131, 0, };
	int zone_attack_potency[6] = { 0, -13, 28, 21, 29, 0, };
	
	int ring_pressure_weight[8] = { 0, 7, 37, 66, 94, 38, 0, 0, };
	int zone_pressure_weight[8] = { 0, 5, 11, 16, 24, 29, 29, 3, };
	
	int mg_average_mobility[6] = { 0, -27, -24, -20, -49, 0, };
	int eg_average_mobility[6] = { 0, 1, 5, -17, -30, 0, };
	int mg_mobility_weight[6] = { 0, 116, 86, 85, 17, 0, };
	int eg_mobility_weight[6] = { 0, 14, 32, 32, 78, 0, };
	
	int mg_isolated_penalty = -20;
	int eg_isolated_penalty = -12;
	
	int mg_doubled_penalty = -4;
	int eg_doubled_penalty = -6;
	
	int mg_backward_penalty = -13;
	int eg_backward_penalty = -13;
	
	int mg_chained_bonus = 12;
	int eg_chained_bonus = 8;
	
	int mg_passed_bonus[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		26, 68, 19, 45, 30, 51, -7, -37,
		48, 12, 28, 12, 3, 2, -35, -32,
		26, 16, 8, -6, 9, 34, -3, -11,
		21, -21, -27, -24, -30, -8, -10, 16,
		7, -1, -21, -37, -1, 33, 17, 35,
		5, 6, 5, -18, -19, 25, 18, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_passed_bonus[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		117, 103, 98, 79, 93, 84, 105, 121,
		154, 147, 112, 93, 66, 104, 123, 143,
		89, 75, 63, 55, 40, 49, 79, 78,
		48, 45, 39, 30, 29, 32, 49, 44,
		20, 19, 19, 19, 10, 6, 23, 17,
		16, 19, 2, 22, 17, 6, 16, 22,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_double_bishop = 29;
	int eg_double_bishop = 53;
	
	int tempo_bonus = 0;

	int taper_start = 6377;
	int taper_end = 321;

	// pawn structure evaluation
	void evaluate_pawn(Board &board, unsigned square, Color friendly)
	{
		Color enemy = swap(friendly);
		uint64_t friendly_pawns = board.pieces[piece_of(friendly, PAWN)];
		uint64_t enemy_pawns = board.pieces[piece_of(enemy, PAWN)];
		unsigned forward = (friendly == WHITE) ? NORTH : SOUTH;
		uint64_t adjacent_files = board.precomputed.isolated_pawn_mask[file(square)];

		bool passed = !(board.precomputed.passed_pawn_mask[friendly][square] & enemy_pawns);
		bool doubled = board.precomputed.forward_file_mask[friendly][square] & friendly_pawns;
		bool neighbored = board.precomputed.neighbor_mask[square] & friendly_pawns;
		bool supported = board.precomputed.passed_pawn_mask[enemy][square] & adjacent_files & friendly_pawns;
		bool chained = board.precomputed.pawn_attacks[enemy][square] & friendly_pawns;

		// punish isolated pawns
		if (!(adjacent_files & friendly_pawns)) {
			mg_bonus[friendly] += mg_isolated_penalty;
			eg_bonus[friendly] += eg_isolated_penalty;
		}

		// punish doubled pawns
		else if (doubled) {
			mg_bonus[friendly] += mg_doubled_penalty;
			eg_bonus[friendly] += eg_doubled_penalty;
		}

		// punish backward pawns
		else if (!(supported || neighbored) && board.precomputed.pawn_attacks[friendly][square + forward] & enemy_pawns) {
			mg_bonus[friendly] += mg_backward_penalty;
			eg_bonus[friendly] += eg_backward_penalty;
		}

		// reward chained pawns
		if (chained) {
			mg_bonus[friendly] += mg_chained_bonus;
			eg_bonus[friendly] += eg_chained_bonus;
		}

		// reward passed pawns
		if (passed && !doubled) {
			mg_bonus[friendly] += mg_passed_bonus[normalize[friendly][square]];
			eg_bonus[friendly] += eg_passed_bonus[normalize[friendly][square]];
		}
	}

	void evaluate_kings()
	{
		mg_bonus[WHITE] -= ring_pressure[WHITE] * ring_pressure_weight[std::min(ring_attackers[WHITE], 7)] / 100;
		mg_bonus[BLACK] -= ring_pressure[BLACK] * ring_pressure_weight[std::min(ring_attackers[BLACK], 7)] / 100;
		mg_bonus[WHITE] -= zone_pressure[WHITE] * zone_pressure_weight[std::min(zone_attackers[WHITE], 7)] / 100;
		mg_bonus[BLACK] -= zone_pressure[BLACK] * zone_pressure_weight[std::min(zone_attackers[BLACK], 7)] / 100;
	}

	template <Piece_type pt>
	void note_king_attacks(uint64_t attacks, Color friendly)
	{
		if (attacks & king_ring[!friendly]) {
			ring_pressure[!friendly] += ring_attack_potency[pt] * pop_count(attacks & king_ring[!friendly]);
			ring_attackers[!friendly]++;
		}
		if (attacks & king_zone[!friendly]) {
			zone_pressure[!friendly] += zone_attack_potency[pt] * pop_count(attacks & king_zone[!friendly]);
			zone_attackers[!friendly]++;
		}
	}

	template <Piece_type pt>
	void evaluate_mobility(Board &board, uint64_t attacks, Color friendly)
	{
		// do not count mobility, when the squares are protected by enemy pawns
		int safe_squares = pop_count(attacks & ~board.pawn_attacks(swap(friendly)));
		mg_bonus[friendly] += (10 * safe_squares - mg_average_mobility[pt]) * mg_mobility_weight[pt] / 100;
		eg_bonus[friendly] += (10 * safe_squares - eg_average_mobility[pt]) * eg_mobility_weight[pt] / 100;
	}

	void evaluate_piece(Board &board, Piece p, unsigned square)
	{
		Color friendly = color_of(p);
		unsigned relative_square = normalize[friendly][square];

		switch (type_of(p)) {
		case PAWN:
			mg_bonus[friendly] += mg_pawn_psqt[relative_square] + mg_piece_value[PAWN];
			eg_bonus[friendly] += eg_pawn_psqt[relative_square] + eg_piece_value[PAWN];
			evaluate_pawn(board, square, friendly);
			return;

		case KNIGHT:
			{
			mg_bonus[friendly] += mg_knight_psqt[relative_square] + mg_piece_value[KNIGHT];
			eg_bonus[friendly] += eg_knight_psqt[relative_square] + eg_piece_value[KNIGHT];
			uint64_t attacks = board.precomputed.attacks_bb<KNIGHT>(square, 0ULL);
			note_king_attacks<KNIGHT>(attacks, friendly);
			evaluate_mobility<KNIGHT>(board, attacks, friendly);
			return;
			}
		case BISHOP:
			{
			mg_bonus[friendly] += mg_bishop_psqt[relative_square] + mg_piece_value[BISHOP];
			eg_bonus[friendly] += eg_bishop_psqt[relative_square] + eg_piece_value[BISHOP];
			// queen is not counted as a blocker, because a bishop behind a queen makes the attack more potent
			uint64_t ray_blockers = board.occ & ~board.pieces[piece_of(friendly, QUEEN)];
			uint64_t attacks = board.precomputed.attacks_bb<BISHOP>(square, ray_blockers);
			note_king_attacks<BISHOP>(attacks, friendly);
			evaluate_mobility<BISHOP>(board, attacks, friendly);
			return;
			}
		case ROOK:
			{
			mg_bonus[friendly] += mg_rook_psqt[relative_square] + mg_piece_value[ROOK];
			eg_bonus[friendly] += eg_rook_psqt[relative_square] + eg_piece_value[ROOK];
			// queen and rooks are not counted as a blockers, because their pressure increases when stacked
			uint64_t ray_blockers = board.occ & ~(board.pieces[piece_of(friendly, QUEEN)] | board.pieces[piece_of(friendly, ROOK)]);
			uint64_t attacks = board.precomputed.attacks_bb<ROOK>(square, ray_blockers);
			note_king_attacks<ROOK>(attacks, friendly);
			evaluate_mobility<ROOK>(board, attacks, friendly);
			return;
			}
		case QUEEN:
			{
			mg_bonus[friendly] += mg_queen_psqt[relative_square] + mg_piece_value[QUEEN];
			eg_bonus[friendly] += eg_queen_psqt[relative_square] + eg_piece_value[QUEEN];
			uint64_t attacks = board.precomputed.attacks_bb<QUEEN>(square, board.occ);
			note_king_attacks<QUEEN>(attacks, friendly);
			evaluate_mobility<QUEEN>(board, attacks, friendly);
			return;
			}
		case KING:
			mg_bonus[friendly] += mg_king_psqt[relative_square];
			eg_bonus[friendly] += eg_king_psqt[relative_square];
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
		ring_pressure[WHITE] = 0;
		ring_pressure[BLACK] = 0;
		zone_pressure[WHITE] = 0;
		zone_pressure[BLACK] = 0;
		ring_attackers[WHITE] = 0;
		ring_attackers[BLACK] = 0;
		zone_attackers[WHITE] = 0;
		zone_attackers[BLACK] = 0;
		unsigned white_king_square = lsb(board.pieces[piece_of(WHITE, KING)]);
		unsigned black_king_square = lsb(board.pieces[piece_of(BLACK, KING)]);
		king_ring[WHITE] = board.precomputed.attacks_bb<KING>(white_king_square, 0ULL);
		king_ring[BLACK] = board.precomputed.attacks_bb<KING>(black_king_square, 0ULL);
		king_zone[WHITE] = board.precomputed.king_zone[WHITE][white_king_square];
		king_zone[BLACK] = board.precomputed.king_zone[BLACK][black_king_square];

		int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
		material = std::max(taper_end, std::min(material, taper_start)); // endgame and midgame limit clamp
		int phase = ((material - taper_end) * 256) / (taper_start - taper_end); // 0(Endgame) - 256(Midgame) linear interpolation

		uint64_t pieces = board.occ;

		while (pieces) {
			unsigned square = pop_lsb(pieces);
			Piece p = board.board[square];
			evaluate_piece(board, p, square);
		}

		evaluate_kings();

		if (pop_count(board.pieces[W_BISHOP]) >= 2) {
			mg_bonus[WHITE] += mg_double_bishop;
			eg_bonus[WHITE] += eg_double_bishop;
		}
		if (pop_count(board.pieces[B_BISHOP]) >= 2) {
			mg_bonus[BLACK] += mg_double_bishop;
			eg_bonus[BLACK] += eg_double_bishop;
		}

		//mg_bonus[board.side_to_move] += tempo_bonus;

		mg_value += mg_bonus[WHITE] - mg_bonus[BLACK];
		eg_value += eg_bonus[WHITE] - eg_bonus[BLACK];

		// Tapered Eval
		// interpolation between midgame and endgame to create a smooth transition
		int value = (mg_value * phase + eg_value * (256 - phase)) / 256;
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
