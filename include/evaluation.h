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

	int mg_piece_value[6] = { 99, 346, 365, 509, 1231, 0 };
	int eg_piece_value[6] = { 107, 294, 308, 547, 925, 0 };

	int mg_pawn_psqt[64] = {
	 0, 0, 0, 0, 0, 0, 0, 0, 
	 39, 70, 27, 52, 29, 67, 0, -27, 
	 -13, -9, 2, 10, 62, 84, 38, 3, 
	 -35, -19, -12, 10, 5, 2, -9, -25, 
	 -39, -33, -9, 3, 10, 4, -12, -30, 
	 -39, -35, -19, -13, 0, -2, 11, -20, 
	 -29, -22, -20, -6, -8, 35, 30, -9, 
	 0, 0, 0, 0, 0, 0, 0, 0, 
	};

	int mg_knight_psqt[64] = {
	 -155, -70, -53, -53, 19, -107, -13, -96, 
	 -81, -46, 76, 16, 12, 40, 2, -33, 
	 -52, 51, 10, 37, 56, 98, 68, 42, 
	 5, 19, 5, 54, 33, 52, 20, 30, 
	 3, 9, 5, 13, 23, 21, 25, 15, 
	 -7, -13, 0, -4, 17, 6, 22, -3, 
	 -3, -37, -17, 14, 7, 20, -1, 12, 
	 -90, 11, -24, -11, 22, -3, 13, 5, 
	};
	 
	int mg_bishop_psqt[64] = {
	 -28, -18, -125, -66, -38, -58, -15, -10, 
	 -37, -1, -46, -52, 0, 22, -12, -81, 
	 -16, 21, 24, 0, 0, 18, 4, -13, 
	 -4, 17, 0, 29, 11, 14, 9, -9, 
	 9, 10, 10, 17, 25, 0, 5, 15, 
	 14, 24, 15, 15, 13, 32, 14, 20, 
	 27, 30, 19, 11, 22, 25, 53, 19, 
	 -17, 20, 22, 10, 23, 16, -14, 2, 
	};

	int mg_rook_psqt[64] = {
	 22, 24, -6, 41, 40, -35, -10, 10, 
	 19, 16, 54, 48, 68, 42, -6, 17, 
	 -5, 25, 21, 37, 0, 30, 56, -1, 
	 -24, -9, 13, 14, 14, 20, -11, -25, 
	 -48, -26, -11, -12, -1, -22, -3, -41, 
	 -42, -22, -10, -12, 4, -7, -12, -37, 
	 -40, -13, -13, 0, 11, 1, -5, -60, 
	 -18, -15, -5, 4, 9, -2, -25, -7, 
	};

	int mg_queen_psqt[64] = {
	 5, -20, 0, -13, 63, 32, 28, 44, 
	 -12, -40, -20, 0, -56, 3, -7, 42, 
	 7, 0, 22, -24, 11, 52, 17, 37, 
	 -19, -11, -22, -34, -17, -17, -20, -7, 
	 11, -23, -1, -12, -8, -11, -3, -4, 
	 -3, 28, 4, 10, 2, 5, 18, 11, 
	 -6, 17, 34, 24, 31, 32, 21, 27, 
	 35, 10, 21, 41, 14, 5, 0, -26, 
	};

	int mg_king_psqt[64] = {
	 -15, 24, 33, 10, -24, -10, 4, 2, 
	 27, 41, 23, 49, 23, 19, -10, -33, 
	 22, 57, 74, 28, 35, 75, 78, -20, 
	 10, 16, 28, 6, -11, -16, 9, -73, 
	 -48, 49, -3, -69, -77, -49, -49, -98, 
	 4, 10, -11, -36, -40, -34, 1, -40, 
	 20, 31, -9, -60, -37, -12, 28, 18, 
	 -27, 50, 25, -65, 9, -25, 42, 15, 
	};

	int eg_pawn_psqt[64] = {
	 0, 0, 0, 0, 0, 0, 0, 0, 
	 113, 102, 97, 79, 91, 79, 104, 121, 
	 9, 6, 11, -7, -4, -15, 3, 4, 
	 -5, -10, -15, -27, -22, -19, -17, -14, 
	 -14, -16, -22, -24, -24, -28, -28, -25, 
	 -23, -22, -23, -16, -17, -23, -37, -32, 
	 -19, -25, -9, -18, -6, -23, -35, -35, 
	 0, 0, 0, 0, 0, 0, 0, 0, 
	};

	int eg_knight_psqt[64] = {
	 -42, -26, 10, -11, -7, -10, -50, -85, 
	 -3, 9, -15, 16, 8, -10, -6, -32, 
	 -5, -3, 27, 23, 10, 6, -6, -26, 
	 -1, 19, 40, 32, 35, 23, 20, -1, 
	 0, 10, 30, 41, 29, 29, 17, -2, 
	 -6, 14, 13, 30, 23, 12, -2, -5, 
	 -20, 0, 10, 5, 10, -1, -4, -33, 
	 -3, -31, -9, 4, -10, -5, -35, -53, 
	};

	int eg_bishop_psqt[64] = {
	 -8, -10, 4, 1, 4, 0, 0, -17, 
	 5, 1, 13, -1, 3, -2, 0, 4, 
	 9, -3, -1, 0, -1, 2, 6, 11, 
	 4, 5, 9, 5, 5, 6, -3, 7, 
	 0, 1, 9, 12, 0, 6, -3, -2, 
	 -4, 0, 9, 10, 13, -2, 0, -5, 
	 -6, -15, -4, 1, 2, -5, -14, -22, 
	 -11, 0, -12, -1, -6, -6, 5, -9, 
	};

	int eg_rook_psqt[64] = {
	 7, 6, 16, 5, 8, 17, 15, 6, 
	 6, 10, 3, 5, -9, 1, 12, 5, 
	 7, 5, 3, 1, 4, -2, -6, 0, 
	 8, 3, 10, 0, 4, 2, 1, 8, 
	 11, 8, 8, 4, -2, -2, -5, 1, 
	 4, 1, -7, -4, -10, -11, -4, -9, 
	 0, -4, -3, -2, -14, -10, -11, -1, 
	 -8, 0, 0, -3, -7, -8, -3, -28, 
	};

	int eg_queen_psqt[64] = {
	 -28, 28, 21, 27, 10, 16, 13, 29, 
	 -23, 6, 23, 29, 58, 28, 35, 16, 
	 -19, -10, -27, 45, 31, 8, 24, 23, 
	 16, 13, 1, 21, 34, 29, 74, 52, 
	 -14, 18, -6, 11, 5, 13, 32, 35, 
	 -7, -52, -14, -18, -3, -4, 0, 20, 
	 -12, -43, -48, -33, -30, -42, -54, -31, 
	 -48, -37, -40, -71, -17, -34, -18, -39, 
	};

	int eg_king_psqt[64] = {
	 -89, -43, -25, -21, -13, 13, -2, -22, 
	 -20, 8, 8, 11, 16, 33, 21, 6, 
	 -2, 11, 13, 11, 14, 41, 36, 8, 
	 -18, 15, 22, 28, 27, 35, 25, 5, 
	 -19, -11, 21, 34, 36, 29, 13, -3, 
	 -25, -5, 13, 25, 28, 21, 6, -6, 
	 -37, -15, 8, 17, 17, 8, -9, -25, 
	 -65, -46, -26, -7, -26, -11, -38, -60, 
	};

	int ring_attack_potency[6] = { 0, 3, 27, 38, 119, 0 };
	int zone_attack_potency[6] = { 0, -9, 20, 16, 20, 0 };

	int ring_pressure_weight[8] = { 0, 7, 38, 67, 105, 49, 0, 0 };
	int zone_pressure_weight[8] = { 0, 5, 12, 20, 30, 40, 37, 4 };

	int mg_average_mobility[6] = { 0, -22, -17, -25, -69, 0 };
	int eg_average_mobility[6] = { 0, 5, 9, -10, -8, 0 };
	int mg_mobility_weight[6] = { 0, 111, 85, 82, 17, 0 };
	int eg_mobility_weight[6] = { 0, 8, 29, 31, 85, 0 };

	int mg_isolated_penalty = -19;
	int eg_isolated_penalty = -12;

	int mg_doubled_penalty = -4;
	int eg_doubled_penalty = -5;

	int mg_backward_penalty = -12;
	int eg_backward_penalty = -13;

	int mg_chained_bonus = 10;
	int eg_chained_bonus = 7;

	int mg_double_bishop = 33;
	int eg_double_bishop = 53;

	int mg_passed_bonus[64] =
	{
	 0, 0, 0, 0, 0, 0, 0, 0, 
	 39, 70, 27, 52, 29, 67, 0, -27, 
	 56, 16, 26, 8, -11, -4, -49, -40, 
	 38, 21, 15, -2, 7, 34, -3, -2, 
	 24, -15, -25, -20, -22, -17, -15, 22, 
	 11, -5, -19, -39, -3, 30, 12, 30, 
	 3, 8, 11, -21, -8, 21, 16, 0, 
	 0, 0, 0, 0, 0, 0, 0, 0, 
	};

	int eg_passed_bonus[64] =
	{
	 0, 0, 0, 0, 0, 0, 0, 0, 
	 113, 102, 97, 79, 91, 79, 104, 121, 
	 151, 150, 115, 101, 81, 107, 130, 143, 
	 84, 73, 61, 52, 41, 47, 79, 76, 
	 46, 45, 38, 28, 29, 34, 51, 43, 
	 19, 19, 18, 18, 9, 5, 26, 17, 
	 16, 17, 1, 18, 19, 8, 18, 20, 
	 0, 0, 0, 0, 0, 0, 0, 0, 
	};
	
	int tempo_bonus = 0;

	int taper_start = 6377;
	int taper_end = 321;

	// pawn structure evaluation
	void evaluate_pawn(Board &board, unsigned square, Color friendly)
	{
		uint64_t friendly_pawns = board.pieces[piece_of(friendly, PAWN)];
		uint64_t enemy_pawns = board.pieces[piece_of(!friendly, PAWN)];
		unsigned forward = (friendly == WHITE) ? NORTH : SOUTH;
		uint64_t adjacent_files = board.precomputed.isolated_pawn_mask[file(square)];

		bool passed = !(board.precomputed.passed_pawn_mask[friendly][square] & enemy_pawns);
		bool doubled = board.precomputed.forward_file_mask[friendly][square] & friendly_pawns;
		bool neighbored = board.precomputed.neighbor_mask[square] & friendly_pawns;
		bool supported = board.precomputed.passed_pawn_mask[!friendly][square] & adjacent_files & friendly_pawns;
		bool chained = board.precomputed.pawn_attacks[!friendly][square] & friendly_pawns;

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

	template <PieceType pt>
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

	template <PieceType pt>
	void evaluate_mobility(Board &board, uint64_t attacks, Color friendly)
	{
		// do not count mobility, when the squares are protected by enemy pawns
		int safe_squares = pop_count(attacks & ~board.pawn_attacks(Color(!friendly)));
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
