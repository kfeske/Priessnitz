#pragma once

#include "utility.h"
#include "board.h"
#include "transposition_table.h"

struct Eval_info : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	unsigned king_attackers[2];
	unsigned king_zone_attacks[2];
	int mg_king_attackers_weight[2];
	int eg_king_attackers_weight[2];
	uint64_t passed_pawns;

	uint64_t attacked[2];
	uint64_t attacked_by_multiple[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		king_attackers[WHITE] = king_attackers[BLACK] = 0;
		king_zone_attacks[WHITE] = king_zone_attacks[BLACK] = 0;
		mg_king_attackers_weight[WHITE] = mg_king_attackers_weight[BLACK] = 0;
		eg_king_attackers_weight[WHITE] = eg_king_attackers_weight[BLACK] = 0;
		unsigned white_king_square = board.square(WHITE, KING);
		unsigned black_king_square = board.square(BLACK, KING);
		king_ring[WHITE] = king_ring_mask(white_king_square);
		king_ring[BLACK] = king_ring_mask(black_king_square);
		passed_pawns = 0ULL;
		attacked[WHITE] = attacked_by_piece[WHITE][KING] = piece_attacks(KING, white_king_square, 0ULL);
		attacked[BLACK] = attacked_by_piece[BLACK][KING] = piece_attacks(KING, black_king_square, 0ULL);
		uint64_t white_pawn_attacks = board.all_pawn_attacks(WHITE);
		attacked_by_multiple[WHITE] = attacked[WHITE] & white_pawn_attacks;
		attacked[WHITE]	 	      |= white_pawn_attacks;
		attacked_by_piece[WHITE][PAWN] = white_pawn_attacks;
		uint64_t black_pawn_attacks = board.all_pawn_attacks(BLACK);
		attacked_by_multiple[BLACK] = attacked[BLACK] & black_pawn_attacks;
		attacked[BLACK]	 	      |= black_pawn_attacks;
		attacked_by_piece[BLACK][PAWN] = black_pawn_attacks;
	}
};

struct Evaluation : Noncopyable
{
	bool use_pawn_hash_table = true;
	Eval_info info {};
	Pawn_hash_table pawn_hash_table;

	int mg_piece_value[6] = { 46, 270, 281, 344, 710, 0, };
	int eg_piece_value[6] = { 96, 348, 356, 630, 1196, 0, };
	
	int mg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        24, 30, 15, 77, 57, 50, -23, -39,
	        11, 6, 23, 34, 32, 52, 22, -13,
	        -6, -5, 1, -1, 15, 16, -5, -3,
	        -10, -9, -2, 6, 8, 4, -3, -13,
	        -15, -11, -4, -0, 9, -4, -1, -21,
	        -4, 1, 2, 5, 15, 4, 14, -11,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        20, 17, 43, 24, 34, 19, 52, 44,
	        21, 30, 12, 1, 6, 5, 30, 28,
	        20, 9, -2, -14, -13, -9, 2, 4,
	        9, 5, -4, -10, -11, -6, -4, -4,
	        6, 1, -3, 1, 0, -5, -7, -5,
	        12, 11, 5, 8, 13, 7, 4, 4,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_knight_psqt[64] = {
	        -98, -100, -72, -40, -17, -72, -103, -71,
	        -39, -29, -25, -12, -17, -3, -22, -15,
	        -22, -14, -9, -9, -1, 23, -7, -3,
	        -13, -3, 6, 26, 18, 26, 1, 16,
	        0, 7, 9, 17, 17, 18, 23, 11,
	        -12, -7, -2, -2, 11, -1, 12, 2,
	        -16, -12, -9, 5, 3, 4, 7, 8,
	        -45, -8, -25, -14, -6, -2, -6, -18,
	};
	int eg_knight_psqt[64] = {
	        -33, -7, 7, -3, 4, -15, -3, -44,
	        3, 7, 8, 9, 2, -2, 4, -9,
	        -0, 2, 12, 14, 10, -7, -6, -8,
	        9, 3, 10, 12, 10, 9, 9, 1,
	        7, -0, 15, 15, 21, 9, 2, 6,
	        -10, -5, -1, 10, 10, -4, -6, -3,
	        -11, -6, -9, -7, -7, -9, -10, 4,
	        -4, -11, -11, -8, -5, -14, -1, 1,
	};
	
