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
	uint64_t passed_pawns;

	uint64_t attacked[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		king_attackers[WHITE] = king_attackers[BLACK] = 0;
		king_zone_attacks[WHITE] = king_zone_attacks[BLACK] = 0;
		mg_king_attackers_weight[WHITE] = mg_king_attackers_weight[BLACK] = 0;
		unsigned white_king_square = board.square(WHITE, KING);
		unsigned black_king_square = board.square(BLACK, KING);
		king_ring[WHITE] = king_ring_mask(white_king_square);
		king_ring[BLACK] = king_ring_mask(black_king_square);
		passed_pawns = 0ULL;
		attacked[WHITE] = attacked_by_piece[WHITE][KING] = piece_attacks(KING, white_king_square, 0ULL);
		attacked[BLACK] = attacked_by_piece[BLACK][KING] = piece_attacks(KING, black_king_square, 0ULL);
		uint64_t white_pawn_attacks = board.all_pawn_attacks(WHITE);
		attacked[WHITE]	 	      |= white_pawn_attacks;
		attacked_by_piece[WHITE][PAWN] = white_pawn_attacks;
		uint64_t black_pawn_attacks = board.all_pawn_attacks(BLACK);
		attacked[BLACK]	 	      |= black_pawn_attacks;
		attacked_by_piece[BLACK][PAWN] = black_pawn_attacks;
	}
};

struct Evaluation : Noncopyable
{
	bool use_pawn_hash_table = true;
	Eval_info info {};
	Pawn_hash_table pawn_hash_table;

	int mg_piece_value[6] = { 49, 225, 244, 316, 717, 0, };
	int eg_piece_value[6] = { 103, 348, 366, 619, 1135, 0, };
	
	int mg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        18, 4, 20, 53, 54, 34, -33, -45,
	        -1, 4, 12, 24, 25, 40, 14, -9,
	        -2, -5, -4, 5, 11, 17, -3, -12,
	        -5, -6, 0, -0, 5, 6, 0, -19,
	        -10, -11, -6, -3, 5, -7, -3, -20,
	        -1, -0, -5, -2, -0, 7, 13, -5,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        -6, 16, 20, 13, 15, 13, 24, 6,
	        21, 19, 13, 3, 7, 12, 17, 15,
	        11, 3, -4, -17, -14, -8, 0, 2,
	        5, -0, -12, -14, -15, -11, -7, -5,
	        -1, -4, -7, -6, -4, -6, -9, -6,
	        2, -0, 3, 3, 9, 5, -2, -8,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_knight_psqt[64] = {
	        -93, -21, -59, -8, 9, -30, -5, -57,
	        -24, -36, -7, -4, 5, 18, -28, -5,
	        -41, -23, -6, -3, 3, 18, -10, 4,
	        -12, 2, 5, 16, 9, 21, -1, 14,
	        2, 5, 10, 15, 17, 11, 25, 13,
	        -5, -1, 1, 6, 10, 2, 8, 0,
	        -16, -11, -8, 4, -3, 2, -5, 4,
	        -47, -1, -25, -8, -0, 2, 1, -46,
	};
	int eg_knight_psqt[64] = {
	        -41, -14, 9, 2, 5, 13, -9, -43,
	        -3, 9, 2, 27, 23, -6, 7, -4,
	        3, 8, 24, 26, 24, 28, 11, 1,
	        7, 1, 22, 31, 33, 24, 20, 11,
	        -6, 4, 20, 24, 24, 23, 7, 10,
	        -32, -15, -5, 10, 9, -5, -8, -27,
	        -19, -11, -24, -11, -8, -21, -6, -1,
	        -47, -33, -20, -10, -13, -20, -18, -19,
	};
	
	int mg_bishop_psqt[64] = {
	        -16, -32, -61, -55, -52, -56, -2, -21,
	        -44, -22, -15, -28, -30, -8, -31, -30,
	        -11, -5, 16, 3, 13, 15, 10, -10,
	        -18, 6, -3, 16, -0, 6, 2, -11,
	        -8, -2, 2, 0, 8, -0, -0, 2,
	        -1, 4, 0, 6, 4, 4, 6, 11,
	        13, 7, 8, -1, -0, 5, 20, 13,
	        11, 10, -1, -7, -6, 0, 2, 15,
	};
	int eg_bishop_psqt[64] = {
	        10, 14, 13, 19, 13, 12, 6, 10,
	        14, 10, 4, 10, 10, 5, 7, 11,
	        7, 10, 3, 5, 5, 17, 13, 13,
	        6, 5, 4, 11, 14, 7, 11, 11,
	        -7, -3, 4, 7, 6, 2, -2, -4,
	        -7, -5, -2, -0, 1, -3, -10, 1,
	        -15, -17, -15, -8, -7, -20, -13, -26,
	        -10, -4, -8, -12, -9, -9, -3, -4,
	};
	
