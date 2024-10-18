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

	int mg_piece_value[6] = { 47, 272, 282, 348, 714, 0, };
	int eg_piece_value[6] = { 97, 352, 360, 635, 1204, 0, };
	
	int mg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        22, 28, 15, 78, 60, 52, -25, -39,
	        10, 5, 23, 34, 33, 50, 21, -17,
	        -6, -5, 0, -0, 16, 16, -4, -0,
	        -11, -9, -1, 6, 9, 6, -2, -11,
	        -16, -11, -4, -1, 9, -4, -1, -20,
	        -4, 1, 2, 5, 15, 4, 14, -10,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        20, 18, 43, 24, 34, 19, 53, 44,
	        22, 31, 12, 2, 6, 6, 31, 29,
	        20, 9, -2, -14, -13, -9, 2, 4,
	        10, 5, -4, -10, -11, -7, -4, -5,
	        6, 1, -3, 1, 0, -5, -7, -6,
	        12, 12, 6, 8, 14, 7, 4, 4,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_knight_psqt[64] = {
	        -99, -96, -74, -41, -23, -79, -104, -73,
	        -38, -28, -24, -13, -18, -7, -20, -19,
	        -21, -13, -8, -10, -6, 20, -18, -7,
	        -12, -3, 7, 23, 15, 22, 0, 10,
	        1, 7, 10, 17, 13, 14, 11, 5,
	        -10, -5, -1, -0, 13, 1, 15, 3,
	        -15, -10, -7, 7, 4, 5, 10, 9,
	        -44, -7, -23, -12, -5, -1, -6, -17,
	};
	int eg_knight_psqt[64] = {
	        -34, -8, 6, -3, 5, -14, -3, -44,
	        3, 7, 8, 10, 2, -1, 4, -8,
	        -0, 2, 13, 15, 12, -6, -4, -7,
	        9, 3, 10, 12, 11, 10, 8, 3,
	        7, -1, 15, 15, 21, 9, 3, 5,
	        -11, -6, -1, 10, 9, -4, -7, -3,
	        -12, -6, -10, -8, -8, -9, -11, 3,
	        -5, -12, -12, -9, -6, -15, -2, 0,
	};
	
	int mg_bishop_psqt[64] = {
	        -20, -61, -52, -85, -80, -82, -41, -55,
	        -25, -26, -20, -34, -22, -20, -50, -32,
	        -9, 5, -4, 3, -2, 16, 0, -4,
	        -15, -6, -3, 5, -6, -2, -9, -18,
	        -5, -17, -4, -1, -4, -4, -11, 2,
	        -3, 8, 2, 3, 5, 7, 9, 15,
	        20, 7, 15, -1, 7, 14, 27, 21,
	        7, 17, 5, -3, 6, 1, 12, 28,
	};
	int eg_bishop_psqt[64] = {
	        4, 12, 6, 15, 13, 5, 0, -5,
	        -6, 4, 0, 6, -2, -2, 7, -4,
	        10, 1, 5, -2, 2, 11, 7, 14,
	        4, 8, 5, 19, 11, 10, 5, 8,
	        -2, 6, 10, 13, 12, 4, 5, -10,
	        -2, 2, 3, 6, 10, 4, -3, -5,
	        0, -13, -11, -5, -1, -8, -9, -14,
	        -12, -3, -4, -6, -8, 3, -10, -24,
	};
	
	int mg_rook_psqt[64] = {
	        4, -6, -18, -26, -8, -1, 14, 28,
	        -15, -16, -8, 4, -12, 4, 11, 18,
	        -17, 11, -2, -0, 19, 21, 58, 16,
	        -13, 1, 0, -0, 8, 15, 17, 6,
	        -17, -18, -11, -7, -2, -11, 7, -11,
	        -15, -12, -9, -8, 2, 2, 25, 3,
	        -15, -10, -2, -0, 6, 8, 24, -7,
	        -1, -3, -2, 4, 11, 8, 9, 7,
	};
	int eg_rook_psqt[64] = {
	        20, 24, 32, 29, 24, 25, 25, 19,
	        8, 17, 20, 11, 13, 13, 10, 3,
	        19, 17, 19, 16, 9, 7, 4, 3,
	        17, 13, 17, 13, 2, -0, 5, 2,
	        5, 7, 4, 3, -0, 1, -4, -4,
	        -5, -6, -8, -7, -11, -15, -24, -22,
	        -13, -10, -12, -13, -18, -20, -29, -21,
	        -6, -10, -6, -10, -16, -10, -14, -19,
	};
	
	int mg_queen_psqt[64] = {
	        -27, -19, -26, 5, -16, -12, 56, 5,
	        -7, -21, -24, -41, -43, -13, -0, 31,
	        0, -8, -14, -10, -5, -1, 15, 8,
	        -10, -6, -14, -26, -22, -6, 2, 4,
	        4, -14, -13, -19, -19, -9, 8, 11,
	        1, 4, -8, -7, -5, 3, 19, 11,
	        7, 4, 10, 10, 12, 15, 24, 26,
	        -0, -8, -1, 11, 7, -3, -1, 16,
	};
	int eg_queen_psqt[64] = {
	        10, 1, 22, 16, 24, 23, -27, 4,
	        -5, 2, 28, 45, 55, 33, 4, 15,
	        -7, -2, 23, 27, 33, 19, 1, 12,
	        13, 14, 22, 38, 34, 18, 27, 13,
	        -7, 18, 19, 31, 30, 12, 5, -1,
	        -23, -7, 9, 5, 9, -0, -20, -23,
	        -27, -25, -27, -20, -18, -40, -61, -65,
	        -25, -21, -23, -20, -28, -37, -38, -43,
	};
	
	int mg_king_psqt[64] = {
	        25, 44, 20, -36, 23, -10, 22, 85,
	        -78, -3, 0, 87, 41, 37, 38, 45,
	        -79, 55, -5, -24, 26, 89, 59, 25,
	        -2, -1, -36, -84, -79, -28, -38, -99,
	        -25, -17, -22, -71, -61, -19, -38, -98,
	        -8, 29, -5, -20, -9, -11, 5, -31,
	        16, -12, -14, -17, -22, -27, -12, -0,
	        25, 22, 14, -22, 29, -29, 13, 32,
	};
	int eg_king_psqt[64] = {
	        -90, -34, -17, 8, -4, 10, 15, -85,
	        -4, 23, 28, 20, 34, 46, 45, 16,
	        0, 21, 35, 44, 47, 41, 39, 7,
	        -17, 15, 33, 44, 45, 40, 32, 8,
	        -28, 3, 19, 34, 32, 21, 14, -4,
	        -32, -8, 5, 13, 12, 9, 1, -16,
	        -29, -1, -0, 2, 6, 5, 6, -16,
	        -69, -25, -24, -17, -32, -16, -21, -66,
	};
	
	int mg_knight_mobility[9] = { -133, -46, -23, -14, -5, -3, 7, 16, 26, };
	int eg_knight_mobility[9] = { -106, -63, -32, -12, -1, 8, 11, 14, 11, };
	
	int mg_bishop_mobility[14] = { -29, -49, -22, -16, -4, 3, 7, 13, 16, 18, 20, 30, 33, 42, };
	int eg_bishop_mobility[14] = { -125, -72, -35, -14, -7, -1, 8, 12, 17, 18, 19, 13, 13, 4, };
	
	int mg_rook_mobility[15] = { -68, -49, -15, -9, -3, -2, -1, -5, -1, 3, 7, 8, 11, 16, 19, };
	int eg_rook_mobility[15] = { -87, -86, -56, -40, -34, -17, -11, -3, -2, 2, 5, 10, 14, 15, 13, };
	
	int mg_queen_mobility[28] = { 0, 0, -91, -52, -40, -14, -8, -7, -6, -6, -4, -1, 1, 4, 4, 4, 4, 2, 4, 7, 13, 26, 43, 55, 47, 77, 0, -30, };
	int eg_queen_mobility[28] = { 0, 0, -63, -144, -92, -68, -55, -37, -22, -3, 1, 4, 10, 10, 13, 17, 17, 19, 18, 12, 7, -12, -25, -40, -41, -65, -51, -65, };
	
	int mg_king_mobility[9] = { 6, -70, -46, -23, -15, 11, -10, 9, 32, };
	int eg_king_mobility[9] = { 47, 117, 33, 23, 12, -5, 1, -2, -15, };
	
	int mg_isolated_pawn = -4;
	int eg_isolated_pawn = -7;
	
	int mg_doubled_pawn = 0;
	int eg_doubled_pawn = -16;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 1;
	
	int mg_backward_pawn_half_open = -16;
	int eg_backward_pawn_half_open = -15;
	
	int mg_chained_pawn[8] = { 0, 112, 19, 17, 15, 12, 1, 0, };
	int eg_chained_pawn[8] = { 0, 68, 56, 22, 10, 8, -3, 0, };
	
	int mg_passed_pawn[8] = { 0, 68, 19, -9, -24, -7, 17, 0, };
	int eg_passed_pawn[8] = { 0, 86, 65, 45, 29, 13, -1, 0, };
	
	int mg_passed_pawn_blocked[8] = { 0, 24, 15, -21, -37, -14, 5, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 21, 2, 22, 17, 14, -0, 0, };
	
	int mg_passed_pawn_safe_advance = 1;
	int eg_passed_pawn_safe_advance = 10;
	
	int mg_passed_pawn_safe_path = -69;
	int eg_passed_pawn_safe_path = 40;
	
	int mg_passed_friendly_distance[8] = { 0, -8, 4, 12, 12, 4, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -11, -17, -16, -11, -5, -2, 0, };
	
	int mg_passed_enemy_distance[8] = { 0, 12, -5, -5, -4, -1, -4, 0, };
	int eg_passed_enemy_distance[8] = { 0, 35, 35, 18, 7, -1, 0, 0, };
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -6;
	
	int mg_knight_outpost_supported = 38;
	int eg_knight_outpost_supported = 21;
	
	int mg_double_bishop = 17;
	int eg_double_bishop = 57;
	
	int mg_rook_open_file = 26;
	int eg_rook_open_file = 7;
	
	int mg_rook_half_open_file = 8;
	int eg_rook_half_open_file = 8;
	
	int mg_rook_on_seventh = 2;
	int eg_rook_on_seventh = 21;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 0, 23, 45, 25, 25, 42, 39, 0, },
	        { 0, 3, 17, 22, 15, 30, 50, 0, },
	        { 0, 8, 6, 14, 15, 14, 41, 0, },
	        { 0, 17, -13, 14, 12, 11, 14, 0, },
	},
	{
	        { 0, 13, 58, 27, 41, 59, 66, 0, },
	        { 0, 26, 32, 25, 23, 55, 62, 0, },
	        { 0, 16, 15, 29, 35, 37, 65, 0, },
	        { 0, -23, -23, 16, 26, 25, 12, 0, },
	},
	};
	int eg_pawn_shelter[2][4][8] = { {
	        { 0, 29, 12, -3, -4, -11, -25, 0, },
	        { 0, 12, 9, 1, -0, -6, -6, 0, },
	        { 0, 37, 11, -0, -4, 4, 1, 0, },
	        { 0, 7, 5, -0, 1, -3, 3, 0, },
	},
	{
	        { 0, 63, 19, -1, -5, -15, -43, 0, },
	        { 0, 31, -1, -1, -3, -5, -13, 0, },
	        { 0, 25, 6, 2, -3, 3, -0, 0, },
	        { 0, 27, 5, 9, 6, -2, 6, 0, },
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
	
	int mg_king_attacker_weight[6] = { 0, 31, 20, 33, 16, 0, };
	int eg_king_attacker_weight[6] = { 0, 94, 96, 6, 10, 0, };
	
	int mg_king_zone_attack_count_weight = 68;
	int eg_king_zone_attack_count_weight = -11;
	
	int mg_king_danger_no_queen_weight = -533;
	int eg_king_danger_no_queen_weight = -347;
	
	int mg_safe_knight_check = 320;
	int eg_safe_knight_check = 5;
	
	int mg_safe_bishop_check = 234;
	int eg_safe_bishop_check = 454;
	
	int mg_safe_rook_check = 337;
	int eg_safe_rook_check = 200;
	
	int mg_safe_queen_check = 177;
	int eg_safe_queen_check = 121;
	
	int mg_king_zone_weak_square = 107;
	int eg_king_zone_weak_square = -3;
	
	int mg_king_danger_offset = 4;
	int eg_king_danger_offset = 12;
	
	int mg_center_control = 4;
	int eg_center_control = -0;
	
	int mg_minor_threatened_by_pawn = -39;
	int eg_minor_threatened_by_pawn = -17;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -20;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 2;
	
	int mg_queen_threatened_by_lesser = -36;
	int eg_queen_threatened_by_lesser = 11;
	
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