	int mg_bishop_psqt[64] = {
	        -21, -60, -52, -82, -78, -80, -43, -51,
	        -26, -28, -22, -36, -25, -22, -43, -31,
	        -10, 4, -5, 1, -3, 21, 5, -0,
	        -16, -8, -4, 2, -7, -1, -7, -17,
	        -6, -19, -5, -0, -3, -2, -10, 5,
	        -4, 7, 2, 3, 6, 4, 8, 15,
	        20, 7, 15, 0, 6, 14, 28, 21,
	        7, 18, 6, -5, 6, 0, 12, 26,
	};
	int eg_bishop_psqt[64] = {
	        5, 12, 6, 14, 13, 4, 1, -5,
	        -6, 4, 1, 6, -1, -1, 6, -4,
	        10, 1, 5, -2, 2, 9, 6, 12,
	        4, 9, 5, 19, 11, 10, 6, 8,
	        -1, 6, 10, 12, 12, 4, 5, -9,
	        -2, 2, 3, 7, 10, 4, -3, -5,
	        0, -13, -11, -5, -0, -8, -9, -14,
	        -12, -3, -4, -5, -8, 3, -9, -23,
	};
	
	int mg_rook_psqt[64] = {
	        2, -10, -21, -29, -12, -5, 11, 26,
	        -17, -17, -11, 3, -12, 5, 12, 21,
	        -16, 12, -0, 4, 27, 31, 69, 25,
	        -14, -1, -2, -3, 6, 18, 19, 10,
	        -18, -19, -13, -8, -4, -10, 11, -6,
	        -17, -14, -10, -10, 0, 3, 29, 8,
	        -17, -11, -4, -2, 4, 9, 26, -6,
	        -2, -5, -4, 2, 10, 7, 11, 8,
	};
	int eg_rook_psqt[64] = {
	        21, 25, 32, 30, 24, 25, 25, 19,
	        9, 17, 20, 11, 13, 13, 10, 3,
	        19, 17, 18, 15, 7, 5, 2, 2,
	        17, 13, 16, 13, 2, -1, 4, 0,
	        5, 7, 4, 3, 0, 0, -6, -5,
	        -5, -6, -8, -6, -11, -15, -25, -23,
	        -13, -10, -11, -12, -18, -20, -30, -22,
	        -6, -10, -6, -10, -15, -10, -15, -19,
	};
	
	int mg_queen_psqt[64] = {
	        -28, -22, -28, 1, -21, -15, 53, 5,
	        -7, -23, -26, -43, -43, -11, -0, 34,
	        -1, -10, -16, -11, -4, 8, 21, 15,
	        -10, -7, -15, -28, -22, -5, 5, 10,
	        3, -14, -14, -18, -18, -7, 11, 16,
	        0, 3, -8, -8, -4, 3, 20, 15,
	        6, 4, 9, 10, 10, 15, 26, 26,
	        -1, -9, -2, 9, 5, -4, -1, 14,
	};
	int eg_queen_psqt[64] = {
	        7, 1, 20, 15, 23, 21, -28, 1,
	        -7, 1, 26, 43, 54, 30, 3, 15,
	        -3, 3, 30, 29, 33, 21, 6, 15,
	        10, 11, 19, 36, 34, 19, 28, 12,
	        -8, 15, 17, 29, 30, 14, 4, 0,
	        -23, -7, 8, 5, 10, -0, -19, -21,
	        -28, -26, -28, -19, -18, -39, -60, -63,
	        -25, -20, -23, -20, -29, -37, -38, -41,
	};
	