	int mg_rook_psqt[64] = {
	        24, 17, -12, -4, -0, 19, 31, 32,
	        -10, -18, -3, 9, 6, 21, 0, 12,
	        -13, 20, 12, 32, 44, 48, 73, 16,
	        -8, 4, 9, 25, 17, 21, 23, 12,
	        -17, -18, -17, -9, -9, -10, 4, -5,
	        -20, -15, -19, -12, -13, -13, 8, -12,
	        -34, -14, -13, -10, -9, -3, 1, -38,
	        -14, -14, -12, -6, -6, -4, 0, -4,
	};
	int eg_rook_psqt[64] = {
	        19, 25, 32, 29, 28, 29, 25, 24,
	        12, 17, 15, 17, 18, 8, 14, 8,
	        21, 13, 20, 11, 7, 16, -1, 14,
	        12, 12, 13, 9, 10, 8, 5, 7,
	        -3, 7, 8, 3, 1, 4, 1, -5,
	        -18, -8, -10, -14, -13, -11, -15, -19,
	        -19, -23, -20, -23, -23, -25, -28, -15,
	        -19, -16, -14, -20, -19, -12, -18, -27,
	};
	
	int mg_queen_psqt[64] = {
	        -7, 7, -7, 1, -7, 23, 38, 35,
	        -20, -52, -25, -48, -56, 3, -30, 13,
	        -18, -12, -21, -18, -10, -7, 7, -35,
	        -3, -5, -21, -31, -29, -19, 8, 2,
	        2, -2, -8, -16, -16, -3, 9, 10,
	        -3, 7, -0, -4, -1, -1, 14, 9,
	        2, 8, 12, 7, 9, 15, 24, 13,
	        12, -0, 6, 11, 9, -7, -4, -2,
	};
	int eg_queen_psqt[64] = {
	        5, 3, 24, 28, 42, 39, 30, 20,
	        5, 20, 15, 45, 76, 61, 55, 42,
	        -9, -9, 5, 21, 47, 66, 68, 78,
	        -20, 4, 8, 40, 54, 61, 54, 44,
	        -25, -5, -1, 27, 25, 18, 9, 18,
	        -31, -22, -9, -15, -14, -7, -24, -21,
	        -40, -40, -53, -32, -35, -67, -73, -49,
	        -55, -49, -61, -46, -54, -48, -54, -42,
	};
	
	int mg_king_psqt[64] = {
	        -1, 0, 2, 2, 1, 1, 1, -5,
	        -3, 5, 18, 9, 7, 13, 10, -2,
	        -3, 21, 29, 11, 22, 39, 41, 1,
	        -5, 30, 19, 0, 1, 45, 44, -8,
	        -2, 32, 46, -20, 7, 29, 37, -31,
	        -17, -3, 19, -9, 4, -5, -18, -29,
	        -6, -34, -20, -51, -31, -44, -27, -5,
	        5, 4, 9, -37, 11, -44, 6, 34,
	};
	int eg_king_psqt[64] = {
	        -46, -20, -20, -8, -18, -13, -12, -48,
	        -30, 27, 21, 10, 14, 24, 37, -33,
	        -4, 31, 29, 20, 18, 35, 36, -6,
	        -4, 18, 26, 24, 26, 26, 22, -7,
	        -27, 3, 10, 23, 19, 15, 9, -17,
	        -22, -1, -1, 8, 6, 8, 9, -15,
	        -14, 4, 3, 5, 1, 11, 9, -15,
	        -49, -21, -20, -24, -49, -10, -23, -67,
	};
	
	int mg_knight_mobility[9] = { -44, -24, -15, -8, -1, 0, 6, 12, 21, };
	int eg_knight_mobility[9] = { -31, -76, -28, 0, 8, 21, 25, 24, 16, };
	
	int mg_bishop_mobility[14] = { -56, -31, -14, -13, -4, 0, 2, 3, 3, 6, 8, 21, 37, 50, };
	int eg_bishop_mobility[14] = { -75, -100, -49, -22, -11, 1, 13, 19, 26, 29, 30, 23, 25, 11, };
	
	int mg_rook_mobility[15] = { -4, -43, -15, -12, -9, -8, -9, -10, -5, -0, 5, 8, 12, 20, 60, };
	int eg_rook_mobility[15] = { -6, -78, -71, -41, -24, -9, 3, 11, 14, 17, 22, 25, 26, 23, 2, };
	
	int mg_queen_mobility[28] = { 0, -0, -3, -27, -42, -15, -1, -2, -2, -1, -0, 0, 2, 4, 4, 4, 4, 1, -1, -2, 8, 9, 8, 14, -4, -11, -4, 0, };
	int eg_queen_mobility[28] = { 0, -0, -2, -12, -29, -64, -74, -43, -26, -11, -3, 10, 16, 19, 24, 26, 25, 28, 28, 23, 15, 11, 2, -6, -6, -12, -14, -1, };
	
