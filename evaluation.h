#pragma once

#include "utility.h"
#include "board.h"
#include "transposition_table.h"

struct Eval_info : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	int mg_king_ring_pressure[2];
	int eg_king_ring_pressure[2];
	int king_ring_attackers[2];
	uint64_t passed_pawns;

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
		passed_pawns = 0ULL;
		attacked[WHITE] = attacked_by_piece[WHITE][KING] = piece_attacks(KING, white_king_square, 0ULL);
		attacked[BLACK] = attacked_by_piece[BLACK][KING] = piece_attacks(KING, black_king_square, 0ULL);
		uint64_t white_pawn_attacks = board.all_pawn_attacks(WHITE);
		attacked[WHITE]	 	      |= white_pawn_attacks;
		attacked_by_piece[WHITE][PAWN] = white_pawn_attacks;
		uint64_t black_pawn_attacks = board.all_pawn_attacks(BLACK);
		attacked[BLACK]	 	      |= black_pawn_attacks;
		attacked_by_piece[BLACK][PAWN] = black_pawn_attacks;
	}
};

struct Evaluation : Noncopyable
{
	Eval_info info {};
	Pawn_hash_table pawn_hash_table;

	int mg_piece_value[6] = { 40, 233, 256, 306, 634, 0, };
	int eg_piece_value[6] = { 69, 263, 272, 475, 904, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 8, 3, 52, 39, 27, -28, -44,
		3, -1, 21, 34, 35, 54, 20, -18,
		-12, -7, 1, 3, 19, 18, -5, -10,
		-13, -11, 3, 11, 14, 7, -5, -19,
		-17, -8, -3, 3, 13, -7, 1, -23,
		-14, -10, -6, -2, 4, -3, 1, -19,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-67, -22, -29, -11, -5, -18, -15, -33,
		-33, -21, -20, -7, -9, 16, -15, -5,
		-18, -13, -6, -5, 14, 37, 1, 4,
		-12, 2, 10, 33, 23, 34, 9, 22,
		4, 7, 13, 21, 23, 23, 27, 16,
		-10, -3, 0, 0, 14, 4, 17, 6,
		-12, -11, -6, 8, 6, 6, 11, 12,
		-35, -4, -22, -11, -3, 3, -2, -12,
	};
	int mg_bishop_psqt[64] = {
		-14, -25, -20, -39, -34, -36, -12, -28,
		-19, -25, -15, -24, -25, -22, -37, -24,
		-7, 7, -1, 4, -3, 27, 9, 1,
		-14, -3, -2, 6, 4, 5, -2, -17,
		-4, -16, -3, 7, 5, 3, -5, 10,
		-1, 10, 6, 7, 10, 9, 14, 20,
		24, 11, 19, 3, 9, 19, 33, 28,
		10, 24, 10, -2, 11, 4, 19, 35,
	};
	int mg_rook_psqt[64] = {
		3, -9, -12, -13, -1, 4, 5, 12,
		-22, -26, -19, -3, -15, 13, 12, 23,
		-30, -3, -10, -6, 24, 24, 52, 22,
		-25, -13, -12, -7, 0, 9, 14, 5,
		-26, -29, -20, -16, -12, -18, 10, -7,
		-25, -21, -17, -16, -7, 0, 31, 9,
		-24, -20, -10, -7, -1, 3, 27, -9,
		-9, -12, -11, -3, 4, 3, 7, 4,
	};
	int mg_queen_psqt[64] = {
		-25, -23, -10, 4, -5, -6, 12, -5,
		-6, -30, -35, -44, -48, -6, -8, 38,
		-2, -10, -18, -11, -5, 12, 29, 33,
		-13, -12, -19, -30, -24, -6, 1, 10,
		-1, -21, -20, -20, -18, -11, 7, 13,
		-4, -1, -14, -12, -9, 0, 20, 15,
		2, 1, 6, 7, 7, 11, 25, 22,
		-6, -12, -4, 7, 3, -7, -2, 1,
	};
	int mg_king_psqt[64] = {
		-3, -1, -1, -2, 0, 0, 3, 0,
		-6, 0, -3, 9, 7, 6, 9, 8,
		-7, 11, 1, -4, 9, 25, 18, 10,
		3, -2, -13, -25, -26, -5, -5, -17,
		-8, -8, -14, -46, -43, -13, -29, -52,
		-7, 22, -6, -15, -6, -18, -5, -35,
		30, -10, -5, -1, -7, -16, -6, 11,
		55, 36, 44, -4, 55, -6, 30, 72,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		9, 9, 26, 11, 18, 14, 28, 24,
		17, 22, 8, -4, 1, 8, 25, 20,
		14, 7, 0, -8, -6, -2, 6, 4,
		4, 1, -5, -8, -8, -4, -3, -3,
		1, -2, -5, -4, -4, -3, -6, -4,
		0, -3, -4, -3, 0, -2, -8, -5,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-42, -15, -3, -6, 3, -20, -16, -54,
		0, 3, 5, 7, 1, -3, 1, -12,
		-1, 2, 7, 12, 6, -4, -4, -6,
		8, 2, 7, 7, 9, 6, 8, 2,
		8, 3, 12, 14, 15, 8, 3, 8,
		-1, -1, 0, 9, 9, -2, -2, 1,
		-3, 0, -3, -1, -2, -3, -4, 7,
		-4, -4, -4, -3, -1, -7, 1, 0,
	};
	int eg_bishop_psqt[64] = {
		1, 4, 1, 2, 2, -5, 0, -10,
		-7, 1, -2, 2, -2, 0, 0, -8,
		7, 0, 2, -3, 2, 4, 3, 6,
		2, 5, 2, 12, 3, 4, 1, 5,
		-1, 2, 4, 5, 5, -1, 1, -7,
		-2, 2, 1, 3, 6, 1, -3, -3,
		1, -9, -8, -4, -1, -5, -5, -7,
		-7, -5, -2, -3, -6, 1, -8, -15,
	};
	int eg_rook_psqt[64] = {
		11, 16, 19, 15, 13, 14, 18, 13,
		6, 13, 15, 7, 8, 8, 8, 1,
		14, 13, 13, 8, 3, 3, 3, -1,
		11, 10, 10, 7, -1, -1, 1, -3,
		2, 3, 1, 0, -2, -3, -8, -9,
		-5, -5, -8, -7, -10, -13, -22, -21,
		-11, -7, -9, -10, -12, -14, -22, -19,
		-8, -9, -7, -11, -14, -11, -13, -16,
	};
	int eg_queen_psqt[64] = {
		2, 1, 8, 11, 10, 14, 0, 4,
		-12, -4, 16, 22, 31, 14, 6, 6,
		-8, -5, 9, 9, 16, 12, -3, -2,
		-2, 5, 7, 18, 17, 5, 16, 2,
		-5, 8, 9, 15, 13, 6, 2, 1,
		-14, -4, 6, 4, 7, -1, -9, -11,
		-14, -14, -13, -7, -5, -18, -34, -31,
		-11, -11, -12, -12, -15, -17, -20, -15,
	};
	int eg_king_psqt[64] = {
		-45, -21, -9, 2, -1, 7, 14, -29,
		-9, 14, 17, 15, 19, 29, 28, 11,
		-2, 16, 20, 25, 31, 31, 26, 5,
		-10, 6, 17, 21, 23, 23, 16, 0,
		-18, -4, 7, 14, 12, 7, 3, -8,
		-22, -9, -2, 1, 0, -1, -5, -12,
		-22, -8, -7, -7, -5, -5, -7, -15,
		-42, -18, -14, -10, -22, -8, -16, -44,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 54, 86, 65, 35, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -31, -6, -5, 103, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 13, 26, 40, 53, 45, 7, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 19, 30, 44, 6, 0, 0, };
	
