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

	uint64_t attacked_by_pawn[2];

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
	}
};

struct Evaluation : Noncopyable
{
	Eval_info info {};

	int mg_piece_value[6] = { 42, 258, 273, 315, 682, 0, };
	int eg_piece_value[6] = { 60, 287, 295, 499, 956, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		7, 26, 6, 64, 48, 42, -14, -47,
		11, 6, 30, 37, 38, 68, 42, 5,
		-9, -3, 8, 12, 30, 33, 10, 12,
		-12, -10, 5, 19, 21, 24, 10, 2,
		-17, -11, -3, 4, 13, 3, 11, 1,
		-9, -5, -1, -4, 7, 20, 24, -6,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-90, -67, -50, -21, -8, -43, -50, -47,
		-37, -27, -6, 2, -19, 31, -26, -5,
		-30, -10, -4, -4, 29, 31, 7, -14,
		-20, -6, 4, 27, 15, 33, 6, 17,
		-4, -1, 5, 12, 16, 15, 21, 6,
		-23, -13, -10, -12, 4, -8, 6, -8,
		-22, -20, -15, 0, -3, 0, 0, -3,
		-50, -16, -31, -19, -12, -6, -13, -22,
	};
	int mg_bishop_psqt[64] = {
		-27, -56, -50, -82, -73, -70, -36, -53,
		-22, -9, -20, -32, -14, -27, -32, -47,
		-14, 1, 2, 5, -8, 20, -2, -8,
		-22, -3, -2, 12, 6, -4, -3, -32,
		-8, -21, -13, 8, 1, -10, -17, 7,
		-7, 1, -4, -4, -1, -1, 1, 11,
		12, 2, 8, -10, -3, 9, 23, 12,
		2, 10, -5, -17, -5, -10, 10, 27,
	};
	int mg_rook_psqt[64] = {
		8, -5, -6, -9, 7, 19, 15, 34,
		-15, -19, -10, 6, -6, 16, 17, 37,
		-25, 1, -8, -6, 22, 19, 61, 30,
		-27, -14, -16, -15, -11, -6, 6, 3,
		-33, -38, -29, -27, -25, -35, -7, -18,
		-31, -26, -23, -22, -11, -11, 17, -3,
		-28, -25, -13, -11, -5, -3, 16, -18,
		-13, -15, -14, -7, 1, -2, 0, -3,
	};
	int mg_queen_psqt[64] = {
		-29, -39, -19, 1, -10, -12, 12, -11,
		1, -26, -37, -49, -57, -16, -16, 35,
		5, -7, -8, -16, -12, 6, 18, 15,
		-12, -6, -17, -27, -28, -21, -13, -9,
		-1, -18, -16, -12, -18, -21, -7, -2,
		-3, 3, -7, -10, -5, -5, 8, 6,
		4, 4, 11, 11, 11, 14, 26, 23,
		0, -9, -3, 10, 6, -4, 5, 8,
	};
	int mg_king_psqt[64] = {
		-23, -16, -16, -37, -19, -6, 22, 18,
		-43, -12, -27, 18, 14, 15, 39, 30,
		-62, 11, -26, -38, -5, 42, 45, -2,
		-40, -30, -58, -94, -87, -40, -37, -74,
		-48, -35, -55, -105, -94, -45, -52, -107,
		-28, 22, -27, -52, -41, -38, -7, -55,
		28, 10, 9, -17, -22, -6, 20, 8,
		26, 34, 23, -50, 11, -36, 23, 44,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		19, 18, 40, 22, 30, 26, 42, 40,
		32, 39, 22, 10, 16, 20, 39, 37,
		26, 19, 10, 0, 2, 6, 15, 15,
		16, 13, 6, 0, 1, 4, 7, 6,
		11, 8, 6, 6, 7, 7, 3, 2,
		13, 12, 10, 10, 14, 10, 5, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-46, -17, -5, -11, -5, -26, -20, -61,
		-6, -4, -4, -1, -6, -15, -6, -19,
		-5, -4, 4, 6, -4, -8, -11, -11,
		3, -1, 5, 6, 6, 4, 2, -4,
		5, 2, 12, 13, 15, 7, 2, 6,
		-4, -2, -1, 8, 8, -4, -3, -2,
		-5, -2, -5, -3, -4, -5, -6, 7,
		-5, -6, -6, -5, -1, -7, -1, -3,
	};
	int eg_bishop_psqt[64] = {
		-1, 2, -1, 2, 1, -7, -2, -10,
		-10, -5, -4, 0, -7, -3, -5, -7,
		4, -3, -2, -7, -1, 1, 0, 4,
		1, 3, -1, 10, 0, 3, -2, 4,
		-3, 0, 5, 3, 4, -1, 1, -13,
		-5, 1, 0, 4, 6, 2, -4, -6,
		1, -9, -9, -4, 0, -5, -3, -8,
		-9, -4, -3, -3, -5, 3, -8, -17,
	};
	int eg_rook_psqt[64] = {
		9, 14, 17, 13, 10, 11, 15, 9,
		1, 7, 10, 2, 3, 2, 1, -6,
		14, 13, 14, 11, 5, 5, 1, -2,
		14, 12, 14, 12, 4, 4, 4, 0,
		6, 8, 6, 5, 3, 4, -2, -4,
		-1, -2, -3, -4, -6, -9, -17, -16,
		-7, -3, -6, -7, -11, -12, -18, -14,
		-5, -6, -3, -8, -11, -8, -9, -12,
	};
	int eg_queen_psqt[64] = {
		1, 10, 19, 16, 19, 26, 6, 12,
		-15, -2, 22, 34, 47, 26, 21, 20,
		-9, -2, 15, 24, 34, 36, 17, 18,
		0, 5, 8, 24, 31, 27, 34, 21,
		-6, 5, 5, 12, 16, 16, 13, 12,
		-16, -9, -2, -1, 3, 2, -5, -5,
		-17, -20, -20, -14, -12, -26, -39, -33,
		-18, -16, -16, -20, -21, -22, -27, -23,
	};
	int eg_king_psqt[64] = {
		-50, -18, -9, 7, 1, 12, 23, -39,
		0, 21, 25, 18, 24, 35, 36, 19,
		7, 22, 27, 32, 36, 34, 32, 13,
		-3, 14, 25, 31, 33, 30, 25, 9,
		-13, 2, 14, 24, 23, 15, 9, 1,
		-19, -8, 3, 10, 9, 5, -3, -7,
		-20, -9, -5, 0, 2, -2, -9, -14,
		-46, -25, -15, -8, -18, -8, -23, -50,
	};
	
