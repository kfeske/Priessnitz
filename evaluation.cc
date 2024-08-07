#include "evaluation.h"

#include "utility.h"
#include "pre_computed.h"
#include "board.h"


void Evaluation::note_king_attacks(Piece_type type, uint64_t attacks, Color friendly)
{
	if (attacks & info.ring[!friendly]) {
		info.ring_pressure[!friendly] += ring_attack_potency[type] * pop_count(attacks & info.ring[!friendly]);
		info.ring_attackers[!friendly]++;
	}
	if (attacks & info.zone[!friendly]) {
		info.zone_pressure[!friendly] += zone_attack_potency[type] * pop_count(attacks & info.zone[!friendly]);
		info.zone_attackers[!friendly]++;
	}
}

void Evaluation::evaluate_pawns(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	Direction forward = (friendly == WHITE) ? UP : DOWN;
	uint64_t friendly_pawns = board.pieces(friendly, PAWN);
	uint64_t enemy_pawns    = board.pieces(enemy,    PAWN);

	info.attacked_by_pawn[friendly] = board.all_pawn_attacks(friendly);

	uint64_t temp = friendly_pawns;
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		info.mg_bonus[friendly] += mg_pawn_psqt[relative_square] + mg_piece_value[PAWN];
		info.eg_bonus[friendly] += eg_pawn_psqt[relative_square] + eg_piece_value[PAWN];

		uint64_t adjacent_files = isolated_pawn_mask(file_num(square));
		bool passed = !(passed_pawn_mask(friendly, square) & enemy_pawns);
		bool doubled = forward_file_mask(friendly, square) & friendly_pawns;
		bool neighbored = neighbor_mask(square) & friendly_pawns;
		bool supported = passed_pawn_mask(enemy, square) & adjacent_files & friendly_pawns;
		bool chained = pawn_attacks(enemy, square) & friendly_pawns;

		// Isolated pawns
		if (!(adjacent_files & friendly_pawns)) {
			info.mg_bonus[friendly] += mg_isolated_penalty;
			info.eg_bonus[friendly] += eg_isolated_penalty;
		}

		// Doubled pawns
		else if (doubled) {
			info.mg_bonus[friendly] += mg_doubled_penalty;
			info.eg_bonus[friendly] += eg_doubled_penalty;
		}

		// Backward pawns
		else if (!(supported || neighbored) && pawn_attacks(friendly, square + forward) & enemy_pawns) {
			info.mg_bonus[friendly] += mg_backward_penalty;
			info.eg_bonus[friendly] += eg_backward_penalty;
		}

		// Pawn chain
		if (chained) {
			info.mg_bonus[friendly] += mg_chained_bonus;
			info.eg_bonus[friendly] += eg_chained_bonus;
		}

		// Passed pawns
		if (passed && !doubled) {
			info.mg_bonus[friendly] += mg_passed_bonus[relative_square];
			info.eg_bonus[friendly] += eg_passed_bonus[relative_square];
		}
	}
}

void Evaluation::evaluate_knights(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	uint64_t temp = board.pieces(friendly, KNIGHT);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];
		info.mg_bonus[friendly] += mg_knight_psqt[relative_square] + mg_piece_value[KNIGHT];
		info.eg_bonus[friendly] += eg_knight_psqt[relative_square] + eg_piece_value[KNIGHT];

		// Knight outpost
		if (1ULL << square & outpost_mask(friendly) &&
		    !(pawn_threat_mask(friendly, square) & board.pieces(enemy, PAWN))) {
			if (pawn_attacks(enemy, square) & board.pieces(friendly, PAWN)) {
				info.mg_bonus[friendly] += mg_knight_outpost_supported;
				info.eg_bonus[friendly] += eg_knight_outpost_supported;
			}
			else {
				info.mg_bonus[friendly] += mg_knight_outpost;
				info.eg_bonus[friendly] += eg_knight_outpost;
			}

		}

		// Attack on the enemy king
		uint64_t attacks = piece_attacks(KNIGHT, square, 0ULL);
		note_king_attacks(KNIGHT, attacks, friendly);

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_pawn[enemy]);
		info.mg_bonus[friendly] += mg_knight_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_knight_mobility[safe_squares];
	}
}

void Evaluation::evaluate_bishops(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	uint64_t temp = board.pieces(friendly, BISHOP);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		info.mg_bonus[friendly] += mg_bishop_psqt[relative_square] + mg_piece_value[BISHOP];
		info.eg_bonus[friendly] += eg_bishop_psqt[relative_square] + eg_piece_value[BISHOP];

		// Attack on the enemy king
		// queen is not counted as a blocker, because a bishop behind a queen makes the attack more potent
		uint64_t ray_blockers = board.occ & ~board.pieces(friendly, QUEEN);
		uint64_t attacks = piece_attacks(BISHOP, square, ray_blockers);
		note_king_attacks(BISHOP, attacks, friendly);

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_pawn[enemy]);
		info.mg_bonus[friendly] += mg_bishop_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_bishop_mobility[safe_squares];
	}
	// Double bishop bonus
	if (pop_count(board.pieces(friendly, BISHOP)) >= 2) {
		info.mg_bonus[friendly] += mg_double_bishop;
		info.eg_bonus[friendly] += eg_double_bishop;
	}
}

