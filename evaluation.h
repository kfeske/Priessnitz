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

	int mg_piece_value[6] = { 41, 258, 274, 316, 690, 0, };
	int eg_piece_value[6] = { 61, 291, 298, 507, 948, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		11, 24, 7, 67, 50, 45, -15, -42,
		13, 7, 27, 35, 37, 66, 44, 6,
		-6, -4, 5, 7, 23, 33, 6, 13,
		-9, -9, 6, 15, 18, 22, 7, 4,
		-15, -9, -3, 2, 9, 1, 11, 1,
		-6, -3, 2, -5, 7, 18, 26, -3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-87, -70, -66, -33, -18, -47, -57, -50,
		-40, -27, -25, -15, -24, 10, -22, -13,
		-24, -17, -12, -13, 9, 29, -2, -2,
		-18, -6, 3, 27, 15, 25, 1, 16,
		-4, -2, 5, 14, 15, 15, 18, 7,
		-19, -12, -8, -8, 5, -5, 9, -2,
		-22, -18, -15, 0, -1, -1, 3, 1,
		-48, -14, -30, -17, -11, -5, -11, -21,
	};
	int mg_bishop_psqt[64] = {
		-28, -53, -48, -75, -71, -68, -36, -52,
		-27, -33, -24, -38, -34, -27, -46, -36,
		-14, 0, -9, -5, -9, 22, 7, -2,
		-22, -13, -11, -1, -5, -5, -10, -24,
		-12, -25, -12, -2, -4, -6, -15, 1,
		-10, 1, -2, -2, 1, 0, 5, 10,
		11, 2, 10, -6, 1, 9, 24, 15,
		3, 13, 1, -10, 2, -7, 10, 27,
	};
	int mg_rook_psqt[64] = {
		0, -14, -17, -19, -5, 4, 3, 12,
		-19, -23, -17, -1, -14, 9, 8, 21,
		-33, -4, -14, -10, 22, 15, 55, 21,
		-27, -13, -15, -11, -4, 1, 11, 7,
		-30, -32, -24, -21, -17, -27, 4, -10,
		-29, -25, -22, -21, -12, -8, 24, 4,
		-27, -22, -13, -12, -5, -3, 20, -16,
		-13, -15, -15, -7, 0, -1, 3, -5,
	};
	int mg_queen_psqt[64] = {
		-29, -31, -15, 6, -8, -13, 14, -7,
		-8, -29, -33, -44, -49, -8, -16, 32,
		-1, -9, -15, -11, -3, 12, 31, 36,
		-13, -11, -19, -30, -24, -10, 2, 10,
		-2, -21, -20, -20, -18, -13, 4, 11,
		-5, -1, -13, -12, -9, -1, 18, 14,
		2, 1, 6, 7, 7, 10, 24, 25,
		-3, -12, -5, 6, 3, -7, 2, 4,
	};
	int mg_king_psqt[64] = {
		-8, -10, -10, -21, -9, -16, 1, 12,
		-41, -11, -19, 26, 18, 12, 23, 18,
		-45, 22, -7, -20, 14, 55, 35, 9,
		-16, -15, -39, -63, -64, -27, -27, -63,
		-31, -18, -34, -82, -76, -33, -47, -93,
		-20, 24, -16, -42, -34, -30, -7, -48,
		31, 1, 3, -27, -34, -16, 7, 12,
		27, 31, 23, -51, 5, -38, 20, 44,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		15, 15, 37, 20, 28, 25, 39, 37,
		30, 38, 23, 12, 17, 21, 38, 35,
		25, 19, 11, 2, 4, 6, 15, 13,
		15, 12, 6, 1, 2, 4, 6, 5,
		10, 7, 6, 7, 7, 7, 2, 1,
		12, 11, 9, 10, 14, 10, 3, 2,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-44, -14, -1, -8, 0, -21, -16, -59,
		-3, -2, 2, 4, -1, -7, -3, -15,
		-4, -1, 4, 9, 3, -7, -7, -10,
		5, 0, 5, 4, 7, 4, 6, -1,
		5, 1, 11, 12, 14, 7, 1, 6,
		-4, -4, -1, 7, 7, -4, -5, -2,
		-6, -3, -7, -4, -5, -7, -8, 7,
		-6, -7, -8, -7, -4, -11, -2, -3,
	};
	int eg_bishop_psqt[64] = {
		0, 5, 2, 6, 5, -4, 1, -8,
		-8, 0, -3, 3, -3, -2, -1, -8,
		6, -1, 1, -4, 0, 3, 1, 5,
		1, 4, 2, 13, 3, 4, 0, 3,
		-2, 1, 4, 5, 5, -1, 0, -10,
		-4, 1, -1, 3, 6, 1, -5, -5,
		-1, -11, -10, -5, -2, -6, -6, -8,
		-10, -6, -5, -5, -8, 0, -10, -17,
	};
	int eg_rook_psqt[64] = {
		15, 21, 24, 19, 17, 18, 22, 17,
		8, 15, 17, 9, 10, 11, 11, 3,
		17, 16, 16, 12, 6, 7, 5, 1,
		14, 13, 14, 10, 2, 3, 3, -2,
		5, 6, 4, 3, 1, 1, -6, -7,
		-3, -3, -5, -5, -7, -12, -21, -20,
		-9, -5, -7, -8, -11, -13, -21, -17,
		-6, -7, -6, -9, -13, -10, -12, -13,
	};
	int eg_queen_psqt[64] = {
		7, 7, 13, 13, 15, 21, 0, 8,
		-9, -2, 18, 25, 35, 19, 10, 10,
		-7, -3, 11, 13, 20, 16, -2, -1,
		2, 8, 9, 21, 19, 9, 19, 5,
		-3, 11, 12, 17, 15, 9, 5, 3,
		-12, -2, 9, 6, 9, 0, -8, -9,
		-13, -13, -12, -6, -4, -19, -33, -32,
		-13, -11, -11, -11, -15, -18, -22, -16,
	};
	int eg_king_psqt[64] = {
		-45, -16, -4, 11, 5, 15, 21, -35,
		0, 21, 24, 18, 23, 35, 34, 18,
		8, 20, 27, 33, 37, 35, 32, 14,
		-2, 12, 26, 31, 34, 32, 23, 12,
		-12, -1, 13, 24, 22, 14, 6, 2,
		-19, -11, 1, 8, 7, 3, -7, -8,
		-23, -12, -7, -1, 2, -3, -13, -18,
		-40, -25, -13, -3, -14, -4, -22, -45,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 64, 87, 79, 45, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -37, -5, -11, 102, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 12, 25, 37, 49, 48, 25, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 19, 30, 44, 27, -4, 0, };
	
	int mg_safe_knight_check = -89;
	int eg_safe_knight_check = -2;
	
	int mg_safe_bishop_check = -25;
	int eg_safe_bishop_check = -17;
	
	int mg_safe_rook_check = -85;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -38;
	int eg_safe_queen_check = -17;
	
	int mg_pawn_shield[6] = { 0, 32, 33, 23, 27, 12, };
	
	int mg_knight_mobility[9] = { -120, -52, -32, -22, -12, -10, 0, 10, 23, };
	int eg_knight_mobility[9] = { -99, -47, -23, -9, -2, 4, 6, 8, 7, };
	
	int mg_bishop_mobility[14] = { -42, -56, -31, -24, -12, -5, 0, 5, 7, 10, 12, 25, 27, 36, };
	int eg_bishop_mobility[14] = { -94, -59, -26, -13, -8, -5, 0, 4, 7, 9, 10, 6, 6, 1, };
	
	int mg_rook_mobility[15] = { -15, -47, -25, -19, -14, -13, -11, -15, -11, -7, -3, -3, -2, 5, 9, };
	int eg_rook_mobility[15] = { -33, -62, -27, -19, -16, -7, -4, 0, 2, 4, 6, 10, 13, 13, 13, };
	
	int mg_queen_mobility[28] = { 0, 0, -18, -51, -50, -18, -12, -11, -10, -10, -7, -5, -3, 0, 1, 0, 1, -3, -1, 1, 7, 16, 29, 27, 21, 37, 1, -11, };
	int eg_queen_mobility[28] = { 0, 0, -13, -64, -45, -39, -30, -20, -11, 1, 2, 5, 8, 8, 9, 12, 11, 15, 14, 11, 9, 0, -5, -5, -6, -9, -18, -17, };
	
	int mg_king_mobility[9] = { 0, -26, -19, -3, -10, 21, -34, -14, 12, };
	int eg_king_mobility[9] = { -1, 4, -10, -1, 5, -5, 8, 8, 0, };
	
	int mg_minor_threatened_by_pawn = -41;
	int eg_minor_threatened_by_pawn = -11;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 0;
	
	int mg_queen_threatened_by_lesser = -12;
	int eg_queen_threatened_by_lesser = -5;
	
	int mg_passed_pawn[8] = { 0, 48, 24, 7, -18, -5, 5, 0, };
	int eg_passed_pawn[8] = { 0, 87, 61, 33, 21, 12, 8, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 27, 24, -2, -29, -10, -4, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 24, 12, 14, 10, 12, 9, 0, };
	
	int mg_passed_pawn_safe_advance = -2;
	int eg_passed_pawn_safe_advance = 10;
	
	int mg_passed_pawn_safe_path = -80;
	int eg_passed_pawn_safe_path = 36;
	
	int mg_passed_friendly_distance[8] = { 0, 0, 6, 11, 13, 5, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -12, -16, -13, -9, -4, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 3, -12, -9, -7, -2, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 34, 29, 16, 7, -1, -1, 0, };
	
	int mg_isolated_penalty = -8;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = -6;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 8;
	int eg_chained_bonus = 7;
	
	int mg_double_bishop = 14;
	int eg_double_bishop = 39;
	
	int mg_rook_open_file = 30;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 12;
	int eg_rook_half_open_file = 5;
	
	int mg_rook_on_seventh = -9;
	int eg_rook_on_seventh = 17;
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 19;
	
	int mg_center_control = 380;
	
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
