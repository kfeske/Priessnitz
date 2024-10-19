
#include "utility.h"
#include "pre_computed.h"
#include "board.h"
#include "evaluation.h"
#include <trace.h>

void Evaluation::evaluate_pawns(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	Direction forward = (friendly == WHITE) ? UP : DOWN;
	uint64_t friendly_pawns = board.pieces(friendly, PAWN);
	uint64_t enemy_pawns    = board.pieces(enemy,    PAWN);

	uint64_t temp = friendly_pawns;
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];
		unsigned relative_rank = rank_num(relative_square);

		// Material + PSQT
		info.mg_bonus[friendly] += mg_pawn_psqt[relative_square] + mg_piece_value[PAWN];
		info.eg_bonus[friendly] += eg_pawn_psqt[relative_square] + eg_piece_value[PAWN];
		record_piece_value(friendly, PAWN);
		record_pawn_psqt(friendly, relative_square);

		uint64_t adjacent_files = isolated_pawn_mask(file_num(square));
		bool passed = !(passed_pawn_mask(friendly, square) & enemy_pawns);
		bool doubled = forward_file_mask(friendly, square) & friendly_pawns;
		bool phalanx = neighbor_mask(square) & friendly_pawns;
		bool potential_support = passed_pawn_mask(enemy, square) & adjacent_files & friendly_pawns;
		bool supported = pawn_attacks(enemy, square) & friendly_pawns;

		// Isolated pawns
		if (!(adjacent_files & friendly_pawns)) {
			info.mg_bonus[friendly] += mg_isolated_pawn;
			info.eg_bonus[friendly] += eg_isolated_pawn;
			record_isolated_pawn(friendly);
		}

		// Doubled pawns
		if (doubled) {
			info.mg_bonus[friendly] += mg_doubled_pawn;
			info.eg_bonus[friendly] += eg_doubled_pawn;
			record_doubled_pawn(friendly);
		}

		// Backward pawns
		if (!(potential_support || phalanx) && pawn_attacks(friendly, square + forward) & enemy_pawns) {
			if (file(square) & enemy_pawns) {
				info.mg_bonus[friendly] += mg_backward_pawn;
				info.eg_bonus[friendly] += eg_backward_pawn;
				record_backward_pawn(friendly);
			}
			else {
				info.mg_bonus[friendly] += mg_backward_pawn_half_open;
				info.eg_bonus[friendly] += eg_backward_pawn_half_open;
				record_backward_pawn_half_open(friendly);
			}
		}

		// Pawn chain
		else if (supported || phalanx) {
			info.mg_bonus[friendly] += mg_chained_pawn[relative_rank];
			info.eg_bonus[friendly] += eg_chained_pawn[relative_rank];
			record_chained_pawn(friendly, relative_rank);
		}

		// Passed pawns
		if (passed && !doubled)
			info.passed_pawns |= 1ULL << square;
	}

	unsigned king_square = board.square(friendly, KING);

	// Pawn Shelter
	unsigned shelter_center = std::max(1U, std::min(6U, file_num(king_square)));
	for (unsigned shelter_file = shelter_center - 1; shelter_file <= shelter_center + 1; shelter_file++) {
		uint64_t shelter_mask = file(shelter_file) & (forward_mask(friendly, king_square) | rank(king_square));
		uint64_t shelter_pawns = board.pieces(friendly, PAWN) & shelter_mask;
		if (shelter_pawns) {
			unsigned pawn_square = (friendly == WHITE) ? msb(shelter_pawns) : lsb(shelter_pawns);
			unsigned rank = rank_num(normalize_square[friendly][pawn_square]);
			unsigned edge_distance = std::min(shelter_file, 7 - shelter_file);
			bool king_file_pawn = shelter_file == file_num(king_square);
			info.mg_bonus[friendly] += mg_pawn_shelter[king_file_pawn][edge_distance][rank];
			info.eg_bonus[friendly] += eg_pawn_shelter[king_file_pawn][edge_distance][rank];
			record_pawn_shelter(friendly, king_file_pawn, edge_distance, rank);
		}
		/*uint64_t storm_pawns = board.pieces(enemy, PAWN) & shelter_mask;
		if (storm_pawns) {
			unsigned pawn_square = (friendly == WHITE) ? msb(storm_pawns) : lsb(storm_pawns);
			unsigned king_distance = rank_distance(pawn_square, king_square);
			unsigned edge_distance = std::min(shelter_file, 7 - shelter_file);
			Direction forward = (board.side_to_move == WHITE) ? UP : DOWN;
			bool blocked = board.board[pawn_square - forward] == piece_of(friendly, PAWN);
			info.mg_bonus[friendly] += mg_pawn_storm[blocked][edge_distance][king_distance];
			info.eg_bonus[friendly] += eg_pawn_storm[blocked][edge_distance][king_distance];
			//record_pawn_storm(friendly, blocked, edge_distance, king_distance);
		}*/
	}

}

