#pragma once

#include "utility.h"
#include "board.h"

struct Eval_info : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	int king_ring_pressure[2];
	int king_ring_attackers[2];

	uint64_t attacked[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		king_ring_pressure[WHITE] = king_ring_pressure[BLACK] = 0;
		king_ring_attackers[WHITE] = king_ring_attackers[BLACK] = 0;
		unsigned white_king_square = board.square(WHITE, KING);
		unsigned black_king_square = board.square(BLACK, KING);
		king_ring[WHITE] = king_ring_mask(white_king_square);
		king_ring[BLACK] = king_ring_mask(black_king_square);
		attacked[WHITE] = 0ULL;
		attacked[BLACK] = 0ULL;
	}
};

struct Evaluation : Noncopyable
{
	Eval_info info {};

	int mg_piece_value[6] = { 40, 259, 272, 316, 681, 0, };
	int eg_piece_value[6] = { 60, 287, 296, 498, 956, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 
		11, 23, 5, 64, 45, 45, -18, -43, 
		13, 6, 26, 36, 35, 67, 42, 7, 
		-7, -6, 6, 5, 22, 32, 6, 13, 
		-9, -10, 5, 17, 18, 24, 7, 4, 
		-16, -11, -1, 3, 11, 2, 10, 1, 
		-6, -4, 2, -3, 8, 19, 25, -5, 
		0, 0, 0, 0, 0, 0, 0, 0, 
	};
	int mg_knight_psqt[64] = {
		-90, -69, -56, -26, -11, -40, -50, -46, 
		-39, -27, -25, -9, -19, 18, -20, -6, 
		-25, -19, -7, -11, 17, 37, 4, 3, 
		-19, -5, 2, 29, 19, 35, 8, 21, 
		-5, -3, 3, 13, 14, 15, 23, 8, 
		-20, -14, -10, -11, 4, -7, 8, -2, 
		-22, -20, -16, 0, -2, 0, 4, 1, 
		-51, -16, -32, -19, -13, -7, -13, -23, 
	};
	int mg_bishop_psqt[64] = {
		-28, -54, -51, -80, -73, -69, -37, -54, 
		-26, -34, -26, -38, -34, -29, -51, -41, 
		-14, 1, -10, -5, -10, 22, 6, 0, 
		-22, -13, -9, -1, -5, -3, -10, -24, 
		-13, -25, -12, 0, -4, -6, -15, 1, 
		-10, 1, -2, -2, 3, 0, 5, 10, 
		11, 2, 9, -5, 0, 11, 25, 15, 
		3, 12, 1, -11, 1, -8, 10, 27, 
	};
	int mg_rook_psqt[64] = {
		4, -10, -14, -16, 0, 13, 11, 28, 
		-16, -20, -15, 1, -11, 12, 17, 35, 
		-28, -1, -13, -8, 23, 22, 68, 35, 
		-26, -13, -16, -14, -4, 4, 18, 11, 
		-32, -34, -26, -23, -16, -24, 7, -7, 
		-31, -27, -23, -23, -12, -7, 27, 5, 
		-30, -25, -16, -14, -7, -1, 21, -16, 
		-15, -17, -17, -10, -2, -2, 3, -3, 
	};
	int mg_queen_psqt[64] = {
		-31, -38, -20, 0, -11, -13, 12, -13, 
		0, -28, -40, -53, -59, -19, -16, 36, 
		3, -9, -15, -22, -17, 3, 21, 22, 
		-12, -9, -21, -37, -32, -21, -5, 0, 
		-1, -20, -19, -17, -19, -16, 2, 8, 
		-5, 0, -9, -10, -4, 1, 17, 13, 
		1, 3, 8, 11, 10, 15, 28, 26, 
		-3, -11, -4, 9, 5, -6, 4, 7, 
	};
	int mg_king_psqt[64] = {
		-23, -18, -17, -38, -19, -7, 21, 18, 
		-44, -11, -27, 18, 15, 16, 40, 30, 
		-62, 12, -27, -41, -7, 42, 45, -1, 
		-39, -30, -61, -99, -92, -41, -36, -73, 
		-47, -35, -59, -113, -103, -51, -52, -106, 
		-26, 25, -25, -55, -47, -39, -5, -53, 
		31, 10, 9, -22, -29, -10, 21, 12, 
		29, 34, 25, -53, 8, -39, 26, 50, 
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 
		18, 18, 40, 22, 30, 26, 43, 40, 
		31, 39, 23, 10, 16, 21, 39, 37, 
		26, 20, 10, 1, 3, 6, 16, 15, 
		16, 13, 6, 0, 1, 3, 7, 6, 
		11, 8, 5, 6, 7, 7, 3, 2, 
		13, 12, 9, 10, 13, 10, 5, 3, 
		0, 0, 0, 0, 0, 0, 0, 0, 
	};
	int eg_knight_psqt[64] = {
		-45, -17, -5, -12, -6, -27, -20, -61, 
		-4, -3, 0, 0, -5, -12, -6, -17, 
		-4, -2, 4, 7, -2, -8, -9, -12, 
		5, 0, 5, 6, 5, 4, 2, -3, 
		5, 1, 11, 12, 14, 6, 1, 6, 
		-4, -3, -1, 8, 7, -3, -4, -2, 
		-5, -2, -5, -3, -4, -5, -5, 7, 
		-5, -6, -6, -6, -2, -7, -1, -3, 
	};
	int eg_bishop_psqt[64] = {
		-1, 2, -1, 2, 1, -7, -2, -10, 
		-9, -2, -5, 0, -6, -3, -3, -8, 
		5, -3, 0, -6, -2, 0, -1, 3, 
		1, 3, 0, 11, 1, 2, -1, 3, 
		-2, 0, 4, 4, 4, -2, 0, -10, 
		-4, 1, 0, 4, 6, 2, -5, -5, 
		0, -11, -9, -4, 0, -5, -5, -8, 
		-10, -4, -3, -2, -6, 3, -8, -16, 
	};
	int eg_rook_psqt[64] = {
		11, 16, 18, 14, 10, 11, 16, 11, 
		2, 8, 10, 3, 4, 2, 2, -5, 
		14, 13, 13, 10, 3, 2, -2, -4, 
		14, 12, 13, 10, 1, 1, 1, -2, 
		6, 7, 4, 3, 1, 0, -6, -6, 
		-1, -1, -4, -4, -7, -11, -19, -18, 
		-6, -2, -5, -6, -10, -12, -18, -13, 
		-3, -4, -2, -7, -10, -7, -9, -11, 
	};
	int eg_queen_psqt[64] = {
		4, 12, 20, 17, 20, 27, 8, 14, 
		-12, 2, 23, 35, 47, 27, 22, 22, 
		-8, -2, 16, 24, 32, 34, 14, 15, 
		1, 5, 7, 23, 27, 23, 29, 17, 
		-4, 6, 5, 9, 13, 11, 7, 8, 
		-14, -8, -1, -1, 1, -3, -9, -8, 
		-14, -17, -18, -13, -10, -25, -38, -33, 
		-15, -13, -14, -16, -18, -20, -25, -21, 
	};
	int eg_king_psqt[64] = {
		-49, -18, -9, 7, 2, 12, 24, -39, 
		0, 22, 25, 18, 24, 35, 37, 19, 
		7, 23, 26, 32, 36, 34, 32, 13, 
		-2, 14, 25, 30, 32, 30, 25, 10, 
		-13, 3, 14, 24, 23, 15, 9, 1, 
		-19, -8, 3, 10, 9, 5, -3, -8, 
		-21, -8, -5, 0, 3, -1, -9, -15, 
		-46, -25, -15, -8, -18, -8, -23, -51, 
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 25, 68, 52, 107, 0, };
	int mg_king_ring_pressure_weight[8] = { 0, 12, 27, 41, 56, 54, 23, 0, };
	int mg_pawn_shield[6] = { 0, 35, 41, 27, 31, 12, };
	
