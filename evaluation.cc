
#include "utility.h"
#include "pre_computed.h"
#include "board.h"
#include "evaluation.h"
#include <trace.h>

int Evaluation::evaluate_pawns(Board &board, Color friendly)
{
	int score = 0;
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
		score += pawn_psqt[relative_square] + piece_value[PAWN];
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
			score += isolated_pawn;
			record_isolated_pawn(friendly);
		}

		// Doubled pawns
		if (doubled) {
			score += doubled_pawn;
			record_doubled_pawn(friendly);
		}

		// Backward pawns
		if (!(potential_support || phalanx) && pawn_attacks(friendly, square + forward) & enemy_pawns) {
			if (file(square) & enemy_pawns) {
				score += backward_pawn;
				record_backward_pawn(friendly);
			}
			else {
				score += backward_pawn_half_open;
				record_backward_pawn_half_open(friendly);
			}
		}

		// Pawn chain
		else if (supported) {
			score += chained_pawn[relative_rank];
			record_chained_pawn(friendly, relative_rank);
		}

		else if (phalanx) {
			score += phalanx_pawn[relative_rank];
			record_phalanx_pawn(friendly, relative_rank);
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
			score += pawn_shelter[king_file_pawn][edge_distance][rank];
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
	return score;

}

int Evaluation::evaluate_knights(Board &board, Color friendly)
{
	int score = 0;
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][KNIGHT] = 0ULL;

	uint64_t temp = board.pieces(friendly, KNIGHT);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		score += knight_psqt[relative_square] + piece_value[KNIGHT];
		record_piece_value(friendly, KNIGHT);
		record_knight_psqt(friendly, relative_square);

		// Knight outpost
		if (1ULL << square & outpost_mask(friendly) &&
		    !(pawn_threat_mask(friendly, square) & board.pieces(enemy, PAWN))) {
			if (pawn_attacks(enemy, square) & board.pieces(friendly, PAWN)) {
				score += knight_outpost_supported;
				record_knight_outpost_supported(friendly);
			}
			else {
				score += knight_outpost;
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
			info.king_attackers_weight[!friendly] += king_attacker_weight[KNIGHT];
			record_king_attacker_weight(swap(friendly), KNIGHT);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		score += knight_mobility[safe_squares];
		record_knight_mobility(friendly, safe_squares);
	}
	return score;
}

int Evaluation::evaluate_bishops(Board &board, Color friendly)
{
	int score = 0;
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][BISHOP] = 0ULL;

	uint64_t temp = board.pieces(friendly, BISHOP);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		score += bishop_psqt[relative_square] + piece_value[BISHOP];
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
			info.king_attackers_weight[!friendly] += king_attacker_weight[BISHOP];
			record_king_attacker_weight(swap(friendly), BISHOP);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		score += bishop_mobility[safe_squares];
		record_bishop_mobility(friendly, safe_squares);

		uint64_t bishop_color_squares = ((1ULL << square) & WHITE_SQUARES) ? WHITE_SQUARES : BLACK_SQUARES;
		unsigned bishop_pawns = pop_count(board.pieces(friendly, PAWN) & bishop_color_squares);
		score += bishop_pawn * bishop_pawns;
		record_bishop_pawn(friendly, bishop_pawns);
	}
	// Double bishop bonus
	if (pop_count(board.pieces(friendly, BISHOP)) >= 2) {
		score += double_bishop;
		record_double_bishop(friendly);
	}
	return score;
}

int Evaluation::evaluate_rooks(Board &board, Color friendly)
{
	int score = 0;
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][ROOK] = 0ULL;

	uint64_t temp = board.pieces(friendly, ROOK);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		score += rook_psqt[relative_square] + piece_value[ROOK];
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
			info.king_attackers_weight[!friendly] += king_attacker_weight[ROOK];
			record_king_attacker_weight(swap(friendly), ROOK);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		score += rook_mobility[safe_squares];
		record_rook_mobility(friendly, safe_squares);

		// Rook (semi)open file
		uint64_t rook_file = file(square);
		if (!(rook_file & board.pieces(friendly, PAWN))) {
			if (!(rook_file & board.pieces(enemy, PAWN))) {
				score += rook_open_file;
				record_rook_open_file(friendly);
			}
			else {
				score += rook_half_open_file;
				record_rook_half_open_file(friendly);
			}
		}

		// Rook on the 7th rank, trapping the king on the last rank
		if (rank_num(relative_square) == 1 && rank_num(normalize_square[friendly][board.square(enemy, KING)]) <= 1) {
			score += rook_on_seventh;
			record_rook_on_seventh(friendly);
		}
	}
	return score;
}

