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

	int mg_piece_value[6] = { 41, 233, 256, 307, 634, 0, };
	int eg_piece_value[6] = { 68, 263, 272, 475, 904, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 9, 2, 53, 39, 28, -28, -45,
		4, 2, 23, 35, 36, 54, 21, -18,
		-12, -7, 1, 2, 17, 16, -8, -9,
		-14, -11, 2, 11, 13, 6, -6, -20,
		-20, -12, -6, 2, 9, -9, -3, -25,
		-10, -5, -3, -1, 9, -1, 9, -16,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-68, -23, -31, -14, -6, -20, -15, -33,
		-33, -21, -20, -8, -9, 16, -15, -5,
		-18, -12, -5, -5, 15, 38, 4, 5,
		-12, 2, 10, 34, 24, 34, 10, 22,
		4, 6, 14, 22, 23, 24, 27, 15,
		-10, -3, 0, 0, 13, 4, 17, 5,
		-13, -11, -6, 8, 7, 7, 12, 11,
		-36, -4, -21, -10, -3, 4, -2, -12,
	};
	int mg_bishop_psqt[64] = {
		-15, -27, -22, -41, -36, -41, -13, -29,
		-18, -23, -15, -25, -25, -21, -36, -23,
		-6, 8, 0, 4, -3, 29, 10, 2,
		-14, -3, -2, 7, 4, 5, -2, -17,
		-4, -16, -2, 8, 5, 4, -5, 10,
		-2, 10, 7, 7, 10, 9, 15, 19,
		21, 12, 20, 3, 10, 18, 34, 25,
		11, 23, 10, -2, 10, 3, 18, 35,
	};
	int mg_rook_psqt[64] = {
		2, -9, -12, -13, -1, 4, 4, 12,
		-23, -26, -19, -3, -14, 12, 12, 22,
		-31, -2, -10, -5, 26, 24, 53, 21,
		-25, -11, -12, -7, 0, 9, 15, 6,
		-27, -29, -20, -16, -12, -19, 10, -7,
		-25, -21, -18, -16, -7, -1, 30, 9,
		-24, -19, -10, -8, -1, 3, 29, -9,
		-10, -12, -11, -3, 4, 2, 6, 4,
	};
	int mg_queen_psqt[64] = {
		-24, -23, -10, 4, -5, -6, 12, -4,
		-6, -27, -33, -43, -47, -6, -5, 39,
		-1, -10, -16, -10, -4, 15, 30, 34,
		-13, -11, -19, -30, -24, -7, 2, 10,
		-2, -22, -21, -21, -19, -12, 6, 12,
		-5, -2, -14, -13, -9, -1, 19, 14,
		2, 1, 6, 6, 6, 10, 26, 22,
		-6, -12, -6, 6, 2, -8, -2, 0,
	};
	int mg_king_psqt[64] = {
		-3, -1, 0, -2, 0, 0, 3, 1,
		-6, -1, -3, 9, 7, 6, 9, 8,
		-7, 11, 1, -4, 9, 24, 18, 10,
		3, -2, -13, -25, -26, -5, -5, -16,
		-8, -8, -14, -46, -43, -12, -28, -51,
		-5, 21, -6, -15, -7, -18, -5, -31,
		33, -10, -4, -1, -6, -16, -5, 16,
		52, 34, 43, -4, 56, -9, 27, 68,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		9, 8, 26, 12, 18, 14, 28, 23,
		18, 24, 9, -2, 3, 9, 27, 22,
		13, 7, 0, -8, -7, -2, 6, 4,
		4, 1, -5, -9, -8, -5, -3, -3,
		-1, -4, -6, -5, -5, -4, -8, -5,
		1, -1, -3, -3, 1, -1, -6, -4,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-43, -16, -3, -7, 2, -21, -16, -55,
		0, 3, 6, 7, 1, -3, 1, -12,
		0, 3, 7, 12, 7, -4, -3, -6,
		8, 3, 8, 7, 10, 7, 8, 2,
		7, 4, 13, 14, 15, 9, 3, 8,
		-2, -1, 1, 9, 9, -1, -2, 0,
		-3, 0, -3, -1, -2, -4, -5, 7,
		-4, -4, -5, -4, -1, -7, 1, -1,
	};
	int eg_bishop_psqt[64] = {
		0, 3, 0, 2, 2, -5, 0, -10,
		-7, 1, -2, 3, -1, 0, 0, -8,
		6, 0, 2, -3, 2, 5, 4, 7,
		2, 5, 3, 12, 3, 5, 1, 5,
		-1, 2, 5, 5, 5, 0, 1, -7,
		-3, 2, 0, 4, 6, 1, -3, -4,
		1, -10, -8, -4, -1, -5, -6, -8,
		-8, -5, -3, -3, -6, 1, -8, -16,
	};
	int eg_rook_psqt[64] = {
		12, 17, 20, 15, 13, 15, 18, 13,
		6, 13, 15, 7, 8, 8, 8, 1,
		14, 13, 13, 8, 3, 3, 3, -1,
		11, 10, 11, 7, -1, -1, 0, -3,
		2, 3, 1, 0, -2, -3, -8, -9,
		-5, -5, -7, -7, -9, -13, -22, -21,
		-11, -7, -9, -10, -13, -15, -23, -19,
		-8, -9, -7, -11, -14, -11, -13, -16,
	};
	int eg_queen_psqt[64] = {
		2, 1, 9, 11, 10, 15, 0, 5,
		-12, -5, 16, 22, 30, 14, 5, 5,
		-9, -5, 9, 9, 16, 12, -3, -2,
		-1, 5, 7, 18, 17, 6, 17, 2,
		-5, 9, 9, 15, 13, 6, 2, 1,
		-13, -4, 7, 4, 8, -1, -9, -11,
		-14, -14, -13, -7, -6, -19, -33, -30,
		-12, -11, -12, -12, -15, -17, -21, -16,
	};
	int eg_king_psqt[64] = {
		-45, -20, -9, 3, 0, 7, 14, -29,
		-9, 14, 17, 15, 19, 29, 28, 12,
		-2, 15, 20, 25, 31, 31, 27, 6,
		-9, 6, 17, 21, 23, 23, 16, 0,
		-18, -4, 6, 14, 12, 7, 2, -8,
		-22, -10, -3, 1, 0, -2, -5, -12,
		-22, -8, -8, -7, -5, -5, -8, -16,
		-41, -19, -14, -9, -22, -8, -17, -44,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 55, 86, 65, 34, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -32, -6, -6, 103, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 13, 26, 40, 53, 46, 7, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 19, 30, 44, 6, 0, 0, };
	
	int mg_safe_knight_check = -86;
	int eg_safe_knight_check = -2;
	
	int mg_safe_bishop_check = -24;
	int eg_safe_bishop_check = -16;
	
	int mg_safe_rook_check = -82;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -37;
	int eg_safe_queen_check = -16;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 34, 50, 54, 40, 33, 48, 3, 0, },
	        { 60, 54, 26, 29, 18, 12, 0, 0, },
	        { 48, 43, 20, 21, 24, 19, 12, 0, },
	        { 10, 17, 10, 16, 20, -3, 2, 0, },
	},
	{
	        { 0, 65, 61, 37, 12, 15, 1, 0, },
	        { 0, 67, 55, 29, 27, 26, 2, 0, },
	        { 0, 62, 31, 38, 26, 2, -2, 0, },
	        { 0, 16, 22, 30, 20, -22, -2, 0, },
	},
	};
	
	int eg_pawn_shelter[2][4][8] = {
	{
	        { -7, -13, -7, -5, -3, 16, 15, 0, },
	        { -3, -8, -3, -8, -6, 12, 14, 0, },
	        { 2, -3, -1, -7, -10, 0, 18, 0, },
	        { 5, -1, -3, -3, -11, -3, 19, 0, },
	},
	{
	        { 0, -16, -6, -5, 19, 37, 12, 0, },
	        { 0, -9, -4, -10, -10, 5, 9, 0, },
	        { 0, -3, -1, -9, -8, -1, -1, 0, },
	        { 0, 3, 2, -5, -8, 4, 10, 0, },
	},
	};
	
	int mg_knight_mobility[9] = { -60, -43, -20, -11, -1, 1, 11, 21, 33, };
	int eg_knight_mobility[9] = { -49, -35, -14, -1, 6, 11, 13, 15, 15, };
	
	int mg_bishop_mobility[14] = { -46, -54, -29, -23, -11, -4, 0, 5, 8, 11, 12, 25, 25, 27, };
	int eg_bishop_mobility[14] = { -64, -48, -17, -6, -1, 2, 7, 10, 13, 15, 16, 13, 13, 10, };
	
	int mg_rook_mobility[15] = { -2, -31, -33, -26, -20, -19, -17, -22, -17, -13, -8, -8, -7, 0, 5, };
	int eg_rook_mobility[15] = { -5, -49, -22, -15, -12, -3, 0, 3, 4, 7, 9, 12, 15, 15, 15, };
	
	int mg_queen_mobility[28] = { 0, 0, -2, -25, -56, -30, -25, -24, -23, -24, -20, -18, -16, -13, -12, -12, -11, -14, -11, -8, 0, 10, 24, 18, 10, 13, -2, -3, };
	int eg_queen_mobility[28] = { 0, 0, -1, -23, -36, -32, -22, -12, -4, 7, 8, 10, 13, 13, 14, 16, 15, 18, 17, 14, 11, 2, -4, -2, -2, 0, -9, -6, };
	
	int mg_king_mobility[9] = { 0, -7, -35, -24, -11, 18, -3, 17, 43, };
	int eg_king_mobility[9] = { 0, 4, -5, 3, 4, -6, 2, 3, -5, };
	
	int mg_minor_threatened_by_pawn = -40;
	int eg_minor_threatened_by_pawn = -11;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -13;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 0;
	
	int mg_queen_threatened_by_lesser = -1;
	int eg_queen_threatened_by_lesser = -1;
	
	int mg_passed_pawn[8] = { 0, 41, 17, -10, -38, -20, 4, 0, };
	int eg_passed_pawn[8] = { 0, 96, 66, 35, 24, 14, 8, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 17, 18, -18, -48, -24, -5, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 39, 20, 17, 13, 15, 9, 0, };
	
	int mg_passed_pawn_safe_advance = -2;
	int eg_passed_pawn_safe_advance = 9;
	
	int mg_passed_pawn_safe_path = -75;
	int eg_passed_pawn_safe_path = 32;
	
	int mg_passed_friendly_distance[8] = { 0, -1, 5, 13, 13, 5, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -14, -16, -13, -8, -3, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 8, -9, -7, -3, 1, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 30, 26, 14, 5, -1, -1, 0, };
	
	int mg_isolated_penalty = -6;
	int eg_isolated_penalty = -6;
	
	int mg_doubled_penalty = 5;
	int eg_doubled_penalty = -10;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 9;
	int eg_chained_bonus = 6;
	
	int mg_double_bishop = 15;
	int eg_double_bishop = 35;
	
	int mg_rook_open_file = 28;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 10;
	int eg_rook_half_open_file = 5;
	
	int mg_rook_on_seventh = -2;
	int eg_rook_on_seventh = 15;
	
	int mg_knight_outpost = 20;
	int eg_knight_outpost = -3;
	
	int mg_knight_outpost_supported = 38;
	int eg_knight_outpost_supported = 17;
	
	int mg_center_control = 350;
	
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
