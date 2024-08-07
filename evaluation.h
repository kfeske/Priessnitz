#pragma once

#include "utility.h"
#include "board.h"

struct Eval_info : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t ring[2];
	uint64_t zone[2];
	int ring_pressure[2];
	int zone_pressure[2];
	int ring_attackers[2];
	int zone_attackers[2];

	uint64_t attacked[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		ring_pressure[WHITE] = ring_pressure[BLACK] = 0;
		zone_pressure[WHITE] = zone_pressure[BLACK] = 0;
		ring_attackers[WHITE] = ring_attackers[BLACK] = 0;
		zone_attackers[WHITE] = zone_attackers[BLACK] = 0;
		unsigned white_king_square = board.square(WHITE, KING);
		unsigned black_king_square = board.square(BLACK, KING);
		ring[WHITE] = piece_attacks(KING, white_king_square, 0ULL);
		ring[BLACK] = piece_attacks(KING, black_king_square, 0ULL);
		zone[WHITE] = king_zone(WHITE, white_king_square);
		zone[BLACK] = king_zone(BLACK, black_king_square);
		attacked[WHITE] = 0ULL;
		attacked[BLACK] = 0ULL;
	}
};

struct Evaluation : Noncopyable
{
	Eval_info info {};

	int mg_piece_value[6] = { 42, 257, 272, 314, 680, 0, };
	int eg_piece_value[6] = { 60, 286, 294, 499, 954, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		8, 27, 6, 63, 46, 41, -14, -46,
		11, 6, 29, 36, 37, 68, 42, 5,
		-9, -4, 7, 9, 27, 32, 9, 12,
		-12, -10, 5, 18, 20, 24, 9, 2,
		-17, -11, -2, 4, 13, 4, 11, 1,
		-9, -5, 0, -3, 7, 20, 24, -7,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-89, -67, -49, -21, -8, -43, -49, -46,
		-36, -27, -7, 1, -20, 30, -26, -4,
		-29, -10, -5, -6, 27, 31, 7, -13,
		-20, -6, 2, 25, 14, 32, 6, 17,
		-3, -2, 4, 10, 13, 14, 20, 6,
		-22, -13, -10, -12, 4, -8, 7, -7,
		-21, -19, -14, 1, -3, 1, 1, -2,
		-49, -15, -30, -18, -11, -6, -12, -21,
	};
	int mg_bishop_psqt[64] = {
		-28, -56, -50, -81, -72, -70, -36, -54,
		-22, -9, -22, -34, -15, -27, -32, -45,
		-13, 2, 2, 4, -9, 20, -1, -7,
		-21, -4, -3, 9, 3, -4, -3, -30,
		-8, -21, -13, 6, 0, -10, -18, 7,
		-7, 1, -3, -4, -1, -1, 1, 11,
		11, 2, 8, -9, -3, 9, 24, 12,
		2, 10, -4, -17, -4, -9, 9, 27,
	};
	int mg_rook_psqt[64] = {
		8, -5, -6, -9, 7, 18, 16, 34,
		-15, -18, -10, 7, -6, 15, 18, 38,
		-26, 0, -10, -9, 19, 17, 61, 31,
		-27, -14, -17, -17, -13, -7, 6, 3,
		-33, -37, -29, -27, -24, -35, -6, -17,
		-30, -26, -23, -21, -10, -10, 18, -2,
		-28, -25, -13, -11, -5, -3, 16, -18,
		-13, -15, -14, -6, 1, -1, 0, -3,
	};
	int mg_queen_psqt[64] = {
		-29, -37, -19, 1, -9, -12, 15, -10,
		2, -26, -38, -51, -58, -18, -15, 36,
		5, -7, -12, -20, -17, 3, 18, 17,
		-12, -8, -19, -33, -32, -23, -13, -7,
		-1, -18, -18, -15, -21, -21, -6, -1,
		-3, 3, -7, -10, -5, -4, 10, 7,
		4, 4, 11, 12, 11, 15, 27, 23,
		1, -8, -2, 12, 7, -3, 6, 8,
	};
	int mg_king_psqt[64] = {
		-21, -14, -15, -36, -18, -4, 23, 20,
		-41, -10, -26, 19, 15, 16, 42, 33,
		-60, 13, -26, -40, -7, 42, 47, 1,
		-39, -30, -60, -99, -92, -43, -38, -73,
		-47, -35, -57, -110, -99, -47, -53, -106,
		-28, 23, -26, -52, -41, -38, -6, -56,
		28, 10, 10, -16, -22, -5, 20, 8,
		27, 34, 23, -51, 11, -36, 24, 45,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		20, 18, 40, 22, 30, 27, 43, 41,
		32, 39, 22, 10, 16, 21, 40, 38,
		27, 19, 10, -1, 1, 5, 15, 16,
		17, 13, 6, -1, 0, 3, 6, 7,
		12, 8, 5, 5, 5, 6, 3, 3,
		14, 11, 9, 9, 13, 10, 5, 4,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-44, -15, -4, -12, -5, -25, -19, -59,
		-5, -4, -3, -2, -7, -15, -6, -17,
		-3, -3, 4, 5, -4, -8, -11, -10,
		4, -1, 4, 5, 4, 4, 2, -3,
		6, 1, 11, 11, 13, 6, 1, 6,
		-3, -2, -1, 8, 7, -4, -4, -1,
		-4, -2, -4, -3, -5, -5, -5, 8,
		-3, -5, -5, -5, -1, -6, 0, -1,
	};
	int eg_bishop_psqt[64] = {
		-2, 2, -1, 4, 3, -6, -3, -11,
		-10, -6, -4, 0, -8, -3, -5, -7,
		5, -2, -1, -6, -1, 2, 0, 5,
		2, 3, -1, 8, -1, 3, -2, 6,
		-1, 0, 4, 2, 2, -1, 1, -11,
		-4, 1, 1, 3, 5, 2, -5, -5,
		0, -10, -9, -4, 0, -5, -4, -8,
		-11, -5, -3, -1, -4, 3, -8, -18,
	};
	int eg_rook_psqt[64] = {
		14, 18, 17, 13, 10, 11, 19, 14,
		5, 11, 9, 2, 3, 3, 5, -1,
		13, 12, 10, 7, 1, 1, -1, -4,
		14, 12, 11, 8, 1, 1, 4, 0,
		7, 8, 3, 2, 0, 1, -3, -4,
		-1, -2, -5, -6, -9, -12, -17, -16,
		-4, 0, -6, -7, -11, -12, -16, -11,
		-3, -3, -3, -8, -10, -7, -7, -10,
	};
	int eg_queen_psqt[64] = {
		5, 14, 19, 18, 20, 28, 10, 16,
		-11, 2, 22, 34, 47, 26, 24, 24,
		-9, -3, 12, 22, 31, 33, 16, 19,
		1, 4, 5, 20, 26, 25, 34, 22,
		-4, 5, 3, 7, 13, 14, 13, 14,
		-15, -10, -3, -3, 1, 0, -5, -4,
		-15, -19, -20, -14, -12, -26, -37, -31,
		-16, -14, -16, -19, -20, -21, -25, -20,
	};
	int eg_king_psqt[64] = {
		-47, -14, -5, 11, 5, 16, 28, -36,
		3, 25, 27, 17, 24, 37, 40, 22,
		10, 24, 26, 29, 33, 34, 34, 16,
		0, 14, 22, 23, 25, 27, 25, 12,
		-10, 2, 11, 18, 17, 12, 8, 4,
		-17, -7, 2, 7, 7, 4, -3, -5,
		-18, -7, -5, -1, 2, -1, -8, -13,
		-44, -24, -14, -7, -17, -7, -21, -49,
	};
	