	int mg_king_mobility[9] = { 1, -4, -41, -15, -6, 15, -9, 9, 31, };
	int eg_king_mobility[9] = { 3, 12, 21, 15, 7, 1, -0, -3, -12, };
	
	int mg_isolated_pawn = -3;
	int eg_isolated_pawn = -9;
	
	int mg_doubled_pawn = -6;
	int eg_doubled_pawn = -15;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 2;
	
	int mg_backward_pawn_half_open = -6;
	int eg_backward_pawn_half_open = -18;
	
	int mg_chained_pawn[8] = { 0, 93, 28, 15, 11, 11, 1, 0, };
	int eg_chained_pawn[8] = { 0, 63, 47, 17, 7, 5, -2, 0, };
	
	int mg_passed_pawn[8] = { 0, 52, 27, 8, -27, -0, 4, 0, };
	int eg_passed_pawn[8] = { 0, 78, 46, 33, 27, 8, 1, 0, };
	
	int mg_passed_pawn_blocked[8] = { 0, 34, 22, 4, -33, -3, -6, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 7, -11, 3, 8, 4, -6, 0, };
	
	int mg_passed_pawn_safe_advance = -2;
	int eg_passed_pawn_safe_advance = 15;
	
	int mg_passed_pawn_safe_path = -67;
	int eg_passed_pawn_safe_path = 32;
	
	int mg_passed_friendly_distance[8] = { 0, 4, 3, 8, 10, 4, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -11, -14, -15, -11, -5, -0, 0, };
	
	int mg_passed_enemy_distance[8] = { 0, 5, -3, -6, -3, -4, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 37, 32, 19, 8, 1, -1, 0, };
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -14;
	
	int mg_knight_outpost_supported = 31;
	int eg_knight_outpost_supported = 14;
	
	int mg_double_bishop = 8;
	int eg_double_bishop = 57;
	
	int mg_rook_open_file = 22;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 8;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = -0;
	int eg_rook_on_seventh = 22;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 24, 28, 32, 25, 9, 13, -12, 0, },
	        { 41, 23, 6, 7, 2, 15, 13, 0, },
	        { 39, 28, 12, 11, 16, 37, 21, 0, },
	        { 12, 20, 5, 12, 6, -28, -7, 0, },
	},
	{
	        { 0, 45, 48, 18, -1, 12, -3, 0, },
	        { 0, 40, 36, 17, 1, -2, -29, 0, },
	        { 0, 37, 20, 29, 22, -6, -0, 0, },
	        { 0, 16, 12, 17, 8, -27, -10, 0, },
	},
	};
	int eg_pawn_shelter[2][4][8] = { {
	        { -14, -21, -6, 0, 7, 28, 23, 0, },
	        { -6, -10, -1, -11, -6, 20, 34, 0, },
	        { -0, -0, 1, -16, -20, -15, 15, 0, },
	        { 9, -2, -2, -7, -18, -2, 10, 0, },
	},
	{
	        { 0, -27, -15, -1, 21, 39, -2, 0, },
	        { 0, -11, -8, -12, -4, 29, -18, 0, },
	        { 0, 0, 6, -8, -5, 15, 12, 0, },
	        { 0, 11, 12, -0, -10, 4, -7, 0, },
	},
	};
	
	int mg_safe_knight_check = -43;
	int eg_safe_knight_check = 2;
	
	int mg_safe_bishop_check = -4;
	int eg_safe_bishop_check = -8;
	
	int mg_safe_rook_check = -24;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -19;
	int eg_safe_queen_check = -24;
	
	int mg_king_attacker_weight[6] = { 0, 2, 9, 14, 36, 0, };
	int eg_king_attacker_weight[6] = { 0, 0, 0, 0, 0, 0, };
	
	int mg_king_zone_attack_count_weight = 145;
	int eg_king_zone_attack_count_weight = 0;
	
	int mg_king_danger_no_queen_weight = -277;
	int eg_king_danger_no_queen_weight = 0;
	
	int mg_king_danger_offset = 9;
	int eg_king_danger_offset = 20;
	
	int mg_center_control = 2;
	int eg_center_control = -1;
	
	int mg_minor_threatened_by_pawn = -20;
	int eg_minor_threatened_by_pawn = -42;
	
	int mg_minor_threatened_by_minor = -15;
	int eg_minor_threatened_by_minor = -34;
	
	int mg_rook_threatened_by_lesser = -18;
	int eg_rook_threatened_by_lesser = -10;
	
	int mg_queen_threatened_by_lesser = -13;
	int eg_queen_threatened_by_lesser = -5;
	
	int mg_minor_threatened_by_major = -14;
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
