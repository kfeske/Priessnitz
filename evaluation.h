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

	int mg_piece_value[6] = { 50, 269, 293, 346, 708, 0, };
	int eg_piece_value[6] = { 99, 354, 372, 637, 1210, 0, };
	
	int mg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        19, 25, 14, 74, 58, 49, -25, -39,
	        9, 4, 22, 33, 32, 48, 21, -16,
	        -6, -6, 1, 0, 16, 16, -4, 0,
	        -10, -9, -0, 7, 9, 5, -1, -11,
	        -15, -12, -4, -0, 10, -4, -2, -18,
	        -4, 1, 2, 5, 16, 3, 13, -9,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        20, 19, 44, 26, 34, 20, 53, 44,
	        22, 31, 12, 1, 6, 5, 31, 28,
	        20, 9, -2, -13, -14, -9, 1, 3,
	        9, 5, -4, -10, -11, -7, -4, -6,
	        6, 2, -3, -0, -0, -5, -8, -7,
	        11, 11, 5, 8, 14, 7, 3, 3,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_knight_psqt[64] = {
	        -99, -91, -71, -38, -22, -76, -99, -73,
	        -38, -28, -24, -12, -17, -7, -21, -18,
	        -22, -13, -7, -10, -5, 20, -18, -8,
	        -12, -4, 6, 22, 14, 21, -1, 10,
	        1, 6, 9, 17, 12, 13, 10, 5,
	        -11, -5, -1, -1, 13, 1, 14, 2,
	        -15, -10, -8, 6, 4, 5, 10, 8,
	        -44, -7, -23, -12, -5, -1, -5, -16,
	};
	int eg_knight_psqt[64] = {
	        -33, -7, 7, -2, 6, -14, -2, -44,
	        3, 8, 8, 11, 3, -1, 5, -8,
	        -1, 1, 10, 13, 11, -9, -4, -6,
	        10, 3, 8, 11, 10, 8, 9, 4,
	        7, 0, 12, 14, 20, 8, 3, 6,
	        -10, -6, -4, 8, 8, -7, -7, -3,
	        -10, -5, -9, -8, -8, -9, -10, 4,
	        -2, -12, -11, -8, -5, -14, -1, 1,
	};
	
	int mg_bishop_psqt[64] = {
	        -18, -58, -51, -78, -76, -73, -41, -51,
	        -23, -27, -17, -33, -18, -18, -47, -32,
	        -10, 5, -5, 6, -3, 17, 0, -1,
	        -12, -7, -1, 5, -3, -3, -6, -17,
	        -5, -14, -4, 1, -5, -2, -11, 6,
	        0, 8, 4, 1, 6, 5, 11, 14,
	        21, 9, 13, 1, 6, 15, 25, 23,
	        11, 16, 7, -4, 10, -0, 13, 28,
	};
	int eg_bishop_psqt[64] = {
	        0, 12, 4, 14, 9, 3, -2, -4,
	        -5, 4, 2, 6, -2, -2, 10, -7,
	        7, 5, 7, 1, 4, 13, 7, 12,
	        4, 9, 10, 24, 17, 12, 6, 5,
	        -5, 9, 12, 19, 15, 7, 5, -12,
	        -2, 2, 6, 8, 13, 4, -2, -8,
	        -2, -10, -11, -2, -1, -8, -12, -15,
	        -9, -6, -0, -7, -7, 3, -10, -25,
	};
	
	int mg_rook_psqt[64] = {
	        4, -5, -17, -25, -7, 1, 13, 26,
	        -14, -15, -8, 6, -11, 5, 10, 19,
	        -15, 12, -2, 0, 19, 21, 56, 15,
	        -12, 1, -1, -1, 7, 13, 16, 5,
	        -16, -18, -11, -8, -3, -10, 6, -11,
	        -16, -13, -9, -9, 1, 2, 24, 2,
	        -15, -10, -2, -1, 6, 8, 23, -8,
	        -1, -4, -3, 4, 10, 7, 8, 7,
	};
	int eg_rook_psqt[64] = {
	        18, 22, 33, 29, 24, 24, 23, 17,
	        5, 14, 20, 10, 13, 13, 9, 1,
	        19, 17, 21, 18, 10, 9, 4, 4,
	        17, 12, 18, 14, 3, 1, 5, 2,
	        5, 7, 6, 4, 1, 2, -4, -4,
	        -4, -6, -7, -6, -10, -14, -25, -21,
	        -15, -12, -12, -12, -18, -20, -30, -23,
	        -7, -11, -6, -11, -16, -11, -16, -20,
	};
	
	int mg_queen_psqt[64] = {
	        -27, -19, -25, 5, -14, -10, 54, 3,
	        -8, -22, -24, -39, -38, -11, -0, 29,
	        0, -8, -14, -9, -4, -2, 15, 9,
	        -11, -6, -13, -23, -20, -6, 2, 5,
	        4, -14, -13, -18, -17, -8, 8, 11,
	        0, 4, -8, -6, -4, 3, 19, 10,
	        6, 4, 9, 10, 11, 14, 23, 25,
	        -1, -8, -2, 10, 6, -3, -2, 16,
	};
	int eg_queen_psqt[64] = {
	        9, 0, 21, 16, 22, 22, -30, 4,
	        -8, 0, 28, 44, 52, 31, 1, 14,
	        -8, -2, 24, 28, 33, 20, -1, 9,
	        12, 13, 22, 38, 36, 17, 26, 9,
	        -9, 18, 20, 32, 31, 13, 5, -1,
	        -22, -6, 10, 5, 11, 1, -20, -22,
	        -27, -25, -26, -19, -18, -37, -60, -66,
	        -24, -22, -22, -21, -28, -37, -38, -45,
	};
	
	int mg_king_psqt[64] = {
	        25, 50, 19, -40, 20, -9, 20, 80,
	        -73, 2, 6, 80, 39, 35, 30, 41,
	        -81, 56, -7, -25, 20, 79, 46, 19,
	        2, 0, -34, -84, -79, -31, -40, -99,
	        -21, -12, -22, -68, -60, -16, -38, -93,
	        -5, 29, -2, -19, -8, -10, 6, -30,
	        15, -9, -13, -16, -21, -25, -12, -1,
	        24, 21, 14, -21, 28, -28, 13, 31,
	};
	int eg_king_psqt[64] = {
	        -92, -38, -18, 9, -6, 9, 14, -86,
	        -6, 21, 26, 21, 34, 46, 46, 16,
	        -0, 20, 36, 45, 49, 43, 41, 7,
	        -19, 14, 34, 47, 48, 42, 33, 8,
	        -30, 3, 20, 36, 35, 22, 15, -5,
	        -34, -8, 5, 14, 13, 9, 0, -17,
	        -29, -2, -1, 2, 6, 5, 5, -17,
	        -71, -26, -25, -18, -33, -17, -21, -67,
	};
	
	int mg_knight_mobility[9] = { -131, -46, -24, -15, -6, -3, 6, 15, 24, };
	int eg_knight_mobility[9] = { -106, -66, -35, -14, -3, 7, 11, 16, 14, };
	
	int mg_bishop_mobility[14] = { -30, -49, -20, -14, -3, 5, 9, 14, 18, 20, 22, 31, 32, 39, };
	int eg_bishop_mobility[14] = { -131, -72, -33, -12, -3, 4, 12, 15, 19, 19, 19, 11, 10, 0, };
	
	int mg_rook_mobility[15] = { -64, -48, -15, -10, -3, -3, -1, -5, -1, 3, 7, 8, 10, 16, 18, };
	int eg_rook_mobility[15] = { -89, -94, -58, -40, -35, -18, -12, -4, -3, 2, 5, 11, 15, 16, 14, };
	
	int mg_queen_mobility[28] = { 0, 0, -88, -52, -40, -14, -8, -7, -6, -6, -4, -1, 1, 3, 3, 4, 3, 2, 3, 6, 11, 24, 39, 52, 43, 71, 2, -29, };
	int eg_queen_mobility[28] = { 0, 0, -61, -138, -88, -67, -56, -37, -23, -4, 0, 3, 9, 10, 12, 17, 16, 18, 18, 12, 8, -12, -22, -39, -38, -61, -50, -64, };
	
	int mg_king_mobility[9] = { 4, -66, -44, -22, -14, 11, -9, 8, 29, };
	int eg_king_mobility[9] = { 42, 114, 32, 23, 11, -6, 1, -2, -15, };
	
	int mg_isolated_pawn = -4;
	int eg_isolated_pawn = -7;
	
	int mg_doubled_pawn = 1;
	int eg_doubled_pawn = -17;
	
	int mg_backward_pawn = 1;
	int eg_backward_pawn = 1;
	
	int mg_backward_pawn_half_open = -15;
	int eg_backward_pawn_half_open = -15;
	
	int mg_chained_pawn[8] = { 0, 114, 19, 17, 14, 11, 1, 0, };
	int eg_chained_pawn[8] = { 0, 64, 55, 23, 11, 8, -2, 0, };
	
	int mg_passed_pawn[8] = { 0, 65, 19, -9, -24, -7, 16, 0, };
	int eg_passed_pawn[8] = { 0, 87, 68, 46, 29, 15, -1, 0, };
	
	int mg_passed_pawn_blocked[8] = { 0, 21, 14, -21, -38, -14, 5, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 23, 6, 24, 18, 16, -0, 0, };
	
	int mg_passed_pawn_safe_advance = 1;
	int eg_passed_pawn_safe_advance = 10;
	
	int mg_passed_pawn_safe_path = -65;
	int eg_passed_pawn_safe_path = 38;
	
	int mg_passed_friendly_distance[8] = { 0, -7, 4, 12, 11, 4, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -17, -16, -11, -5, -2, 0, };
	
	int mg_passed_enemy_distance[8] = { 0, 12, -5, -5, -4, -1, -3, 0, };
	int eg_passed_enemy_distance[8] = { 0, 36, 35, 18, 7, -1, -0, 0, };
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -6;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 20;
	
	int mg_bishop_pawn = -4;
	int eg_bishop_pawn = -7;
	
	int mg_double_bishop = 17;
	int eg_double_bishop = 56;
	
	int mg_rook_open_file = 26;
	int eg_rook_open_file = 8;
	
	int mg_rook_half_open_file = 8;
	int eg_rook_half_open_file = 9;
	
	int mg_rook_on_seventh = 3;
	int eg_rook_on_seventh = 21;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 0, 21, 45, 24, 24, 41, 39, 0, },
	        { 0, 3, 17, 22, 15, 30, 50, 0, },
	        { 0, 9, 7, 14, 17, 13, 41, 0, },
	        { 0, 15, -13, 14, 11, 11, 14, 0, },
	},
	{
	        { 0, 14, 57, 26, 40, 57, 64, 0, },
	        { 0, 26, 32, 26, 23, 55, 61, 0, },
	        { 0, 17, 14, 29, 34, 36, 64, 0, },
	        { 0, -25, -22, 16, 26, 24, 12, 0, },
	},
	};
	int eg_pawn_shelter[2][4][8] = { {
	        { 0, 30, 12, -3, -5, -11, -25, 0, },
	        { 0, 11, 9, 1, -1, -6, -6, 0, },
	        { 0, 36, 12, -1, -5, 4, -0, 0, },
	        { 0, 7, 5, -0, 1, -2, 4, 0, },
	},
	{
	        { 0, 61, 18, -1, -6, -15, -44, 0, },
	        { 0, 28, -0, -1, -3, -5, -13, 0, },
	        { 0, 25, 6, 1, -3, 3, -0, 0, },
	        { 0, 28, 4, 9, 6, -0, 7, 0, },
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
	
	int mg_king_attacker_weight[6] = { 0, 32, 18, 33, 17, 0, };
	int eg_king_attacker_weight[6] = { 0, 88, 82, 6, 14, 0, };
	
	int mg_king_zone_attack_count_weight = 68;
	int eg_king_zone_attack_count_weight = -11;
	
	int mg_king_danger_no_queen_weight = -530;
	int eg_king_danger_no_queen_weight = -311;
	
	int mg_safe_knight_check = 320;
	int eg_safe_knight_check = 3;
	
	int mg_safe_bishop_check = 230;
	int eg_safe_bishop_check = 421;
	
	int mg_safe_rook_check = 338;
	int eg_safe_rook_check = 182;
	
	int mg_safe_queen_check = 175;
	int eg_safe_queen_check = 129;
	
	int mg_king_zone_weak_square = 105;
	int eg_king_zone_weak_square = -3;
	
	int mg_king_danger_offset = 4;
	int eg_king_danger_offset = 9;
	
	int mg_center_control = 3;
	int eg_center_control = -1;
	
	int mg_minor_threatened_by_pawn = -38;
	int eg_minor_threatened_by_pawn = -18;
	
	int mg_minor_threatened_by_minor = -17;
	int eg_minor_threatened_by_minor = -18;
	
	int mg_rook_threatened_by_lesser = -41;
	int eg_rook_threatened_by_lesser = 2;
	
	int mg_queen_threatened_by_lesser = -36;
	int eg_queen_threatened_by_lesser = 12;
	
	int mg_minor_threatened_by_major = -10;
	int eg_minor_threatened_by_major = -14;

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
