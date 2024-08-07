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
		attacked[WHITE] = attacked_by_piece[WHITE][KING] = piece_attacks(KING, white_king_square, 0ULL);
		attacked[BLACK] = attacked_by_piece[BLACK][KING] = piece_attacks(KING, black_king_square, 0ULL);
	}
};

struct Evaluation : Noncopyable
{
	Eval_info info {};

	int mg_piece_value[6] = { 40, 257, 272, 316, 692, 0, };
	int eg_piece_value[6] = { 61, 291, 299, 506, 946, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		10, 25, 7, 66, 53, 47, -14, -41,
		12, 6, 27, 36, 38, 67, 44, 5,
		-8, -5, 6, 8, 23, 33, 5, 12,
		-9, -10, 6, 16, 18, 22, 6, 3,
		-16, -10, -2, 3, 10, 1, 10, 0,
		-7, -3, 2, -4, 8, 19, 25, -4,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-86, -67, -63, -30, -13, -44, -56, -49,
		-39, -26, -24, -12, -22, 12, -20, -12,
		-24, -17, -12, -13, 10, 30, -1, -1,
		-18, -5, 2, 27, 15, 25, 1, 15,
		-4, -2, 5, 13, 15, 15, 18, 7,
		-19, -12, -9, -9, 5, -5, 9, -2,
		-22, -19, -15, 0, -1, -1, 3, 1,
		-49, -14, -30, -17, -11, -5, -11, -21,
	};
	int mg_bishop_psqt[64] = {
		-28, -52, -45, -72, -68, -66, -34, -52,
		-26, -32, -23, -35, -32, -27, -46, -36,
		-13, 0, -8, -4, -9, 22, 8, -2,
		-22, -12, -11, -1, -5, -5, -10, -24,
		-13, -25, -12, -2, -4, -6, -15, 1,
		-11, 1, -2, -3, 1, 0, 5, 9,
		11, 2, 9, -6, 1, 9, 23, 15,
		2, 12, 1, -11, 1, -8, 9, 27,
	};
	int mg_rook_psqt[64] = {
		7, -4, -6, -9, 5, 15, 12, 22,
		-17, -20, -13, 4, -10, 14, 15, 26,
		-29, 1, -10, -6, 25, 20, 61, 25,
		-26, -11, -14, -10, -3, 2, 12, 7,
		-31, -33, -24, -21, -17, -27, 3, -11,
		-30, -26, -23, -21, -12, -9, 23, 2,
		-28, -24, -14, -13, -6, -4, 18, -18,
		-14, -16, -16, -8, -1, -2, 1, -6,
	};
	int mg_queen_psqt[64] = {
		-26, -29, -13, 7, -6, -12, 15, -6,
		-4, -27, -31, -43, -48, -7, -14, 35,
		3, -7, -14, -11, -3, 12, 32, 38,
		-12, -10, -19, -30, -24, -10, 2, 10,
		-2, -21, -21, -20, -19, -13, 4, 11,
		-6, -1, -14, -13, -9, -1, 17, 13,
		1, 1, 6, 7, 6, 10, 23, 25,
		-4, -12, -6, 6, 3, -8, 1, 4,
	};
	int mg_king_psqt[64] = {
		-12, -8, -7, -21, -7, -14, 3, 12,
		-42, -7, -16, 31, 22, 15, 25, 16,
		-47, 21, -2, -14, 19, 58, 34, 4,
		-21, -18, -34, -57, -59, -21, -31, -72,
		-36, -20, -30, -76, -71, -28, -48, -95,
		-22, 22, -14, -39, -32, -27, -8, -49,
		30, 0, 3, -27, -34, -16, 6, 12,
		26, 31, 23, -51, 5, -38, 20, 44,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		18, 18, 40, 23, 30, 27, 43, 39,
		32, 39, 23, 11, 17, 21, 40, 37,
		27, 20, 11, 1, 3, 6, 16, 15,
		16, 13, 6, 0, 1, 4, 7, 6,
		11, 8, 5, 6, 6, 7, 3, 3,
		13, 11, 9, 9, 13, 10, 4, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-45, -17, -5, -10, -4, -25, -19, -61,
		-5, -3, 1, 2, -3, -9, -5, -17,
		-4, -1, 4, 9, 2, -8, -8, -12,
		5, 0, 6, 5, 8, 4, 7, -1,
		5, 2, 12, 13, 14, 7, 1, 6,
		-4, -3, -1, 8, 8, -3, -4, -2,
		-5, -2, -6, -4, -5, -6, -7, 7,
		-4, -6, -7, -7, -3, -10, -1, -2,
	};
	int eg_bishop_psqt[64] = {
		0, 3, 0, 3, 2, -6, 0, -8,
		-9, -1, -4, 1, -5, -2, -2, -8,
		5, -1, 1, -4, 0, 3, 1, 4,
		1, 4, 1, 12, 2, 4, 0, 4,
		-1, 1, 4, 5, 5, -1, 0, -9,
		-3, 1, 0, 4, 6, 1, -5, -5,
		0, -10, -9, -4, -1, -6, -6, -8,
		-9, -5, -3, -4, -7, 1, -9, -16,
	};
	int eg_rook_psqt[64] = {
		10, 15, 18, 14, 12, 11, 16, 11,
		4, 10, 13, 5, 6, 5, 5, -2,
		14, 13, 13, 10, 4, 4, 1, -2,
		14, 12, 13, 10, 1, 2, 3, -2,
		6, 7, 4, 4, 1, 1, -5, -6,
		-1, -1, -4, -4, -6, -10, -19, -18,
		-6, -2, -5, -6, -9, -11, -18, -13,
		-4, -5, -3, -7, -11, -8, -9, -11,
	};
	int eg_queen_psqt[64] = {
		4, 3, 11, 11, 13, 19, -2, 6,
		-13, -4, 16, 24, 34, 18, 7, 7,
		-10, -6, 10, 13, 20, 16, -3, -4,
		0, 8, 9, 22, 19, 10, 19, 4,
		-2, 11, 12, 18, 16, 10, 6, 4,
		-11, -1, 10, 7, 10, 1, -7, -8,
		-11, -12, -11, -6, -4, -18, -32, -30,
		-12, -10, -10, -10, -14, -17, -20, -15,
	};
	int eg_king_psqt[64] = {
		-46, -17, -6, 10, 3, 11, 20, -38,
		1, 21, 25, 17, 22, 34, 35, 18,
		8, 22, 26, 31, 35, 33, 33, 13,
		-2, 14, 24, 29, 31, 29, 25, 12,
		-12, 1, 12, 22, 21, 13, 8, 1,
		-19, -9, 1, 8, 8, 3, -5, -9,
		-23, -9, -5, 0, 3, -1, -10, -18,
		-44, -26, -14, -3, -14, -5, -22, -50,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 65, 86, 80, 44, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -37, -4, -11, 102, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 12, 25, 37, 49, 48, 25, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 19, 29, 44, 27, -4, 0, };
	
