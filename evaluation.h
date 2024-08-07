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

	int mg_piece_value[6] = { 40, 235, 258, 307, 633, 0, };
int eg_piece_value[6] = { 67, 263, 273, 474, 903, 0, };

int mg_pawn_psqt[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	-2, 10, 2, 51, 38, 29, -26, -47,
	7, 4, 25, 39, 39, 56, 23, -17,
	-11, -7, -1, 1, 17, 15, -8, -11,
	-14, -13, -2, 5, 8, 1, -9, -23,
	-19, -14, -8, -1, 7, -13, -5, -27,
	-7, -1, 1, 3, 13, 3, 12, -15,
	0, 0, 0, 0, 0, 0, 0, 0,
};
int mg_knight_psqt[64] = {
	-68, -23, -30, -12, -6, -19, -15, -34,
	-33, -19, -19, -5, -9, 17, -14, -5,
	-16, -10, -4, -3, 16, 40, 5, 5,
	-12, 2, 11, 34, 24, 34, 9, 22,
	3, 7, 13, 20, 22, 23, 27, 14,
	-11, -4, 0, 0, 13, 4, 17, 5,
	-14, -12, -6, 8, 6, 6, 10, 10,
	-36, -5, -22, -11, -3, 3, -3, -13,
};
int mg_bishop_psqt[64] = {
	-13, -27, -20, -39, -35, -37, -12, -28,
	-19, -23, -15, -25, -24, -22, -35, -24,
	-7, 8, 0, 5, -3, 29, 9, 1,
	-13, -3, -3, 6, 4, 5, -2, -17,
	-4, -16, -2, 7, 5, 4, -5, 10,
	-1, 10, 6, 6, 9, 9, 14, 19,
	23, 12, 19, 3, 9, 18, 33, 27,
	11, 23, 10, -2, 11, 3, 17, 35,
};
int mg_rook_psqt[64] = {
	3, -9, -12, -13, -1, 3, 4, 13,
	-24, -26, -20, -4, -15, 11, 11, 22,
	-31, -2, -10, -6, 24, 24, 53, 21,
	-25, -12, -12, -8, -1, 10, 15, 7,
	-26, -29, -19, -17, -11, -17, 11, -6,
	-25, -21, -18, -16, -7, 0, 31, 9,
	-24, -20, -9, -8, 0, 4, 28, -9,
	-9, -12, -11, -3, 5, 3, 7, 5,
};
int mg_queen_psqt[64] = {
	-25, -24, -10, 3, -5, -6, 12, -5,
	-6, -28, -35, -44, -48, -7, -5, 39,
	-1, -9, -17, -12, -5, 14, 28, 34,
	-12, -11, -20, -31, -25, -6, 2, 10,
	-1, -21, -20, -21, -18, -11, 6, 13,
	-4, -1, -14, -13, -9, 0, 19, 15,
	2, 2, 7, 7, 7, 11, 26, 22,
	-6, -13, -5, 7, 3, -7, -4, 1,
};
int mg_king_psqt[64] = {
	-3, -1, -1, -2, 0, -1, 3, 0,
	-7, -1, -3, 9, 7, 6, 9, 8,
	-7, 11, 1, -3, 10, 25, 18, 10,
	2, -2, -14, -25, -24, -4, -4, -17,
	-8, -8, -13, -45, -41, -10, -27, -50,
	-7, 22, -5, -14, -6, -18, -5, -33,
	29, -10, -4, -1, -6, -16, -5, 11,
	55, 32, 41, -6, 53, -11, 26, 72,
};
int eg_pawn_psqt[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	8, 5, 23, 10, 17, 13, 27, 23,
	15, 18, 4, -8, -3, 3, 21, 19,
	13, 5, -3, -11, -9, -5, 3, 4,
	5, 2, -5, -8, -8, -5, -2, -2,
	2, -1, -4, -3, -3, -3, -5, -4,
	6, 6, 2, 1, 6, 4, 2, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};