	int mg_king_psqt[64] = {
	        16, 25, 8, -54, -4, -24, 9, 85,
	        -67, 6, 9, 93, 48, 43, 47, 58,
	        -66, 63, 3, -15, 34, 97, 69, 37,
	        10, 7, -27, -74, -70, -20, -28, -84,
	        -13, -11, -16, -64, -56, -14, -33, -85,
	        2, 31, 0, -16, -6, -9, 6, -22,
	        19, -12, -12, -15, -20, -24, -13, 2,
	        29, 12, 11, -26, 26, -34, 6, 38,
	};
	int eg_king_psqt[64] = {
	        -88, -32, -17, 9, -2, 11, 16, -83,
	        -4, 23, 28, 19, 33, 45, 45, 16,
	        0, 20, 35, 43, 46, 40, 38, 6,
	        -18, 14, 32, 43, 44, 39, 31, 7,
	        -28, 3, 18, 33, 31, 20, 14, -5,
	        -32, -7, 5, 13, 12, 9, 1, -16,
	        -28, -0, -0, 2, 6, 5, 6, -15,
	        -69, -24, -24, -17, -32, -16, -20, -66,
	};
	
	int mg_knight_mobility[9] = { -131, -46, -23, -14, -5, -3, 7, 17, 26, };
	int eg_knight_mobility[9] = { -104, -62, -31, -11, -1, 8, 12, 15, 12, };
	
	int mg_bishop_mobility[14] = { -29, -49, -22, -16, -4, 3, 8, 13, 16, 19, 20, 31, 32, 43, };
	int eg_bishop_mobility[14] = { -122, -71, -34, -14, -7, -0, 8, 12, 17, 18, 19, 13, 13, 5, };
	
	int mg_rook_mobility[15] = { -65, -48, -15, -10, -4, -2, -1, -5, -1, 3, 8, 8, 11, 17, 20, };
	int eg_rook_mobility[15] = { -85, -86, -55, -39, -34, -17, -11, -3, -2, 2, 5, 10, 14, 15, 13, };
	
	int mg_queen_mobility[28] = { 0, 0, -90, -51, -42, -14, -9, -8, -7, -7, -4, -1, 1, 4, 4, 4, 4, 3, 4, 7, 12, 24, 41, 52, 45, 76, 3, -28, };
	int eg_queen_mobility[28] = { 0, 0, -62, -140, -88, -69, -57, -38, -24, -5, -1, 3, 8, 9, 11, 16, 16, 18, 18, 12, 8, -10, -22, -37, -37, -59, -49, -62, };
	
	int mg_king_mobility[9] = { 7, -81, -56, -29, -16, 14, -7, 12, 35, };
	int eg_king_mobility[9] = { 46, 116, 35, 23, 12, -5, 1, -2, -15, };
	
	int mg_isolated_pawn = -4;
	int eg_isolated_pawn = -7;
	
	int mg_doubled_pawn = 1;
	int eg_doubled_pawn = -16;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 1;
	
	int mg_backward_pawn_half_open = -16;
	int eg_backward_pawn_half_open = -15;
	
	int mg_chained_pawn[8] = { 0, 113, 18, 17, 15, 11, 1, 0, };
	int eg_chained_pawn[8] = { 0, 66, 56, 22, 10, 8, -3, 0, };
	
	int mg_passed_pawn[8] = { 0, 67, 20, -7, -23, -9, 13, 0, };
	int eg_passed_pawn[8] = { 0, 85, 65, 44, 28, 13, -1, 0, };
	
	int mg_passed_pawn_blocked[8] = { 0, 26, 15, -19, -36, -14, 1, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 20, 3, 22, 17, 14, 0, 0, };
	
	int mg_passed_pawn_safe_advance = 1;
	int eg_passed_pawn_safe_advance = 10;
	
	int mg_passed_pawn_safe_path = -69;
	int eg_passed_pawn_safe_path = 39;
	
	int mg_passed_friendly_distance[8] = { 0, -8, 3, 11, 11, 4, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -11, -17, -16, -11, -5, -2, 0, };
	
	int mg_passed_enemy_distance[8] = { 0, 12, -5, -5, -4, -1, -3, 0, };
	int eg_passed_enemy_distance[8] = { 0, 35, 34, 18, 7, -1, 0, 0, };
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -6;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 22;
	
	int mg_double_bishop = 17;
	int eg_double_bishop = 56;
	