void Evaluation::evaluate_knights(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][KNIGHT] = 0ULL;

	uint64_t temp = board.pieces(friendly, KNIGHT);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		info.mg_bonus[friendly] += mg_knight_psqt[relative_square] + mg_piece_value[KNIGHT];
		info.eg_bonus[friendly] += eg_knight_psqt[relative_square] + eg_piece_value[KNIGHT];
		record_piece_value(friendly, KNIGHT);
		record_knight_psqt(friendly, relative_square);

		// Knight outpost
		if (1ULL << square & outpost_mask(friendly) &&
		    !(pawn_threat_mask(friendly, square) & board.pieces(enemy, PAWN))) {
			if (pawn_attacks(enemy, square) & board.pieces(friendly, PAWN)) {
				info.mg_bonus[friendly] += mg_knight_outpost_supported;
				info.eg_bonus[friendly] += eg_knight_outpost_supported;
				record_knight_outpost_supported(friendly);
			}
			else {
				info.mg_bonus[friendly] += mg_knight_outpost;
				info.eg_bonus[friendly] += eg_knight_outpost;
				record_knight_outpost(friendly);
			}

		}
		uint64_t attacks = piece_attacks(KNIGHT, square, 0ULL);
		info.attacked_by_multiple[friendly] |= info.attacked[friendly] & attacks;
		info.attacked[friendly]			 |= attacks;
		info.attacked_by_piece[friendly][KNIGHT] |= attacks;

		// Attack on the enemy king
		uint64_t zone_attacks = attacks & info.king_ring[!friendly];
		if (zone_attacks) {
			info.king_attackers[!friendly]++;
			info.king_zone_attacks[!friendly] += pop_count(attacks & info.attacked_by_piece[enemy][KING]);
			info.mg_king_attackers_weight[!friendly] += mg_king_attacker_weight[KNIGHT];
			info.eg_king_attackers_weight[!friendly] += eg_king_attacker_weight[KNIGHT];
			record_king_attacker_weight(swap(friendly), KNIGHT);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		info.mg_bonus[friendly] += mg_knight_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_knight_mobility[safe_squares];
		record_knight_mobility(friendly, safe_squares);
	}
}

void Evaluation::evaluate_bishops(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][BISHOP] = 0ULL;

	uint64_t temp = board.pieces(friendly, BISHOP);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		info.mg_bonus[friendly] += mg_bishop_psqt[relative_square] + mg_piece_value[BISHOP];
		info.eg_bonus[friendly] += eg_bishop_psqt[relative_square] + eg_piece_value[BISHOP];
		record_piece_value(friendly, BISHOP);
		record_bishop_psqt(friendly, relative_square);

		// A queen is not counted as a blocker, because a bishop behind a queen makes the attack stronger
		uint64_t ray_blockers = board.occ & ~board.pieces(friendly, QUEEN);
		uint64_t attacks = piece_attacks(BISHOP, square, ray_blockers);
		info.attacked_by_multiple[friendly] |= info.attacked[friendly] & attacks;
		info.attacked[friendly]			 |= attacks;
		info.attacked_by_piece[friendly][BISHOP] |= attacks;

		// Attack on the enemy king
		uint64_t zone_attacks = attacks & info.king_ring[!friendly];
		if (zone_attacks) {
			info.king_attackers[!friendly]++;
			info.king_zone_attacks[!friendly] += pop_count(attacks & info.attacked_by_piece[enemy][KING]);
			info.mg_king_attackers_weight[!friendly] += mg_king_attacker_weight[BISHOP];
			info.eg_king_attackers_weight[!friendly] += eg_king_attacker_weight[BISHOP];
			record_king_attacker_weight(swap(friendly), BISHOP);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		info.mg_bonus[friendly] += mg_bishop_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_bishop_mobility[safe_squares];
		record_bishop_mobility(friendly, safe_squares);
	}
	// Double bishop bonus
	if (pop_count(board.pieces(friendly, BISHOP)) >= 2) {
		info.mg_bonus[friendly] += mg_double_bishop;
		info.eg_bonus[friendly] += eg_double_bishop;
		record_double_bishop(friendly);
	}
}

