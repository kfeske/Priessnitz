#pragma once

#include "utility.h"
#include "board.h"

struct Evaluation : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t ring[2];
	uint64_t zone[2];
	int ring_pressure[2];
	int zone_pressure[2];
	int ring_attackers[2];
	int zone_attackers[2];
	
	int mg_piece_value[6] = { 98, 347, 365, 501, 1178, 0, };
	int eg_piece_value[6] = { 108, 292, 306, 550, 951, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		26, 68, 19, 45, 30, 51, -7, -37,
		-11, -7, 0, 3, 56, 79, 27, 0,
		-34, -18, -9, 15, 11, 7, -8, -23,
		-38, -32, -5, 7, 15, 8, -11, -28,
		-37, -34, -16, -9, 3, 1, 14, -16,
		-27, -19, -19, -4, -6, 39, 34, -6,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-112, -44, -28, -29, 16, -76, -10, -63,
		-73, -47, 76, 18, 12, 31, -8, -35,
		-58, 43, 10, 34, 50, 82, 61, 40,
		1, 16, -2, 51, 30, 50, 14, 29,
		-3, 7, 2, 6, 22, 18, 20, 12,
		-11, -17, -4, -11, 14, 2, 20, -7,
		-1, -35, -23, 12, 4, 21, -12, 7,
		-58, 11, -24, -21, 18, -9, 12, -5,
	};
	int mg_bishop_psqt[64] = {
		-36, -18, -79, -49, -12, -20, -14, -9,
		-43, -3, -54, -43, 0, 22, -18, -85,
		-23, 25, 21, -5, -3, 26, -1, -19,
		-5, 15, 4, 26, 10, 9, 10, -18,
		9, 8, 6, 16, 22, -1, 4, 23,
		13, 20, 14, 13, 12, 32, 8, 19,
		21, 30, 19, 9, 18, 22, 53, 22,
		-22, 19, 23, 8, 13, 14, -21, 0,
	};
	int mg_rook_psqt[64] = {
		13, 28, -15, 40, 26, -32, 1, 5,
		25, 20, 61, 49, 65, 41, -5, 14,
		-7, 33, 23, 45, -11, 28, 46, 1,
		-27, -5, 15, 15, 23, 16, -16, -26,
		-55, -21, -14, -12, 3, -27, -2, -42,
		-45, -24, -11, -14, 7, -6, -15, -39,
		-43, -11, -14, 0, 13, 2, 0, -61,
		-20, -16, -5, 6, 11, 0, -28, -5,
	};
	int mg_queen_psqt[64] = {
		8, -9, 7, -6, 44, 25, 24, 38,
		-11, -46, -9, 12, -29, 10, 1, 40,
		6, 1, 15, -27, 8, 34, 11, 38,
		-23, -14, -23, -43, -23, -23, -16, -12,
		14, -32, -1, -14, -12, -14, -5, -3,
		-4, 32, 5, 9, -3, 4, 20, 6,
		-2, 19, 38, 26, 32, 33, 18, 20,
		28, 10, 24, 44, 14, 4, -6, -34,
	};
	int mg_king_psqt[64] = {
		-10, 4, 15, 3, -13, 2, 3, -1,
		15, 14, 12, 25, 14, 13, -7, -13,
		9, 27, 36, 16, 16, 48, 49, -8,
		-4, 6, 21, 7, -5, -9, 15, -37,
		-16, 35, -2, -57, -61, -40, -29, -67,
		3, 17, -5, -28, -32, -28, 5, -38,
		26, 32, -7, -62, -36, -8, 34, 27,
		-20, 59, 31, -66, 15, -20, 52, 23,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		117, 103, 98, 79, 93, 84, 105, 121,
		7, 9, 13, 0, 9, -12, 6, 2,
		-8, -13, -16, -29, -23, -22, -18, -16,
		-15, -16, -24, -25, -25, -28, -29, -26,
		-27, -23, -26, -20, -19, -24, -38, -32,
		-20, -27, -11, -21, -8, -24, -36, -37,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-67, -32, 4, -14, -8, -18, -47, -84,
		-9, 9, -15, 17, 6, -8, -2, -33,
		-3, -3, 25, 25, 12, 13, -2, -20,
		0, 19, 42, 33, 34, 24, 23, 3,
		1, 10, 29, 43, 30, 29, 22, 1,
		-5, 16, 13, 30, 24, 11, -2, -4,
		-21, 0, 10, 5, 10, -3, -4, -31,
		-17, -32, -11, 6, -11, -7, -37, -41,
	};
	int eg_bishop_psqt[64] = {
		-6, -12, -6, -2, -4, -8, 6, -17,
		4, 0, 14, -8, 3, 0, 2, 5,
		9, -4, 0, 2, -3, 0, 6, 12,
		3, 7, 7, 5, 7, 6, -4, 12,
		-1, 3, 12, 12, 0, 7, -5, -5,
		-5, 2, 9, 11, 15, -2, 3, -5,
		-5, -14, -3, 2, 3, -6, -13, -18,
		-10, 2, -14, -1, -4, -7, 5, -7,
	};
	int eg_rook_psqt[64] = {
		10, 7, 18, 7, 12, 15, 14, 9,
		4, 9, 3, 7, -9, 2, 11, 8,
		8, 5, 4, 0, 8, -4, -5, 0,
		11, 2, 10, 0, 1, 6, 2, 8,
		13, 7, 11, 3, -4, -3, -7, 2,
		3, 2, -6, -4, -11, -13, -1, -7,
		0, -3, -3, -4, -15, -10, -14, -2,
		-8, 1, 0, -4, -9, -10, -2, -31,
	};
	int eg_queen_psqt[64] = {
		-26, 18, 21, 17, 25, 20, 20, 34,
		-21, 5, 11, 23, 37, 23, 22, 24,
		-17, -13, -21, 42, 37, 23, 28, 26,
		15, 13, 2, 21, 33, 25, 63, 51,
		-19, 19, -8, 7, 4, 14, 30, 27,
		-3, -53, -13, -18, -1, -6, 3, 22,
		-22, -42, -50, -29, -29, -37, -45, -24,
		-33, -29, -37, -63, -11, -26, -21, -37,
	};
	int eg_king_psqt[64] = {
		-66, -49, -18, -19, -17, 5, -5, -16,
		-16, 14, 7, 15, 17, 34, 17, 4,
		3, 15, 17, 12, 15, 46, 37, 5,
		-15, 18, 22, 27, 27, 35, 24, 1,
		-25, -9, 22, 31, 35, 29, 9, -9,
		-26, -7, 14, 23, 26, 21, 5, -8,
		-39, -15, 8, 17, 17, 8, -9, -26,
		-64, -46, -27, -8, -28, -14, -40, -61,
	};
	
	int ring_attack_potency[6] = { 0, 3, 32, 39, 131, 0, };
	int zone_attack_potency[6] = { 0, -13, 28, 21, 29, 0, };
	
	int ring_pressure_weight[8] = { 0, 7, 37, 66, 94, 38, 0, 0, };
	int zone_pressure_weight[8] = { 0, 5, 11, 16, 24, 29, 29, 3, };
	
	int mg_average_mobility[6] = { 0, -27, -24, -20, -49, 0, };
	int eg_average_mobility[6] = { 0, 1, 5, -17, -30, 0, };
	int mg_mobility_weight[6] = { 0, 116, 86, 85, 17, 0, };
	int eg_mobility_weight[6] = { 0, 14, 32, 32, 78, 0, };
	
	int mg_isolated_penalty = -20;
	int eg_isolated_penalty = -12;
	
	int mg_doubled_penalty = -4;
	int eg_doubled_penalty = -6;
	
	int mg_backward_penalty = -13;
	int eg_backward_penalty = -13;
	
	int mg_chained_bonus = 12;
	int eg_chained_bonus = 8;
	
	int mg_passed_bonus[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		26, 68, 19, 45, 30, 51, -7, -37,
		48, 12, 28, 12, 3, 2, -35, -32,
		26, 16, 8, -6, 9, 34, -3, -11,
		21, -21, -27, -24, -30, -8, -10, 16,
		7, -1, -21, -37, -1, 33, 17, 35,
		5, 6, 5, -18, -19, 25, 18, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_passed_bonus[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		117, 103, 98, 79, 93, 84, 105, 121,
		154, 147, 112, 93, 66, 104, 123, 143,
		89, 75, 63, 55, 40, 49, 79, 78,
		48, 45, 39, 30, 29, 32, 49, 44,
		20, 19, 19, 19, 10, 6, 23, 17,
		16, 19, 2, 22, 17, 6, 16, 22,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	
	int mg_double_bishop = 29;
	int eg_double_bishop = 53;
	
	int tempo_bonus = 0;

	int taper_start = 6377;
	int taper_end = 321;

	void evaluate_pawn(Board &board, unsigned square, Color friendly);

	void evaluate_kings();

	template <Piece_type pt>
	void note_king_attacks(uint64_t attacks, Color friendly);

	template <Piece_type pt>
	void evaluate_mobility(Board &board, uint64_t attacks, Color friendly);

	void evaluate_piece(Board &board, Piece p, unsigned square);

	int evaluate(Board &board);
};