	int mg_rook_open_file = 26;
	int eg_rook_open_file = 7;
	
	int mg_rook_half_open_file = 9;
	int eg_rook_half_open_file = 8;
	
	int mg_rook_on_seventh = 4;
	int eg_rook_on_seventh = 21;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 0, 26, 44, 28, 28, 45, 43, 0, },
	        { 0, 4, 20, 22, 16, 30, 52, 0, },
	        { 0, 8, 11, 14, 16, 13, 41, 0, },
	        { 0, 18, -11, 15, 13, 10, 14, 0, },
	},
	{
	        { 0, 17, 54, 28, 42, 59, 68, 0, },
	        { 0, 27, 35, 27, 27, 55, 65, 0, },
	        { 0, 19, 14, 30, 36, 36, 66, 0, },
	        { 0, -21, -25, 16, 28, 25, 12, 0, },
	},
	};
	int eg_pawn_shelter[2][4][8] = { {
	        { 0, 29, 12, -3, -5, -11, -26, 0, },
	        { 0, 12, 9, 1, -1, -5, -6, 0, },
	        { 0, 36, 10, -0, -4, 5, 1, 0, },
	        { 0, 6, 5, -0, 1, -3, 4, 0, },
	},
	{
	        { 0, 62, 19, -1, -6, -15, -42, 0, },
	        { 0, 31, -1, -1, -4, -5, -12, 0, },
	        { 0, 24, 6, 2, -3, 3, 1, 0, },
	        { 0, 26, 5, 8, 5, -2, 8, 0, },
	},
	};
	
	int mg_pawn_storm[2][4][8] = {
	{
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	},
	{
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	},
	};
	int eg_pawn_storm[2][4][8] = { {
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	},
	{
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	        { 0, 0, 0, 0, 0, 0, 0, 0, },
	},
	};
	
	int mg_king_attacker_weight[6] = { 0, 24, 31, 33, 11, 0, };
	int eg_king_attacker_weight[6] = { 0, 90, 113, 17, 22, 0, };
	
	int mg_king_zone_attack_count_weight = 69;
	int eg_king_zone_attack_count_weight = -23;
	
	int mg_king_danger_no_queen_weight = -582;
	int eg_king_danger_no_queen_weight = -304;
	
	int mg_safe_knight_check = 359;
	int eg_safe_knight_check = 9;
	
	int mg_safe_bishop_check = 238;
	int eg_safe_bishop_check = 426;
	
	int mg_safe_rook_check = 344;
	int eg_safe_rook_check = 215;
	
	int mg_safe_queen_check = 186;
	int eg_safe_queen_check = 74;
	
	int mg_king_zone_weak_square = 101;
	int eg_king_zone_weak_square = -14;
	
	int mg_king_danger_offset = 25;
	int eg_king_danger_offset = 16;
	
	int mg_center_control = 4;
	int eg_center_control = -0;
	
	int mg_minor_threatened_by_pawn = -38;
	int eg_minor_threatened_by_pawn = -17;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -19;
	
	int mg_rook_threatened_by_lesser = -41;
	int eg_rook_threatened_by_lesser = 2;
	
	int mg_queen_threatened_by_lesser = -36;
	int eg_queen_threatened_by_lesser = 9;
	
	int mg_minor_threatened_by_major = -10;
	int eg_minor_threatened_by_major = -13;

	int tempo_bonus = 19;

	int taper_start = 6377;
	int taper_end = 321;

	void evaluate_pawns(  Board &board, Color friendly);
	void evaluate_knights(Board &board, Color friendly);
	void evaluate_bishops(Board &board, Color friendly);
	void evaluate_rooks(  Board &board, Color friendly);
	void evaluate_queens( Board &board, Color friendly);
	void evaluate_kings(  Board &board, Color friendly);

	void evaluate_passed_pawns(Board &board, Color friendly);
	
	void evaluate_threats(Board &board, Color friendly);

	void evaluate_center_control(Color friendly);

	void note_king_attacks(Piece_type type, uint64_t attacks, Color friendly);

	int scale_factor(Board &board, int eg_value);

	int evaluate(Board &board);
};