	int mg_safe_knight_check = -90;
	int eg_safe_knight_check = -1;
	
	int mg_safe_bishop_check = -25;
	int eg_safe_bishop_check = -17;
	
	int mg_safe_rook_check = -87;
	int eg_safe_rook_check = -1;
	
	int mg_safe_queen_check = -38;
	int eg_safe_queen_check = -17;
	
	int mg_pawn_shield[6] = { 0, 31, 33, 23, 26, 12, };
	
	int mg_knight_mobility[9] = { -120, -53, -32, -22, -13, -10, 0, 10, 23, };
	int eg_knight_mobility[9] = { -98, -46, -22, -9, -2, 4, 6, 8, 8, };
	
	int mg_bishop_mobility[14] = { -42, -56, -31, -24, -12, -5, -1, 4, 7, 10, 11, 24, 24, 31, };
	int eg_bishop_mobility[14] = { -95, -60, -27, -14, -9, -6, 0, 4, 7, 10, 11, 8, 10, 6, };
	
	int mg_rook_mobility[15] = { -14, -48, -24, -19, -14, -13, -11, -15, -11, -8, -4, -4, -3, 5, 10, };
	int eg_rook_mobility[15] = { -31, -61, -28, -19, -17, -7, -4, 0, 2, 4, 6, 10, 13, 13, 12, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -50, -50, -18, -12, -11, -10, -10, -7, -5, -3, 0, 1, 1, 1, -3, -1, 1, 8, 16, 29, 27, 21, 36, 0, -12, };
	int eg_queen_mobility[28] = { 0, 0, -13, -64, -45, -38, -30, -20, -11, 1, 2, 5, 8, 8, 9, 12, 10, 14, 13, 11, 9, 0, -5, -6, -7, -10, -17, -18, };
	
	int mg_king_mobility[9] = { 0, -26, -22, -3, -10, 21, -35, -13, 13, };
	int eg_king_mobility[9] = { -2, 18, -1, 4, 7, -5, 7, 5, -3, };
	
	int mg_minor_threatened_by_pawn = -41;
	int eg_minor_threatened_by_pawn = -11;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -43;
	int eg_rook_threatened_by_lesser = 1;
	
	int mg_queen_threatened_by_lesser = -12;
	int eg_queen_threatened_by_lesser = -6;
	
	int mg_passed_bonus[8] = { 0, 45, 20, 2, -24, -12, 0, 0, };
	int eg_passed_bonus[8] = { 0, 93, 70, 40, 29, 22, 21, 0, };
	int mg_passed_bonus_blocked[8] = { 0, 32, 27, -3, -32, -14, -5, 0, };
	int eg_passed_bonus_blocked[8] = { 0, 28, 17, 20, 17, 22, 20, 0, };
	
	int mg_passed_friendly_distance[8] = { 0, -1, 5, 11, 13, 5, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -16, -13, -8, -3, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 2, -13, -11, -9, -3, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 35, 31, 18, 8, 0, -2, 0, };
	
	int mg_isolated_penalty = -8;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = -5;
	int eg_doubled_penalty = -11;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 9;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 14;
	int eg_double_bishop = 39;
	
	int mg_rook_open_file = 29;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 10;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = -6;
	int eg_rook_on_seventh = 18;
	
	int mg_knight_outpost = 19;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 18;
	
	int mg_center_control = 387;
	
	int pawn_count_scale_offset = 69;
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