void Evaluation::evaluate_rooks(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][ROOK] = 0ULL;

	uint64_t temp = board.pieces(friendly, ROOK);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		info.mg_bonus[friendly] += mg_rook_psqt[relative_square] + mg_piece_value[ROOK];
		info.eg_bonus[friendly] += eg_rook_psqt[relative_square] + eg_piece_value[ROOK];
		record_piece_value(friendly, ROOK);
		record_rook_psqt(friendly, relative_square);

		// Queens and rooks are not counted as a blockers, because their strength increases when stacked
		uint64_t ray_blockers = board.occ & ~(board.pieces(friendly, QUEEN) | board.pieces(friendly, ROOK));
		uint64_t attacks = piece_attacks(ROOK, square, ray_blockers);
		info.attacked_by_multiple[friendly] |= info.attacked[friendly] & attacks;
		info.attacked[friendly]		       |= attacks;
		info.attacked_by_piece[friendly][ROOK] |= attacks;

		// Attack on the enemy king
		uint64_t zone_attacks = attacks & info.king_ring[!friendly];
		if (zone_attacks) {
			info.king_attackers[!friendly]++;
			info.king_zone_attacks[!friendly] += pop_count(attacks & info.attacked_by_piece[enemy][KING]);
			info.mg_king_attackers_weight[!friendly] += mg_king_attacker_weight[ROOK];
			info.eg_king_attackers_weight[!friendly] += eg_king_attacker_weight[ROOK];
			record_king_attacker_weight(swap(friendly), ROOK);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		info.mg_bonus[friendly] += mg_rook_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_rook_mobility[safe_squares];
		record_rook_mobility(friendly, safe_squares);

		// Rook (semi)open file
		uint64_t rook_file = file(square);
		if (!(rook_file & board.pieces(friendly, PAWN))) {
			if (!(rook_file & board.pieces(enemy, PAWN))) {
				info.mg_bonus[friendly] += mg_rook_open_file;
				info.eg_bonus[friendly] += eg_rook_open_file;
				record_rook_open_file(friendly);
			}
			else {
				info.mg_bonus[friendly] += mg_rook_half_open_file;
				info.eg_bonus[friendly] += eg_rook_half_open_file;
				record_rook_half_open_file(friendly);
			}
		}

		// Rook on the 7th rank, trapping the king on the last rank
		if (rank_num(relative_square) == 1 && rank_num(normalize_square[friendly][board.square(enemy, KING)]) <= 1) {
			info.mg_bonus[friendly] += mg_rook_on_seventh;
			info.eg_bonus[friendly] += eg_rook_on_seventh;
			record_rook_on_seventh(friendly);
		}
	}
}

void Evaluation::evaluate_queens(Board &board, Color friendly)
{
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][QUEEN] = 0ULL;

	uint64_t temp = board.pieces(friendly, QUEEN);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		info.mg_bonus[friendly] += mg_queen_psqt[relative_square] + mg_piece_value[QUEEN];
		info.eg_bonus[friendly] += eg_queen_psqt[relative_square] + eg_piece_value[QUEEN];
		record_piece_value(friendly, QUEEN);
		record_queen_psqt(friendly, relative_square);

		uint64_t attacks = piece_attacks(QUEEN, square, board.occ);
		info.attacked_by_multiple[friendly] |= info.attacked[friendly] & attacks;
		info.attacked[friendly]			|= attacks;
		info.attacked_by_piece[friendly][QUEEN] |= attacks;

		// Attack on the enemy king
		uint64_t zone_attacks = attacks & info.king_ring[!friendly];
		if (zone_attacks) {
			info.king_attackers[!friendly]++;
			info.king_zone_attacks[!friendly] += pop_count(attacks & info.attacked_by_piece[enemy][KING]);
			info.mg_king_attackers_weight[!friendly] += mg_king_attacker_weight[QUEEN];
			info.eg_king_attackers_weight[!friendly] += eg_king_attacker_weight[QUEEN];
			record_king_attacker_weight(swap(friendly), QUEEN);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		info.mg_bonus[friendly] += mg_queen_mobility[safe_squares];
		info.eg_bonus[friendly] += eg_queen_mobility[safe_squares];
		record_queen_mobility(friendly, safe_squares);
	}
}

