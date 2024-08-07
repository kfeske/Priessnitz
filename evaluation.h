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

	int mg_piece_value[6] = { 39, 258, 275, 319, 685, 0, };
	int eg_piece_value[6] = { 54, 278, 285, 481, 921, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		29, 35, 25, 51, 36, 28, -17, -33,
		7, 1, 22, 26, 36, 70, 55, 19,
		-8, -1, 8, 11, 31, 35, 11, 14,
		-9, -7, 9, 20, 22, 26, 10, 4,
		-15, -8, 0, 5, 14, 5, 11, 2,
		-7, -3, 2, -2, 8, 22, 24, -5,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-89, -67, -49, -21, -8, -43, -52, -47,
		-34, -26, -5, 4, -18, 32, -26, -4,
		-28, -11, -2, -4, 29, 32, 6, -15,
		-18, -5, 4, 26, 15, 33, 6, 17,
		-4, -1, 5, 12, 16, 15, 21, 6,
		-23, -13, -10, -11, 4, -8, 6, -7,
		-23, -20, -15, 0, -3, 1, 1, -3,
		-51, -16, -31, -18, -12, -6, -12, -23,
	};
	int mg_bishop_psqt[64] = {
		-27, -56, -51, -83, -71, -70, -36, -53,
		-19, -9, -19, -30, -13, -25, -33, -45,
		-13, 3, 3, 5, -7, 22, -1, -7,
		-22, -2, -2, 13, 6, -4, -2, -31,
		-8, -21, -12, 8, 1, -9, -17, 7,
		-7, 1, -4, -4, -1, -1, 1, 11,
		11, 1, 8, -10, -3, 9, 23, 12,
		1, 9, -4, -17, -5, -10, 11, 27,
	};
	int mg_rook_psqt[64] = {
		8, -7, -7, -8, 7, 17, 17, 35,
		-13, -18, -6, 10, -3, 20, 23, 42,
		-24, 0, -8, -5, 22, 19, 59, 32,
		-27, -14, -16, -15, -11, -7, 4, 5,
		-34, -39, -30, -27, -25, -36, -7, -19,
		-32, -27, -24, -22, -12, -11, 17, -4,
		-30, -26, -14, -13, -6, -4, 16, -18,
		-14, -16, -15, -7, 0, -3, 1, -3,
	};
	int mg_queen_psqt[64] = {
		-29, -38, -20, -1, -9, -12, 11, -12,
		3, -27, -36, -51, -58, -18, -16, 32,
		6, -6, -9, -15, -12, 6, 17, 15,
		-11, -5, -16, -27, -27, -20, -12, -8,
		-1, -18, -15, -11, -18, -20, -7, -2,
		-4, 3, -7, -10, -6, -5, 8, 6,
		3, 4, 11, 10, 10, 14, 25, 24,
		1, -8, -3, 10, 5, -4, 6, 7,
	};
	int mg_king_psqt[64] = {
		-26, -15, -12, -33, -19, -6, 17, 9,
		-46, -8, -26, 22, 19, 20, 37, 10,
		-67, 16, -19, -29, 4, 57, 47, -5,
		-52, -32, -49, -78, -69, -31, -33, -73,
		-57, -37, -51, -90, -83, -43, -51, -108,
		-33, 15, -34, -54, -39, -40, -6, -56,
		29, 12, 6, -21, -24, -5, 26, 11,
		22, 34, 22, -51, 10, -36, 24, 37,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		88, 80, 80, 60, 61, 66, 80, 89,
		34, 34, 31, 38, 33, 33, 49, 42,
		21, 16, 15, 9, 10, 11, 14, 12,
		12, 11, 11, 10, 10, 9, 7, 5,
		9, 7, 10, 15, 14, 12, 3, 2,
		11, 10, 14, 18, 21, 15, 4, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-47, -19, -7, -13, -8, -27, -19, -60,
		-7, -4, -4, -3, -7, -15, -7, -19,
		-7, -4, 3, 5, -5, -9, -10, -11,
		1, -1, 4, 6, 4, 4, 2, -5,
		4, 1, 11, 12, 14, 7, 1, 4,
		-4, -2, -1, 8, 7, -4, -4, -3,
		-3, -2, -4, -3, -5, -6, -6, 7,
		-1, -6, -6, -7, -3, -9, -3, -2,
	};
	int eg_bishop_psqt[64] = {
		-3, 0, -3, 0, -3, -7, -3, -11,
		-10, -7, -7, -4, -9, -6, -5, -8,
		2, -5, -4, -8, -3, -1, -2, 2,
		-2, 1, -2, 8, -1, 2, -3, 1,
		-3, 0, 4, 3, 3, -2, 0, -12,
		-4, 2, 1, 4, 6, 1, -5, -6,
		2, -9, -8, -3, -1, -5, -3, -6,
		-8, -3, -5, -3, -6, 2, -10, -16,
	};
	int eg_rook_psqt[64] = {
		7, 12, 15, 10, 8, 10, 11, 6,
		9, 15, 18, 11, 11, 9, 6, 0,
		9, 10, 10, 6, 2, 1, -1, -6,
		10, 8, 10, 8, 1, 2, 0, -5,
		4, 6, 4, 3, 1, 2, -5, -6,
		-2, -3, -4, -5, -7, -10, -19, -18,
		-7, -4, -6, -7, -11, -13, -21, -16,
		-6, -7, -5, -9, -12, -9, -13, -16,
	};
	int eg_queen_psqt[64] = {
		-2, 8, 18, 15, 17, 25, 6, 11,
		-17, -3, 20, 33, 45, 25, 18, 19,
		-12, -4, 13, 21, 31, 32, 14, 15,
		-4, 2, 6, 22, 28, 24, 30, 18,
		-6, 4, 4, 11, 15, 14, 12, 10,
		-16, -10, -2, 0, 3, 2, -5, -6,
		-17, -20, -20, -14, -12, -25, -38, -35,
		-19, -17, -17, -20, -21, -22, -29, -25,
	};
	int eg_king_psqt[64] = {
		-85, -30, -18, -1, -6, 1, 6, -80,
		-12, 24, 31, 25, 31, 39, 36, 0,
		-3, 28, 37, 43, 47, 42, 37, 0,
		-13, 23, 36, 43, 42, 38, 29, -6,
		-19, 12, 26, 36, 34, 23, 14, -11,
		-25, 2, 15, 22, 20, 13, 2, -17,
		-28, -4, 4, 10, 11, 4, -7, -25,
		-74, -34, -22, -15, -25, -16, -31, -78,
	};
	
	int ring_attack_potency[6] = { 0, 31, 67, 38, 108, 0, };
	int zone_attack_potency[6] = { 0, -10, 6, 83, 56, 0, };
	
	int ring_pressure_weight[8] = { 0, 12, 26, 38, 52, 51, 20, 0, };
	int zone_pressure_weight[8] = { 0, -5, 1, 5, 9, 9, 10, 4, };
	int mg_pawn_shield[6] = { 0, 33, 39, 27, 31, 13, };
	
	int mg_knight_mobility[9] = { -116, -50, -31, -22, -12, -10, 1, 10, 22, };
	int eg_knight_mobility[9] = { -92, -46, -24, -12, -5, 1, 3, 4, 4, };
	
	int mg_bishop_mobility[14] = { -51, -59, -32, -24, -12, -5, 1, 5, 8, 11, 13, 26, 25, 34, };
	int eg_bishop_mobility[14] = { -86, -59, -30, -16, -11, -8, -3, 1, 5, 7, 8, 5, 6, 1, };
	
	int mg_rook_mobility[15] = { -14, -54, -25, -19, -14, -14, -11, -17, -12, -9, -5, -4, -2, 9, 12, };
	int eg_rook_mobility[15] = { -31, -51, -30, -21, -18, -8, -5, -1, -2, 1, 4, 7, 9, 9, 11, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -63, -53, -23, -15, -12, -11, -10, -7, -5, -3, 0, 0, 0, -1, -4, -4, -4, 2, 11, 28, 29, 31, 60, 33, 18, };
	int eg_queen_mobility[28] = { 0, 0, -12, -75, -56, -49, -42, -33, -24, -11, -9, -5, -1, 0, 2, 7, 9, 15, 18, 21, 24, 21, 20, 25, 28, 32, 29, 30, };
	
	int mg_king_mobility[9] = { 0, 11, -4, 12, -5, 22, -40, -24, -6, };
	int eg_king_mobility[9] = { 4, 111, 41, 20, 13, -7, 0, -2, -13, };
	
	int mg_isolated_penalty = -13;
	int eg_isolated_penalty = -10;
	
	int mg_doubled_penalty = -5;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -11;
	int eg_backward_penalty = -10;
	
	int mg_chained_bonus = 8;
	int eg_chained_bonus = 6;
	
	int mg_passed_bonus[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		29, 35, 25, 51, 36, 28, -17, -33,
		16, 25, 17, 19, 8, 8, -22, -52,
		17, 10, 18, 17, 3, 1, -9, -14,
		2, -6, -23, -10, -21, -19, -16, -9,
		3, -16, -23, -19, -17, -10, -15, 10,
		-4, -3, -15, -9, 4, -4, 8, 7,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_passed_bonus[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		88, 80, 80, 60, 61, 66, 80, 89,
		111, 110, 80, 44, 50, 64, 79, 95,
		69, 65, 42, 33, 31, 42, 61, 65,
		42, 40, 27, 18, 21, 29, 44, 41,
		17, 22, 15, 8, 10, 13, 29, 17,
		16, 18, 14, 3, 3, 8, 14, 16,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_double_bishop = 16;
	int eg_double_bishop = 41;
	
	int mg_rook_open_file = 29;
	int eg_rook_open_file = 7;
	
	int mg_rook_half_open_file = 8;
	int eg_rook_half_open_file = 9;
	
	int mg_knight_outpost = 28;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 40;
	int eg_knight_outpost_supported = 18;
	
	int pawn_count_scale_offset = 76;
	int pawn_count_scale_weight = 29;

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
