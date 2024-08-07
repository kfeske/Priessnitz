#include <psqt.h>

struct Evaluation : Noncopyable
{
	PSQT psqt;
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_zone[2];
	int king_danger[2];
	int king_attackers[2];

	int attack_potency[6] = { 0, -4, 27, 32, 87, 0 };
	int king_danger_weight[8] = { 0, 11, 55, 98, 136, 43, 0, 0 };

	int average_mobility[6] = { 0, -42, -21, -49, -112, 0 };
	int mg_mobility_weight[6] = { 0, 144, 91, 77, 22, 0 };
	int eg_mobility_weight[6] = { 0, -15, 29, 29, 69, 0 };

	int mg_isolated_penalty = 21;
	int eg_isolated_penalty = 11;

	int tempo_bonus = 0;

	int mg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 
	55, 84, 44, 67, 46, 79, 14, -10, 
	67, 24, 28, 14, 0, 16, -23, -25, 
	38, 13, 10, -4, 2, 29, -7, 0, 
	26, -13, -20, -19, -20, -15, -12, 25, 
	10, -5, -21, -36, -3, 24, 11, 27, 
	7, 10, 15, -19, -13, 20, 19, 4, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	};

	int eg_passed_bonus[64] =
	{
	0, 0, 0, 0, 0, 0, 0, 0, 
	115, 106, 101, 81, 94, 83, 105, 121, 
	144, 145, 111, 95, 74, 102, 123, 135, 
	78, 72, 58, 49, 36, 41, 75, 69, 
	43, 43, 33, 26, 24, 28, 48, 38, 
	20, 19, 20, 18, 9, 5, 25, 16, 
	16, 19, 1, 17, 19, 8, 17, 18, 
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
			mg_bonus[color] += mg_passed_bonus[normalize[color][square]];
			eg_bonus[color] += eg_passed_bonus[normalize[color][square]];
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

			mg_value += psqt.midgame[p][square] + piece_value(p, MIDGAME);
			eg_value += psqt.endgame[p][square] + piece_value(p, ENDGAME);

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