	int ring_attack_potency[6] = { 0, 28, 65, 40, 108, 0, };
	int zone_attack_potency[6] = { 0, -11, 4, 81, 58, 0, };
	
	int ring_pressure_weight[8] = { 0, 12, 26, 39, 52, 51, 20, 0, };
	int zone_pressure_weight[8] = { 0, -5, 0, 5, 9, 9, 10, 3, };
	int mg_pawn_shield[6] = { 0, 35, 40, 27, 31, 13, };
	
	int mg_knight_mobility[9] = { -118, -51, -32, -23, -12, -10, 0, 10, 22, };
	int eg_knight_mobility[9] = { -99, -47, -24, -11, -4, 3, 5, 7, 7, };
	
	int mg_bishop_mobility[14] = { -46, -58, -31, -25, -12, -5, 0, 5, 7, 10, 12, 24, 24, 31, };
	int eg_bishop_mobility[14] = { -94, -59, -27, -14, -9, -6, -1, 3, 6, 9, 10, 7, 9, 4, };
	
	int mg_rook_mobility[15] = { -14, -50, -23, -18, -12, -13, -10, -16, -12, -9, -6, -5, -3, 7, 11, };
	int eg_rook_mobility[15] = { -32, -64, -32, -22, -19, -8, -5, 0, 1, 4, 6, 10, 13, 13, 13, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -61, -51, -21, -13, -11, -9, -9, -6, -4, -2, 0, 1, 0, -1, -5, -5, -5, 0, 8, 25, 27, 28, 56, 31, 15, };
	int eg_queen_mobility[28] = { 0, 0, -13, -72, -56, -48, -41, -32, -22, -9, -7, -2, 2, 3, 5, 10, 12, 19, 21, 24, 27, 24, 22, 25, 29, 31, 30, 26, };
	
	int mg_king_mobility[9] = { 0, -21, -13, 3, -5, 23, -37, -19, 0, };
	int eg_king_mobility[9] = { -2, 23, 4, 7, 8, -5, 6, 4, -5, };
	
	int mg_passed_bonus[8] = { 0, 43, 15, 1, -20, -3, 10, 0, };
	int eg_passed_bonus[8] = { 0, 93, 71, 41, 29, 23, 24, 0, };
	int mg_passed_bonus_blocked[8] = { 0, 23, 21, -5, -29, -4, 4, 0, };
	int eg_passed_bonus_blocked[8] = { 0, 30, 19, 22, 19, 24, 23, 0, };
	
	int mg_passed_friendly_distance[8] = { 0, 1, 7, 12, 13, 4, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -16, -13, -8, -3, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 0, -14, -12, -9, -4, -2, 0, };
	int eg_passed_enemy_distance[8] = { 0, 34, 30, 17, 8, 0, -2, 0, };
	
	int mg_isolated_penalty = -9;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = -4;
	int eg_doubled_penalty = -11;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 8;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 17;
	int eg_double_bishop = 41;
	
	int mg_rook_open_file = 30;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 9;
	int eg_rook_half_open_file = 8;
	
	int mg_rook_on_seventh = -5;
	int eg_rook_on_seventh = 18;
	
	int mg_knight_outpost = 28;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 40;
	int eg_knight_outpost_supported = 18;
	
	int mg_center_control = 274;
	
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
	void evaluate_center_control(Color friendly);

	void note_king_attacks(Piece_type type, uint64_t attacks, Color friendly);

	int scale_factor(Board &board, int eg_value);

	int evaluate(Board &board);
};