	int ring_attack_potency[6] = { 0, 29, 67, 40, 107, 0, };
	int zone_attack_potency[6] = { 0, -10, 8, 78, 59, 0, };
	
	int ring_pressure_weight[8] = { 0, 12, 26, 39, 52, 50, 20, 0, };
	int zone_pressure_weight[8] = { 0, -4, 1, 5, 9, 10, 10, 4, };
	int mg_pawn_shield[6] = { 0, 35, 40, 27, 31, 13, };
	
	int mg_knight_mobility[9] = { -117, -51, -31, -22, -12, -10, 0, 9, 21, };
	int eg_knight_mobility[9] = { -99, -46, -23, -10, -4, 3, 5, 7, 7, };
	
	int mg_bishop_mobility[14] = { -46, -58, -32, -25, -12, -5, 0, 5, 8, 10, 13, 25, 24, 30, };
	int eg_bishop_mobility[14] = { -94, -60, -28, -15, -10, -6, -1, 3, 7, 9, 11, 8, 10, 6, };
	
	int mg_rook_mobility[15] = { -14, -50, -24, -18, -12, -13, -10, -16, -12, -9, -5, -5, -3, 7, 11, };
	int eg_rook_mobility[15] = { -33, -65, -32, -23, -19, -8, -6, -1, 0, 3, 6, 10, 14, 14, 14, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -62, -52, -22, -14, -12, -10, -10, -7, -4, -2, 0, 1, 0, -1, -4, -4, -4, 1, 9, 26, 28, 30, 60, 35, 21, };
	int eg_queen_mobility[28] = { 0, 0, -13, -73, -57, -48, -42, -34, -24, -11, -8, -4, 1, 2, 5, 10, 12, 19, 22, 25, 29, 26, 25, 30, 34, 37, 37, 34, };
	
	int mg_king_mobility[9] = { 0, -22, -13, 3, -5, 23, -36, -19, 0, };
	int eg_king_mobility[9] = { -2, 22, 4, 6, 7, -6, 6, 5, -4, };
	
	int mg_passed_bonus[8] = { 0, 43, 15, 1, -20, -4, 10, 0, };
	int eg_passed_bonus[8] = { 0, 92, 70, 41, 29, 23, 23, 0, };
	int mg_passed_bonus_blocked[8] = { 0, 23, 21, -5, -29, -4, 4, 0, };
	int eg_passed_bonus_blocked[8] = { 0, 30, 18, 21, 18, 23, 22, 0, };
	
	int mg_passed_friendly_distance[8] = { 0, 1, 7, 12, 13, 4, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -16, -13, -8, -3, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 0, -14, -12, -9, -4, -2, 0, };
	int eg_passed_enemy_distance[8] = { 0, 34, 30, 17, 8, 0, -2, 0, };
	
	int mg_isolated_penalty = -9;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = -3;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 8;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 17;
	int eg_double_bishop = 41;
	
	int mg_rook_open_file = 31;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 9;
	int eg_rook_half_open_file = 7;
	
	int mg_rook_on_seventh = -5;
	int eg_rook_on_seventh = 19;
	
	int mg_knight_outpost = 28;
	int eg_knight_outpost = -1;
	
	int mg_knight_outpost_supported = 40;
	int eg_knight_outpost_supported = 18;
	
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

	void note_king_attacks(Piece_type type, uint64_t attacks, Color friendly);

	int scale_factor(Board &board, int eg_value);

	int evaluate(Board &board);
};