int Evaluation::evaluate_queens(Board &board, Color friendly)
{
	int score = 0;
	Color enemy = swap(friendly);
	info.attacked_by_piece[friendly][QUEEN] = 0ULL;

	uint64_t temp = board.pieces(friendly, QUEEN);
	while (temp) {
		unsigned square = pop_lsb(temp);
		unsigned relative_square = normalize_square[friendly][square];

		// Material + PSQT
		score += queen_psqt[relative_square] + piece_value[QUEEN];
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
			info.king_attackers_weight[!friendly] += king_attacker_weight[QUEEN];
			record_king_attacker_weight(swap(friendly), QUEEN);
		}

		// Mobility
		unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
		score += queen_mobility[safe_squares];
		record_queen_mobility(friendly, safe_squares);
	}
	return score;
}

int Evaluation::evaluate_kings(Board &board, Color friendly)
{
	int score = 0;
	Color enemy = swap(friendly);

	unsigned square = board.square(friendly, KING);
	unsigned relative_square = normalize_square[friendly][square];

	// PSQT
	score += king_psqt[relative_square];
	record_king_psqt(friendly, relative_square);

	uint64_t attacks = piece_attacks(KING, square, 0ULL);

	// Mobility
	unsigned safe_squares = pop_count(attacks & ~info.attacked_by_piece[enemy][PAWN]);
	score += king_mobility[safe_squares];
	record_king_mobility(friendly, safe_squares);
	
	// Only consider king safety, when we have at least two attackers or one attacker when they have a queen
	if (info.king_attackers[friendly] > (1 - pop_count(board.pieces(enemy, QUEEN)))) {

		int king_danger = 0;
		
		uint64_t weak_squares = info.attacked[enemy] & ~info.attacked_by_multiple[friendly] & 
					(~info.attacked[friendly] | info.attacked_by_piece[friendly][KING]);
		unsigned weak_king_ring = pop_count(info.king_ring[friendly] & weak_squares);
		king_danger += king_zone_weak_square * weak_king_ring;
		record_king_zone_weak_square(friendly, weak_king_ring);

		// Safe checks by enemy pieces that do not lose material
		uint64_t safe = (~info.attacked[friendly] | (weak_squares & info.attacked_by_multiple[enemy])) & ~board.pieces(enemy);
		unsigned knight_checks = pop_count(piece_attacks(KNIGHT, square, 0ULL)      & safe & info.attacked_by_piece[enemy][KNIGHT]);
		king_danger += safe_knight_check * knight_checks;
		record_safe_knight_check(friendly, knight_checks);

		unsigned bishop_checks = pop_count(piece_attacks(BISHOP, square, board.occ) & safe & info.attacked_by_piece[enemy][BISHOP]);
		king_danger += safe_bishop_check * bishop_checks;
		record_safe_bishop_check(friendly, bishop_checks);

		unsigned rook_checks   = pop_count(piece_attacks(ROOK,   square, board.occ) & safe & info.attacked_by_piece[enemy][ROOK]);
		king_danger += safe_rook_check * rook_checks;
		record_safe_rook_check(friendly, rook_checks);

		unsigned queen_checks  = pop_count(piece_attacks(QUEEN,  square, board.occ) & safe & info.attacked_by_piece[enemy][QUEEN]);
		king_danger += safe_queen_check * queen_checks;
		record_safe_queen_check(friendly, queen_checks);

		king_danger += info.king_attackers_weight[friendly] * info.king_attackers[friendly] +
			       king_zone_attack_count_weight * info.king_zone_attacks[friendly] +
			       king_danger_no_queen_weight * !board.pieces(enemy, QUEEN) +
			       king_danger_offset;

		int mg_king_danger = std::max(0, int(mg_score(king_danger)));
		int eg_king_danger = std::max(0, int(eg_score(king_danger)));
		score -= merge_score(mg_king_danger * mg_king_danger / 4096, eg_king_danger / 16);

		record_adjust_king_attacker_weights(friendly, info.king_attackers[friendly]);
		record_king_zone_attack_count_weight(friendly, info.king_zone_attacks[friendly]);
		record_king_danger_no_queen_weight(friendly, !board.pieces(enemy, QUEEN));
		record_king_danger_offset(friendly);
	}
	else record_clear_attacker_weights(friendly);
	return score;
}

int Evaluation::evaluate_passed_pawns(Board &board, Color friendly)
{
	int score = 0;
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
			score += passed_pawn[relative_rank];
			record_passed_pawn(friendly, relative_rank);
		}
		else {
			score += passed_pawn_blocked[relative_rank];
			record_passed_pawn_blocked(friendly, relative_rank);
		}

		if (safe_advance) {
			score += passed_pawn_safe_advance;
			record_passed_pawn_safe_advance(friendly);
		}

		if (safe_path) {
			score += passed_pawn_safe_path;
			record_passed_pawn_safe_path(friendly);
		}

		// Evaluate based on the distance of the two kings from the passer
		unsigned friendly_distance = square_distance(square, board.square(friendly, KING));
		score += friendly_distance * passed_friendly_distance[relative_rank];
		record_passed_friendly_distance(friendly, relative_rank, friendly_distance);

		unsigned enemy_distance    = square_distance(square, board.square(enemy,    KING));
		score += enemy_distance * passed_enemy_distance[relative_rank];
		record_passed_enemy_distance(friendly, relative_rank, enemy_distance);
	}
	return score;
}

