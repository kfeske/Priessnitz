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

	int mg_piece_value[6] = { 35, 260, 276, 320, 694, 0, };
	int eg_piece_value[6] = { 62, 291, 298, 507, 949, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		11, 22, 10, 69, 55, 39, -20, -43,
		12, 9, 28, 39, 42, 60, 36, -7,
		-3, 0, 7, 8, 24, 25, 3, 3,
		-5, -4, 9, 17, 19, 14, 3, -8,
		-12, -5, 1, 8, 15, -2, 7, -13,
		-2, 2, 4, 5, 15, 7, 19, -4,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-89, -71, -67, -33, -20, -48, -58, -51,
		-40, -27, -25, -13, -15, 12, -22, -13,
		-25, -18, -12, -12, 10, 32, -2, 0,
		-19, -6, 3, 27, 16, 26, 2, 15,
		-4, -2, 5, 13, 15, 15, 19, 7,
		-19, -11, -9, -8, 5, -5, 9, -3,
		-22, -19, -15, 0, -2, -1, 4, 2,
		-49, -12, -29, -18, -11, -4, -10, -20,
	};
	int mg_bishop_psqt[64] = {
		-28, -54, -48, -76, -72, -70, -37, -52,
		-28, -33, -24, -38, -34, -29, -46, -33,
		-15, 0, -9, -4, -11, 21, 3, -5,
		-22, -12, -11, -2, -5, -4, -11, -25,
		-12, -25, -12, -1, -4, -5, -14, 1,
		-11, 1, -2, -2, 1, 0, 5, 10,
		12, 3, 11, -6, 1, 9, 25, 16,
		3, 15, 2, -10, 2, -6, 11, 26,
	};
	int mg_rook_psqt[64] = {
		0, -13, -16, -18, -4, 3, 4, 13,
		-21, -24, -17, -1, -13, 11, 11, 22,
		-34, -5, -14, -9, 22, 20, 54, 19,
		-28, -15, -16, -11, -4, 5, 12, 3,
		-31, -33, -24, -21, -17, -23, 6, -10,
		-30, -25, -22, -20, -11, -6, 26, 5,
		-27, -23, -14, -12, -5, -1, 24, -13,
		-14, -16, -16, -8, 0, -2, 3, -1,
	};
	int mg_queen_psqt[64] = {
		-28, -31, -13, 5, -7, -12, 16, -7,
		-8, -28, -32, -45, -49, -8, -9, 34,
		-1, -10, -16, -11, -4, 13, 28, 32,
		-13, -11, -19, -30, -23, -7, 2, 9,
		-2, -21, -20, -20, -18, -12, 5, 11,
		-5, -1, -13, -12, -9, 0, 19, 13,
		2, 2, 6, 6, 7, 11, 25, 23,
		-4, -12, -5, 6, 3, -8, 0, 4,
	};
	int mg_king_psqt[64] = {
		-3, -4, -5, -16, -5, -11, 6, 16,
		-34, -8, -17, 27, 20, 14, 26, 27,
		-36, 25, -4, -17, 17, 58, 41, 20,
		-7, -11, -33, -55, -56, -18, -19, -52,
		-28, -16, -26, -68, -61, -22, -40, -87,
		-21, 23, -11, -23, -15, -22, -5, -51,
		20, -4, -4, -8, -13, -18, 0, -1,
		20, 17, 19, -35, 25, -34, 9, 33,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		16, 17, 39, 22, 29, 26, 40, 35,
		30, 37, 22, 11, 16, 22, 39, 33,
		25, 19, 11, 2, 4, 8, 16, 14,
		15, 12, 5, 1, 2, 6, 7, 6,
		10, 7, 5, 6, 6, 6, 2, 4,
		12, 10, 8, 8, 12, 10, 4, 5,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-44, -14, -1, -8, 0, -21, -16, -58,
		-3, -1, 2, 4, -2, -7, -3, -15,
		-4, -1, 4, 9, 3, -8, -7, -11,
		5, 0, 5, 4, 7, 4, 6, -1,
		5, 1, 11, 12, 14, 7, 0, 6,
		-5, -3, -2, 7, 7, -4, -5, -2,
		-6, -3, -6, -4, -5, -7, -8, 5,
		-5, -7, -8, -7, -4, -11, -2, -3,
	};
	int eg_bishop_psqt[64] = {
		0, 5, 2, 6, 5, -3, 1, -8,
		-8, 0, -3, 3, -3, -1, -1, -9,
		6, -1, 1, -4, 1, 3, 2, 6,
		1, 4, 2, 12, 3, 4, 0, 4,
		-2, 1, 4, 5, 5, -1, 0, -9,
		-4, 1, 0, 3, 6, 1, -5, -6,
		-1, -11, -10, -5, -2, -6, -7, -9,
		-10, -6, -4, -5, -8, 0, -10, -18,
	};
	int eg_rook_psqt[64] = {
		15, 21, 24, 19, 17, 18, 22, 16,
		8, 15, 17, 9, 10, 10, 11, 3,
		17, 16, 16, 12, 6, 6, 4, 1,
		15, 13, 14, 10, 2, 2, 3, -1,
		5, 6, 4, 3, 1, 0, -6, -7,
		-3, -3, -5, -5, -7, -12, -21, -20,
		-9, -4, -7, -8, -11, -13, -22, -17,
		-6, -7, -5, -9, -12, -9, -11, -15,
	};
	int eg_queen_psqt[64] = {
		7, 7, 13, 13, 15, 20, 0, 8,
		-9, -2, 18, 26, 36, 18, 9, 11,
		-7, -3, 11, 12, 19, 16, 0, 2,
		1, 8, 9, 21, 19, 9, 20, 5,
		-3, 11, 11, 18, 15, 8, 5, 4,
		-12, -2, 8, 6, 10, 0, -8, -9,
		-13, -13, -12, -6, -4, -19, -32, -31,
		-12, -10, -11, -12, -15, -17, -21, -17,
	};
	int eg_king_psqt[64] = {
		-45, -18, -5, 9, 4, 13, 20, -36,
		-2, 18, 22, 16, 21, 33, 32, 15,
		5, 18, 25, 31, 35, 33, 30, 11,
		-5, 10, 23, 27, 30, 29, 21, 8,
		-14, -2, 10, 19, 17, 11, 6, -1,
		-20, -10, -1, 4, 3, 0, -5, -8,
		-22, -9, -7, -5, -3, -4, -9, -14,
		-36, -18, -11, -3, -16, -3, -16, -39,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 60, 92, 74, 42, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -34, -6, -8, 104, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 12, 24, 36, 48, 47, 26, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 19, 30, 45, 28, -3, 0, };
	
	int mg_safe_knight_check = -87;
	int eg_safe_knight_check = -2;
	
	int mg_safe_bishop_check = -24;
	int eg_safe_bishop_check = -17;
	
	int mg_safe_rook_check = -82;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -37;
	int eg_safe_queen_check = -18;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 25, 43, 47, 33, 25, 43, 6, 0, },
	        { 55, 51, 22, 25, 13, 16, -9, 0, },
	        { 46, 42, 19, 20, 23, 20, 14, 0, },
	        { 10, 18, 11, 17, 21, -2, -5, 0, },
	},
	{
	        { 0, 55, 51, 26, 3, 16, 1, 0, },
	        { 0, 63, 51, 25, 21, 30, 6, 0, },
	        { 0, 61, 30, 36, 22, 2, -13, 0, },
	        { 0, 17, 24, 31, 22, -28, -17, 0, },
	},
	};
	
	int eg_pawn_shelter[2][4][8] = {
	{
	        { -5, -11, -5, -2, 1, 20, 25, 0, },
	        { -2, -7, -1, -7, -4, 11, 25, 0, },
	        { 2, -3, -1, -7, -11, -1, 16, 0, },
	        { 5, -2, -4, -4, -12, -4, 22, 0, },
	},
	{
	        { 0, -14, -3, -1, 23, 43, 35, 0, },
	        { 0, -8, -3, -9, -8, 3, 16, 0, },
	        { 0, -3, 0, -9, -7, -1, -3, 0, },
	        { 0, 3, 2, -6, -9, 6, 23, 0, },
	},
	};
	
	int mg_knight_mobility[9] = { -120, -53, -30, -22, -12, -10, 1, 11, 23, };
	int eg_knight_mobility[9] = { -100, -46, -23, -9, -2, 4, 6, 8, 7, };
	
	int mg_bishop_mobility[14] = { -43, -55, -30, -24, -12, -5, 0, 5, 8, 11, 12, 25, 27, 35, };
	int eg_bishop_mobility[14] = { -94, -59, -26, -13, -8, -5, 0, 4, 7, 9, 10, 7, 7, 2, };
	
	int mg_rook_mobility[15] = { -14, -49, -28, -21, -14, -13, -11, -15, -11, -7, -2, -2, -1, 6, 11, };
	int eg_rook_mobility[15] = { -31, -61, -27, -19, -17, -6, -4, 1, 1, 4, 6, 10, 13, 13, 13, };
	
	int mg_queen_mobility[28] = { 0, 0, -19, -50, -50, -18, -11, -10, -9, -10, -7, -5, -3, 0, 0, 0, 1, -3, -1, 2, 8, 17, 30, 28, 21, 38, 1, -11, };
	int eg_queen_mobility[28] = { 0, 0, -13, -64, -45, -39, -30, -21, -12, 0, 2, 5, 8, 8, 9, 12, 11, 15, 14, 12, 10, 0, -5, -5, -6, -9, -16, -16, };
	
	int mg_king_mobility[9] = { 0, -24, -19, -9, -12, 16, -24, -6, 19, };
	int eg_king_mobility[9] = { -2, 3, -10, -1, 5, -4, 7, 8, 0, };
	
	int mg_minor_threatened_by_pawn = -41;
	int eg_minor_threatened_by_pawn = -11;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 0;
	
	int mg_queen_threatened_by_lesser = -12;
	int eg_queen_threatened_by_lesser = -4;
	
	int mg_passed_pawn[8] = { 0, 48, 23, 7, -18, -5, 5, 0, };
	int eg_passed_pawn[8] = { 0, 88, 63, 33, 21, 12, 7, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 25, 24, -2, -29, -10, -4, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 26, 14, 14, 10, 12, 8, 0, };
	
	int mg_passed_pawn_safe_advance = -3;
	int eg_passed_pawn_safe_advance = 10;
	
	int mg_passed_pawn_safe_path = -78;
	int eg_passed_pawn_safe_path = 35;
	
	int mg_passed_friendly_distance[8] = { 0, -1, 5, 10, 13, 5, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -13, -16, -13, -9, -4, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 5, -11, -8, -7, -2, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 34, 29, 16, 7, 0, -1, 0, };
	
	int mg_isolated_penalty = -7;
	int eg_isolated_penalty = -7;
	
	int mg_doubled_penalty = 5;
	int eg_doubled_penalty = -11;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 9;
	int eg_chained_bonus = 6;
	
	int mg_double_bishop = 14;
	int eg_double_bishop = 39;
	
	int mg_rook_open_file = 28;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 10;
	int eg_rook_half_open_file = 5;
	
	int mg_rook_on_seventh = -8;
	int eg_rook_on_seventh = 17;
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 19;
	
	int mg_center_control = 384;
	
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