	int mg_knight_mobility[9] = { -121, -54, -33, -23, -12, -10, 1, 11, 24, };
	int eg_knight_mobility[9] = { -100, -46, -24, -11, -3, 3, 5, 8, 7, };
	
	int mg_bishop_mobility[14] = { -44, -58, -32, -25, -12, -5, 0, 5, 8, 11, 12, 25, 27, 35, };
	int eg_bishop_mobility[14] = { -96, -60, -28, -15, -10, -6, -1, 3, 7, 9, 11, 8, 10, 6, };
	
	int mg_rook_mobility[15] = { -14, -45, -24, -19, -14, -13, -11, -16, -12, -8, -4, -4, -2, 7, 11, };
	int eg_rook_mobility[15] = { -32, -65, -32, -22, -19, -8, -5, 0, 1, 3, 6, 10, 13, 13, 13, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -58, -52, -19, -12, -10, -9, -10, -6, -4, -2, 1, 1, 0, -1, -6, -6, -6, -1, 6, 23, 25, 28, 58, 32, 19, };
	int eg_queen_mobility[28] = { 0, 0, -13, -72, -55, -50, -42, -32, -22, -10, -7, -3, 1, 2, 5, 10, 11, 19, 22, 25, 29, 26, 24, 29, 32, 35, 35, 32, };
	
