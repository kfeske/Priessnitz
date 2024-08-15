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

	int mg_piece_value[6] = { 36, 172, 192, 245, 575, 0, };
	int eg_piece_value[6] = { 69, 272, 282, 473, 861, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		8, -3, 18, 53, 50, 30, -34, -40,
		-4, 1, 11, 26, 24, 36, 2, -15,
		-4, -5, -3, 7, 12, 15, -6, -17,
		-6, -5, 1, 1, 6, 5, -2, -22,
		-10, -10, -6, -2, 5, -9, -5, -22,
		-2, 0, -5, -2, -1, 3, 10, -7,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-71, -14, -45, -7, 6, -23, -3, -39,
		-17, -29, -4, -4, 5, 19, -25, -1,
		-36, -20, -3, -3, 13, 23, 0, 11,
		-8, 5, 6, 16, 10, 22, 10, 16,
		5, 7, 11, 15, 17, 12, 27, 16,
		-2, 2, 3, 7, 11, 5, 10, 3,
		-12, -7, -5, 5, 0, 5, -1, 7,
		-31, 2, -20, -5, 3, 4, 4, -38,
	};
	int mg_bishop_psqt[64] = {
		-13, -22, -47, -45, -44, -48, -2, -16,
		-38, -17, -12, -27, -29, -8, -26, -31,
		-7, -2, 18, 5, 14, 20, 16, -3,
		-15, 8, -1, 17, 3, 8, 5, -8,
		-4, 1, 4, 3, 10, 3, 3, 5,
		3, 7, 3, 8, 6, 7, 9, 14,
		16, 11, 11, 1, 2, 9, 23, 17,
		16, 14, 3, -3, -2, 3, 7, 18,
	};
	int mg_rook_psqt[64] = {
		14, 8, -19, -7, -2, 14, 23, 24,
		-18, -27, -12, 1, -2, 17, -9, 7,
		-22, 10, 0, 23, 35, 38, 61, 5,
		-17, -6, 0, 18, 9, 14, 14, 5,
		-22, -26, -23, -14, -15, -15, -4, -10,
		-25, -21, -24, -17, -18, -16, 4, -15,
		-39, -19, -17, -14, -13, -6, -1, -42,
		-19, -20, -17, -11, -11, -9, -4, -7,
	};
	int mg_queen_psqt[64] = {
		-7, 4, -9, 1, -6, 12, 23, 26,
		-19, -50, -25, -51, -49, 2, -25, 11,
		-19, -13, -23, -18, -12, 11, 22, -4,
		-8, -12, -24, -36, -33, -19, 6, 7,
		-3, -7, -13, -21, -22, -6, 5, 8,
		-6, 3, -5, -9, -6, -5, 11, 9,
		-3, 4, 7, 2, 3, 12, 22, 12,
		9, -3, 2, 7, 4, -10, 0, 2,
	};
	int mg_king_psqt[64] = {
		0, 1, 2, 1, 1, 1, 2, -3,
		-2, 3, 12, 6, 4, 9, 7, 0,
		-1, 13, 17, 5, 12, 23, 27, 0,
		-4, 17, 7, -8, -7, 26, 25, -6,
		-1, 18, 33, -26, -3, 17, 24, -26,
		-13, -7, 16, -17, -1, -15, -27, -21,
		1, -43, -26, -61, -38, -52, -36, 4,
		31, 10, 22, -26, 30, -34, 16, 62,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		-1, 14, 14, 10, 13, 10, 19, 6,
		18, 16, 10, 1, 6, 11, 17, 15,
		11, 4, -1, -10, -7, -2, 5, 6,
		5, 2, -6, -7, -8, -3, 0, 1,
		1, -1, -2, -2, 0, -1, -2, -1,
		4, 2, 4, 4, 8, 7, 3, -1,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-46, -13, 4, 0, 3, 6, -9, -48,
		-5, 4, 2, 20, 18, -4, 3, -6,
		1, 6, 17, 18, 15, 20, 9, -2,
		4, 2, 15, 21, 22, 18, 12, 8,
		-3, 4, 14, 17, 17, 16, 7, 5,
		-19, -8, -2, 8, 7, -2, -4, -16,
		-12, -7, -14, -4, -4, -12, -4, 1,
		-42, -19, -14, -6, -7, -11, -11, -20,
	};
	int eg_bishop_psqt[64] = {
		4, 7, 6, 11, 8, 7, 3, 2,
		6, 5, 2, 6, 6, 3, 3, 4,
		3, 7, 2, 1, 2, 10, 7, 6,
		3, 3, 2, 5, 6, 2, 5, 5,
		-6, -4, 1, 2, 1, -1, -3, -5,
		-7, -5, -3, -1, 0, -3, -8, -2,
		-12, -12, -10, -5, -5, -14, -9, -18,
		-9, -6, -6, -8, -6, -6, -5, -3,
	};
	int eg_rook_psqt[64] = {
		14, 17, 20, 17, 17, 19, 17, 18,
		9, 12, 9, 11, 11, 4, 12, 5,
		13, 8, 13, 6, 3, 10, -2, 9,
		8, 8, 8, 4, 5, 4, 3, 3,
		-4, 5, 3, -1, -2, -1, -1, -6,
		-13, -7, -9, -12, -11, -12, -13, -17,
		-14, -16, -15, -17, -17, -20, -21, -12,
		-14, -12, -12, -15, -14, -11, -14, -20,
	};
	int eg_queen_psqt[64] = {
		-4, -4, 9, 10, 17, 20, 18, 9,
		-1, 6, 3, 23, 36, 29, 25, 20,
		-5, -5, 3, 6, 21, 23, 24, 23,
		-8, 8, 2, 20, 26, 28, 27, 13,
		-10, -1, 0, 14, 14, 6, 1, 6,
		-17, -11, -4, -7, -4, -4, -16, -17,
		-19, -20, -26, -13, -13, -36, -39, -30,
		-29, -26, -29, -20, -24, -25, -37, -30,
	};
	int eg_king_psqt[64] = {
		-31, -10, -10, -2, -11, -6, -7, -36,
		-20, 22, 17, 7, 11, 18, 29, -24,
		1, 25, 23, 14, 14, 29, 30, -1,
		1, 15, 20, 18, 20, 22, 20, -2,
		-19, 2, 8, 15, 13, 12, 7, -11,
		-15, -2, -1, 5, 3, 5, 6, -11,
		-11, 2, 2, 3, -1, 7, 5, -13,
		-37, -15, -16, -19, -39, -9, -18, -50,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 47, 60, 74, 58, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -11, -1, -1, 119, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 11, 25, 37, 44, 45, 1, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 8, 19, 35, 66, 7, 0, 0, };
	
	int mg_safe_knight_check = -42;
	int eg_safe_knight_check = 2;
	
	int mg_safe_bishop_check = -2;
	int eg_safe_bishop_check = -6;
	
	int mg_safe_rook_check = -23;
	int eg_safe_rook_check = -3;
	
	int mg_safe_queen_check = -17;
	int eg_safe_queen_check = -17;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 31, 34, 38, 30, 15, 16, -8, 0, },
	        { 44, 26, 7, 9, 4, 9, 9, 0, },
	        { 41, 30, 13, 13, 18, 40, 17, 0, },
	        { 12, 21, 6, 12, 7, -27, -4, 0, },
	},
	{
	        { 0, 50, 52, 22, 2, 10, -2, 0, },
	        { 0, 43, 38, 21, 5, -1, -21, 0, },
	        { 0, 38, 18, 30, 22, -4, 0, 0, },
	        { 0, 15, 11, 16, 7, -23, -6, 0, },
	},
	};
	
	int eg_pawn_shelter[2][4][8] = {
	{
	        { -11, -15, -6, -2, 2, 17, 16, 0, },
	        { -4, -7, -1, -8, -4, 18, 29, 0, },
	        { 0, 0, 1, -9, -11, -9, 17, 0, },
	        { 6, -2, -3, -5, -12, -1, 8, 0, },
	},
	{
	        { 0, -20, -12, -3, 12, 29, -2, 0, },
	        { 0, -8, -6, -10, -5, 19, -17, 0, },
	        { 0, 0, 3, -6, -3, 11, 11, 0, },
	        { 0, 7, 7, 0, -7, 1, -6, 0, },
	},
	};
	
	int mg_knight_mobility[9] = { -32, -32, -25, -19, -12, -11, -6, 0, 8, };
	int eg_knight_mobility[9] = { -35, -47, -15, 2, 6, 14, 16, 17, 13, };
	
	int mg_bishop_mobility[14] = { -42, -35, -23, -23, -15, -11, -9, -8, -9, -7, -5, 8, 25, 36, };
	int eg_bishop_mobility[14] = { -69, -62, -25, -10, -4, 3, 10, 15, 20, 22, 23, 18, 20, 10, };
	
	int mg_rook_mobility[15] = { -2, -35, -36, -34, -31, -31, -32, -33, -29, -24, -20, -17, -13, -3, 36, };
	int eg_rook_mobility[15] = { -5, -66, -36, -20, -11, -3, 4, 9, 11, 14, 16, 18, 20, 17, 4, };
	
	int mg_queen_mobility[28] = { 0, 0, -2, -19, -45, -23, -16, -19, -18, -18, -17, -17, -15, -13, -13, -13, -12, -15, -16, -15, -6, -4, -2, 6, -6, -9, -3, 0, };
	int eg_queen_mobility[28] = { 0, 0, -2, -15, -34, -47, -35, -16, -7, 0, 4, 11, 14, 15, 17, 18, 17, 18, 17, 14, 9, 6, -2, -8, -9, -14, -11, -1, };
	
	int mg_king_mobility[9] = { 0, -4, -52, -27, -5, 17, 3, 23, 44, };
	int eg_king_mobility[9] = { 2, 5, 10, 7, 1, -3, -5, -6, -11, };
	
	int mg_minor_threatened_by_pawn = -18;
	int eg_minor_threatened_by_pawn = -27;
	
	int mg_minor_threatened_by_minor = -12;
	int eg_minor_threatened_by_minor = -24;
	
	int mg_rook_threatened_by_lesser = -16;
	int eg_rook_threatened_by_lesser = -8;
	
	int mg_queen_threatened_by_lesser = 0;
	int eg_queen_threatened_by_lesser = 1;
	
	int mg_minor_threatened_by_major = -13;
	int eg_minor_threatened_by_major = -11;
	
	int mg_passed_pawn[8] = { 0, 47, 27, -3, -40, -16, -7, 0, };
	int eg_passed_pawn[8] = { 0, 68, 43, 29, 24, 12, 8, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 34, 30, -2, -42, -18, -16, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 16, 0, 5, 9, 8, 2, 0, };
	
	int mg_passed_pawn_safe_advance = -5;
	int eg_passed_pawn_safe_advance = 12;
	
	int mg_passed_pawn_safe_path = -73;
	int eg_passed_pawn_safe_path = 27;
	
	int mg_passed_friendly_distance[8] = { 0, 2, 2, 11, 11, 4, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -10, -12, -12, -8, -3, 0, 0, };
	int mg_passed_enemy_distance[8] = { 0, 3, -6, -8, -2, 0, 1, 0, };
	int eg_passed_enemy_distance[8] = { 0, 28, 24, 14, 6, 0, -2, 0, };
	
	int mg_isolated_pawn = -2;
	int eg_isolated_pawn = -6;
	
	int mg_doubled_pawn = -2;
	int eg_doubled_pawn = -12;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 1;
	int mg_backward_pawn_half_open = -5;
	int eg_backward_pawn_half_open = -12;
	
	int mg_chained_pawn[8] = { 0, 66, 22, 13, 10, 10, 1, 0, };
	int eg_chained_pawn[8] = { 0, 51, 33, 12, 5, 4, -2, 0, };
	
	int mg_double_bishop = 1;
	int eg_double_bishop = 39;
	
	int mg_rook_open_file = 21;
	int eg_rook_open_file = 4;
	
	int mg_rook_half_open_file = 7;
	int eg_rook_half_open_file = 4;
	
	int mg_rook_on_seventh = -3;
	int eg_rook_on_seventh = 14;
	
	int mg_knight_outpost = 19;
	int eg_knight_outpost = -10;
	
	int mg_knight_outpost_supported = 29;
	int eg_knight_outpost_supported = 10;
	
	int mg_center_control = 271;
	
	int pawn_count_scale_offset = 80;
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

	void evaluate_passed_pawns(Board &board, Color friendly);
	
	void evaluate_threats(Board &board, Color friendly);

	void evaluate_center_control(Color friendly);

	void note_king_attacks(Piece_type type, uint64_t attacks, Color friendly);

	int scale_factor(Board &board, int eg_value);

	int evaluate(Board &board);
};