void Evaluation::evaluate_kings(Board &board, Color friendly)
{
	Color enemy = swap(friendly);

	unsigned square = board.square(friendly, KING);
	unsigned relative_square = normalize_square[friendly][square];

	// PSQT
	info.mg_bonus[friendly] += mg_king_psqt[relative_square];
	info.eg_bonus[friendly] += eg_king_psqt[relative_square];
	record_king_psqt(friendly, relative_square);

	uint64_t attacks = piece_attacks(KING, square, 0ULL);

	// Mobility
	unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
	info.mg_bonus[friendly] += mg_king_mobility[safe_squares];
	info.eg_bonus[friendly] += eg_king_mobility[safe_squares];
	record_king_mobility(friendly, safe_squares);
	
	// Only consider king safety, when we have at least two attackers or one attacker when they have a queen
	if (info.king_attackers[friendly] > (1 - pop_count(board.pieces(enemy, QUEEN)))) {

		int mg_king_danger = 0;
		int eg_king_danger = 0;
		
		uint64_t weak_squares = info.attacked[enemy] & ~info.attacked_by_multiple[friendly] & 
					(~info.attacked[friendly] | info.attacked_by_piece[friendly][KING]);
		unsigned weak_king_ring = pop_count(info.king_ring[friendly] & weak_squares);
		mg_king_danger += mg_king_zone_weak_square * weak_king_ring;
		eg_king_danger += eg_king_zone_weak_square * weak_king_ring;
		record_king_zone_weak_square(friendly, weak_king_ring);

		// Safe checks by enemy pieces
		uint64_t safe = ~info.attacked[friendly] & ~board.pieces(enemy);
		unsigned knight_checks = pop_count(piece_attacks(KNIGHT, square, 0ULL)      & safe & info.attacked_by_piece[enemy][KNIGHT]);
		mg_king_danger += mg_safe_knight_check * knight_checks;
		eg_king_danger += eg_safe_knight_check * knight_checks;
		record_safe_knight_check(friendly, knight_checks);

		unsigned bishop_checks = pop_count(piece_attacks(BISHOP, square, board.occ) & safe & info.attacked_by_piece[enemy][BISHOP]);
		mg_king_danger += mg_safe_bishop_check * bishop_checks;
		eg_king_danger += eg_safe_bishop_check * bishop_checks;
		record_safe_bishop_check(friendly, bishop_checks);

		unsigned rook_checks   = pop_count(piece_attacks(ROOK,   square, board.occ) & safe & info.attacked_by_piece[enemy][ROOK]);
		mg_king_danger += mg_safe_rook_check   * rook_checks;
		eg_king_danger += eg_safe_rook_check   * rook_checks;
		record_safe_rook_check(friendly, rook_checks);

		unsigned queen_checks  = pop_count(piece_attacks(QUEEN,  square, board.occ) & safe & info.attacked_by_piece[enemy][QUEEN]);
		mg_king_danger += mg_safe_queen_check  * queen_checks;
		eg_king_danger += eg_safe_queen_check  * queen_checks;
		record_safe_queen_check(friendly, queen_checks);

		mg_king_danger += info.mg_king_attackers_weight[friendly] * info.king_attackers[friendly] +
				  mg_king_zone_attack_count_weight * info.king_zone_attacks[friendly] +
				  mg_king_danger_no_queen_weight * !board.pieces(enemy, QUEEN) +
				  mg_king_danger_offset;


		mg_king_danger = std::max(0, mg_king_danger);
		info.mg_bonus[friendly] -= mg_king_danger * mg_king_danger / 4096;

		eg_king_danger += info.eg_king_attackers_weight[friendly] * info.king_attackers[friendly] +
				  eg_king_zone_attack_count_weight * info.king_zone_attacks[friendly] +
				  eg_king_danger_no_queen_weight * !board.pieces(enemy, QUEEN) +
				  eg_king_danger_offset;

		eg_king_danger = std::max(0, eg_king_danger);
		info.eg_bonus[friendly] -= eg_king_danger / 16;

		record_adjust_king_attacker_weights(friendly, info.king_attackers[friendly]);
		record_king_zone_attack_count_weight(friendly, info.king_zone_attacks[friendly]);
		record_king_danger_no_queen_weight(friendly, !board.pieces(enemy, QUEEN));
		record_king_danger_offset(friendly);
	}
	else record_clear_attacker_weights(friendly);
}

