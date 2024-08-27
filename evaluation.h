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

	int mg_piece_value[6] = { 48, 221, 240, 310, 702, 0, };
	int eg_piece_value[6] = { 104, 351, 368, 623, 1133, 0, };
	
	int mg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        17, 4, 19, 53, 54, 33, -33, -45,
	        -1, 4, 12, 24, 25, 41, 14, -9,
	        -2, -5, -4, 5, 11, 17, -3, -12,
	        -5, -6, 0, -0, 5, 6, 0, -19,
	        -10, -11, -6, -3, 5, -7, -3, -20,
	        -1, -0, -5, -2, -0, 7, 13, -5,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        -6, 16, 20, 13, 16, 13, 24, 5,
	        21, 19, 13, 3, 7, 12, 17, 15,
	        11, 3, -4, -17, -13, -7, 0, 2,
	        4, -0, -12, -14, -15, -11, -7, -5,
	        -1, -4, -7, -6, -4, -6, -9, -6,
	        1, -1, 3, 3, 8, 4, -2, -8,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_knight_psqt[64] = {
	        -92, -20, -58, -9, 9, -30, -5, -57,
	        -23, -35, -7, -4, 5, 19, -28, -5,
	        -41, -22, -5, -3, 4, 18, -8, 5,
	        -11, 2, 6, 16, 10, 21, 1, 15,
	        2, 5, 10, 15, 17, 11, 25, 13,
	        -5, -1, 1, 6, 10, 2, 8, -0,
	        -16, -11, -8, 3, -3, 2, -5, 4,
	        -47, -1, -25, -8, -0, 2, 0, -46,
	};
	int eg_knight_psqt[64] = {
	        -42, -15, 9, 1, 4, 12, -9, -43,
	        -3, 9, 1, 26, 22, -8, 6, -5,
	        3, 7, 24, 25, 22, 26, 9, -1,
	        7, 1, 21, 30, 31, 22, 17, 10,
	        -5, 4, 20, 24, 24, 23, 7, 10,
	        -32, -14, -4, 10, 9, -5, -8, -27,
	        -19, -10, -24, -10, -8, -20, -5, -1,
	        -47, -32, -20, -10, -12, -20, -18, -19,
	};
	
	int mg_bishop_psqt[64] = {
	        -16, -31, -61, -55, -54, -56, -3, -21,
	        -43, -22, -15, -28, -29, -8, -31, -28,
	        -11, -4, 16, 3, 13, 16, 11, -8,
	        -18, 6, -3, 16, 0, 6, 2, -10,
	        -7, -2, 2, 0, 8, -0, -0, 2,
	        -1, 4, 0, 6, 4, 4, 6, 11,
	        14, 7, 8, -2, -1, 5, 20, 13,
	        12, 10, -1, -7, -6, 0, 3, 15,
	};
	int eg_bishop_psqt[64] = {
	        10, 14, 12, 18, 12, 11, 5, 8,
	        14, 10, 4, 10, 9, 4, 7, 9,
	        7, 11, 3, 4, 3, 15, 11, 11,
	        6, 6, 4, 9, 13, 5, 10, 9,
	        -7, -3, 3, 6, 5, 2, -2, -5,
	        -8, -6, -2, -1, 1, -3, -10, 1,
	        -16, -17, -15, -8, -7, -19, -13, -26,
	        -11, -5, -8, -11, -8, -9, -3, -4,
	};
	
	int mg_rook_psqt[64] = {
	        24, 16, -14, -5, -1, 18, 31, 33,
	        -10, -18, -3, 9, 6, 21, 1, 13,
	        -13, 20, 12, 32, 44, 48, 73, 16,
	        -8, 4, 9, 25, 17, 21, 24, 13,
	        -17, -18, -17, -9, -10, -10, 4, -5,
	        -20, -15, -19, -13, -13, -12, 9, -12,
	        -34, -14, -13, -10, -9, -3, 1, -37,
	        -14, -14, -12, -6, -6, -4, 1, -4,
	};
	int eg_rook_psqt[64] = {
	        18, 24, 32, 28, 28, 29, 24, 23,
	        11, 16, 15, 16, 17, 7, 13, 7,
	        20, 13, 20, 11, 6, 16, -2, 13,
	        12, 12, 14, 9, 10, 8, 4, 6,
	        -3, 7, 8, 3, 1, 4, 1, -6,
	        -18, -8, -10, -14, -13, -12, -15, -19,
	        -19, -23, -20, -23, -23, -26, -28, -15,
	        -18, -16, -14, -20, -19, -12, -19, -27,
	};
	
	int mg_queen_psqt[64] = {
	        -5, 9, -5, 2, -6, 23, 38, 36,
	        -19, -51, -24, -46, -51, 6, -27, 16,
	        -18, -12, -21, -17, -8, -1, 13, -26,
	        -4, -7, -21, -30, -28, -17, 10, 6,
	        1, -2, -8, -16, -16, -2, 10, 12,
	        -4, 6, -1, -4, -2, -1, 14, 10,
	        1, 7, 11, 6, 7, 14, 24, 13,
	        12, -1, 5, 10, 7, -8, -3, -1,
	};
	int eg_queen_psqt[64] = {
	        -0, -3, 18, 21, 34, 32, 23, 13,
	        2, 17, 11, 40, 68, 53, 49, 35,
	        -6, -5, 8, 17, 40, 56, 58, 65,
	        -14, 10, 7, 37, 50, 55, 50, 35,
	        -20, -3, 0, 27, 24, 15, 5, 13,
	        -30, -19, -7, -13, -9, -6, -26, -23,
	        -37, -37, -50, -27, -28, -63, -70, -49,
	        -53, -47, -56, -41, -47, -44, -52, -41,
	};
	
	int mg_king_psqt[64] = {
	        -1, 0, 2, 2, 1, 1, 1, -5,
	        -3, 5, 18, 9, 7, 13, 10, -2,
	        -4, 22, 30, 11, 23, 39, 41, -0,
	        -5, 30, 20, 2, 2, 47, 44, -9,
	        -2, 31, 48, -17, 9, 30, 37, -32,
	        -18, -4, 18, -8, 4, -5, -18, -29,
	        -7, -35, -21, -52, -32, -45, -27, -5,
	        5, 4, 9, -37, 12, -44, 6, 35,
	};
	int eg_king_psqt[64] = {
	        -47, -20, -21, -9, -18, -14, -13, -49,
	        -30, 27, 21, 10, 14, 24, 37, -34,
	        -4, 31, 29, 20, 18, 34, 36, -6,
	        -4, 18, 26, 25, 27, 26, 23, -7,
	        -27, 3, 11, 24, 20, 16, 9, -17,
	        -22, -1, -1, 9, 7, 8, 9, -15,
	        -14, 4, 4, 5, 1, 11, 9, -15,
	        -50, -22, -20, -25, -50, -10, -24, -69,
	};
	
	int mg_knight_mobility[9] = { -44, -24, -15, -8, -1, 0, 6, 12, 21, };
	int eg_knight_mobility[9] = { -31, -78, -29, -1, 7, 20, 23, 23, 15, };
	
	int mg_bishop_mobility[14] = { -55, -31, -14, -13, -4, 0, 2, 3, 3, 6, 8, 21, 37, 49, };
	int eg_bishop_mobility[14] = { -75, -101, -49, -22, -12, 1, 12, 18, 25, 27, 28, 21, 23, 9, };
	
	int mg_rook_mobility[15] = { -4, -43, -15, -12, -9, -8, -9, -10, -5, -0, 5, 8, 12, 21, 61, };
	int eg_rook_mobility[15] = { -6, -78, -72, -42, -25, -10, 2, 10, 13, 16, 21, 24, 25, 22, 1, };
	
	int mg_queen_mobility[28] = { 0, -0, -3, -26, -42, -15, -1, -2, -2, -1, -0, 0, 2, 4, 4, 5, 5, 1, 0, -1, 9, 9, 8, 13, -5, -12, -5, -0, };
	int eg_queen_mobility[28] = { 0, -0, -2, -12, -29, -63, -73, -42, -25, -11, -3, 10, 16, 17, 22, 23, 23, 25, 24, 20, 11, 7, -2, -10, -10, -16, -16, -3, };
	
	int mg_king_mobility[9] = { 1, -4, -41, -15, -6, 15, -9, 9, 31, };
	int eg_king_mobility[9] = { 2, 11, 21, 14, 7, 1, 0, -3, -11, };
	
	int mg_isolated_pawn = -3;
	int eg_isolated_pawn = -9;
	
	int mg_doubled_pawn = -6;
	int eg_doubled_pawn = -15;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 2;
	
	int mg_backward_pawn_half_open = -6;
	int eg_backward_pawn_half_open = -18;
	
	int mg_chained_pawn[8] = { 0, 92, 28, 15, 11, 11, 1, 0, };
	int eg_chained_pawn[8] = { 0, 63, 47, 17, 7, 5, -2, 0, };
	
	int mg_passed_pawn[8] = { 0, 52, 28, 8, -27, -1, 4, 0, };
	int eg_passed_pawn[8] = { 0, 78, 46, 33, 26, 8, 1, 0, };
	
	int mg_passed_pawn_blocked[8] = { 0, 34, 22, 4, -33, -3, -6, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 7, -11, 2, 7, 4, -6, 0, };
	
	int mg_passed_pawn_safe_advance = -2;
	int eg_passed_pawn_safe_advance = 15;
	
	int mg_passed_pawn_safe_path = -65;
	int eg_passed_pawn_safe_path = 32;
	
	int mg_passed_friendly_distance[8] = { 0, 4, 3, 8, 10, 4, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -11, -15, -15, -11, -5, -0, 0, };
	
	int mg_passed_enemy_distance[8] = { 0, 5, -3, -6, -3, -4, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 37, 32, 19, 8, 1, -0, 0, };
	
	int mg_knight_outpost = 17;
	int eg_knight_outpost = -13;
	
	int mg_knight_outpost_supported = 31;
	int eg_knight_outpost_supported = 14;
	
	int mg_double_bishop = 8;
	int eg_double_bishop = 57;
	
	int mg_rook_open_file = 22;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 8;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = 0;
	int eg_rook_on_seventh = 21;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 25, 29, 33, 26, 10, 14, -12, 0, },
	        { 41, 23, 6, 7, 2, 15, 13, 0, },
	        { 38, 28, 12, 11, 16, 36, 21, 0, },
	        { 12, 20, 5, 12, 6, -28, -7, 0, },
	},
	{
	        { 0, 45, 48, 18, -1, 12, -3, 0, },
	        { 0, 40, 36, 17, 1, -2, -29, 0, },
	        { 0, 37, 20, 29, 22, -6, -0, 0, },
	        { 0, 16, 12, 17, 7, -27, -10, 0, },
	},
	};
	int eg_pawn_shelter[2][4][8] = { {
	        { -14, -21, -6, -0, 7, 28, 23, 0, },
	        { -6, -10, -1, -11, -7, 20, 33, 0, },
	        { -1, -0, 1, -17, -19, -14, 16, 0, },
	        { 8, -2, -2, -7, -18, -2, 10, 0, },
	},
	{
	        { 0, -27, -15, -1, 21, 39, -2, 0, },
	        { 0, -11, -8, -12, -4, 29, -18, 0, },
	        { 0, 0, 5, -9, -5, 15, 12, 0, },
	        { 0, 11, 11, -0, -10, 4, -7, 0, },
	},
	};
	
	int mg_safe_knight_check = -43;
	int eg_safe_knight_check = 2;
	
	int mg_safe_bishop_check = -4;
	int eg_safe_bishop_check = -9;
	
	int mg_safe_rook_check = -24;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -19;
	int eg_safe_queen_check = -25;
	
	int mg_king_attacker_weight[6] = { 0, 9, 10, 18, 31, 0, };
	int eg_king_attacker_weight[6] = { 0, -17, 19, -11, 100, 0, };
	
	int mg_king_zone_attack_count_weight = 139;
	int eg_king_zone_attack_count_weight = 75;
	
	int mg_king_danger_no_queen_weight = -348;
	int eg_king_danger_no_queen_weight = -148;
	
	int mg_king_danger_offset = -22;
	int eg_king_danger_offset = 69;
	
	int mg_center_control = 2;
	int eg_center_control = -1;
	
	int mg_minor_threatened_by_pawn = -20;
	int eg_minor_threatened_by_pawn = -42;
	
	int mg_minor_threatened_by_minor = -15;
	int eg_minor_threatened_by_minor = -35;
	
	int mg_rook_threatened_by_lesser = -17;
	int eg_rook_threatened_by_lesser = -10;
	
	int mg_queen_threatened_by_lesser = -13;
	int eg_queen_threatened_by_lesser = -6;
	
	int mg_minor_threatened_by_major = -14;
	int eg_minor_threatened_by_major = -15;

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