	int mg_king_mobility[9] = { 0, -21, -12, 4, -4, 25, -41, -22, 0, };
	int eg_king_mobility[9] = { -2, 23, 3, 6, 7, -6, 6, 5, -4, };
	
	int mg_minor_threatened_by_pawn = -41;
	int eg_minor_threatened_by_pawn = -10;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -44;
	int eg_rook_threatened_by_lesser = 1;
	
	int mg_queen_threatened_by_lesser = -13;
	int eg_queen_threatened_by_lesser = -9;
	
	int mg_passed_bonus[8] = { 0, 38, 15, 0, -21, -4, 9, 0, };
	int eg_passed_bonus[8] = { 0, 92, 70, 41, 29, 23, 24, 0, };
	int mg_passed_bonus_blocked[8] = { 0, 27, 23, -5, -29, -6, 3, 0, };
	int eg_passed_bonus_blocked[8] = { 0, 29, 18, 21, 18, 23, 23, 0, };
	
	int mg_passed_friendly_distance[8] = { 0, 0, 7, 12, 13, 5, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -16, -13, -8, -3, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 0, -14, -12, -9, -4, -2, 0, };
	int eg_passed_enemy_distance[8] = { 0, 34, 30, 17, 8, 0, -2, 0, };
	
	int mg_isolated_penalty = -9;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = -5;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -7;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 8;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 15;
	int eg_double_bishop = 39;
	
	int mg_rook_open_file = 30;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 11;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = -7;
	int eg_rook_on_seventh = 19;
	
	int mg_knight_outpost = 20;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 17;
	
	int mg_center_control = 434;
	
	int pawn_count_scale_offset = 72;
	int pawn_count_scale_weight = 28;

	int tempo_bonus = 19;

	int taper_start = 6377;
	int taper_end = 321;

	void evaluate_pawns(  Board &board, Color friendly);
	void evaluate_knights(Board &board, Color friendly);
	void evaluate_bishops(Board &board, Color friendly);
	void evaluate_rooks(  Board &board, Color friendly);
	void evaluate_queens( Board &board, Color friendly);
	void evaluate_kings(  Board &board, Color friendly);
	
	void evaluate_threats(Board &board, Color friendly);

	void evaluate_center_control(Color friendly);

	void note_king_attacks(Piece_type type, uint64_t attacks, Color friendly);

	int scale_factor(Board &board, int eg_value);

	int evaluate(Board &board);
};
