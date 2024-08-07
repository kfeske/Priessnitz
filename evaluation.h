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
	uint64_t passed_pawns[2];

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
		passed_pawns[WHITE] = passed_pawns[BLACK] = 0ULL;
		attacked[WHITE] = attacked_by_piece[WHITE][KING] = piece_attacks(KING, white_king_square, 0ULL);
		attacked[BLACK] = attacked_by_piece[BLACK][KING] = piece_attacks(KING, black_king_square, 0ULL);
	}
};

struct Evaluation : Noncopyable
{
	Eval_info info {};

	int mg_piece_value[6] = { 41, 257, 273, 315, 691, 0, };
	int eg_piece_value[6] = { 61, 292, 299, 508, 949, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		9, 24, 7, 69, 53, 47, -15, -42,
		12, 7, 28, 36, 38, 67, 44, 5,
		-8, -5, 6, 8, 23, 33, 6, 12,
		-10, -10, 6, 16, 18, 22, 7, 3,
		-16, -10, -2, 3, 10, 1, 10, 0,
		-7, -3, 2, -5, 8, 19, 26, -4,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-86, -68, -64, -30, -15, -45, -56, -49,
		-39, -26, -25, -13, -22, 11, -21, -12,
		-24, -17, -12, -13, 10, 29, -2, -2,
		-18, -6, 2, 27, 15, 25, 1, 15,
		-4, -2, 5, 14, 15, 15, 18, 7,
		-19, -12, -9, -9, 5, -5, 9, -2,
		-22, -19, -15, 0, -1, -1, 3, 1,
		-50, -14, -30, -17, -11, -5, -11, -21,
	};
	int mg_bishop_psqt[64] = {
		-28, -52, -46, -72, -68, -67, -34, -51,
		-26, -32, -23, -36, -32, -27, -46, -36,
		-13, 0, -9, -5, -9, 22, 8, -2,
		-22, -12, -11, -1, -5, -5, -10, -24,
		-13, -25, -12, -1, -4, -6, -15, 1,
		-11, 1, -2, -3, 1, 0, 5, 9,
		11, 2, 9, -6, 1, 9, 23, 14,
		2, 12, 1, -11, 1, -7, 9, 26,
	};
	int mg_rook_psqt[64] = {
		5, -6, -8, -11, 3, 13, 10, 19,
		-17, -20, -13, 3, -10, 13, 14, 25,
		-30, 0, -10, -6, 25, 20, 60, 24,
		-26, -11, -14, -10, -3, 2, 13, 7,
		-30, -33, -24, -21, -17, -27, 4, -10,
		-30, -26, -23, -21, -12, -9, 23, 3,
		-28, -24, -14, -13, -6, -4, 18, -17,
		-14, -16, -16, -8, -1, -2, 1, -6,
	};
	int mg_queen_psqt[64] = {
		-27, -29, -13, 8, -6, -12, 15, -7,
		-6, -28, -32, -44, -49, -7, -16, 33,
		2, -7, -15, -11, -3, 12, 31, 37,
		-12, -10, -19, -30, -25, -10, 2, 10,
		-2, -21, -20, -20, -19, -13, 4, 11,
		-5, -1, -13, -13, -9, -1, 17, 13,
		1, 1, 6, 7, 7, 10, 24, 25,
		-4, -12, -5, 6, 2, -8, 1, 4,
	};
	int mg_king_psqt[64] = {
		-9, -8, -8, -20, -7, -14, 3, 12,
		-42, -8, -16, 30, 20, 14, 25, 17,
		-46, 21, -1, -14, 19, 57, 35, 6,
		-20, -17, -36, -57, -59, -24, -31, -70,
		-35, -20, -32, -78, -72, -30, -48, -95,
		-22, 22, -16, -40, -33, -29, -8, -49,
		31, 0, 2, -27, -35, -17, 6, 12,
		27, 32, 23, -50, 5, -38, 20, 45,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		15, 15, 37, 19, 27, 24, 39, 36,
		32, 38, 22, 11, 17, 20, 39, 36,
		26, 19, 11, 1, 3, 6, 15, 14,
		16, 13, 6, 1, 1, 4, 7, 6,
		11, 8, 5, 6, 7, 7, 2, 2,
		13, 11, 9, 10, 13, 10, 3, 2,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-45, -17, -4, -10, -3, -24, -19, -60,
		-5, -2, 1, 3, -2, -9, -4, -16,
		-4, -1, 4, 10, 3, -7, -8, -11,
		5, 0, 6, 5, 8, 5, 7, -1,
		5, 2, 11, 12, 14, 7, 1, 6,
		-4, -3, -1, 8, 8, -3, -4, -2,
		-5, -3, -6, -4, -5, -7, -8, 7,
		-3, -6, -7, -7, -4, -11, -1, -2,
	};
	int eg_bishop_psqt[64] = {
		0, 4, 0, 3, 3, -5, 0, -8,
		-9, -1, -4, 1, -4, -2, -2, -9,
		5, -1, 1, -4, 0, 3, 1, 5,
		2, 4, 2, 13, 3, 4, 0, 4,
		-1, 1, 4, 5, 5, -1, 1, -9,
		-3, 1, 0, 3, 5, 1, -5, -5,
		0, -11, -10, -5, -2, -6, -6, -8,
		-9, -5, -4, -4, -7, 1, -9, -17,
	};
	int eg_rook_psqt[64] = {
		12, 17, 20, 16, 13, 14, 18, 13,
		5, 12, 14, 6, 7, 7, 7, -1,
		15, 14, 14, 10, 4, 4, 2, -1,
		14, 12, 13, 10, 2, 2, 3, -2,
		5, 7, 4, 3, 1, 1, -5, -7,
		-2, -2, -4, -4, -6, -10, -19, -19,
		-7, -2, -5, -7, -10, -11, -18, -15,
		-4, -5, -4, -8, -11, -8, -9, -11,
	};
	int eg_queen_psqt[64] = {
		5, 5, 12, 11, 14, 21, -1, 8,
		-11, -3, 17, 25, 35, 19, 9, 9,
		-9, -5, 11, 13, 20, 16, -2, -2,
		1, 8, 9, 21, 20, 10, 19, 5,
		-3, 11, 12, 18, 15, 9, 6, 4,
		-12, -2, 9, 6, 10, 1, -7, -8,
		-12, -12, -12, -6, -4, -19, -33, -31,
		-12, -10, -10, -11, -14, -17, -20, -16,
	};
	int eg_king_psqt[64] = {
		-44, -17, -5, 11, 4, 13, 21, -35,
		1, 20, 25, 17, 23, 34, 35, 19,
		9, 21, 26, 32, 36, 34, 32, 15,
		-1, 12, 24, 29, 32, 30, 24, 13,
		-11, 0, 13, 22, 21, 13, 7, 2,
		-19, -10, 1, 8, 7, 3, -6, -9,
		-23, -11, -6, -1, 2, -2, -11, -18,
		-41, -25, -13, -3, -14, -4, -22, -47,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 65, 86, 80, 44, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -38, -4, -11, 102, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 12, 25, 37, 49, 47, 25, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 19, 29, 44, 26, -4, 0, };
	
