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

	int mg_piece_value[6] = { 49, 224, 243, 316, 699, 0, };
	int eg_piece_value[6] = { 104, 352, 370, 625, 1152, 0, };
	
	int mg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        17, 4, 19, 53, 56, 34, -33, -46,
	        -1, 4, 12, 24, 25, 41, 14, -9,
	        -2, -5, -3, 5, 11, 17, -3, -12,
	        -5, -5, 0, 0, 5, 6, 0, -19,
	        -10, -11, -6, -2, 6, -7, -4, -20,
	        -1, -0, -5, -2, -0, 7, 13, -5,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_pawn_psqt[64] = {
	        0, 0, 0, 0, 0, 0, 0, 0,
	        -6, 16, 20, 13, 15, 13, 24, 6,
	        21, 19, 13, 2, 7, 12, 17, 15,
	        11, 3, -4, -17, -14, -8, 0, 2,
	        4, -0, -12, -14, -15, -11, -7, -5,
	        -1, -4, -7, -6, -4, -6, -9, -6,
	        2, -0, 3, 3, 9, 5, -1, -8,
	        0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_knight_psqt[64] = {
	        -91, -20, -56, -9, 8, -31, -5, -58,
	        -23, -35, -7, -3, 5, 18, -28, -7,
	        -40, -22, -3, -2, 6, 18, -7, 5,
	        -11, 3, 6, 17, 11, 22, 3, 16,
	        2, 6, 11, 15, 17, 12, 28, 14,
	        -5, -0, 1, 6, 11, 3, 9, 0,
	        -16, -10, -8, 4, -2, 3, -5, 4,
	        -47, -1, -24, -8, 0, 2, 1, -46,
	};
	int eg_knight_psqt[64] = {
	        -43, -15, 9, 0, 3, 11, -10, -45,
	        -3, 9, 2, 27, 22, -7, 6, -5,
	        3, 7, 24, 25, 21, 25, 9, -1,
	        7, 2, 22, 31, 31, 23, 17, 10,
	        -6, 4, 20, 24, 24, 23, 7, 9,
	        -32, -14, -5, 10, 9, -5, -8, -27,
	        -20, -11, -24, -11, -8, -21, -5, -1,
	        -47, -33, -20, -10, -12, -18, -18, -19,
	};
	
	int mg_bishop_psqt[64] = {
	        -14, -31, -62, -55, -55, -57, -2, -19,
	        -43, -22, -15, -28, -29, -8, -30, -28,
	        -11, -4, 16, 3, 13, 17, 12, -8,
	        -18, 6, -2, 16, 1, 6, 3, -9,
	        -7, -2, 2, 1, 8, 0, 0, 2,
	        -1, 4, 1, 6, 4, 5, 6, 11,
	        14, 8, 9, -1, 0, 6, 21, 13,
	        12, 11, -1, -6, -5, 1, 4, 16,
	};
	int eg_bishop_psqt[64] = {
	        10, 13, 12, 18, 12, 11, 5, 8,
	        14, 10, 4, 10, 9, 4, 7, 9,
	        7, 11, 3, 4, 4, 15, 11, 11,
	        6, 6, 4, 9, 13, 5, 11, 10,
	        -7, -3, 3, 6, 5, 2, -2, -5,
	        -8, -6, -2, -0, 1, -3, -10, 1,
	        -15, -17, -15, -7, -6, -19, -12, -26,
	        -11, -5, -7, -11, -8, -7, -3, -4,
	};
	
	int mg_rook_psqt[64] = {
	        24, 15, -14, -5, -2, 18, 29, 29,
	        -10, -18, -4, 9, 6, 24, 1, 12,
	        -13, 20, 12, 32, 44, 49, 73, 17,
	        -8, 4, 9, 25, 17, 22, 24, 13,
	        -16, -17, -17, -8, -9, -9, 4, -4,
	        -20, -15, -18, -12, -12, -11, 9, -11,
	        -34, -14, -13, -9, -8, -2, 2, -37,
	        -13, -13, -11, -5, -5, -3, 1, -3,
	};
	int eg_rook_psqt[64] = {
	        19, 25, 33, 29, 28, 29, 25, 25,
	        12, 16, 15, 16, 17, 6, 13, 7,
	        21, 13, 20, 11, 6, 16, -2, 13,
	        12, 12, 14, 9, 11, 8, 4, 7,
	        -3, 7, 8, 3, 1, 4, 1, -5,
	        -18, -8, -11, -15, -13, -12, -15, -19,
	        -19, -23, -20, -23, -23, -26, -29, -15,
	        -19, -16, -14, -20, -18, -12, -19, -28,
	};
	
	int mg_queen_psqt[64] = {
	        -6, 9, -5, 5, 1, 25, 37, 33,
	        -19, -51, -22, -44, -48, 10, -32, 13,
	        -19, -14, -24, -17, -8, -1, 15, -22,
	        -7, -8, -21, -31, -28, -16, 10, 7,
	        0, -2, -8, -16, -16, -2, 10, 12,
	        -4, 7, -0, -3, -1, -1, 14, 10,
	        1, 7, 11, 6, 8, 15, 24, 13,
	        12, -1, 5, 10, 7, -7, -3, -1,
	};
	int eg_queen_psqt[64] = {
	        3, -1, 18, 19, 31, 31, 20, 15,
	        4, 18, 9, 37, 64, 52, 47, 33,
	        1, 2, 19, 19, 39, 56, 57, 66,
	        -7, 13, 9, 39, 51, 54, 50, 37,
	        -17, -3, -0, 28, 27, 17, 6, 15,
	        -28, -20, -8, -14, -10, -5, -24, -20,
	        -37, -38, -50, -29, -31, -65, -70, -46,
	        -51, -45, -55, -41, -47, -45, -53, -38,
	};
	
	int mg_king_psqt[64] = {
	        -2, -0, 1, 2, 0, 0, 1, -5,
	        -3, 5, 18, 10, 7, 12, 10, -3,
	        -4, 21, 30, 11, 23, 40, 42, -1,
	        -6, 31, 20, 2, 2, 48, 45, -10,
	        -2, 32, 48, -17, 9, 32, 37, -32,
	        -18, -4, 19, -6, 6, -3, -19, -29,
	        -7, -36, -21, -50, -31, -44, -29, -5,
	        6, 3, 10, -36, 11, -42, 6, 36,
	};
	int eg_king_psqt[64] = {
	        -48, -21, -21, -9, -19, -14, -13, -50,
	        -30, 28, 21, 11, 15, 24, 38, -33,
	        -4, 32, 29, 20, 18, 35, 36, -6,
	        -4, 18, 26, 25, 27, 26, 23, -7,
	        -28, 3, 11, 23, 20, 15, 9, -17,
	        -22, -0, -1, 9, 7, 8, 9, -15,
	        -14, 5, 4, 5, 1, 12, 10, -15,
	        -51, -22, -20, -25, -50, -11, -24, -70,
	};
	
	int mg_knight_mobility[9] = { -44, -22, -13, -7, 1, 2, 8, 14, 22, };
	int eg_knight_mobility[9] = { -31, -79, -30, -2, 6, 19, 23, 23, 15, };
	
	int mg_bishop_mobility[14] = { -55, -29, -13, -12, -3, 2, 3, 5, 4, 7, 9, 22, 38, 49, };
	int eg_bishop_mobility[14] = { -76, -101, -50, -23, -12, 0, 12, 18, 25, 28, 29, 22, 24, 10, };
	
	int mg_rook_mobility[15] = { -4, -43, -14, -12, -8, -8, -8, -10, -4, 0, 5, 8, 12, 21, 61, };
	int eg_rook_mobility[15] = { -6, -80, -73, -43, -26, -11, 1, 10, 13, 16, 21, 24, 26, 23, 2, };
	
	int mg_queen_mobility[28] = { 0, -0, -3, -26, -41, -14, -0, -2, -1, -1, 1, 1, 3, 5, 5, 5, 5, 1, -0, -2, 7, 8, 5, 10, -8, -14, -6, -2, };
	int eg_queen_mobility[28] = { 0, -0, -2, -12, -28, -63, -73, -43, -26, -12, -3, 10, 16, 18, 23, 25, 25, 28, 27, 23, 15, 10, 1, -9, -11, -18, -19, -6, };
	
	int mg_king_mobility[9] = { 1, -4, -41, -16, -6, 15, -10, 9, 32, };
	int eg_king_mobility[9] = { 3, 12, 22, 15, 8, 1, -0, -3, -12, };
	
	int mg_isolated_pawn = -3;
	int eg_isolated_pawn = -9;
	
	int mg_doubled_pawn = -6;
	int eg_doubled_pawn = -15;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 2;
	
	int mg_backward_pawn_half_open = -6;
	int eg_backward_pawn_half_open = -18;
	
	int mg_chained_pawn[8] = { 0, 94, 28, 15, 11, 11, 1, 0, };
	int eg_chained_pawn[8] = { 0, 63, 48, 17, 7, 5, -2, 0, };
	
	int mg_passed_pawn[8] = { 0, 54, 28, 9, -27, -3, 2, 0, };
	int eg_passed_pawn[8] = { 0, 78, 46, 33, 26, 8, 0, 0, };
	
	int mg_passed_pawn_blocked[8] = { 0, 34, 23, 5, -33, -5, -8, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 7, -11, 2, 8, 4, -7, 0, };
	
	int mg_passed_pawn_safe_advance = -2;
	int eg_passed_pawn_safe_advance = 15;
	
	int mg_passed_pawn_safe_path = -63;
	int eg_passed_pawn_safe_path = 32;
	
	int mg_passed_friendly_distance[8] = { 0, 4, 2, 8, 10, 4, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -15, -15, -11, -5, -0, 0, };
	
	int mg_passed_enemy_distance[8] = { 0, 5, -3, -5, -3, -3, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 37, 32, 19, 9, 1, -0, 0, };
	
	int mg_knight_outpost = 17;
	int eg_knight_outpost = -14;
	
	int mg_knight_outpost_supported = 31;
	int eg_knight_outpost_supported = 14;
	
	int mg_double_bishop = 9;
	int eg_double_bishop = 56;
	
	int mg_rook_open_file = 22;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 9;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = 0;
	int eg_rook_on_seventh = 21;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 24, 29, 33, 26, 10, 14, -12, 0, },
	        { 40, 23, 6, 7, 1, 15, 14, 0, },
	        { 38, 29, 13, 12, 16, 36, 21, 0, },
	        { 12, 20, 5, 12, 7, -28, -6, 0, },
	},
	{
	        { 0, 45, 48, 18, -1, 12, -3, 0, },
	        { 0, 40, 37, 18, 1, -2, -29, 0, },
	        { 0, 38, 20, 29, 22, -6, -0, 0, },
	        { 0, 17, 12, 17, 8, -26, -10, 0, },
	},
	};
	int eg_pawn_shelter[2][4][8] = { {
	        { -14, -21, -6, -0, 7, 28, 24, 0, },
	        { -5, -9, -1, -11, -7, 21, 34, 0, },
	        { 0, 0, 1, -17, -20, -14, 16, 0, },
	        { 9, -2, -3, -7, -18, -2, 10, 0, },
	},
	{
	        { 0, -26, -14, -1, 21, 39, -2, 0, },
	        { 0, -10, -7, -12, -4, 29, -18, 0, },
	        { 0, 1, 6, -8, -5, 15, 12, 0, },
	        { 0, 11, 12, -0, -10, 4, -7, 0, },
	},
	};
	
	int mg_king_attacker_weight[6] = { 0, 9, 13, 23, 39, 0, };
	int eg_king_attacker_weight[6] = { 0, -8, 30, -6, 43, 0, };
	
	int mg_king_zone_attack_count_weight = 126;
	int eg_king_zone_attack_count_weight = 30;
	
	int mg_king_danger_no_queen_weight = -320;
	int eg_king_danger_no_queen_weight = -111;
	
	int mg_safe_knight_check = 308;
	int eg_safe_knight_check = 63;
	
	int mg_safe_bishop_check = 97;
	int eg_safe_bishop_check = 29;
	
	int mg_safe_rook_check = 201;
	int eg_safe_rook_check = 69;
	
	int mg_safe_queen_check = 206;
	int eg_safe_queen_check = 106;
	
	int mg_king_danger_offset = -9;
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
	int eg_queen_threatened_by_lesser = -4;
	
	int mg_minor_threatened_by_major = -15;
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