int Evaluation::evaluate_threats(Board &board, Color friendly)
{
	int score = 0;
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
	score += minor_threatened_by_pawn * minors_threatened_by_pawns;
	record_minor_threatened_by_pawn(friendly, minors_threatened_by_pawns);

	// Our minors attacked by enemy minors
	uint64_t minors_threatened_by_minors = pop_count((friendly_knights | friendly_bishops) & attacked_by_minor);
	score += minor_threatened_by_minor * minors_threatened_by_minors;
	record_minor_threatened_by_minor(friendly, minors_threatened_by_minors);

	// Our rooks attacked by enemy minors or pawns
	uint64_t rooks_threatened_by_lesser = pop_count(friendly_rooks & (attacked_by_pawn | attacked_by_minor));
	score += rook_threatened_by_lesser * rooks_threatened_by_lesser;
	record_rook_threatened_by_lesser(friendly, rooks_threatened_by_lesser);

	// Our queens attacked by lesser pieces
	uint64_t queens_threatened_by_lesser = pop_count(friendly_queens & (info.attacked[enemy] & ~info.attacked_by_piece[enemy][QUEEN]));
	score += queen_threatened_by_lesser * queens_threatened_by_lesser;
	record_queen_threatened_by_lesser(friendly, queens_threatened_by_lesser);

	// Undefended minors attacked by majors
	uint64_t weak = info.attacked[enemy] & ~info.attacked[friendly];
	uint64_t minors_threatened_by_majors = pop_count((friendly_knights | friendly_bishops) & weak & attacked_by_major);
	score += minor_threatened_by_major * minors_threatened_by_majors;
	record_minor_threatened_by_major(friendly, minors_threatened_by_majors);
	
	return score;
}

int Evaluation::evaluate_center_control(Color friendly)
{
	int score = 0;
	Color enemy = swap(friendly);
	unsigned center_attacks = pop_count(~info.attacked[enemy] & info.attacked[friendly] & CENTER);
	score += center_control * center_attacks;
	record_center_control(friendly, center_attacks);

	return score;
}

int Evaluation::scale_factor(Board &board, int eg_value)
{
	Color strong_side = (eg_value > 0) ? WHITE : BLACK;

	if (pop_count(board.pieces(WHITE, BISHOP)) == 1 && pop_count(board.pieces(BLACK, BISHOP)) == 1 &&
	    pop_count(board.pieces(BISHOP) & WHITE_SQUARES) == 1) {
		return std::min(128U, 40 + pop_count(board.pieces(strong_side)) * 6);
	}

	return std::min(128U, 80 + pop_count(board.pieces(strong_side, PAWN)) * 25);
}

// main evaluation function
int Evaluation::evaluate(Board &board)
{
	info.init(board);
	int score = 0;

	int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
	material = std::max(taper_end, std::min(material, taper_start)); // endgame and midgame limit clamp
	int phase = ((material - taper_end) * 256) / (taper_start - taper_end); // 0(Endgame) - 256(Midgame) linear interpolation
	
	record_phase(phase);

	Pawn_hash_entry &entry = pawn_hash_table.probe(board.zobrist.pawn_key);
	if (pawn_hash_table.hit && use_pawn_hash_table) {
		score += entry.score;
		info.passed_pawns = entry.passed_pawns;
	}
	else {
		score += evaluate_pawns(  board, WHITE) - evaluate_pawns(  board, BLACK);
		pawn_hash_table.store(board.zobrist.pawn_key, score, info.passed_pawns);
	}
	score += evaluate_knights(board, WHITE) - evaluate_knights(board, BLACK);
	score += evaluate_bishops(board, WHITE) - evaluate_bishops(board, BLACK);
	score += evaluate_rooks(  board, WHITE) - evaluate_rooks(  board, BLACK);
	score += evaluate_queens( board, WHITE) - evaluate_queens( board, BLACK);
	score += evaluate_kings(  board, WHITE) - evaluate_kings(  board, BLACK);

	score += evaluate_passed_pawns(board, WHITE) - evaluate_passed_pawns(board, BLACK);

	score += evaluate_threats(board, WHITE) - evaluate_threats(board, BLACK);

	score += evaluate_center_control(WHITE) - evaluate_center_control(BLACK);

	int mg = mg_score(score);
	int eg = eg_score(score);
	mg += (board.side_to_move == WHITE) ? tempo_bonus : -tempo_bonus;

	// Scale down drawish looking endgames
	int scale = scale_factor(board, eg);
	record_scale_factor(scale);

	// Tapered Eval
	// interpolation between midgame and endgame to create a smooth transition
	int value = (mg * phase + eg * (256 - phase) * scale / 128) / 256;
	return (board.side_to_move == WHITE) ? value: -value;
}