	int mg_safe_knight_check = -90;
	int eg_safe_knight_check = -1;
	
	int mg_safe_bishop_check = -25;
	int eg_safe_bishop_check = -17;
	
	int mg_safe_rook_check = -86;
	int eg_safe_rook_check = -1;
	
	int mg_safe_queen_check = -38;
	int eg_safe_queen_check = -17;
	
	int mg_pawn_shield[6] = { 0, 32, 33, 23, 26, 12, };
	
	int mg_knight_mobility[9] = { -120, -53, -32, -23, -13, -10, 0, 10, 23, };
	int eg_knight_mobility[9] = { -99, -46, -22, -9, -2, 4, 6, 8, 7, };
	
	int mg_bishop_mobility[14] = { -42, -56, -31, -24, -12, -5, -1, 4, 7, 10, 12, 24, 25, 33, };
	int eg_bishop_mobility[14] = { -94, -59, -26, -13, -8, -5, 0, 4, 7, 9, 11, 7, 9, 4, };
	
	int mg_rook_mobility[15] = { -14, -47, -25, -19, -14, -13, -11, -15, -11, -7, -3, -4, -3, 4, 9, };
	int eg_rook_mobility[15] = { -32, -62, -28, -19, -16, -7, -4, 0, 2, 4, 6, 10, 13, 13, 13, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -50, -50, -18, -12, -11, -10, -10, -7, -5, -3, 0, 1, 1, 1, -3, -1, 2, 8, 17, 30, 27, 21, 37, 0, -12, };
	int eg_queen_mobility[28] = { 0, 0, -13, -63, -45, -39, -30, -20, -11, 1, 2, 5, 8, 8, 9, 12, 11, 15, 14, 11, 9, 0, -6, -6, -7, -10, -18, -18, };
	
	int mg_king_mobility[9] = { 0, -27, -20, -3, -10, 20, -34, -12, 13, };
	int eg_king_mobility[9] = { -1, 9, -7, 1, 6, -5, 7, 7, -1, };
	
	int mg_minor_threatened_by_pawn = -41;
	int eg_minor_threatened_by_pawn = -11;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 0;
	
	int mg_queen_threatened_by_lesser = -12;
	int eg_queen_threatened_by_lesser = -6;
	
	int mg_passed_pawn[8] = { 0, 41, 19, 4, -21, -9, 2, 0, };
	int eg_passed_pawn[8] = { 0, 91, 63, 34, 21, 12, 8, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 34, 30, 2, -25, -9, -2, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 16, 4, 7, 4, 8, 5, 0, };
	
	int mg_passed_pawn_safe_advance = -17;
	int eg_passed_pawn_safe_advance = 23;
	
	int mg_passed_friendly_distance[8] = { 0, 0, 6, 12, 13, 5, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -16, -13, -8, -3, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 3, -12, -9, -7, -1, 1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 34, 30, 16, 7, -1, -2, 0, };
	
	int mg_isolated_penalty = -8;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = -6;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 9;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 14;
	int eg_double_bishop = 39;
	
	int mg_rook_open_file = 29;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 12;
	int eg_rook_half_open_file = 5;
	
	int mg_rook_on_seventh = -7;
	int eg_rook_on_seventh = 18;
	
	int mg_knight_outpost = 19;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 18;
	
	int mg_center_control = 382;
	
	int pawn_count_scale_offset = 68;
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

	void evaluate_passed_pawns(Board &board, Color friendly);
	
	void evaluate_threats(Board &board, Color friendly);

	void evaluate_center_control(Color friendly);

	void note_king_attacks(Piece_type type, uint64_t attacks, Color friendly);

	int scale_factor(Board &board, int eg_value);

	int evaluate(Board &board);
};