void Evaluation::evaluate_passed_pawns(Board &board, Color friendly)
{
	Color enemy = swap(friendly);

	uint64_t temp = info.passed_pawns & board.pieces(friendly);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];
		unsigned relative_rank = rank_num(relative_square);
		uint64_t advance_square = pawn_pushes(friendly, 1ULL << square);
		bool blocked = advance_square & board.occ;
		bool safe_advance = advance_square & ~info.attacked[enemy];
		bool safe_path = !(passed_pawn_mask(friendly, square) & file(square) & (board.pieces(enemy) | info.attacked[enemy]));

		// Evaluate based on rank and ability to advance
		if (!blocked) {
			info.mg_bonus[friendly] += mg_passed_pawn[relative_rank];
			info.eg_bonus[friendly] += eg_passed_pawn[relative_rank];
			record_passed_pawn(friendly, relative_rank);
		}
		else {
			info.mg_bonus[friendly] += mg_passed_pawn_blocked[relative_rank];
			info.eg_bonus[friendly] += eg_passed_pawn_blocked[relative_rank];
			record_passed_pawn_blocked(friendly, relative_rank);
		}

		if (safe_advance) {
			info.mg_bonus[friendly] += mg_passed_pawn_safe_advance;
			info.eg_bonus[friendly] += eg_passed_pawn_safe_advance;
			record_passed_pawn_safe_advance(friendly);
		}

		if (safe_path) {
			info.mg_bonus[friendly] += mg_passed_pawn_safe_path;
			info.eg_bonus[friendly] += eg_passed_pawn_safe_path;
			record_passed_pawn_safe_path(friendly);
		}

		// Evaluate based on the distance of the two kings from the passer
		unsigned friendly_distance = square_distance(square, board.square(friendly, KING));
		info.mg_bonus[friendly] += friendly_distance * mg_passed_friendly_distance[relative_rank];
		info.eg_bonus[friendly] += friendly_distance * eg_passed_friendly_distance[relative_rank];
		record_passed_friendly_distance(friendly, relative_rank, friendly_distance);

		unsigned enemy_distance    = square_distance(square, board.square(enemy,    KING));
		info.mg_bonus[friendly] += enemy_distance * mg_passed_enemy_distance[relative_rank];
		info.eg_bonus[friendly] += enemy_distance * eg_passed_enemy_distance[relative_rank];
		record_passed_enemy_distance(friendly, relative_rank, enemy_distance);
	}
}

void Evaluation::evaluate_threats(Board &board, Color friendly)
{
	Color enemy = swap(friendly);

	uint64_t friendly_knights = board.pieces(friendly, KNIGHT);
	uint64_t friendly_bishops = board.pieces(friendly, BISHOP);
	uint64_t friendly_rooks   = board.pieces(friendly, ROOK);
	uint64_t friendly_queens  = board.pieces(friendly, QUEEN);

	uint64_t attacked_by_pawn  = info.attacked_by_piece[enemy][PAWN];
	uint64_t attacked_by_minor = info.attacked_by_piece[enemy][KNIGHT] | info.attacked_by_piece[enemy][BISHOP];
	uint64_t attacked_by_major = info.attacked_by_piece[enemy][ROOK]   | info.attacked_by_piece[enemy][QUEEN];

	// Our minors attacked by enemy pawns
	uint64_t minors_threatened_by_pawns = pop_count((friendly_knights | friendly_bishops) & attacked_by_pawn);
	info.mg_bonus[friendly] += mg_minor_threatened_by_pawn * minors_threatened_by_pawns;
	info.eg_bonus[friendly] += eg_minor_threatened_by_pawn * minors_threatened_by_pawns;
	record_minor_threatened_by_pawn(friendly, minors_threatened_by_pawns);

	// Our minors attacked by enemy minors
	uint64_t minors_threatened_by_minors = pop_count((friendly_knights | friendly_bishops) & attacked_by_minor);
	info.mg_bonus[friendly] += mg_minor_threatened_by_minor * minors_threatened_by_minors;
	info.eg_bonus[friendly] += eg_minor_threatened_by_minor * minors_threatened_by_minors;
	record_minor_threatened_by_minor(friendly, minors_threatened_by_minors);

	// Our rooks attacked by enemy minors or pawns
	uint64_t rooks_threatened_by_lesser = pop_count(friendly_rooks & (attacked_by_pawn | attacked_by_minor));
	info.mg_bonus[friendly] += mg_rook_threatened_by_lesser * rooks_threatened_by_lesser;
	info.eg_bonus[friendly] += eg_rook_threatened_by_lesser * rooks_threatened_by_lesser;
	record_rook_threatened_by_lesser(friendly, rooks_threatened_by_lesser);

	// Our queens attacked by lesser pieces
	uint64_t queens_threatened_by_lesser = pop_count(friendly_queens & (info.attacked[enemy] & ~info.attacked_by_piece[enemy][QUEEN]));
	info.mg_bonus[friendly] += mg_queen_threatened_by_lesser * queens_threatened_by_lesser;
	info.eg_bonus[friendly] += eg_queen_threatened_by_lesser * queens_threatened_by_lesser;
	record_queen_threatened_by_lesser(friendly, queens_threatened_by_lesser);

	// Undefended minors attacked by majors
	uint64_t weak = info.attacked[enemy] & ~info.attacked[friendly];
	uint64_t minors_threatened_by_majors = pop_count((friendly_knights | friendly_bishops) & weak & attacked_by_major);
	info.mg_bonus[friendly] += mg_minor_threatened_by_major * minors_threatened_by_majors;
	info.eg_bonus[friendly] += eg_minor_threatened_by_major * minors_threatened_by_majors;
	record_minor_threatened_by_major(friendly, minors_threatened_by_majors);
}

