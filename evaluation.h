#pragma once

#include "utility.h"
#include "board.h"

struct Eval_info : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	int mg_king_ring_pressure[2];
	int eg_king_ring_pressure[2];
	int king_ring_attackers[2];

	uint64_t attacked[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		mg_king_ring_pressure[WHITE] = mg_king_ring_pressure[BLACK] = 0;
		eg_king_ring_pressure[WHITE] = eg_king_ring_pressure[BLACK] = 0;
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

	int mg_piece_value[6] = { 40, 257, 271, 315, 689, 0, };
	int eg_piece_value[6] = { 61, 290, 297, 502, 949, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		11, 23, 5, 64, 46, 45, -18, -43,
		13, 6, 26, 36, 35, 67, 43, 7,
		-7, -6, 6, 5, 22, 32, 6, 13,
		-10, -10, 5, 17, 18, 24, 8, 4,
		-16, -11, -1, 3, 11, 2, 11, 2,
		-7, -4, 2, -4, 7, 20, 25, -5,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-91, -70, -56, -28, -12, -42, -51, -48,
		-39, -27, -26, -12, -23, 14, -21, -7,
		-25, -19, -8, -15, 11, 32, -2, -1,
		-19, -5, 1, 28, 15, 30, 1, 18,
		-4, -3, 3, 13, 14, 14, 22, 8,
		-20, -13, -8, -10, 5, -6, 9, -2,
		-22, -19, -15, 1, -1, 1, 4, 1,
		-50, -15, -31, -19, -12, -6, -12, -22,
	};
	int mg_bishop_psqt[64] = {
		-28, -54, -51, -80, -74, -70, -37, -54,
		-25, -33, -26, -38, -34, -30, -51, -38,
		-14, 1, -10, -5, -11, 21, 4, -2,
		-22, -13, -10, -2, -6, -5, -10, -25,
		-13, -25, -13, -1, -4, -6, -15, 1,
		-10, 1, -2, -3, 3, 1, 5, 10,
		11, 2, 9, -5, 1, 12, 25, 15,
		3, 12, 0, -11, 1, -7, 10, 27,
	};
	int mg_rook_psqt[64] = {
		2, -11, -16, -18, -3, 12, 11, 26,
		-15, -20, -16, 0, -12, 10, 15, 33,
		-29, -2, -14, -10, 21, 18, 64, 31,
		-26, -12, -16, -14, -5, 1, 15, 9,
		-31, -34, -26, -22, -17, -26, 5, -9,
		-31, -26, -23, -23, -12, -8, 25, 4,
		-29, -24, -15, -13, -7, -1, 21, -16,
		-14, -16, -16, -9, -1, -2, 3, -3,
	};
	int mg_queen_psqt[64] = {
		-26, -34, -15, 4, -6, -12, 14, -6,
		3, -27, -36, -48, -52, -8, -7, 45,
		2, -9, -16, -16, -7, 14, 34, 39,
		-14, -12, -20, -33, -28, -12, 1, 10,
		-4, -20, -20, -18, -19, -14, 5, 12,
		-6, -2, -11, -12, -7, -1, 17, 14,
		0, 0, 5, 7, 6, 13, 26, 26,
		-4, -13, -7, 6, 2, -7, 2, 7,
	};
	int mg_king_psqt[64] = {
		-24, -18, -17, -37, -19, -8, 19, 18,
		-44, -11, -25, 19, 16, 16, 40, 27,
		-63, 12, -26, -40, -5, 43, 44, -1,
		-38, -29, -60, -99, -92, -41, -37, -74,
		-46, -34, -59, -113, -104, -50, -51, -106,
		-26, 25, -25, -55, -47, -37, -4, -53,
		31, 10, 9, -22, -29, -10, 20, 12,
		29, 34, 24, -52, 8, -39, 25, 50,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		18, 18, 40, 22, 30, 26, 43, 40,
		32, 39, 23, 10, 17, 21, 40, 37,
		26, 20, 10, 1, 3, 6, 16, 15,
		16, 13, 6, 0, 1, 3, 7, 6,
		11, 8, 5, 6, 7, 7, 3, 2,
		13, 12, 9, 10, 14, 10, 5, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-45, -17, -4, -10, -3, -24, -18, -60,
		-5, -3, 1, 3, -2, -9, -5, -16,
		-5, -2, 5, 10, 3, -4, -6, -10,
		4, 0, 6, 8, 8, 7, 7, 0,
		5, 1, 11, 12, 15, 7, 2, 6,
		-5, -4, -3, 7, 7, -5, -5, -3,
		-6, -3, -6, -5, -6, -7, -6, 6,
		-6, -7, -7, -7, -3, -9, -2, -4,
	};
	int eg_bishop_psqt[64] = {
		-1, 2, -1, 3, 2, -6, -2, -9,
		-9, -2, -4, 0, -5, -3, -3, -8,
		5, -3, 0, -5, -1, 1, 0, 5,
		1, 4, 0, 11, 2, 3, -1, 4,
		-1, 0, 4, 4, 5, -1, 1, -10,
		-4, 1, 0, 4, 6, 1, -5, -5,
		1, -11, -9, -4, 0, -5, -5, -8,
		-9, -4, -3, -3, -6, 3, -8, -17,
	};
	int eg_rook_psqt[64] = {
		12, 17, 20, 16, 13, 13, 17, 12,
		3, 9, 12, 4, 5, 4, 3, -3,
		14, 14, 14, 11, 4, 5, 0, -2,
		14, 12, 13, 11, 2, 2, 3, -1,
		6, 7, 4, 3, 1, 1, -5, -6,
		-2, -2, -4, -4, -6, -11, -19, -18,
		-7, -3, -6, -7, -10, -12, -18, -14,
		-4, -5, -3, -7, -10, -7, -9, -12,
	};
	int eg_queen_psqt[64] = {
		-2, 5, 11, 6, 7, 16, 0, 3,
		-13, 1, 19, 30, 38, 14, 13, 12,
		-6, 0, 17, 19, 23, 19, 1, -3,
		6, 10, 8, 22, 23, 15, 23, 7,
		1, 8, 7, 12, 14, 9, 6, 4,
		-12, -4, 4, 3, 7, 0, -8, -9,
		-11, -13, -13, -7, -4, -21, -34, -32,
		-11, -9, -8, -10, -12, -17, -22, -19,
	};
	int eg_king_psqt[64] = {
		-49, -18, -9, 7, 2, 12, 23, -39,
		0, 21, 25, 17, 23, 35, 37, 19,
		7, 22, 26, 31, 35, 34, 32, 13,
		-2, 14, 24, 29, 32, 29, 25, 10,
		-13, 2, 13, 23, 22, 14, 9, 2,
		-19, -8, 2, 9, 8, 4, -3, -7,
		-21, -8, -5, 0, 3, -1, -8, -15,
		-46, -25, -15, -7, -18, -7, -22, -51,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 63, 83, 82, 46, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -36, -9, -17, 98, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 12, 25, 38, 49, 46, 24, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 10, 20, 29, 42, 24, -5, 0, };
	
	int mg_pawn_shield[6] = { 0, 35, 41, 27, 31, 12, };
	
	int mg_knight_mobility[9] = { -120, -53, -33, -23, -12, -11, 0, 11, 24, };
	int eg_knight_mobility[9] = { -100, -46, -24, -10, -3, 4, 6, 9, 9, };
	
	int mg_bishop_mobility[14] = { -44, -57, -32, -25, -12, -5, -1, 4, 7, 10, 12, 25, 26, 34, };
	int eg_bishop_mobility[14] = { -96, -60, -28, -15, -10, -6, 0, 4, 8, 10, 12, 9, 11, 7, };
	
	int mg_rook_mobility[15] = { -14, -45, -24, -19, -13, -13, -11, -16, -12, -8, -4, -5, -3, 6, 10, };
	int eg_rook_mobility[15] = { -31, -65, -32, -22, -19, -8, -5, 0, 1, 4, 6, 11, 14, 14, 15, };
	
	int mg_queen_mobility[28] = { 0, 0, -19, -57, -52, -20, -14, -12, -11, -11, -7, -5, -2, 1, 1, 1, 0, -4, -3, -3, 3, 11, 27, 29, 29, 57, 29, 15, };
	int eg_queen_mobility[28] = { 0, 0, -13, -72, -53, -47, -39, -30, -20, -7, -5, -1, 2, 3, 5, 9, 10, 17, 19, 21, 24, 20, 18, 21, 25, 27, 28, 23, };
	
	int mg_king_mobility[9] = { 0, -21, -13, 4, -5, 24, -40, -21, 1, };
	int eg_king_mobility[9] = { -2, 23, 4, 6, 7, -6, 6, 5, -4, };
	
	int mg_minor_threatened_by_pawn = -41;
	int eg_minor_threatened_by_pawn = -10;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -43;
	int eg_rook_threatened_by_lesser = 0;
	
	int mg_queen_threatened_by_lesser = -13;
	int eg_queen_threatened_by_lesser = -9;
	
	int mg_passed_bonus[8] = { 0, 39, 16, 1, -22, -4, 9, 0, };
	int eg_passed_bonus[8] = { 0, 92, 70, 41, 29, 23, 23, 0, };
	int mg_passed_bonus_blocked[8] = { 0, 27, 23, -5, -30, -6, 2, 0, };
	int eg_passed_bonus_blocked[8] = { 0, 29, 18, 21, 18, 23, 22, 0, };
	
	int mg_passed_friendly_distance[8] = { 0, 0, 6, 12, 13, 5, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -16, -13, -8, -3, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 0, -14, -11, -9, -4, -2, 0, };
	int eg_passed_enemy_distance[8] = { 0, 34, 30, 17, 8, 0, -2, 0, };
	
	int mg_isolated_penalty = -9;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = -5;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -7;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 8;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 14;
	int eg_double_bishop = 40;
	
	int mg_rook_open_file = 30;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 11;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = -10;
	int eg_rook_on_seventh = 20;
	
	int mg_knight_outpost = 20;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 18;
	
	int mg_center_control = 437;
	
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