int eg_knight_psqt[64] = {
	-43, -16, -3, -6, 2, -20, -17, -56,
	-1, 3, 6, 8, 2, -2, 1, -12,
	0, 3, 8, 12, 8, -3, -2, -6,
	8, 3, 8, 7, 10, 7, 8, 2,
	7, 3, 13, 14, 16, 9, 3, 7,
	-2, 0, 1, 10, 9, -1, -2, 0,
	-4, -1, -4, -1, -2, -4, -5, 6,
	-6, -4, -5, -4, -1, -7, 0, -1,
};
int eg_bishop_psqt[64] = {
	0, 4, 0, 2, 1, -5, 0, -10,
	-7, 1, -2, 3, -1, 0, 0, -8,
	6, 0, 3, -2, 2, 5, 4, 7,
	1, 5, 3, 12, 3, 5, 1, 4,
	-1, 2, 5, 6, 5, 0, 1, -8,
	-3, 2, 0, 3, 6, 1, -3, -4,
	0, -9, -8, -4, -1, -5, -6, -8,
	-9, -5, -3, -3, -6, 1, -7, -16,
};
int eg_rook_psqt[64] = {
	11, 16, 19, 15, 13, 15, 18, 13,
	6, 12, 14, 6, 7, 7, 7, 0,
	14, 13, 13, 9, 3, 4, 3, -1,
	11, 10, 10, 7, 0, -1, 1, -3,
	2, 3, 1, 0, -2, -3, -8, -9,
	-5, -5, -7, -7, -9, -13, -22, -21,
	-11, -7, -9, -10, -13, -15, -23, -18,
	-8, -9, -7, -11, -14, -11, -13, -16,
};
int eg_queen_psqt[64] = {
	2, 1, 8, 11, 10, 15, 0, 4,
	-12, -5, 16, 22, 31, 14, 5, 4,
	-9, -6, 9, 10, 16, 13, -1, -3,
	-2, 5, 7, 18, 17, 6, 16, 2,
	-5, 9, 9, 15, 13, 6, 3, 1,
	-14, -4, 6, 4, 8, -1, -9, -11,
	-14, -14, -13, -7, -6, -18, -32, -30,
	-11, -11, -12, -12, -15, -17, -20, -16,
};
int eg_king_psqt[64] = {
	-48, -21, -9, 2, -2, 7, 14, -31,
	-10, 15, 18, 16, 20, 30, 30, 12,
	-3, 16, 21, 26, 32, 32, 28, 5,
	-11, 7, 18, 22, 24, 24, 17, -1,
	-20, -3, 8, 15, 13, 8, 4, -9,
	-23, -9, -2, 1, 1, -1, -4, -13,
	-23, -7, -7, -6, -5, -5, -7, -17,
	-46, -20, -15, -10, -22, -9, -18, -49,
};

int mg_king_ring_attack_potency[6] = { 0, 54, 85, 66, 35, 0, };
int eg_king_ring_attack_potency[6] = { 0, -32, -7, -6, 102, 0, };

int mg_king_ring_pressure_weight[8] = { 0, 13, 26, 39, 53, 46, 7, 0, };
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
        { 37, 50, 54, 41, 35, 48, 3, 0, },
        { 63, 53, 27, 28, 18, 12, 0, 0, },
        { 48, 41, 20, 20, 22, 18, 10, 0, },
        { 12, 17, 11, 17, 19, -4, 2, 0, },
},
{
        { 0, 63, 61, 39, 13, 14, 1, 0, },
        { 0, 67, 58, 31, 28, 25, 3, 0, },
        { 0, 62, 32, 39, 26, 2, -2, 0, },
        { 0, 17, 23, 31, 21, -22, -2, 0, },
},
};

