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

	int mg_piece_value[6] = { 39, 235, 258, 307, 632, 0, };
	int eg_piece_value[6] = { 65, 263, 273, 474, 903, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		-2, 10, 3, 52, 39, 28, -26, -46,
		7, 4, 25, 39, 39, 57, 23, -16,
		-12, -7, -2, 0, 17, 15, -6, -10,
		-15, -13, -2, 5, 9, 1, -8, -22,
		-20, -15, -8, -2, 7, -13, -5, -26,
		-8, -2, 0, 2, 12, 3, 12, -14,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-68, -23, -30, -12, -6, -18, -15, -34,
		-33, -20, -19, -5, -9, 17, -15, -6,
		-17, -11, -4, -3, 16, 39, 4, 5,
		-12, 2, 11, 34, 24, 34, 9, 21,
		3, 7, 13, 21, 23, 23, 27, 14,
		-11, -3, 0, 0, 13, 4, 17, 5,
		-14, -11, -6, 8, 6, 6, 11, 10,
		-36, -5, -22, -11, -3, 3, -3, -14,
	};
	int mg_bishop_psqt[64] = {
		-14, -26, -20, -39, -35, -37, -12, -29,
		-18, -23, -15, -26, -25, -22, -36, -24,
		-7, 8, 0, 5, -3, 28, 9, 1,
		-13, -3, -3, 6, 4, 5, -2, -17,
		-4, -16, -2, 7, 5, 4, -5, 10,
		-1, 10, 6, 6, 10, 9, 14, 19,
		23, 12, 19, 3, 9, 18, 33, 27,
		11, 23, 10, -2, 11, 3, 18, 35,
	};
	int mg_rook_psqt[64] = {
		3, -9, -12, -13, -1, 3, 4, 12,
		-24, -26, -20, -4, -15, 11, 12, 21,
		-31, -2, -10, -5, 24, 24, 53, 21,
		-25, -11, -11, -7, 0, 10, 16, 7,
		-26, -29, -19, -16, -12, -18, 11, -6,
		-26, -21, -18, -16, -7, -1, 30, 9,
		-24, -19, -9, -7, 0, 3, 28, -9,
		-9, -12, -11, -3, 4, 2, 7, 4,
	};
	int mg_queen_psqt[64] = {
		-25, -24, -10, 3, -5, -6, 12, -5,
		-6, -28, -35, -45, -48, -7, -5, 38,
		0, -10, -17, -11, -5, 14, 28, 33,
		-12, -11, -19, -30, -25, -6, 2, 10,
		-1, -21, -20, -20, -18, -11, 6, 13,
		-4, -1, -14, -13, -9, 0, 19, 15,
		2, 1, 6, 7, 7, 11, 26, 22,
		-6, -12, -5, 7, 3, -7, -2, 1,
	};
	int mg_king_psqt[64] = {
		-3, -1, -1, -2, 0, -1, 3, 0,
		-6, 0, -3, 9, 7, 6, 9, 8,
		-7, 12, 1, -3, 10, 25, 18, 10,
		3, -2, -13, -24, -24, -4, -4, -17,
		-8, -8, -12, -45, -41, -10, -27, -50,
		-7, 22, -5, -14, -5, -17, -6, -33,
		29, -11, -4, -1, -7, -16, -6, 11,
		55, 32, 41, -7, 53, -10, 25, 72,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		8, 5, 24, 11, 17, 14, 27, 24,
		14, 17, 3, -9, -3, 2, 20, 18,
		13, 5, -3, -11, -9, -5, 3, 3,
		5, 2, -4, -8, -7, -4, -1, -2,
		1, -2, -4, -3, -3, -3, -5, -4,
		5, 5, 1, 1, 5, 3, 1, -1,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-43, -16, -3, -6, 2, -20, -16, -55,
		0, 3, 6, 8, 2, -3, 1, -12,
		-1, 3, 8, 12, 7, -3, -3, -6,
		8, 3, 8, 7, 10, 7, 8, 2,
		7, 3, 13, 14, 16, 9, 3, 7,
		-2, 0, 1, 10, 9, -1, -2, 0,
		-4, 0, -3, -1, -2, -4, -5, 6,
		-6, -4, -5, -4, -1, -8, 0, -1,
	};
	int eg_bishop_psqt[64] = {
		0, 4, 0, 2, 2, -5, 0, -10,
		-6, 1, -2, 3, -1, 0, 1, -8,
		6, 1, 3, -2, 2, 5, 3, 7,
		1, 5, 3, 13, 3, 5, 1, 5,
		-1, 1, 5, 6, 6, 0, 1, -8,
		-3, 1, 0, 3, 5, 1, -3, -4,
		0, -9, -8, -4, -1, -5, -6, -8,
		-9, -5, -3, -4, -6, 1, -6, -16,
	};
	int eg_rook_psqt[64] = {
		11, 16, 19, 15, 13, 14, 18, 13,
		6, 12, 15, 7, 7, 7, 7, 0,
		14, 13, 13, 9, 3, 4, 2, -1,
		11, 10, 11, 7, 0, -1, 1, -3,
		2, 3, 1, 0, -2, -3, -8, -9,
		-5, -5, -7, -7, -9, -13, -22, -21,
		-11, -7, -9, -10, -13, -15, -22, -18,
		-7, -9, -7, -11, -14, -11, -13, -16,
	};
	int eg_queen_psqt[64] = {
		2, 1, 9, 11, 10, 15, 0, 4,
		-12, -5, 16, 22, 30, 14, 5, 5,
		-9, -5, 9, 10, 16, 13, -2, -2,
		-2, 5, 7, 18, 17, 6, 17, 2,
		-5, 9, 9, 15, 13, 6, 3, 1,
		-14, -4, 6, 3, 8, -1, -9, -11,
		-14, -14, -13, -7, -6, -18, -33, -30,
		-12, -11, -12, -12, -15, -17, -21, -16,
	};
	int eg_king_psqt[64] = {
		-47, -21, -9, 2, -1, 7, 14, -31,
		-10, 15, 18, 16, 20, 30, 30, 12,
		-3, 16, 21, 26, 32, 32, 27, 5,
		-10, 7, 18, 22, 24, 24, 16, -1,
		-20, -3, 7, 15, 13, 8, 3, -9,
		-23, -9, -2, 1, 1, -1, -5, -13,
		-23, -8, -7, -6, -4, -5, -8, -17,
		-46, -20, -15, -10, -22, -9, -18, -49,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 54, 85, 66, 35, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -31, -7, -6, 102, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 13, 26, 40, 53, 46, 7, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 19, 30, 44, 6, 0, 0, };
	
	int mg_safe_knight_check = -86;
	int eg_safe_knight_check = -2;
	
	int mg_safe_bishop_check = -24;
	int eg_safe_bishop_check = -16;
	
	int mg_safe_rook_check = -81;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -37;
	int eg_safe_queen_check = -16;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 37, 50, 54, 41, 35, 48, 4, 0, },
	        { 63, 53, 27, 28, 17, 12, 0, 0, },
	        { 50, 42, 21, 22, 23, 19, 10, 0, },
	        { 12, 17, 11, 17, 20, -4, 2, 0, },
	},
	{
	        { 0, 63, 61, 38, 12, 14, 1, 0, },
	        { 0, 67, 58, 31, 27, 26, 3, 0, },
	        { 0, 62, 32, 39, 25, 2, -2, 0, },
	        { 0, 17, 23, 31, 21, -22, -2, 0, },
	},
	};
	
	int eg_pawn_shelter[2][4][8] = {
	{
	        { -7, -13, -7, -4, -2, 18, 18, 0, },
	        { -2, -8, -2, -7, -5, 11, 12, 0, },
	        { 2, -3, -1, -7, -10, 2, 18, 0, },
	        { 5, -2, -4, -4, -10, -2, 20, 0, },
	},
	{
	        { 0, -15, -5, -4, 20, 37, 12, 0, },
	        { 0, -9, -3, -8, -9, 5, 8, 0, },
	        { 0, -3, 0, -9, -8, -2, -2, 0, },
	        { 0, 2, 2, -5, -8, 2, 10, 0, },
	},
	};
	
	int mg_knight_mobility[9] = { -55, -43, -19, -11, -1, 0, 10, 20, 31, };
	int eg_knight_mobility[9] = { -43, -35, -14, -1, 5, 10, 13, 15, 13, };
	
	int mg_bishop_mobility[14] = { -42, -56, -30, -24, -12, -4, 0, 5, 8, 11, 12, 25, 26, 26, };
	int eg_bishop_mobility[14] = { -53, -46, -19, -7, -2, 1, 6, 9, 12, 14, 15, 12, 12, 9, };
	
	int mg_rook_mobility[15] = { -2, -30, -33, -27, -20, -19, -17, -22, -18, -13, -9, -8, -7, 0, 5, };
	int eg_rook_mobility[15] = { -4, -47, -22, -15, -13, -3, -1, 3, 4, 6, 8, 12, 15, 15, 15, };
	
	int mg_queen_mobility[28] = { 0, 0, -2, -25, -56, -31, -25, -24, -23, -24, -21, -19, -16, -13, -12, -12, -11, -14, -11, -8, 0, 11, 25, 18, 10, 13, -2, -3, };
	int eg_queen_mobility[28] = { 0, 0, -1, -24, -37, -31, -21, -12, -4, 7, 7, 10, 13, 12, 13, 16, 15, 18, 17, 14, 11, 2, -3, -1, -2, 0, -9, -6, };
	
	int mg_king_mobility[9] = { 0, -3, -41, -23, -9, 21, -5, 16, 43, };
	int eg_king_mobility[9] = { 1, 22, 1, 3, -1, -9, -3, -3, -10, };
	
	int mg_minor_threatened_by_pawn = -40;
	int eg_minor_threatened_by_pawn = -11;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -13;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 0;
	
	int mg_queen_threatened_by_lesser = -1;
	int eg_queen_threatened_by_lesser = -1;
	
	int mg_minor_threatened_by_major = -9;
	int eg_minor_threatened_by_major = -10;
	
	int mg_passed_pawn[8] = { 0, 41, 18, -9, -34, -20, 2, 0, };
	int eg_passed_pawn[8] = { 0, 92, 63, 34, 23, 14, 5, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 17, 20, -17, -44, -23, -6, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 37, 17, 17, 13, 14, 5, 0, };
	
	int mg_passed_pawn_safe_advance = -3;
	int eg_passed_pawn_safe_advance = 9;
	
	int mg_passed_pawn_safe_path = -76;
	int eg_passed_pawn_safe_path = 33;
	
	int mg_passed_friendly_distance[8] = { 0, -1, 5, 13, 13, 5, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -13, -16, -12, -8, -4, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 7, -10, -7, -3, 1, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 30, 27, 14, 5, -1, -1, 0, };
	
	int mg_isolated_penalty = -4;
	int eg_isolated_penalty = -4;
	
	int mg_doubled_penalty = 6;
	int eg_doubled_penalty = -12;
	
	int mg_backward_penalty = -5;
	int eg_backward_penalty = -3;
	
	int mg_chained_bonus[8] = { 0, 41, 11, 15, 14, 12, 2, 0, };
	int eg_chained_bonus[8] = { 0, 68, 41, 14, 7, 6, -1, 0, };
	
	int mg_double_bishop = 15;
	int eg_double_bishop = 35;
	
	int mg_rook_open_file = 28;
	int eg_rook_open_file = 4;
	
	int mg_rook_half_open_file = 10;
	int eg_rook_half_open_file = 5;
	
	int mg_rook_on_seventh = -1;
	int eg_rook_on_seventh = 15;
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -3;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 17;
	
	int mg_center_control = 334;
	
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
