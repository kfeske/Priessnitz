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

	int mg_piece_value[6] = { 49, 227, 246, 322, 712, 0, };
	int eg_piece_value[6] = { 104, 351, 368, 623, 1145, 0, };
	
	int mg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        28, 12, 29, 65, 69, 40, -37, -43,
	        -1, 3, 11, 24, 24, 38, 7, -12,
	        -2, -5, -3, 5, 11, 16, -4, -13,
	        -5, -5, 1, 0, 5, 6, -0, -21,
	        -10, -10, -5, -2, 6, -7, -4, -21,
	        -1, -0, -5, -1, 0, 7, 12, -6,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        -7, 16, 19, 11, 13, 12, 27, 6,
	        21, 19, 13, 2, 7, 12, 18, 16,
	        11, 3, -4, -17, -14, -8, 0, 2,
	        4, -0, -12, -14, -15, -11, -7, -4,
	        -1, -4, -7, -6, -4, -6, -9, -6,
	        2, -1, 3, 3, 9, 5, -1, -7,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_knight_psqt[64] = {
	        -103, -33, -79, -14, 10, -50, -9, -76,
	        -23, -35, -7, -4, 4, 17, -32, -7,
	        -41, -21, -3, -2, 6, 17, -7, 5,
	        -11, 4, 7, 18, 11, 23, 4, 17,
	        3, 7, 12, 16, 18, 13, 29, 15,
	        -4, 1, 2, 7, 12, 4, 10, 2,
	        -14, -9, -7, 5, -1, 4, -4, 5,
	        -54, 0, -23, -7, 1, 3, 2, -48,
	};
	int eg_knight_psqt[64] = {
	        -36, -14, 13, 1, 2, 14, -12, -48,
	        -3, 10, 2, 28, 23, -6, 8, -4,
	        4, 8, 25, 26, 22, 26, 10, -0,
	        7, 2, 22, 32, 31, 23, 17, 10,
	        -7, 3, 19, 24, 24, 22, 6, 8,
	        -33, -15, -6, 9, 9, -6, -8, -28,
	        -21, -11, -25, -11, -8, -21, -6, -2,
	        -47, -34, -21, -11, -12, -19, -19, -19,
	};
	
	int mg_bishop_psqt[64] = {
	        -17, -45, -85, -70, -73, -70, -4, -26,
	        -43, -21, -15, -28, -29, -7, -30, -27,
	        -10, -3, 17, 4, 14, 17, 12, -7,
	        -17, 7, -2, 17, 1, 7, 3, -9,
	        -6, -1, 3, 2, 9, 1, 1, 3,
	        0, 5, 2, 7, 5, 5, 7, 12,
	        15, 8, 10, -0, 1, 6, 22, 14,
	        13, 12, 0, -5, -4, 1, 4, 17,
	};
	int eg_bishop_psqt[64] = {
	        11, 16, 17, 20, 15, 14, 5, 10,
	        14, 10, 4, 10, 9, 4, 6, 9,
	        6, 10, 3, 3, 3, 15, 11, 11,
	        6, 5, 3, 9, 12, 5, 10, 9,
	        -7, -4, 3, 6, 5, 1, -2, -5,
	        -9, -6, -2, -1, 1, -3, -10, 0,
	        -16, -17, -15, -8, -7, -20, -13, -26,
	        -11, -5, -8, -11, -8, -7, -3, -3,
	};
	
	int mg_rook_psqt[64] = {
	        23, 15, -15, -6, -3, 23, 36, 32,
	        -9, -17, -3, 10, 7, 26, 3, 14,
	        -13, 20, 12, 32, 44, 48, 74, 17,
	        -8, 4, 9, 25, 17, 22, 24, 13,
	        -16, -17, -17, -8, -9, -9, 4, -5,
	        -20, -15, -18, -12, -12, -12, 9, -11,
	        -34, -14, -12, -9, -8, -2, 2, -37,
	        -13, -13, -11, -5, -5, -3, 1, -3,
	};
	int eg_rook_psqt[64] = {
	        19, 25, 33, 29, 29, 28, 24, 24,
	        11, 16, 15, 16, 17, 6, 13, 7,
	        21, 13, 20, 11, 7, 16, -2, 13,
	        12, 12, 14, 9, 11, 8, 4, 7,
	        -3, 7, 8, 3, 1, 4, 1, -5,
	        -18, -8, -11, -15, -13, -12, -16, -19,
	        -20, -23, -20, -23, -23, -26, -29, -15,
	        -19, -16, -14, -20, -18, -12, -19, -28,
	};
	
	int mg_queen_psqt[64] = {
	        -6, 11, -9, 3, -4, 28, 47, 39,
	        -19, -51, -23, -47, -50, 8, -37, 14,
	        -20, -14, -25, -17, -7, -2, 15, -22,
	        -7, -8, -21, -31, -28, -16, 10, 8,
	        0, -2, -8, -16, -16, -2, 11, 12,
	        -3, 7, -0, -3, -2, -1, 15, 11,
	        1, 8, 11, 6, 7, 15, 24, 17,
	        12, -0, 5, 10, 7, -7, 4, 7,
	};
	int eg_queen_psqt[64] = {
	        3, -3, 21, 21, 34, 29, 13, 10,
	        5, 19, 10, 39, 66, 54, 55, 34,
	        2, 3, 20, 19, 39, 57, 57, 66,
	        -7, 14, 10, 40, 51, 54, 50, 36,
	        -17, -3, 0, 28, 28, 17, 6, 15,
	        -28, -20, -7, -14, -10, -5, -25, -22,
	        -37, -38, -50, -29, -31, -65, -71, -54,
	        -51, -45, -56, -42, -48, -46, -64, -51,
	};
	
	int mg_king_psqt[64] = {
	        -2, 2, 4, 5, 2, 2, 3, -7,
	        -3, 5, 30, 16, 10, 20, 15, -1,
	        -6, 31, 46, 16, 36, 62, 64, 2,
	        -8, 44, 24, -4, -3, 58, 53, -7,
	        1, 35, 51, -28, 1, 25, 29, -29,
	        -21, -15, 10, -15, -3, -12, -30, -29,
	        -8, -48, -30, -59, -39, -53, -40, -6,
	        20, 2, 12, -34, 13, -41, 5, 49,
	};
	int eg_king_psqt[64] = {
	        -78, -28, -28, -13, -24, -18, -17, -77,
	        -33, 28, 21, 11, 15, 24, 39, -35,
	        -4, 32, 28, 21, 17, 33, 35, -7,
	        -4, 18, 27, 27, 28, 26, 22, -8,
	        -28, 3, 11, 26, 22, 17, 12, -18,
	        -21, 2, 2, 11, 9, 10, 12, -14,
	        -13, 8, 6, 8, 3, 14, 13, -14,
	        -56, -21, -21, -26, -50, -11, -23, -74,
	};
	
	int mg_knight_mobility[9] = { -67, -21, -12, -6, 2, 3, 8, 15, 23, };
	int eg_knight_mobility[9] = { -49, -77, -29, -1, 6, 19, 23, 23, 15, };
	
	int mg_bishop_mobility[14] = { -53, -29, -12, -11, -3, 2, 4, 5, 5, 8, 10, 22, 38, 52, };
	int eg_bishop_mobility[14] = { -104, -103, -50, -22, -12, 1, 12, 18, 26, 28, 29, 22, 24, 10, };
	
	int mg_rook_mobility[15] = { -7, -45, -14, -11, -8, -7, -8, -9, -4, 1, 6, 8, 13, 21, 62, };
	int eg_rook_mobility[15] = { -11, -92, -73, -43, -26, -10, 1, 10, 13, 16, 21, 24, 26, 23, 2, };
	
	int mg_queen_mobility[28] = { 0, -0, -6, -43, -40, -9, 0, -2, -1, -0, 1, 1, 3, 6, 5, 6, 6, 2, 1, -1, 8, 9, 7, 16, -8, -17, -8, -3, };
	int eg_queen_mobility[28] = { 0, -0, -3, -20, -41, -86, -75, -41, -24, -10, -1, 11, 17, 20, 25, 27, 27, 29, 28, 24, 15, 10, 1, -12, -10, -17, -27, -10, };
	
	int mg_king_mobility[9] = { 2, -9, -58, -29, -7, 16, -1, 19, 42, };
	int eg_king_mobility[9] = { 6, 23, 28, 19, 8, 1, -2, -5, -15, };
	
	int mg_isolated_pawn = -3;
	int eg_isolated_pawn = -9;
	
	int mg_doubled_pawn = -5;
	int eg_doubled_pawn = -15;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 2;
	
	int mg_backward_pawn_half_open = -6;
	int eg_backward_pawn_half_open = -18;
	
	int mg_chained_pawn[8] = { 0, 130, 28, 15, 11, 11, 1, 0, };
	int eg_chained_pawn[8] = { 0, 55, 48, 17, 7, 5, -2, 0, };
	
	int mg_passed_pawn[8] = { 0, 78, 36, 7, -31, -5, 1, 0, };
	int eg_passed_pawn[8] = { 0, 77, 46, 34, 27, 9, 1, 0, };
	
	int mg_passed_pawn_blocked[8] = { 0, 57, 30, 2, -37, -8, -8, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 6, -11, 4, 9, 5, -6, 0, };
	
	int mg_passed_pawn_safe_advance = -2;
	int eg_passed_pawn_safe_advance = 14;
	
	int mg_passed_pawn_safe_path = -62;
	int eg_passed_pawn_safe_path = 32;
	
	int mg_passed_friendly_distance[8] = { 0, -2, 1, 8, 10, 4, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -11, -14, -15, -11, -5, -0, 0, };
	
	int mg_passed_enemy_distance[8] = { 0, 5, -2, -5, -2, -3, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 37, 32, 19, 8, 1, -0, 0, };
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -15;
	
	int mg_knight_outpost_supported = 31;
	int eg_knight_outpost_supported = 13;
	
	int mg_double_bishop = 9;
	int eg_double_bishop = 56;
	
	int mg_rook_open_file = 22;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 8;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = -1;
	int eg_rook_on_seventh = 21;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 26, 31, 35, 28, 12, 16, -21, 0, },
	        { 41, 23, 6, 8, 2, 17, 20, 0, },
	        { 39, 30, 13, 12, 17, 38, 25, 0, },
	        { 12, 20, 5, 11, 7, -30, -14, 0, },
	},
	{
	        { 0, 48, 51, 21, 1, 11, -5, 0, },
	        { 0, 41, 38, 19, 2, -4, -39, 0, },
	        { 0, 38, 20, 30, 24, -12, -1, 0, },
	        { 0, 17, 12, 17, 7, -36, -19, 0, },
	},
	};
	int eg_pawn_shelter[2][4][8] = { {
	        { -15, -22, -7, -1, 6, 27, 33, 0, },
	        { -6, -10, -1, -11, -7, 20, 42, 0, },
	        { 0, -0, 1, -17, -20, -15, 16, 0, },
	        { 9, -2, -3, -7, -18, -0, 15, 0, },
	},
	{
	        { 0, -26, -15, -2, 20, 43, -3, 0, },
	        { 0, -10, -8, -13, -5, 31, -21, 0, },
	        { 0, 1, 6, -8, -5, 18, 21, 0, },
	        { 0, 12, 12, -0, -10, 10, -10, 0, },
	},
	};
	
	int mg_king_attacker_weight[6] = { 0, 9, 14, 23, 34, 0, };
	int eg_king_attacker_weight[6] = { 0, 11, 45, 17, 34, 0, };
	
	int mg_king_zone_attack_count_weight = 130;
	int eg_king_zone_attack_count_weight = 36;
	
	int mg_king_danger_no_queen_weight = -417;
	int eg_king_danger_no_queen_weight = -165;
	
	int mg_safe_knight_check = 339;
	int eg_safe_knight_check = 53;
	
	int mg_safe_bishop_check = 119;
	int eg_safe_bishop_check = 47;
	
	int mg_safe_rook_check = 212;
	int eg_safe_rook_check = 94;
	
	int mg_safe_queen_check = 202;
	int eg_safe_queen_check = 133;
	
	int mg_king_danger_offset = -16;
	int eg_king_danger_offset = 26;
	
	int mg_center_control = 2;
	int eg_center_control = -1;
	
	int mg_minor_threatened_by_pawn = -20;
	int eg_minor_threatened_by_pawn = -42;
	
	int mg_minor_threatened_by_minor = -15;
	int eg_minor_threatened_by_minor = -35;
	
	int mg_rook_threatened_by_lesser = -18;
	int eg_rook_threatened_by_lesser = -10;
	
	int mg_queen_threatened_by_lesser = -14;
	int eg_queen_threatened_by_lesser = -5;
	
	int mg_minor_threatened_by_major = -14;
	int eg_minor_threatened_by_major = -16;

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