int eg_pawn_shelter[2][4][8] = {
{
        { -7, -13, -7, -4, -2, 18, 17, 0, },
        { -2, -8, -2, -7, -6, 11, 12, 0, },
        { 2, -3, -1, -7, -10, 1, 17, 0, },
        { 6, -2, -4, -4, -10, -2, 19, 0, },
},
{
        { 0, -16, -5, -4, 19, 36, 12, 0, },
        { 0, -9, -4, -9, -9, 4, 8, 0, },
        { 0, -3, -1, -9, -8, -3, -2, 0, },
        { 0, 2, 2, -6, -8, 2, 10, 0, },
},
};

int mg_knight_mobility[9] = { -55, -43, -19, -11, -1, 0, 11, 21, 31, };
int eg_knight_mobility[9] = { -43, -35, -14, -1, 5, 10, 13, 14, 13, };

int mg_bishop_mobility[14] = { -40, -56, -30, -24, -12, -4, 0, 5, 8, 11, 12, 24, 26, 26, };
int eg_bishop_mobility[14] = { -54, -47, -19, -6, -2, 1, 6, 9, 12, 14, 15, 11, 12, 9, };

int mg_rook_mobility[15] = { -2, -31, -34, -27, -21, -19, -17, -22, -18, -13, -9, -8, -6, 0, 5, };
int eg_rook_mobility[15] = { -5, -48, -22, -15, -13, -3, -1, 3, 4, 6, 8, 12, 15, 15, 15, };

int mg_queen_mobility[28] = { 0, 0, -2, -25, -56, -30, -25, -24, -23, -24, -21, -19, -17, -13, -13, -12, -11, -14, -11, -8, 0, 11, 25, 18, 10, 13, -2, -3, };
int eg_queen_mobility[28] = { 0, 0, -1, -24, -37, -31, -21, -12, -4, 7, 7, 10, 12, 12, 13, 16, 14, 18, 17, 14, 11, 2, -4, -1, -1, 1, -9, -5, };

int mg_king_mobility[9] = { 0, -3, -42, -23, -9, 21, -4, 16, 43, };
int eg_king_mobility[9] = { 1, 22, 1, 3, -1, -9, -4, -3, -10, };

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

int mg_passed_pawn[8] = { 0, 41, 18, -11, -35, -19, 2, 0, };
int eg_passed_pawn[8] = { 0, 90, 59, 31, 21, 11, 2, 0, };
int mg_passed_pawn_blocked[8] = { 0, 16, 19, -20, -45, -23, -6, 0, };
int eg_passed_pawn_blocked[8] = { 0, 35, 13, 14, 11, 11, 2, 0, };

int mg_passed_pawn_safe_advance = -2;
int eg_passed_pawn_safe_advance = 9;

int mg_passed_pawn_safe_path = -76;
int eg_passed_pawn_safe_path = 32;

int mg_passed_friendly_distance[8] = { 0, -1, 5, 13, 13, 5, 1, 0, };
int eg_passed_friendly_distance[8] = { 0, -13, -16, -12, -8, -4, -1, 0, };
int mg_passed_enemy_distance[8] = { 0, 7, -11, -7, -3, 1, -1, 0, };
int eg_passed_enemy_distance[8] = { 0, 30, 27, 14, 5, -1, -1, 0, };

int mg_isolated_pawn = -4;
int eg_isolated_pawn = -5;

int mg_doubled_pawn = 5;
int eg_doubled_pawn = -14;

int mg_backward_pawn = 0;
int eg_backward_pawn = 1;
int mg_backward_pawn_half_open = -16;
int eg_backward_pawn_half_open = -11;

int mg_chained_pawn[8] = { 0, 41, 13, 16, 14, 11, 1, 0, };
int eg_chained_pawn[8] = { 0, 67, 41, 15, 7, 5, -2, 0, };

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

int mg_knight_outpost_supported = 36;
int eg_knight_outpost_supported = 16;

int mg_center_control = 336;

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
