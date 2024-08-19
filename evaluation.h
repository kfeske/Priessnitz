#pragma once

#include "utility.h"
#include "board.h"
#include "transposition_table.h"

struct Eval_info : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	//int mg_king_ring_pressure[2];
	//int eg_king_ring_pressure[2];
	unsigned king_attackers[2];
	unsigned king_zone_attacks[2];
	int mg_king_attackers_weight[2];
	uint64_t passed_pawns;

	uint64_t attacked[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		//mg_king_ring_pressure[WHITE] = mg_king_ring_pressure[BLACK] = 0;
		//eg_king_ring_pressure[WHITE] = eg_king_ring_pressure[BLACK] = 0;
		king_attackers[WHITE] = king_attackers[BLACK] = 0;
		king_zone_attacks[WHITE] = king_zone_attacks[BLACK] = 0;
		mg_king_attackers_weight[WHITE] = mg_king_attackers_weight[BLACK] = 0;
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

	int mg_piece_value[6] = { 38, 237, 253, 296, 704, 0, };
int eg_piece_value[6] = { 67, 292, 298, 495, 894, 0, };

int mg_pawn_psqt[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	23, 11, 30, 66, 73, 47, -26, -39,
	-1, 5, 15, 30, 31, 42, 18, -6,
	0, -2, 0, 10, 16, 20, -1, -10,
	-2, -2, 5, 5, 10, 10, 4, -16,
	-7, -7, -2, 2, 9, -3, 0, -15,
	2, 3, -1, 2, 3, 10, 16, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};
int mg_knight_psqt[64] = {
	-122, -70, -111, -27, 2, -79, -29, -96,
	-29, -40, -14, -12, -6, 10, -43, -12,
	-50, -31, -13, -12, -4, 7, -20, -7,
	-19, -5, -3, 8, 0, 12, -11, 7,
	-5, -3, 2, 6, 9, 3, 17, 6,
	-12, -8, -6, -2, 2, -5, 1, -6,
	-23, -18, -15, -4, -10, -5, -13, -4,
	-62, -8, -31, -15, -7, -5, -6, -55,
};
int mg_bishop_psqt[64] = {
	-31, -63, -105, -89, -91, -88, -18, -39,
	-51, -31, -23, -41, -40, -20, -40, -42,
	-19, -13, 8, -5, 4, 6, 0, -20,
	-26, -2, -11, 7, -8, -2, -5, -20,
	-15, -9, -6, -7, 1, -7, -7, -5,
	-8, -3, -7, -2, -4, -3, -1, 4,
	7, 1, 1, -9, -8, -1, 13, 7,
	5, 4, -8, -13, -13, -7, -4, 8,
};
int mg_rook_psqt[64] = {
	16, 9, -19, -7, -4, 17, 29, 23,
	-16, -25, -9, 4, 0, 14, -12, 3,
	-21, 13, 3, 27, 39, 40, 65, 6,
	-15, -3, 3, 21, 12, 16, 16, 7,
	-21, -24, -21, -12, -13, -14, -2, -9,
	-24, -19, -22, -14, -15, -15, 6, -14,
	-38, -17, -15, -12, -11, -6, -1, -42,
	-17, -18, -15, -8, -9, -7, -2, -5,
};
int mg_queen_psqt[64] = {
	-18, -4, -27, -14, -29, 3, 23, 18,
	-24, -55, -32, -61, -70, -17, -50, -5,
	-22, -14, -26, -22, -19, -19, -1, -44,
	-6, -10, -24, -38, -36, -25, 3, -2,
	-1, -6, -11, -20, -21, -7, 5, 6,
	-5, 5, -3, -7, -3, -4, 12, 9,
	-1, 6, 10, 5, 6, 15, 24, 14,
	12, -1, 5, 10, 7, -9, 4, 9,
};
int mg_king_psqt[64] = {
	25, 19, 30, 15, 17, 16, 25, -8,
	-4, 14, 73, 52, 30, 52, 33, 8,
	-18, 48, 59, 23, 53, 74, 80, -11,
	-25, 50, 20, -13, -11, 46, 42, -26,
	-8, 40, 49, -30, -2, 19, 34, -46,
	-31, 2, 22, -14, 2, -6, -12, -34,
	-8, -23, -15, -52, -27, -40, -16, -6,
	1, 3, 6, -46, 11, -51, 7, 31,
};
int eg_pawn_psqt[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	6, 22, 22, 16, 17, 16, 28, 15,
	26, 22, 16, 7, 11, 17, 21, 20,
	17, 10, 5, -5, -2, 3, 10, 11,
	12, 8, 0, -2, -3, 1, 4, 6,
	7, 5, 3, 4, 5, 5, 3, 4,
	10, 8, 10, 10, 14, 13, 9, 4,
	0, 0, 0, 0, 0, 0, 0, 0,
};
int eg_knight_psqt[64] = {
	-36, -10, 10, -1, -1, 12, -11, -44,
	-8, 1, -2, 17, 15, -8, 2, -9,
	-2, 3, 15, 16, 14, 19, 8, -4,
	0, -2, 13, 19, 21, 15, 12, 6,
	-7, 1, 12, 15, 15, 14, 3, 3,
	-24, -13, -6, 5, 4, -6, -8, -22,
	-16, -11, -19, -9, -8, -17, -7, -3,
	-42, -24, -18, -10, -11, -16, -16, -20,
};
int eg_bishop_psqt[64] = {
	7, 13, 15, 18, 14, 14, 6, 7,
	8, 6, 2, 8, 7, 5, 5, 7,
	4, 7, 2, 3, 3, 13, 10, 9,
	4, 4, 3, 7, 8, 4, 6, 8,
	-5, -3, 2, 3, 2, -1, -2, -4,
	-6, -5, -2, -1, 0, -3, -8, -1,
	-11, -13, -10, -6, -5, -15, -10, -19,
	-9, -5, -6, -9, -6, -6, -4, -3,
};
int eg_rook_psqt[64] = {
	18, 22, 26, 22, 23, 24, 21, 22,
	12, 16, 13, 15, 16, 9, 17, 11,
	18, 12, 17, 9, 6, 15, 2, 14,
	11, 12, 11, 7, 9, 8, 7, 7,
	-1, 7, 6, 1, 1, 3, 3, -3,
	-12, -5, -7, -11, -9, -8, -10, -15,
	-13, -15, -14, -16, -16, -18, -19, -10,
	-13, -11, -11, -14, -13, -9, -12, -20,
};
int eg_queen_psqt[64] = {
	10, 9, 27, 27, 40, 37, 29, 24,
	5, 14, 13, 37, 59, 53, 49, 39,
	-4, -5, 5, 14, 36, 51, 49, 54,
	-11, 6, 5, 29, 38, 44, 38, 29,
	-14, -1, 1, 19, 18, 13, 7, 14,
	-18, -14, -4, -8, -7, -3, -14, -14,
	-22, -24, -31, -18, -19, -42, -45, -33,
	-33, -30, -36, -26, -31, -29, -44, -39,
};
int eg_king_psqt[64] = {
	-70, -13, -13, 0, -12, -7, -8, -63,
	-19, 25, 15, 6, 12, 19, 32, -22,
	6, 26, 24, 17, 14, 29, 30, 4,
	6, 15, 23, 23, 24, 24, 22, 3,
	-16, 3, 10, 20, 17, 15, 9, -6,
	-11, 0, 0, 8, 6, 8, 6, -8,
	-8, 1, 3, 5, 0, 8, 4, -10,
	-33, -14, -12, -15, -35, -5, -17, -46,
};

int mg_king_attacker_weight[6] = { 0, -1, 7, 14, 32, 0, };

int mg_king_zone_attack_count_weight = 153;

int mg_king_danger_no_queen_weight = -319;

int mg_king_danger_offset = 2;

int mg_safe_knight_check = -45;
int eg_safe_knight_check = 2;

int mg_safe_bishop_check = -3;
int eg_safe_bishop_check = -7;

int mg_safe_rook_check = -23;
int eg_safe_rook_check = -2;

int mg_safe_queen_check = -18;
int eg_safe_queen_check = -18;

int mg_pawn_shelter[2][4][8] = {
{
        { 24, 28, 31, 25, 9, 9, -47, 0, },
        { 42, 24, 6, 8, 2, 10, 17, 0, },
        { 39, 29, 12, 12, 18, 41, 27, 0, },
        { 12, 22, 6, 13, 8, -30, -35, 0, },
},
{
        { 0, 47, 50, 17, -4, 3, -22, 0, },
        { 0, 41, 38, 19, 2, -6, -49, 0, },
        { 0, 38, 20, 31, 24, -22, -20, 0, },
        { 0, 17, 12, 18, 9, -37, -51, 0, },
},
};

int eg_pawn_shelter[2][4][8] = {
{
        { -10, -15, -4, 0, 5, 22, 32, 0, },
        { -4, -7, 0, -8, -4, 19, 38, 0, },
        { 0, 0, 2, -10, -13, -10, 13, 0, },
        { 6, -2, -2, -5, -12, 0, 18, 0, },
},
{
        { 0, -19, -11, 0, 16, 35, -7, 0, },
        { 0, -8, -6, -10, -4, 22, -14, 0, },
        { 0, 0, 4, -6, -3, 16, 30, 0, },
        { 0, 7, 8, 0, -8, 8, -8, 0, },
},
};

int mg_knight_mobility[9] = { -87, -37, -30, -24, -17, -16, -11, -4, 4, };
int eg_knight_mobility[9] = { -94, -62, -27, -9, -4, 4, 7, 7, 2, };

int mg_bishop_mobility[14] = { -44, -36, -23, -23, -14, -10, -8, -7, -8, -6, -4, 9, 26, 41, };
int eg_bishop_mobility[14] = { -103, -78, -38, -22, -15, -7, 1, 6, 11, 14, 15, 10, 12, 0, };

int mg_rook_mobility[15] = { -36, -41, -22, -20, -17, -16, -17, -19, -14, -9, -4, -2, 2, 12, 51, };
int eg_rook_mobility[15] = { -65, -79, -50, -32, -21, -12, -4, 1, 3, 6, 9, 12, 13, 10, -4, };

int mg_queen_mobility[28] = { 0, 0, -28, -58, -32, -10, -4, -6, -6, -6, -5, -4, -3, -1, -1, -1, 0, -4, -5, -5, 4, 5, 6, 17, -7, -18, 4, 10, };
int eg_queen_mobility[28] = { 0, -1, -27, -48, -63, -68, -51, -30, -19, -11, -6, 1, 5, 7, 10, 11, 11, 12, 12, 9, 4, 2, -5, -13, -9, -11, -22, -4, };

int mg_king_mobility[9] = { 10, -18, -36, -13, -7, 13, -21, -4, 16, };
int eg_king_mobility[9] = { 27, 18, 13, 9, 5, 1, 1, 0, -4, };

int mg_minor_threatened_by_pawn = -19;
int eg_minor_threatened_by_pawn = -29;

int mg_minor_threatened_by_minor = -13;
int eg_minor_threatened_by_minor = -25;

int mg_rook_threatened_by_lesser = -17;
int eg_rook_threatened_by_lesser = -8;

int mg_queen_threatened_by_lesser = 2;
int eg_queen_threatened_by_lesser = 12;

int mg_minor_threatened_by_major = -13;
int eg_minor_threatened_by_major = -11;

int mg_passed_pawn[8] = { 0, 61, 29, 7, -32, -6, -2, 0, };
int eg_passed_pawn[8] = { 0, 68, 44, 29, 24, 11, 7, 0, };
int mg_passed_pawn_blocked[8] = { 0, 45, 30, 6, -35, -9, -11, 0, };
int eg_passed_pawn_blocked[8] = { 0, 13, -1, 4, 8, 8, 2, 0, };

int mg_passed_pawn_safe_advance = -4;
int eg_passed_pawn_safe_advance = 12;

int mg_passed_pawn_safe_path = -76;
int eg_passed_pawn_safe_path = 29;

int mg_passed_friendly_distance[8] = { 0, 2, 3, 9, 10, 4, 0, 0, };
int eg_passed_friendly_distance[8] = { 0, -10, -13, -12, -9, -3, 0, 0, };
int mg_passed_enemy_distance[8] = { 0, 1, -6, -7, -3, -2, 0, 0, };
int eg_passed_enemy_distance[8] = { 0, 29, 25, 15, 6, 0, -2, 0, };

int mg_isolated_pawn = -3;
int eg_isolated_pawn = -6;

int mg_doubled_pawn = -3;
int eg_doubled_pawn = -12;

int mg_backward_pawn = 0;
int eg_backward_pawn = 1;
int mg_backward_pawn_half_open = -5;
int eg_backward_pawn_half_open = -13;

int mg_chained_pawn[8] = { 0, 151, 25, 14, 10, 10, 1, 0, };
int eg_chained_pawn[8] = { 0, 39, 35, 12, 6, 4, -2, 0, };

int mg_double_bishop = 1;
int eg_double_bishop = 42;

int mg_rook_open_file = 22;
int eg_rook_open_file = 4;

int mg_rook_half_open_file = 8;
int eg_rook_half_open_file = 5;

int mg_rook_on_seventh = -4;
int eg_rook_on_seventh = 17;

int mg_knight_outpost = 19;
int eg_knight_outpost = -10;

int mg_knight_outpost_supported = 30;
int eg_knight_outpost_supported = 11;

int mg_center_control = 258;

int pawn_count_scale_offset = 78;
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