void Evaluation::evaluate_rooks(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	uint64_t temp = board.pieces(friendly, ROOK);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		info.mg_bonus[friendly] += mg_rook_psqt[relative_square] + mg_piece_value[ROOK];
		info.eg_bonus[friendly] += eg_rook_psqt[relative_square] + eg_piece_value[ROOK];

		// Attack on the enemy king
		// queen and rooks are not counted as a blockers, because their pressure increases when stacked
		uint64_t ray_blockers = board.occ & ~(board.pieces(friendly, QUEEN) | board.pieces(friendly, ROOK));
		uint64_t attacks = piece_attacks(ROOK, square, ray_blockers);
		note_king_attacks(ROOK, attacks, friendly);

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_pawn[enemy]);
		info.mg_bonus[friendly] += mg_rook_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_rook_mobility[safe_squares];

		// Rook (semi)open file
		uint64_t rook_file = file(square);
		if (!(rook_file & board.pieces(friendly, PAWN))) {
			if (!(rook_file & board.pieces(enemy, PAWN))) {
				info.mg_bonus[friendly] += mg_rook_open_file;
				info.eg_bonus[friendly] += eg_rook_open_file;
			}
			else {
				info.mg_bonus[friendly] += mg_rook_half_open_file;
				info.eg_bonus[friendly] += eg_rook_half_open_file;
			}
		}

		if (rank_num(relative_square) == 1 && rank_num(normalize_square[friendly][board.square(enemy, KING)]) <= 1) {
			info.mg_bonus[friendly] += mg_rook_on_seventh;
			info.eg_bonus[friendly] += eg_rook_on_seventh;
		}
	}
}

void Evaluation::evaluate_queens(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	uint64_t temp = board.pieces(friendly, QUEEN);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		info.mg_bonus[friendly] += mg_queen_psqt[relative_square] + mg_piece_value[QUEEN];
		info.eg_bonus[friendly] += eg_queen_psqt[relative_square] + eg_piece_value[QUEEN];

		// Attack on the enemy king
		uint64_t attacks = piece_attacks(QUEEN, square, board.occ);
		note_king_attacks(QUEEN, attacks, friendly);

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_pawn[enemy]);
		info.mg_bonus[friendly] += mg_queen_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_queen_mobility[safe_squares];
	}
}

void Evaluation::evaluate_kings(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	unsigned square = board.square(friendly, KING);
	unsigned relative_square = normalize_square[friendly][square];

	info.mg_bonus[friendly] += mg_king_psqt[relative_square];
	info.eg_bonus[friendly] += eg_king_psqt[relative_square];

	uint64_t attacks = piece_attacks(KING, square, 0ULL);

	// Mobility
	unsigned safe_squares = pop_count(attacks & ~info.attacked_by_pawn[enemy]);
	info.mg_bonus[friendly] += mg_king_mobility[safe_squares];
	info.eg_bonus[friendly] += eg_king_mobility[safe_squares];

	// Pawn Shelter
	uint64_t shield_pawns = board.pieces(friendly, PAWN) & pawn_shield(friendly, square);
	while (shield_pawns) {
		unsigned pawn_square = pop_lsb(shield_pawns);
		unsigned shield_square = rank_distance(square, pawn_square) * 2 + file_distance(square, pawn_square);
		info.mg_bonus[friendly] += mg_pawn_shield[shield_square];
	}
	
	info.mg_bonus[friendly] -= info.ring_pressure[friendly] * ring_pressure_weight[std::min(info.ring_attackers[friendly], 7)] / 100;
	info.mg_bonus[friendly] -= info.zone_pressure[friendly] * zone_pressure_weight[std::min(info.zone_attackers[friendly], 7)] / 100;
}

int Evaluation::scale_factor(Board &board, int eg_value)
{
	Color strong_side = (eg_value > 0) ? WHITE : BLACK;

	return pawn_count_scale_offset + pop_count(board.pieces(strong_side, PAWN)) * pawn_count_scale_weight;
}

// main evaluation function
int Evaluation::evaluate(Board &board)
{
	info.init(board);

	int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
	material = std::max(taper_end, std::min(material, taper_start)); // endgame and midgame limit clamp
	int phase = ((material - taper_end) * 256) / (taper_start - taper_end); // 0(Endgame) - 256(Midgame) linear interpolation

	evaluate_pawns(  board, WHITE);
	evaluate_pawns(  board, BLACK);
	evaluate_knights(board, WHITE);
	evaluate_knights(board, BLACK);
	evaluate_bishops(board, WHITE);
	evaluate_bishops(board, BLACK);
	evaluate_rooks(  board, WHITE);
	evaluate_rooks(  board, BLACK);
	evaluate_queens( board, WHITE);
	evaluate_queens( board, BLACK);
	evaluate_kings(  board, WHITE);
	evaluate_kings(  board, BLACK);

	info.mg_bonus[board.side_to_move] += tempo_bonus;

	int mg_value = info.mg_bonus[WHITE] - info.mg_bonus[BLACK];
	int eg_value = info.eg_bonus[WHITE] - info.eg_bonus[BLACK];

	int scale = scale_factor(board, eg_value);
	eg_value = eg_value * scale / 128;

	// Tapered Eval
	// interpolation between midgame and endgame to create a smooth transition
	int value = (mg_value * phase + eg_value * (256 - phase)) / 256;
	return (board.side_to_move == WHITE) ? value: -value;
}
