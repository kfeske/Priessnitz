#pragma once

#include <utility.h>

struct Trace {
	int phase {};
	int scale_factor {};

	unsigned piece_value[2][6] {};

	unsigned pawn_psqt[2][64] {};
	unsigned knight_psqt[2][64] {};
	unsigned bishop_psqt[2][64] {};
	unsigned rook_psqt[2][64] {};
	unsigned queen_psqt[2][64] {};
	unsigned king_psqt[2][64] {};

	unsigned knight_mobility[2][9] {};
	unsigned bishop_mobility[2][14] {};
	unsigned rook_mobility[2][15] {};
	unsigned queen_mobility[2][28] {};
	unsigned king_mobility[2][9] {};

	unsigned isolated_pawn[2] {};
	unsigned doubled_pawn[2] {};
	unsigned backward_pawn[2] {};
	unsigned backward_pawn_half_open[2] {};
	unsigned chained_pawn[2][8] {};

	unsigned passed_pawn[2][8] {};
	unsigned passed_pawn_blocked[2][8] {};
	unsigned passed_pawn_safe_advance[2] {};
	unsigned passed_pawn_safe_path[2] {};
	unsigned passed_friendly_distance[2][8] {};
	unsigned passed_enemy_distance[2][8] {};

	unsigned knight_outpost[2] {};
	unsigned knight_outpost_supported[2] {};

	unsigned double_bishop[2] {};

	unsigned rook_open_file[2] {};
	unsigned rook_half_open_file[2] {};
	unsigned rook_on_seventh[2] {};

	unsigned pawn_shelter[2][2][4][8] {};
	unsigned pawn_storm[2][2][4][8] {};

	unsigned safe_knight_check[2] {};
	unsigned safe_bishop_check[2] {};
	unsigned safe_rook_check[2] {};
	unsigned safe_queen_check[2] {};

	unsigned king_zone_weak_square[2] {};

	unsigned king_attacker_weight[2][6] {};
	unsigned king_zone_attack_count_weight[2] {};
	unsigned king_danger_no_queen_weight[2] {};
	unsigned king_danger_offset[2] {};

	unsigned center_control[2] {};

	unsigned minor_threatened_by_pawn[2] {};
	unsigned minor_threatened_by_minor[2] {};
	unsigned rook_threatened_by_lesser[2] {};
	unsigned queen_threatened_by_lesser[2] {};
	unsigned minor_threatened_by_major[2] {};
};

Trace &trace();

static inline void record_phase(int phase) { trace().phase = phase; };
static inline void record_scale_factor(int scale_factor) { trace().scale_factor = scale_factor; }

static inline void record_piece_value(Color friendly, Piece_type type) { trace().piece_value[friendly][type]++; }

static inline void record_pawn_psqt(  Color friendly, unsigned square) { trace().pawn_psqt[friendly][square]++; }
static inline void record_knight_psqt(Color friendly, unsigned square) { trace().knight_psqt[friendly][square]++; }
static inline void record_bishop_psqt(Color friendly, unsigned square) { trace().bishop_psqt[friendly][square]++; }
static inline void record_rook_psqt(  Color friendly, unsigned square) { trace().rook_psqt[friendly][square]++; }
static inline void record_queen_psqt( Color friendly, unsigned square) { trace().queen_psqt[friendly][square]++; }
static inline void record_king_psqt(  Color friendly, unsigned square) { trace().king_psqt[friendly][square]++; }

static inline void record_knight_mobility(Color friendly, unsigned safe_squares) { trace().knight_mobility[friendly][safe_squares]++; }
static inline void record_bishop_mobility(Color friendly, unsigned safe_squares) { trace().bishop_mobility[friendly][safe_squares]++; }
static inline void record_rook_mobility(  Color friendly, unsigned safe_squares) { trace().rook_mobility[friendly][safe_squares]++; }
static inline void record_queen_mobility( Color friendly, unsigned safe_squares) { trace().queen_mobility[friendly][safe_squares]++; }
static inline void record_king_mobility(  Color friendly, unsigned safe_squares) { trace().king_mobility[friendly][safe_squares]++; }

static inline void record_isolated_pawn(Color friendly)               { trace().isolated_pawn[friendly]++; }
static inline void record_doubled_pawn(Color friendly)                { trace().doubled_pawn[friendly]++; }
static inline void record_backward_pawn(Color friendly)               { trace().backward_pawn[friendly]++; }
static inline void record_backward_pawn_half_open(Color friendly)     { trace().backward_pawn_half_open[friendly]++; }
static inline void record_chained_pawn(Color friendly, unsigned rank) { trace().chained_pawn[friendly][rank]++; }

