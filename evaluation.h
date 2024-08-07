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

	int mg_piece_value[6] = { 32, 254, 273, 333, 705, 0, };
	int eg_piece_value[6] = { 58, 288, 299, 516, 960, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		3, 14, 5, 57, 43, 31, -23, -44,
		16, 13, 33, 44, 46, 64, 36, -6,
		0, 3, 11, 12, 28, 27, 4, 3,
		-3, -1, 13, 21, 23, 17, 5, -8,
		-9, -1, 4, 12, 19, 1, 8, -14,
		1, 6, 7, 9, 19, 10, 21, -4,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-78, -28, -37, -17, -9, -25, -19, -40,
		-37, -24, -23, -10, -12, 14, -18, -8,
		-22, -16, -9, -9, 12, 35, 1, 2,
		-16, -3, 6, 30, 19, 29, 5, 17,
		-1, 1, 8, 16, 18, 18, 22, 10,
		-16, -8, -6, -6, 8, -2, 12, 0,
		-19, -16, -12, 3, 1, 1, 7, 5,
		-41, -9, -27, -15, -8, -2, -7, -17,
	};
	int mg_bishop_psqt[64] = {
		-21, -33, -28, -48, -42, -47, -16, -35,
		-25, -29, -20, -30, -30, -26, -41, -30,
		-12, 3, -6, -1, -8, 24, 6, -3,
		-19, -9, -8, 2, -1, -1, -7, -22,
		-9, -22, -8, 2, 0, -2, -11, 4,
		-7, 5, 1, 2, 4, 4, 9, 13,
		15, 6, 14, -3, 4, 13, 28, 19,
		6, 18, 5, -7, 5, -2, 13, 30,
	};
	int mg_rook_psqt[64] = {
		-3, -14, -16, -18, -5, 2, 3, 10,
		-24, -27, -20, -4, -16, 10, 10, 19,
		-37, -9, -17, -12, 19, 18, 48, 15,
		-32, -18, -19, -14, -7, 2, 9, -1,
		-34, -36, -27, -23, -19, -26, 3, -14,
		-33, -28, -25, -23, -14, -8, 23, 2,
		-31, -27, -16, -15, -8, -4, 22, -16,
		-17, -19, -19, -11, -3, -5, -1, -4,
	};
	int mg_queen_psqt[64] = {
		-27, -25, -10, 5, -6, -6, 12, -6,
		-7, -28, -33, -43, -46, -6, -6, 37,
		-1, -10, -16, -10, -3, 14, 30, 33,
		-13, -11, -19, -29, -23, -7, 2, 9,
		-2, -22, -20, -20, -18, -11, 6, 12,
		-5, -1, -13, -12, -9, 0, 19, 14,
		2, 1, 6, 6, 6, 11, 26, 22,
		-5, -12, -6, 6, 2, -8, -3, 1,
	};
	int mg_king_psqt[64] = {
		-4, -2, -1, -3, 0, -1, 4, 1,
		-9, -1, -4, 11, 8, 7, 11, 9,
		-10, 13, 2, -4, 11, 29, 21, 11,
		0, -3, -15, -28, -30, -6, -6, -23,
		-13, -9, -15, -50, -46, -13, -31, -60,
		-11, 22, -6, -16, -8, -18, -4, -42,
		24, -7, -4, -3, -8, -17, -3, 4,
		31, 22, 29, -20, 40, -24, 15, 45,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		17, 17, 38, 23, 30, 26, 40, 34,
		37, 43, 28, 15, 21, 27, 45, 40,
		31, 25, 16, 8, 10, 15, 23, 21,
		21, 18, 11, 7, 7, 12, 14, 13,
		16, 12, 10, 12, 12, 12, 8, 10,
		18, 16, 13, 13, 18, 15, 10, 12,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-46, -17, -4, -8, 1, -23, -18, -59,
		-1, 1, 5, 6, 0, -5, -1, -14,
		-2, 2, 7, 12, 6, -5, -5, -8,
		8, 2, 8, 7, 10, 7, 9, 2,
		8, 4, 14, 15, 17, 9, 3, 8,
		-2, -1, 1, 10, 10, -1, -2, 0,
		-4, -1, -4, -1, -3, -4, -5, 7,
		-5, -4, -5, -5, -1, -8, 1, -1,
	};
	int eg_bishop_psqt[64] = {
		1, 4, 1, 3, 2, -5, 0, -10,
		-7, 1, -2, 3, -2, 0, 0, -8,
		7, 1, 3, -2, 2, 5, 4, 8,
		3, 6, 4, 14, 4, 6, 2, 5,
		0, 2, 6, 6, 6, 0, 2, -8,
		-3, 2, 1, 5, 7, 2, -3, -5,
		1, -10, -9, -3, 0, -5, -6, -8,
		-9, -5, -3, -4, -6, 2, -8, -17,
	};
	int eg_rook_psqt[64] = {
		14, 19, 22, 18, 15, 16, 20, 15,
		7, 14, 16, 7, 8, 9, 9, 1,
		16, 15, 15, 11, 4, 5, 4, 0,
		14, 12, 13, 9, 1, 0, 2, -2,
		4, 5, 2, 1, -1, -2, -8, -9,
		-5, -4, -7, -7, -9, -13, -23, -22,
		-10, -6, -8, -9, -13, -15, -24, -19,
		-7, -9, -7, -11, -14, -11, -13, -16,
	};
	int eg_queen_psqt[64] = {
		6, 4, 12, 13, 14, 18, 2, 7,
		-10, -3, 18, 25, 35, 18, 8, 9,
		-8, -4, 12, 12, 19, 16, -1, 0,
		1, 8, 9, 21, 19, 8, 20, 4,
		-4, 11, 12, 18, 15, 8, 4, 3,
		-13, -3, 8, 6, 10, 0, -9, -10,
		-14, -14, -13, -6, -5, -19, -35, -31,
		-13, -11, -12, -12, -15, -18, -20, -16,
	};
	int eg_king_psqt[64] = {
		-45, -21, -9, 3, 0, 8, 16, -29,
		-9, 14, 18, 15, 20, 30, 30, 13,
		-2, 16, 21, 26, 33, 32, 28, 7,
		-10, 5, 18, 22, 25, 24, 17, 1,
		-20, -6, 6, 14, 12, 7, 2, -8,
		-25, -13, -4, -1, -2, -3, -8, -13,
		-25, -11, -10, -9, -7, -7, -11, -17,
		-42, -21, -15, -9, -22, -8, -18, -45,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 60, 93, 72, 39, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -33, -7, -7, 104, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 12, 24, 37, 49, 45, 9, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 7, 20, 32, 44, 6, 0, 0, };
	
	int mg_safe_knight_check = -87;
	int eg_safe_knight_check = -2;
	
	int mg_safe_bishop_check = -24;
	int eg_safe_bishop_check = -18;
	
	int mg_safe_rook_check = -82;
	int eg_safe_rook_check = -2;
	
	int mg_safe_queen_check = -37;
	int eg_safe_queen_check = -18;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 31, 48, 52, 38, 31, 47, 3, 0, },
	        { 58, 54, 26, 28, 17, 14, -1, 0, },
	        { 47, 43, 20, 21, 24, 20, 12, 0, },
	        { 10, 18, 11, 17, 20, -2, 2, 0, },
	},
	{
	        { 0, 62, 58, 34, 10, 16, 1, 0, },
	        { 0, 66, 54, 29, 26, 27, 3, 0, },
	        { 0, 63, 32, 38, 25, 3, -3, 0, },
	        { 0, 17, 23, 31, 21, -22, -2, 0, },
	},
	};
	
	int eg_pawn_shelter[2][4][8] = {
	{
	        { -7, -13, -7, -4, -2, 19, 16, 0, },
	        { -3, -8, -3, -8, -6, 12, 14, 0, },
	        { 2, -3, -1, -8, -12, -1, 17, 0, },
	        { 5, -2, -4, -4, -12, -4, 18, 0, },
	},
	{
	        { 0, -17, -6, -4, 22, 39, 13, 0, },
	        { 0, -10, -4, -10, -11, 5, 9, 0, },
	        { 0, -3, -1, -10, -8, -1, -2, 0, },
	        { 0, 3, 2, -6, -9, 4, 11, 0, },
	},
	};
	
	int mg_knight_mobility[9] = { -70, -51, -27, -19, -8, -6, 4, 14, 26, };
	int eg_knight_mobility[9] = { -54, -41, -18, -4, 4, 10, 12, 14, 13, };
	
	int mg_bishop_mobility[14] = { -50, -56, -31, -25, -13, -6, -1, 4, 7, 10, 11, 24, 25, 28, };
	int eg_bishop_mobility[14] = { -69, -56, -23, -10, -5, -2, 4, 8, 11, 13, 14, 10, 11, 7, };
	
	int mg_rook_mobility[15] = { -2, -37, -38, -31, -25, -24, -22, -26, -22, -18, -13, -13, -12, -5, 0, };
	int eg_rook_mobility[15] = { -6, -52, -25, -17, -14, -3, -1, 3, 4, 7, 9, 13, 16, 16, 16, };
	
	int mg_queen_mobility[28] = { 0, 0, -2, -27, -53, -22, -15, -14, -13, -14, -11, -9, -7, -4, -3, -3, -2, -6, -4, -1, 6, 15, 29, 23, 13, 17, -2, -3, };
	int eg_queen_mobility[28] = { 0, 0, -1, -24, -35, -33, -26, -16, -7, 5, 6, 9, 13, 13, 14, 17, 16, 20, 19, 16, 14, 3, -2, 0, 0, 2, -8, -5, };
	
	int mg_king_mobility[9] = { 0, -8, -23, -12, -10, 19, -13, 5, 31, };
	int eg_king_mobility[9] = { 0, 2, -6, 3, 7, -3, 8, 8, 0, };
	
	int mg_minor_threatened_by_pawn = -41;
	int eg_minor_threatened_by_pawn = -12;
	
	int mg_minor_threatened_by_minor = -18;
	int eg_minor_threatened_by_minor = -14;
	
	int mg_rook_threatened_by_lesser = -42;
	int eg_rook_threatened_by_lesser = 0;
	
	int mg_queen_threatened_by_lesser = -1;
	int eg_queen_threatened_by_lesser = -1;
	
	int mg_passed_pawn[8] = { 0, 48, 22, 1, -26, -10, 5, 0, };
	int eg_passed_pawn[8] = { 0, 103, 69, 36, 23, 13, 9, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 24, 22, -8, -37, -15, -4, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 40, 18, 16, 12, 14, 9, 0, };
	
	int mg_passed_pawn_safe_advance = -3;
	int eg_passed_pawn_safe_advance = 10;
	
	int mg_passed_pawn_safe_path = -76;
	int eg_passed_pawn_safe_path = 35;
	
	int mg_passed_friendly_distance[8] = { 0, 0, 5, 11, 13, 5, 1, 0, };
	int eg_passed_friendly_distance[8] = { 0, -14, -17, -13, -9, -4, -1, 0, };
	int mg_passed_enemy_distance[8] = { 0, 6, -10, -8, -5, -1, -1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 33, 29, 16, 6, -1, -1, 0, };
	
	int mg_isolated_penalty = -6;
	int eg_isolated_penalty = -7;
	
	int mg_doubled_penalty = 5;
	int eg_doubled_penalty = -12;
	
	int mg_backward_penalty = -8;
	int eg_backward_penalty = -5;
	
	int mg_chained_bonus = 9;
	int eg_chained_bonus = 6;
	
	int mg_double_bishop = 15;
	int eg_double_bishop = 40;
	
	int mg_rook_open_file = 28;
	int eg_rook_open_file = 5;
	
	int mg_rook_half_open_file = 10;
	int eg_rook_half_open_file = 6;
	
	int mg_rook_on_seventh = -7;
	int eg_rook_on_seventh = 17;
	
	int mg_knight_outpost = 19;
	int eg_knight_outpost = -2;
	
	int mg_knight_outpost_supported = 37;
	int eg_knight_outpost_supported = 19;
	
	int mg_center_control = 351;
	
	int pawn_count_scale_offset = 66;
	int pawn_count_scale_weight = 27;

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
