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

	int mg_piece_value[6] = { 41, 258, 275, 317, 686, 0, };
	int eg_piece_value[6] = { 55, 281, 289, 488, 934, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		30, 35, 23, 75, 43, 38, -40, -68,
		9, 5, 34, 45, 48, 77, 49, 8,
		-9, -3, 9, 13, 32, 35, 10, 11,
		-8, -8, 7, 20, 22, 25, 9, 3,
		-14, -10, -1, 6, 14, 4, 10, 2,
		-7, -4, 1, -2, 8, 21, 24, -6,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-89, -66, -49, -21, -7, -42, -50, -46,
		-37, -27, -6, 3, -18, 31, -27, -5,
		-28, -11, -3, -3, 29, 32, 6, -15,
		-19, -5, 4, 26, 15, 33, 6, 17,
		-4, -1, 5, 12, 16, 15, 21, 6,
		-23, -13, -10, -12, 4, -8, 6, -7,
		-22, -19, -15, 0, -3, 1, 1, -3,
		-49, -16, -31, -18, -12, -6, -13, -22,
	};
	int mg_bishop_psqt[64] = {
		-26, -56, -50, -83, -72, -70, -36, -52,
		-22, -10, -19, -30, -13, -26, -33, -46,
		-13, 1, 2, 5, -8, 21, -1, -8,
		-22, -3, -2, 13, 5, -4, -2, -32,
		-8, -21, -13, 8, 1, -10, -17, 7,
		-7, 2, -4, -4, -1, -1, 1, 11,
		12, 1, 8, -10, -3, 9, 23, 13,
		2, 10, -5, -17, -4, -10, 11, 27,
	};
	int mg_rook_psqt[64] = {
		5, -6, -6, -8, 7, 19, 17, 36,
		-15, -19, -9, 6, -5, 16, 17, 39,
		-24, 0, -8, -6, 22, 19, 60, 31,
		-26, -14, -16, -16, -12, -7, 5, 3,
		-34, -38, -30, -27, -25, -36, -7, -18,
		-31, -27, -23, -22, -12, -11, 17, -3,
		-28, -25, -13, -12, -5, -4, 17, -17,
		-13, -15, -14, -7, 0, -2, 1, -2,
	};
	int mg_queen_psqt[64] = {
		-29, -38, -19, 0, -10, -12, 13, -11,
		1, -28, -37, -50, -57, -17, -16, 35,
		5, -7, -9, -16, -12, 6, 19, 16,
		-13, -6, -17, -27, -27, -21, -12, -8,
		-1, -18, -16, -12, -18, -20, -7, -2,
		-3, 3, -7, -10, -6, -5, 8, 6,
		4, 4, 11, 11, 10, 14, 26, 24,
		0, -8, -3, 10, 6, -4, 5, 8,
	};
	int mg_king_psqt[64] = {
		-26, -17, -15, -36, -19, -1, 21, 11,
		-49, -7, -27, 21, 16, 19, 38, 10,
		-70, 9, -24, -33, 1, 51, 45, -8,
		-54, -35, -55, -86, -78, -37, -38, -80,
		-57, -41, -54, -98, -92, -48, -56, -108,
		-32, 16, -35, -56, -42, -42, -5, -55,
		30, 13, 8, -19, -23, -3, 27, 12,
		24, 35, 22, -51, 10, -36, 25, 40,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		77, 60, 58, 24, 24, 27, 56, 71,
		52, 51, 25, 1, 4, 20, 44, 46,
		30, 22, 13, 2, 3, 8, 18, 18,
		19, 15, 9, 4, 5, 7, 9, 9,
		14, 10, 8, 9, 10, 10, 5, 6,
		16, 13, 12, 11, 15, 13, 7, 7,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-46, -18, -7, -11, -6, -26, -18, -60,
		-7, -4, -4, -2, -6, -14, -6, -18,
		-6, -4, 4, 6, -4, -8, -10, -10,
		2, -1, 5, 6, 5, 4, 2, -4,
		4, 1, 11, 13, 14, 7, 2, 5,
		-5, -2, -1, 8, 7, -4, -4, -3,
		-6, -4, -5, -3, -5, -6, -6, 6,
		-5, -5, -6, -6, -1, -8, -2, -5,
	};
	int eg_bishop_psqt[64] = {
		-2, 0, -3, 1, -1, -6, -2, -11,
		-10, -6, -5, -1, -8, -4, -5, -6,
		4, -3, -2, -6, -2, 1, -1, 4,
		1, 2, -1, 9, 1, 3, -2, 4,
		-4, 0, 4, 3, 4, -1, 1, -12,
		-5, 1, 0, 3, 5, 1, -5, -7,
		-1, -10, -9, -4, -1, -5, -4, -8,
		-9, -3, -3, -3, -6, 2, -10, -18,
	};
	int eg_rook_psqt[64] = {
		11, 13, 17, 12, 9, 10, 12, 7,
		0, 6, 9, 2, 2, 2, 0, -6,
		13, 12, 13, 10, 4, 4, 1, -2,
		13, 11, 13, 11, 4, 4, 3, 0,
		5, 7, 6, 5, 3, 4, -3, -4,
		-2, -2, -3, -4, -6, -9, -18, -17,
		-8, -4, -6, -7, -11, -12, -19, -16,
		-6, -6, -4, -8, -11, -8, -11, -15,
	};
	int eg_queen_psqt[64] = {
		-1, 8, 18, 15, 17, 25, 5, 11,
		-16, -1, 21, 34, 46, 25, 19, 20,
		-9, -2, 15, 24, 33, 34, 15, 17,
		-1, 3, 7, 23, 29, 26, 32, 20,
		-6, 4, 4, 11, 15, 15, 12, 11,
		-17, -10, -2, -1, 3, 2, -5, -5,
		-19, -21, -20, -15, -13, -26, -38, -36,
		-19, -17, -17, -20, -21, -22, -28, -25,
	};
	int eg_king_psqt[64] = {
		-75, -29, -17, -1, -6, 1, 12, -70,
		-8, 22, 30, 22, 29, 38, 36, 6,
		1, 28, 36, 42, 45, 41, 37, 5,
		-9, 23, 35, 42, 42, 38, 29, 0,
		-18, 10, 24, 35, 32, 22, 13, -9,
		-25, -2, 12, 19, 18, 11, -2, -17,
		-28, -6, 1, 7, 9, 2, -9, -24,
		-68, -31, -19, -11, -21, -12, -29, -72,
	};
	
	int ring_attack_potency[6] = { 0, 30, 66, 38, 108, 0, };
	int zone_attack_potency[6] = { 0, -9, 6, 82, 57, 0, };
	
	int ring_pressure_weight[8] = { 0, 12, 26, 39, 52, 51, 20, 0, };
	int zone_pressure_weight[8] = { 0, -5, 1, 5, 9, 9, 10, 4, };
	int mg_pawn_shield[6] = { 0, 33, 40, 27, 31, 13, };
	
	int mg_knight_mobility[9] = { -115, -50, -31, -22, -11, -10, 1, 10, 21, };
	int eg_knight_mobility[9] = { -91, -46, -24, -11, -4, 1, 4, 5, 5, };
	
	int mg_bishop_mobility[14] = { -48, -58, -32, -24, -12, -5, 1, 6, 8, 11, 13, 25, 25, 32, };
	int eg_bishop_mobility[14] = { -88, -60, -29, -16, -11, -7, -2, 2, 5, 8, 9, 6, 7, 3, };
	
	int mg_rook_mobility[15] = { -16, -52, -24, -19, -13, -13, -11, -17, -13, -10, -6, -5, -2, 8, 12, };
	int eg_rook_mobility[15] = { -38, -69, -33, -23, -20, -9, -6, -1, 0, 3, 5, 9, 12, 12, 14, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -63, -53, -22, -15, -12, -10, -10, -7, -5, -3, 0, 0, 0, -1, -5, -4, -4, 2, 10, 28, 29, 31, 61, 34, 21, };
	int eg_queen_mobility[28] = { 0, 0, -13, -76, -58, -50, -43, -34, -25, -12, -10, -5, 0, 1, 3, 8, 10, 17, 20, 23, 26, 23, 22, 27, 30, 34, 32, 33, };
	
	int mg_king_mobility[9] = { 0, 3, -5, 11, -3, 23, -40, -25, -7, };
	int eg_king_mobility[9] = { 1, 91, 33, 15, 11, -8, 2, 1, -10, };
	
	int mg_passed_bonus[8] = { 0, 54, -3, 9, -12, -10, -2, 0, };
	
	int eg_passed_bonus[8] = { 0, 129, 102, 53, 33, 15, 12, 0, };
	
	int mg_passed_bonus_blocked[8] = { 0, 28, 13, 7, -22, -13, -8, 0, };
	
	int eg_passed_bonus_blocked[8] = { 0, 54, 34, 27, 23, 18, 13, 0, };
	
	int mg_isolated_penalty = -12;
	int eg_isolated_penalty = -9;
	
	int mg_doubled_penalty = -5;
	int eg_doubled_penalty = -8;
	
	int mg_backward_penalty = -11;
	int eg_backward_penalty = -10;
	
	int mg_chained_bonus = 8;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 16;
	int eg_double_bishop = 41;
	
	int mg_rook_open_file = 31;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 9;
	int eg_rook_half_open_file = 6;
	
	int mg_rook_on_seventh = -6;
	int eg_rook_on_seventh = 19;
	
	int mg_knight_outpost = 28;
	int eg_knight_outpost = -1;
	
	int mg_knight_outpost_supported = 40;
	int eg_knight_outpost_supported = 19;
	
	int pawn_count_scale_offset = 75;
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