static inline void record_passed_pawn(             Color friendly, unsigned rank)                    { trace().passed_pawn[friendly][rank]++; }
static inline void record_passed_pawn_blocked(     Color friendly, unsigned rank)                    { trace().passed_pawn_blocked[friendly][rank]++; }
static inline void record_passed_pawn_safe_advance(Color friendly)                                   { trace().passed_pawn_safe_advance[friendly]++; }
static inline void record_passed_pawn_safe_path(   Color friendly)                                   { trace().passed_pawn_safe_path[friendly]++; }
static inline void record_passed_friendly_distance(Color friendly, unsigned rank, unsigned distance) { trace().passed_friendly_distance[friendly][rank]	+= distance; }
static inline void record_passed_enemy_distance(   Color friendly, unsigned rank, unsigned distance) { trace().passed_enemy_distance[friendly][rank]    += distance; }

static inline void record_knight_outpost(          Color friendly) { trace().knight_outpost[friendly]++; }
static inline void record_knight_outpost_supported(Color friendly) { trace().knight_outpost_supported[friendly]++; }

static inline void record_double_bishop(Color friendly) { trace().double_bishop[friendly]++; }

static inline void record_rook_open_file(     Color friendly) { trace().rook_open_file[friendly]++; }
static inline void record_rook_half_open_file(Color friendly) { trace().rook_half_open_file[friendly]++; }
static inline void record_rook_on_seventh(    Color friendly) { trace().rook_on_seventh[friendly]++; }

static inline void record_pawn_shelter(Color friendly, bool king_file, unsigned edge_dist, unsigned king_dist) { trace().pawn_shelter[friendly][king_file][edge_dist][king_dist]++; }
static inline void record_pawn_storm(Color friendly, bool blocked, unsigned edge_dist, unsigned king_dist) { trace().pawn_storm[friendly][blocked][edge_dist][king_dist]++; }

static inline void record_safe_knight_check(Color friendly, unsigned count) { trace().safe_knight_check[friendly] += count; }
static inline void record_safe_bishop_check(Color friendly, unsigned count) { trace().safe_bishop_check[friendly] += count; }
static inline void record_safe_rook_check(  Color friendly, unsigned count) { trace().safe_rook_check[friendly]   += count; }
static inline void record_safe_queen_check( Color friendly, unsigned count) { trace().safe_queen_check[friendly]  += count; }

static inline void record_king_zone_weak_square(Color friendly, unsigned count) { trace().king_zone_weak_square[friendly] += count; }

static inline void record_king_attacker_weight(Color friendly, Piece_type type) { trace().king_attacker_weight[friendly][type]++; }
static inline void record_adjust_king_attacker_weights(Color friendly, unsigned attackers)
{
	for (Piece_type type : { KNIGHT, BISHOP, ROOK, QUEEN })
		trace().king_attacker_weight[friendly][type] *= attackers;
}
static inline void record_clear_attacker_weights(Color friendly)
{
	for (Piece_type type : { KNIGHT, BISHOP, ROOK, QUEEN })
		trace().king_attacker_weight[friendly][type] = 0;
}
static inline void record_king_zone_attack_count_weight(Color friendly, unsigned count) { trace().king_zone_attack_count_weight[friendly] += count; }
static inline void record_king_danger_no_queen_weight(Color friendly, bool no_queen) { trace().king_danger_no_queen_weight[friendly] += no_queen; }
static inline void record_king_danger_offset(Color friendly) { trace().king_danger_offset[friendly]++; }

static inline void record_center_control(Color friendly, unsigned center_attacks) { trace().center_control[friendly] += center_attacks; }

static inline void record_minor_threatened_by_pawn(  Color friendly, unsigned count) { trace().minor_threatened_by_pawn[friendly]   += count; }
static inline void record_minor_threatened_by_minor( Color friendly, unsigned count) { trace().minor_threatened_by_minor[friendly]  += count; }
static inline void record_rook_threatened_by_lesser( Color friendly, unsigned count) { trace().rook_threatened_by_lesser[friendly]  += count; }
static inline void record_queen_threatened_by_lesser(Color friendly, unsigned count) { trace().queen_threatened_by_lesser[friendly] += count; }
static inline void record_minor_threatened_by_major( Color friendly, unsigned count) { trace().minor_threatened_by_major[friendly]  += count; }
