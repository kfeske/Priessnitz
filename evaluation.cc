#include "evaluation.h"

#include "utility.h"
#include "pre_computed.h"
#include "board.h"


// pawn structure evaluation
void Evaluation::evaluate_pawn(Board &board, unsigned square, Color friendly)
{
	Color enemy = swap(friendly);
	uint64_t friendly_pawns = board.pieces[piece_of(friendly, PAWN)];
	uint64_t enemy_pawns = board.pieces[piece_of(enemy, PAWN)];
	unsigned forward = (friendly == WHITE) ? UP : DOWN;
	uint64_t adjacent_files = isolated_pawn_mask(file_num(square));

	bool passed = !(passed_pawn_mask(friendly, square) & enemy_pawns);
	bool doubled = forward_file_mask(friendly, square) & friendly_pawns;
	bool neighbored = neighbor_mask(square) & friendly_pawns;
	bool supported = passed_pawn_mask(enemy, square) & adjacent_files & friendly_pawns;
	bool chained = pawn_attacks(enemy, square) & friendly_pawns;

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
	else if (!(supported || neighbored) && pawn_attacks(friendly, square + forward) & enemy_pawns) {
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

void Evaluation::evaluate_kings()
{
	mg_bonus[WHITE] -= ring_pressure[WHITE] * ring_pressure_weight[std::min(ring_attackers[WHITE], 7)] / 100;
	mg_bonus[BLACK] -= ring_pressure[BLACK] * ring_pressure_weight[std::min(ring_attackers[BLACK], 7)] / 100;
	mg_bonus[WHITE] -= zone_pressure[WHITE] * zone_pressure_weight[std::min(zone_attackers[WHITE], 7)] / 100;
	mg_bonus[BLACK] -= zone_pressure[BLACK] * zone_pressure_weight[std::min(zone_attackers[BLACK], 7)] / 100;
}

void Evaluation::note_king_attacks(Piece_type type, uint64_t attacks, Color friendly)
{
	if (attacks & ring[!friendly]) {
		ring_pressure[!friendly] += ring_attack_potency[type] * pop_count(attacks & ring[!friendly]);
		ring_attackers[!friendly]++;
	}
	if (attacks & zone[!friendly]) {
		zone_pressure[!friendly] += zone_attack_potency[type] * pop_count(attacks & zone[!friendly]);
		zone_attackers[!friendly]++;
	}
}

/*void Evaluation::evaluate_mobility(Board &board, Piece_type type, uint64_t attacks, Color friendly)
{
	// do not count mobility, when the squares are protected by enemy pawns
	int safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
	mg_bonus[friendly] += (10 * safe_squares - mg_average_mobility[type]) * mg_mobility_weight[type] / 100;
	eg_bonus[friendly] += (10 * safe_squares - eg_average_mobility[type]) * eg_mobility_weight[type] / 100;
}*/

void Evaluation::evaluate_piece(Board &board, Piece p, unsigned square)
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
		uint64_t attacks = piece_attacks(KNIGHT, square, 0ULL);
		note_king_attacks(KNIGHT, attacks, friendly);

		unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
		mg_bonus[friendly] += mg_knight_mobility[safe_squares];
		eg_bonus[friendly] += eg_knight_mobility[safe_squares];
		return;
		}
	case BISHOP:
		{
		mg_bonus[friendly] += mg_bishop_psqt[relative_square] + mg_piece_value[BISHOP];
		eg_bonus[friendly] += eg_bishop_psqt[relative_square] + eg_piece_value[BISHOP];
		// queen is not counted as a blocker, because a bishop behind a queen makes the attack more potent
		uint64_t ray_blockers = board.occ & ~board.pieces[piece_of(friendly, QUEEN)];
		uint64_t attacks = piece_attacks(BISHOP, square, ray_blockers);
		note_king_attacks(BISHOP, attacks, friendly);

		unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
		mg_bonus[friendly] += mg_bishop_mobility[safe_squares];
		eg_bonus[friendly] += eg_bishop_mobility[safe_squares];
		return;
		}
	case ROOK:
		{
		mg_bonus[friendly] += mg_rook_psqt[relative_square] + mg_piece_value[ROOK];
		eg_bonus[friendly] += eg_rook_psqt[relative_square] + eg_piece_value[ROOK];
		// queen and rooks are not counted as a blockers, because their pressure increases when stacked
		uint64_t ray_blockers = board.occ & ~(board.pieces[piece_of(friendly, QUEEN)] | board.pieces[piece_of(friendly, ROOK)]);
		uint64_t attacks = piece_attacks(ROOK, square, ray_blockers);
		note_king_attacks(ROOK, attacks, friendly);

		unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
		mg_bonus[friendly] += mg_rook_mobility[safe_squares];
		eg_bonus[friendly] += eg_rook_mobility[safe_squares];
		return;
		}
	case QUEEN:
		{
		mg_bonus[friendly] += mg_queen_psqt[relative_square] + mg_piece_value[QUEEN];
		eg_bonus[friendly] += eg_queen_psqt[relative_square] + eg_piece_value[QUEEN];
		uint64_t attacks = piece_attacks(QUEEN, square, board.occ);
		note_king_attacks(QUEEN, attacks, friendly);

		unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
		mg_bonus[friendly] += mg_queen_mobility[safe_squares];
		eg_bonus[friendly] += eg_queen_mobility[safe_squares];
		return;
		}
	case KING:
		mg_bonus[friendly] += mg_king_psqt[relative_square];
		eg_bonus[friendly] += eg_king_psqt[relative_square];
		uint64_t attacks = piece_attacks(KING, square, 0ULL);

		unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
		mg_bonus[friendly] += mg_king_mobility[safe_squares];
		eg_bonus[friendly] += eg_king_mobility[safe_squares];
		return;
	}
}

// main evaluation function
int Evaluation::evaluate(Board &board)
{
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
	ring[WHITE] = piece_attacks(KING, white_king_square, 0ULL);
	ring[BLACK] = piece_attacks(KING, black_king_square, 0ULL);
	zone[WHITE] = king_zone(WHITE, white_king_square);
	zone[BLACK] = king_zone(BLACK, black_king_square);

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

	int mg_value = mg_bonus[WHITE] - mg_bonus[BLACK];
	int eg_value = eg_bonus[WHITE] - eg_bonus[BLACK];

	// Tapered Eval
	// interpolation between midgame and endgame to create a smooth transition
	int value = (mg_value * phase + eg_value * (256 - phase)) / 256;
	return (board.side_to_move == WHITE) ? value: -value;
}