void Evaluation::evaluate_center_control(Color friendly)
{
	Color enemy = swap(friendly);
	unsigned center_attacks = pop_count(~info.attacked[enemy] & info.attacked[friendly] & CENTER);
	info.mg_bonus[friendly] += center_attacks * mg_center_control;
	info.eg_bonus[friendly] += center_attacks * eg_center_control;
	record_center_control(friendly, center_attacks);
}

int Evaluation::scale_factor(Board &board, int eg_value)
{
	Color strong_side = (eg_value > 0) ? WHITE : BLACK;

	if (pop_count(board.pieces(WHITE, BISHOP)) == 1 && pop_count(board.pieces(BLACK, BISHOP)) == 1 &&
	    pop_count(board.pieces(BISHOP) & WHITE_SQUARES) == 1) {
		return 90;
	}

	return std::min(128U, 80 + pop_count(board.pieces(strong_side, PAWN)) * 25);
}

// main evaluation function
int Evaluation::evaluate(Board &board)
{
	info.init(board);
	int mg_value = 0;
	int eg_value = 0;

	int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
	material = std::max(taper_end, std::min(material, taper_start)); // endgame and midgame limit clamp
	int phase = ((material - taper_end) * 256) / (taper_start - taper_end); // 0(Endgame) - 256(Midgame) linear interpolation
	
	record_phase(phase);

	Pawn_hash_entry &entry = pawn_hash_table.probe(board.zobrist.pawn_key);
	if (pawn_hash_table.hit && use_pawn_hash_table) {
		mg_value = entry.mg_evaluation;
		eg_value = entry.eg_evaluation;
		info.passed_pawns = entry.passed_pawns;
	}
	else {
		evaluate_pawns(  board, WHITE);
		evaluate_pawns(  board, BLACK);
		pawn_hash_table.store(board.zobrist.pawn_key, info.mg_bonus[WHITE] - info.mg_bonus[BLACK], info.eg_bonus[WHITE] - info.eg_bonus[BLACK], info.passed_pawns);
	}
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

	evaluate_passed_pawns(board, WHITE);
	evaluate_passed_pawns(board, BLACK);

	evaluate_threats(board, WHITE);
	evaluate_threats(board, BLACK);

	evaluate_center_control(WHITE);
	evaluate_center_control(BLACK);

	info.mg_bonus[board.side_to_move] += tempo_bonus;

	mg_value += info.mg_bonus[WHITE] - info.mg_bonus[BLACK];
	eg_value += info.eg_bonus[WHITE] - info.eg_bonus[BLACK];

	// Scale down drawish looking endgames
	int scale = scale_factor(board, eg_value);
	record_scale_factor(scale);

	// Tapered Eval
	// interpolation between midgame and endgame to create a smooth transition
	int value = (mg_value * phase + eg_value * (256 - phase) * scale / 128) / 256;
	return (board.side_to_move == WHITE) ? value: -value;
}