	int mg_safe_knight_check = -86;
	int eg_safe_knight_check = -2;
	
	int mg_safe_bishop_check = -24;
	int eg_safe_bishop_check = -16;
	
	int mg_safe_rook_check = -82;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -36;
	int eg_safe_queen_check = -17;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 41, 49, 54, 40, 33, 47, 3, 0, },
	        { 67, 52, 27, 27, 16, 11, 0, 0, },
	        { 50, 41, 20, 20, 23, 18, 12, 0, },
	        { 15, 17, 11, 17, 20, -3, 2, 0, },
	},
	{
	        { 0, 62, 62, 36, 12, 14, 1, 0, },
	        { 0, 65, 57, 28, 25, 25, 2, 0, },
	        { 0, 60, 32, 37, 26, 2, -2, 0, },
	        { 0, 18, 23, 30, 21, -23, -2, 0, },
	},
	};
	
	int eg_pawn_shelter[2][4][8] = {
	{
	        { -6, -14, -6, -5, -3, 16, 15, 0, },
	        { -2, -9, -2, -8, -6, 12, 14, 0, },
	        { 2, -4, -1, -7, -10, 0, 18, 0, },
	        { 4, -2, -3, -3, -11, -3, 19, 0, },
	},
	{
	        { 0, -16, -5, -5, 18, 37, 12, 0, },
	        { 0, -11, -3, -10, -11, 4, 8, 0, },
	        { 0, -4, 0, -9, -8, -1, -1, 0, },
	        { 0, 2, 3, -5, -8, 4, 10, 0, },
	},
	};
	
	int mg_knight_mobility[9] = { -60, -44, -19, -11, -1, 1, 11, 21, 32, };
	int eg_knight_mobility[9] = { -50, -35, -14, -1, 6, 12, 14, 16, 15, };
	
	int mg_bishop_mobility[14] = { -49, -57, -29, -23, -11, -3, 1, 6, 9, 11, 12, 24, 26, 27, };
	int eg_bishop_mobility[14] = { -65, -50, -19, -6, -1, 2, 7, 10, 14, 16, 17, 13, 14, 11, };
	
	int mg_rook_mobility[15] = { -2, -32, -33, -27, -20, -19, -17, -22, -18, -13, -8, -8, -7, -1, 5, };
	int eg_rook_mobility[15] = { -5, -50, -23, -15, -12, -3, 0, 3, 4, 7, 9, 12, 15, 16, 16, };
	
	int mg_queen_mobility[28] = { 0, 0, -2, -26, -56, -31, -25, -24, -23, -23, -20, -18, -16, -13, -12, -12, -11, -14, -12, -9, 0, 10, 24, 18, 10, 14, -2, -3, };
	int eg_queen_mobility[28] = { 0, 0, -1, -25, -37, -30, -22, -13, -5, 6, 7, 10, 13, 13, 14, 16, 15, 18, 17, 14, 12, 2, -3, -2, -2, 0, -9, -6, };
	
	int mg_king_mobility[9] = { 0, -7, -35, -22, -10, 19, -4, 16, 42, };
	int eg_king_mobility[9] = { 0, 4, -3, 3, 4, -6, 2, 2, -6, };
	
	int mg_minor_threatened_by_pawn = -39;
	int eg_minor_threatened_by_pawn = -10;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -13;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 1;
	
	int mg_queen_threatened_by_lesser = -1;
	int eg_queen_threatened_by_lesser = -1;
	
	int mg_minor_threatened_by_major = -9;
	int eg_minor_threatened_by_major = -10;
	
	int mg_passed_pawn[8] = { 0, 40, 18, -11, -38, -22, 3, 0, };
	int eg_passed_pawn[8] = { 0, 97, 69, 36, 24, 14, 9, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 18, 20, -20, -49, -27, -6, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 40, 23, 18, 13, 14, 10, 0, };
	
	int mg_passed_pawn_safe_advance = -2;
	int eg_passed_pawn_safe_advance = 9;
	
	int mg_passed_pawn_safe_path = -75;
	int eg_passed_pawn_safe_path = 32;
	
	int mg_passed_friendly_distance[8] = { 0, -2, 5, 13, 13, 5, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -14, -16, -13, -8, -4, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 8, -9, -7, -2, 1, 0, 0, };
	int eg_passed_enemy_distance[8] = { 0, 29, 26, 14, 5, -1, -1, 0, };
	
	int mg_isolated_penalty = -6;
	int eg_isolated_penalty = -7;
	
	int mg_doubled_penalty = 5;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -6;
	int eg_backward_penalty = -4;
	
	int mg_chained_bonus = 6;
	int eg_chained_bonus = 3;
	
	int mg_double_bishop = 15;
	int eg_double_bishop = 36;
	
	int mg_rook_open_file = 28;
	int eg_rook_open_file = 4;
	
	int mg_rook_half_open_file = 10;
	int eg_rook_half_open_file = 5;
	
	int mg_rook_on_seventh = -2;
	int eg_rook_on_seventh = 15;
	
	int mg_knight_outpost = 20;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 17;
	
	int mg_center_control = 339;
	
	int pawn_count_scale_offset = 71;
	int pawn_count_scale_weight = 31;

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
