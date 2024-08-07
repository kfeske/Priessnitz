#include <psqt.h>

struct Evaluation : Noncopyable
{
	PSQT psqt;
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	uint64_t king_zone[2];
	int ring_pressure[2];
	int zone_pressure[2];
	int ring_attackers[2];
	int zone_attackers[2];

	int mg_piece_value[6] = { 98, 344, 385, 506, 1213, 0 };
	int eg_piece_value[6] = { 107, 293, 311, 546, 928, 0 };

	int ring_attack_potency[6] = { 0, 2, 30, 38, 119, 0 };
	int zone_attack_potency[6] = { 0, -11, 23, 17, 20, 0 };

	int ring_pressure_weight[8] = { 0, 7, 38, 67, 103, 48, 0, 0 };
	int zone_pressure_weight[8] = { 0, 3, 11, 20, 29, 38, 35, 4 };

	int mg_average_mobility[6] = { 0, -23, -27, -24, -65, 0 };
	int eg_average_mobility[6] = { 0, 5, 9, -10, -11, 0 };
	int mg_mobility_weight[6] = { 0, 111, 84, 82, 18, 0 };
	int eg_mobility_weight[6] = { 0, 10, 27, 31, 85, 0 };

	int mg_isolated_penalty = -19;
	int eg_isolated_penalty = -11;

	int mg_doubled_penalty = -4;
	int eg_doubled_penalty = -5;

	int mg_backward_penalty = -12;
	int eg_backward_penalty = -13;

	int mg_chained_bonus = 10;
	int eg_chained_bonus = 7;

	int mg_passed_bonus[64] =
	{
	 0, 0, 0, 0, 0, 0, 0, 0, 
	 39, 71, 27, 52, 31, 69, 0, -28, 
	 56, 16, 27, 9, -8, -2, -50, -41, 
	 38, 20, 15, -2, 7, 33, -4, -2, 
	 24, -16, -25, -21, -22, -17, -16, 22, 
	 11, -6, -19, -40, -2, 29, 11, 30, 
	 2, 7, 11, -21, -9, 22, 18, 0, 
	 0, 0, 0, 0, 0, 0, 0, 0, 
	};

	int eg_passed_bonus[64] =
	{
	 0, 0, 0, 0, 0, 0, 0, 0, 
	 113, 102, 97, 78, 90, 79, 103, 120, 
	 151, 150, 114, 100, 80, 107, 130, 143, 
	 84, 73, 61, 52, 41, 47, 79, 77, 
	 47, 46, 37, 28, 29, 34, 51, 43, 
	 19, 20, 18, 18, 9, 6, 26, 17, 
	 17, 17, 2, 18, 19, 8, 18, 20, 
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

		switch (type_of(p)) {
		case PAWN:
			evaluate_pawn(board, square, friendly);
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
			// queen and rooks are not counted as a blockers, because their pressure increases when stacked
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
			PieceType type = type_of(p);
			Color friendly = color_of(p);

			mg_bonus[friendly] += psqt.midgame[type][normalize[friendly][square]] + mg_piece_value[type];
			eg_bonus[friendly] += psqt.endgame[type][normalize[friendly][square]] + eg_piece_value[type];

			evaluate_piece(board, p, square);
		}

		evaluate_kings();

		//mg_bonus[board.side_to_move] += tempo_bonus;

		mg_value += mg_bonus[WHITE] - mg_bonus[BLACK];
		eg_value += eg_bonus[WHITE] - eg_bonus[BLACK];

		// Tapered Eval
		// interpolation between midgame and endgame to create a smooth transition
		int value = (mg_value * phase + eg_value * (256 - phase)) / 256;
		return (board.side_to_move == WHITE) ? value: -value;
	}
};
