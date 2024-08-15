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

	int mg_piece_value[6] = { 37, 231, 247, 287, 707, 0, };
	int eg_piece_value[6] = { 68, 294, 300, 498, 888, 0, };
	
	int mg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		22, 10, 30, 66, 73, 46, -27, -39,
		0, 5, 14, 30, 31, 43, 18, -7,
		0, -2, 0, 11, 16, 20, -1, -10,
		-2, -2, 5, 5, 10, 10, 4, -15,
		-7, -7, -2, 1, 9, -3, 1, -16,
		2, 3, -1, 1, 3, 9, 16, -1,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int mg_knight_psqt[64] = {
		-122, -69, -111, -30, -1, -80, -28, -96,
		-29, -40, -14, -12, -2, 12, -43, -13,
		-49, -30, -12, -12, 6, 18, -8, 1,
		-19, -6, -4, 7, 0, 12, 0, 6,
		-6, -4, 0, 5, 7, 1, 17, 5,
		-13, -9, -8, -4, 1, -6, 0, -8,
		-24, -19, -16, -6, -11, -6, -12, -5,
		-62, -9, -32, -16, -9, -7, -7, -56,
	};
	int mg_bishop_psqt[64] = {
		-31, -62, -106, -89, -94, -89, -20, -42,
		-51, -30, -24, -40, -41, -20, -39, -44,
		-19, -13, 8, -6, 4, 11, 6, -14,
		-26, -3, -12, 7, -8, -3, -5, -20,
		-15, -10, -7, -7, 0, -8, -8, -6,
		-8, -4, -8, -3, -5, -4, -1, 3,
		6, 0, 0, -10, -9, -2, 13, 6,
		5, 3, -9, -14, -13, -8, -3, 7,
	};
	int mg_rook_psqt[64] = {
		17, 10, -17, -4, 1, 23, 34, 30,
		-16, -25, -8, 4, 2, 20, -9, 8,
		-21, 12, 3, 27, 40, 41, 66, 8,
		-15, -4, 2, 21, 12, 16, 16, 8,
		-21, -25, -22, -12, -13, -14, -2, -9,
		-24, -20, -23, -15, -16, -15, 6, -13,
		-38, -18, -16, -13, -11, -5, 1, -41,
		-18, -19, -16, -9, -10, -8, -3, -6,
	};
	int mg_queen_psqt[64] = {
		-9, 5, -15, 0, -12, 19, 38, 34,
		-21, -54, -27, -56, -54, 1, -36, 7,
		-21, -14, -25, -18, -11, 10, 26, -4,
		-9, -12, -25, -37, -33, -20, 7, 7,
		-3, -7, -13, -20, -22, -6, 5, 8,
		-7, 3, -4, -8, -6, -5, 11, 9,
		-3, 5, 8, 2, 3, 13, 23, 14,
		11, -3, 2, 7, 4, -10, 5, 9,
	};
	int mg_king_psqt[64] = {
		23, 21, 32, 14, 19, 17, 26, -8,
		-3, 14, 74, 51, 31, 52, 35, 11,
		-18, 50, 60, 24, 53, 74, 81, -13,
		-27, 51, 22, -11, -9, 47, 42, -28,
		-8, 40, 53, -26, 2, 23, 33, -47,
		-31, 3, 24, -8, 8, -6, -13, -34,
		-8, -26, -15, -52, -28, -43, -19, -6,
		2, 2, 6, -45, 12, -52, 6, 33,
	};
	int eg_pawn_psqt[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		6, 22, 22, 16, 18, 16, 28, 14,
		26, 23, 17, 7, 12, 18, 22, 21,
		17, 11, 5, -5, -2, 4, 10, 11,
		12, 8, 0, -2, -3, 2, 5, 6,
		7, 5, 3, 4, 5, 5, 3, 4,
		10, 8, 10, 10, 14, 13, 9, 4,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int eg_knight_psqt[64] = {
		-35, -10, 10, -1, 0, 13, -11, -44,
		-8, 1, -2, 17, 15, -8, 2, -9,
		-2, 3, 14, 16, 12, 17, 6, -6,
		1, -2, 13, 20, 20, 15, 10, 5,
		-6, 2, 12, 16, 15, 14, 4, 3,
		-24, -12, -6, 5, 5, -6, -7, -21,
		-16, -11, -19, -8, -7, -16, -7, -2,
		-41, -23, -18, -10, -11, -15, -15, -19,
	};
	int eg_bishop_psqt[64] = {
		7, 13, 15, 18, 14, 13, 6, 7,
		8, 7, 3, 8, 7, 4, 5, 6,
		5, 8, 3, 2, 3, 11, 8, 7,
		4, 4, 3, 6, 8, 4, 6, 7,
		-5, -3, 2, 3, 2, -1, -2, -4,
		-7, -5, -2, 0, 0, -2, -8, -1,
		-12, -12, -10, -5, -4, -14, -9, -19,
		-9, -5, -6, -8, -6, -5, -4, -2,
	};
	int eg_rook_psqt[64] = {
		18, 22, 25, 22, 22, 23, 20, 21,
		12, 16, 13, 15, 15, 8, 16, 9,
		18, 12, 17, 9, 6, 14, 1, 13,
		12, 12, 12, 8, 9, 7, 6, 6,
		0, 8, 7, 2, 1, 2, 2, -3,
		-11, -4, -6, -10, -9, -9, -11, -14,
		-12, -14, -13, -15, -15, -18, -20, -10,
		-12, -9, -9, -13, -12, -8, -12, -18,
	};
	int eg_queen_psqt[64] = {
		1, 0, 16, 15, 25, 23, 16, 10,
		4, 13, 8, 31, 46, 37, 39, 27,
		0, -1, 8, 11, 27, 30, 28, 29,
		-4, 13, 7, 27, 33, 35, 33, 19,
		-8, 3, 4, 20, 19, 11, 5, 10,
		-15, -8, 0, -4, -1, -1, -14, -15,
		-17, -19, -25, -11, -11, -36, -40, -31,
		-29, -25, -28, -19, -23, -23, -41, -37,
	};
	int eg_king_psqt[64] = {
		-70, -12, -13, 0, -12, -7, -8, -63,
		-19, 25, 15, 6, 12, 18, 32, -23,
		6, 26, 23, 17, 13, 29, 29, 4,
		6, 15, 23, 22, 24, 24, 22, 3,
		-16, 3, 9, 19, 17, 15, 10, -6,
		-11, 0, 1, 8, 6, 8, 7, -8,
		-9, 2, 3, 5, 1, 9, 5, -10,
		-34, -14, -13, -15, -36, -5, -17, -48,
	};
	
	int mg_king_ring_attack_potency[6] = { 0, 45, 59, 76, 65, 0, };
	int eg_king_ring_attack_potency[6] = { 0, -10, -1, -3, 116, 0, };
	
	int mg_king_ring_pressure_weight[8] = { 0, 11, 26, 38, 44, 67, 17, 0, };
	int eg_king_ring_pressure_weight[8] = { 0, 9, 20, 38, 90, 50, 0, 0, };
	
	int mg_safe_knight_check = -43;
	int eg_safe_knight_check = 2;
	
	int mg_safe_bishop_check = -2;
	int eg_safe_bishop_check = -7;
	
	int mg_safe_rook_check = -23;
	int eg_safe_rook_check = -3;
	
	int mg_safe_queen_check = -19;
	int eg_safe_queen_check = -18;
	
	int mg_pawn_shelter[2][4][8] = {
	{
	        { 26, 31, 34, 26, 10, 11, -44, 0, },
	        { 44, 25, 7, 9, 2, 10, 18, 0, },
	        { 40, 30, 12, 12, 18, 41, 27, 0, },
	        { 13, 23, 7, 13, 8, -29, -36, 0, },
	},
	{
	        { 0, 47, 49, 17, -4, 3, -20, 0, },
	        { 0, 42, 37, 19, 2, -7, -49, 0, },
	        { 0, 40, 20, 32, 24, -23, -20, 0, },
	        { 0, 17, 12, 19, 9, -37, -51, 0, },
	},
	};
	
	int eg_pawn_shelter[2][4][8] = {
	{
	        { -10, -15, -5, 0, 4, 21, 33, 0, },
	        { -5, -7, -1, -8, -4, 19, 38, 0, },
	        { 0, 0, 1, -10, -12, -10, 13, 0, },
	        { 6, -2, -3, -6, -13, 0, 18, 0, },
	},
	{
	        { 0, -20, -11, -1, 15, 35, -6, 0, },
	        { 0, -8, -6, -10, -4, 23, -14, 0, },
	        { 0, 0, 4, -7, -3, 16, 30, 0, },
	        { 0, 7, 8, 0, -8, 8, -8, 0, },
	},
	};
	
	int mg_knight_mobility[9] = { -87, -38, -31, -25, -18, -17, -12, -6, 3, };
	int eg_knight_mobility[9] = { -95, -62, -27, -9, -4, 5, 8, 8, 3, };
	
	int mg_bishop_mobility[14] = { -44, -37, -25, -24, -15, -11, -10, -8, -9, -7, -5, 9, 26, 43, };
	int eg_bishop_mobility[14] = { -102, -77, -37, -21, -14, -6, 1, 6, 11, 14, 15, 10, 12, 0, };
	
	int mg_rook_mobility[15] = { -36, -41, -23, -21, -17, -17, -18, -19, -14, -10, -5, -2, 2, 13, 53, };
	int eg_rook_mobility[15] = { -65, -79, -49, -31, -21, -11, -4, 1, 4, 7, 10, 12, 13, 10, -5, };
	
	int mg_queen_mobility[28] = { 0, 0, -28, -55, -33, -10, -4, -7, -6, -6, -5, -5, -3, -1, -1, 0, 0, -3, -3, -3, 6, 6, 8, 19, -7, -18, 0, 9, };
	int eg_queen_mobility[28] = { 0, -1, -27, -48, -61, -65, -49, -28, -17, -9, -5, 3, 6, 7, 10, 10, 10, 11, 10, 7, 1, -2, -9, -18, -13, -16, -27, -8, };
	
	int mg_king_mobility[9] = { 10, -20, -36, -12, -7, 13, -23, -5, 15, };
	int eg_king_mobility[9] = { 28, 17, 12, 8, 4, 1, 1, 0, -4, };
	
	int mg_minor_threatened_by_pawn = -18;
	int eg_minor_threatened_by_pawn = -29;
	
	int mg_minor_threatened_by_minor = -13;
	int eg_minor_threatened_by_minor = -25;
	
	int mg_rook_threatened_by_lesser = -17;
	int eg_rook_threatened_by_lesser = -8;
	
	int mg_queen_threatened_by_lesser = 2;
	int eg_queen_threatened_by_lesser = 14;
	
	int mg_minor_threatened_by_major = -13;
	int eg_minor_threatened_by_major = -12;
	
	int mg_passed_pawn[8] = { 0, 60, 28, 7, -31, -5, 0, 0, };
	int eg_passed_pawn[8] = { 0, 68, 44, 29, 23, 11, 7, 0, };
	int mg_passed_pawn_blocked[8] = { 0, 44, 30, 6, -34, -8, -10, 0, };
	int eg_passed_pawn_blocked[8] = { 0, 13, -2, 4, 8, 7, 1, 0, };
	
	int mg_passed_pawn_safe_advance = -5;
	int eg_passed_pawn_safe_advance = 12;
	
	int mg_passed_pawn_safe_path = -74;
	int eg_passed_pawn_safe_path = 29;
	
	int mg_passed_friendly_distance[8] = { 0, 1, 3, 9, 11, 4, 0, 0, };
	int eg_passed_friendly_distance[8] = { 0, -10, -13, -12, -9, -3, 0, 0, };
	int mg_passed_enemy_distance[8] = { 0, 2, -6, -8, -3, -2, 0, 0, };
	int eg_passed_enemy_distance[8] = { 0, 30, 26, 15, 6, 0, -2, 0, };
	
	int mg_isolated_pawn = -3;
	int eg_isolated_pawn = -6;
	
	int mg_doubled_pawn = -3;
	int eg_doubled_pawn = -13;
	
	int mg_backward_pawn = 0;
	int eg_backward_pawn = 1;
	int mg_backward_pawn_half_open = -5;
	int eg_backward_pawn_half_open = -13;
	
	int mg_chained_pawn[8] = { 0, 151, 24, 14, 10, 11, 1, 0, };
	int eg_chained_pawn[8] = { 0, 39, 35, 12, 6, 4, -2, 0, };
	
	int mg_double_bishop = 1;
	int eg_double_bishop = 42;
	
	int mg_rook_open_file = 22;
	int eg_rook_open_file = 4;
	
	int mg_rook_half_open_file = 8;
	int eg_rook_half_open_file = 4;
	
	int mg_rook_on_seventh = -4;
	int eg_rook_on_seventh = 15;
	
	int mg_knight_outpost = 18;
	int eg_knight_outpost = -10;
	
	int mg_knight_outpost_supported = 29;
	int eg_knight_outpost_supported = 11;
	
	int mg_center_control = 246;
	
	int pawn_count_scale_offset = 77;
	int pawn_count_scale_weight = 25;

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
