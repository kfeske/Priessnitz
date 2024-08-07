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

	int mg_piece_value[6] = { 41, 259, 271, 317, 682, 0, };
	int eg_piece_value[6] = { 60, 287, 295, 498, 955, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		11, 23, 6, 64, 45, 45, -19, -44,
		13, 7, 27, 36, 36, 65, 42, 6,
		-8, -6, 6, 6, 22, 31, 7, 12,
		-10, -10, 5, 17, 19, 24, 9, 4,
		-16, -11, -1, 3, 12, 4, 11, 2,
		-8, -4, 1, -3, 7, 21, 25, -5,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-89, -68, -54, -25, -12, -42, -50, -45,
		-37, -25, -23, -10, -20, 14, -22, -8,
		-24, -17, -6, -12, 13, 35, 0, 0,
		-17, -4, 3, 28, 17, 32, 8, 20,
		-3, -1, 4, 12, 14, 14, 22, 6,
		-19, -13, -9, -12, 4, -6, 8, -3,
		-21, -18, -16, 0, -3, 0, 3, 0,
		-48, -15, -31, -19, -13, -6, -12, -20,
	};
	int mg_bishop_psqt[64] = {
		-28, -55, -52, -81, -72, -70, -36, -56,
		-25, -32, -26, -40, -36, -32, -54, -43,
		-13, 3, -8, -5, -12, 18, 2, -5,
		-21, -12, -7, 1, -7, -6, -12, -26,
		-11, -23, -11, 0, -4, -6, -16, 0,
		-9, 2, -1, -2, 3, 2, 4, 9,
		12, 3, 9, -5, 1, 12, 26, 13,
		3, 10, -1, -13, -1, -7, 11, 28,
	};
	int mg_rook_psqt[64] = {
		10, -3, -7, -9, 7, 18, 17, 36,
		-14, -17, -12, 5, -6, 14, 20, 40,
		-27, -1, -13, -10, 18, 17, 62, 31,
		-27, -14, -18, -18, -12, -6, 7, 4,
		-33, -36, -29, -27, -23, -33, -4, -16,
		-30, -26, -22, -21, -10, -9, 20, -1,
		-28, -23, -14, -12, -5, -2, 18, -17,
		-13, -15, -15, -7, 0, -2, 1, -3,
	};
	int mg_queen_psqt[64] = {
		-29, -36, -18, 2, -9, -11, 14, -9,
		3, -25, -39, -52, -58, -18, -14, 38,
		5, -7, -14, -22, -18, 2, 18, 18,
		-11, -9, -21, -38, -36, -25, -13, -5,
		0, -19, -19, -18, -23, -22, -6, 0,
		-3, 3, -7, -10, -5, -3, 11, 7,
		3, 5, 11, 12, 12, 15, 29, 24,
		0, -9, -1, 13, 8, -3, 5, 8,
	};
	int mg_king_psqt[64] = {
		-22, -15, -15, -36, -17, -5, 23, 20,
		-42, -10, -26, 19, 15, 16, 41, 32,
		-59, 13, -26, -41, -7, 43, 46, 1,
		-38, -31, -61, -101, -94, -43, -38, -72,
		-46, -35, -58, -112, -102, -49, -52, -105,
		-27, 23, -27, -55, -44, -38, -6, -55,
		29, 11, 9, -18, -23, -6, 21, 9,
		26, 35, 24, -50, 10, -36, 24, 44,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		19, 18, 40, 22, 30, 26, 43, 40,
		32, 39, 22, 10, 16, 21, 40, 37,
		26, 20, 10, 1, 3, 6, 16, 15,
		16, 13, 6, 0, 1, 3, 7, 6,
		11, 8, 5, 6, 6, 6, 3, 2,
		13, 12, 9, 9, 14, 10, 5, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-45, -17, -5, -12, -5, -26, -20, -61,
		-5, -3, 0, 0, -5, -12, -6, -16,
		-4, -2, 4, 7, -1, -8, -9, -11,
		4, 0, 5, 6, 5, 4, 3, -3,
		5, 1, 11, 12, 14, 6, 1, 6,
		-4, -3, -1, 7, 7, -4, -4, -2,
		-5, -2, -5, -3, -5, -6, -6, 7,
		-5, -5, -6, -6, -1, -7, -1, -3,
	};
	int eg_bishop_psqt[64] = {
		-1, 2, -1, 2, 1, -7, -2, -10,
		-9, -2, -4, 0, -5, -3, -2, -7,
		5, -3, -1, -5, -1, 1, 0, 4,
		1, 3, -1, 10, 1, 3, -1, 3,
		-2, 0, 4, 4, 4, -2, 1, -10,
		-4, 1, 0, 4, 5, 1, -5, -5,
		1, -10, -9, -4, -1, -6, -5, -8,
		-10, -4, -3, -2, -5, 3, -8, -17,
	};
	int eg_rook_psqt[64] = {
		10, 15, 17, 13, 10, 11, 15, 10,
		2, 8, 10, 2, 3, 2, 2, -5,
		14, 13, 13, 10, 4, 4, 0, -3,
		14, 12, 14, 11, 3, 3, 4, 0,
		6, 7, 5, 4, 2, 3, -3, -4,
		-1, -2, -4, -4, -7, -10, -17, -16,
		-6, -2, -6, -7, -10, -12, -18, -13,
		-4, -5, -3, -8, -10, -8, -8, -11,
	};
	int eg_queen_psqt[64] = {
		3, 11, 19, 17, 19, 27, 8, 13,
		-13, 0, 22, 34, 47, 26, 22, 21,
		-9, -3, 15, 24, 33, 35, 16, 18,
		0, 5, 7, 24, 30, 26, 34, 20,
		-5, 5, 4, 11, 15, 15, 13, 13,
		-16, -10, -2, -2, 2, 0, -5, -4,
		-16, -19, -20, -14, -13, -26, -38, -32,
		-16, -15, -16, -19, -20, -22, -26, -22,
	};
	int eg_king_psqt[64] = {
		-49, -18, -8, 8, 2, 13, 24, -39,
		0, 22, 25, 18, 24, 35, 37, 19,
		7, 23, 27, 32, 36, 34, 33, 13,
		-3, 14, 25, 30, 32, 30, 25, 10,
		-13, 2, 14, 24, 23, 15, 9, 1,
		-19, -8, 3, 9, 9, 4, -3, -7,
		-20, -8, -5, 0, 2, -2, -9, -14,
		-46, -26, -15, -8, -18, -8, -23, -50,
	};
	
	int ring_attack_potency[6] = { 0, 28, 70, 39, 106, 0, };
	int zone_attack_potency[6] = { 0, -17, 2, 79, 58, 0, };
	
	int ring_pressure_weight[8] = { 0, 12, 26, 39, 52, 50, 20, 0, };
	int zone_pressure_weight[8] = { 0, -5, 0, 5, 9, 10, 11, 3, };
	int mg_pawn_shield[6] = { 0, 35, 40, 28, 31, 13, };
	
	int mg_knight_mobility[9] = { -122, -54, -33, -23, -12, -10, 1, 11, 25, };
	int eg_knight_mobility[9] = { -100, -46, -24, -11, -3, 3, 5, 8, 7, };
	
	int mg_bishop_mobility[14] = { -45, -57, -31, -25, -12, -6, -1, 4, 7, 10, 11, 24, 25, 34, };
	int eg_bishop_mobility[14] = { -96, -60, -28, -15, -10, -6, -1, 3, 7, 9, 11, 8, 10, 6, };
	
	int mg_rook_mobility[15] = { -14, -46, -23, -18, -12, -12, -10, -16, -12, -9, -6, -6, -3, 7, 11, };
	int eg_rook_mobility[15] = { -31, -65, -32, -23, -19, -8, -5, -1, 0, 3, 6, 10, 14, 13, 14, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -58, -51, -18, -11, -9, -8, -9, -6, -4, -2, 0, 0, -1, -2, -7, -6, -6, -1, 6, 24, 26, 28, 58, 33, 19, };
	int eg_queen_mobility[28] = { 0, 0, -13, -71, -56, -51, -43, -34, -23, -11, -8, -4, 1, 3, 5, 10, 12, 19, 22, 25, 29, 26, 25, 29, 33, 35, 35, 32, };
	
	int mg_king_mobility[9] = { 0, -21, -12, 4, -4, 23, -37, -20, -1, };
	int eg_king_mobility[9] = { -2, 23, 4, 6, 7, -6, 6, 5, -4, };
	
	int mg_minor_threatened_by_pawn = -42;
	int eg_minor_threatened_by_pawn = -10;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -44;
	int eg_rook_threatened_by_lesser = 1;
	
	int mg_queen_threatened_by_lesser = -13;
	int eg_queen_threatened_by_lesser = -9;
	
	int mg_passed_bonus[8] = { 0, 39, 15, 1, -21, -3, 11, 0, };
	int eg_passed_bonus[8] = { 0, 92, 70, 41, 29, 23, 24, 0, };
	int mg_passed_bonus_blocked[8] = { 0, 26, 22, -5, -29, -5, 4, 0, };
	int eg_passed_bonus_blocked[8] = { 0, 30, 18, 21, 19, 23, 23, 0, };
	
	int mg_passed_friendly_distance[8] = { 0, 0, 7, 12, 13, 4, 0, 0, };
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
	
	int mg_double_bishop = 15;
	int eg_double_bishop = 39;
	
	int mg_rook_open_file = 30;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 9;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = -5;
	int eg_rook_on_seventh = 18;
	
	int mg_knight_outpost = 20;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 17;
	
	int mg_center_control = 423;
	
	int pawn_count_scale_offset = 73;
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
