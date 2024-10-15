#pragma once

#include "utility.h"
#include "board.h"
#include "transposition_table.h"

struct Eval_info : Noncopyable
{
	int mg_bonus[2];
	int eg_bonus[2];
	uint64_t king_ring[2];
	unsigned king_attackers[2];
	unsigned king_zone_attacks[2];
	int mg_king_attackers_weight[2];
	int eg_king_attackers_weight[2];
	uint64_t passed_pawns;

	uint64_t attacked[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		king_attackers[WHITE] = king_attackers[BLACK] = 0;
		king_zone_attacks[WHITE] = king_zone_attacks[BLACK] = 0;
		mg_king_attackers_weight[WHITE] = mg_king_attackers_weight[BLACK] = 0;
		eg_king_attackers_weight[WHITE] = eg_king_attackers_weight[BLACK] = 0;
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
	bool use_pawn_hash_table = true;
	Eval_info info {};
	Pawn_hash_table pawn_hash_table;

	int mg_piece_value[6] = { 46, 268, 278, 345, 714, 0, };
int eg_piece_value[6] = { 96, 342, 351, 626, 1192, 0, };

int mg_pawn_psqt[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        26, 34, 17, 77, 56, 49, -19, -39,
        12, 6, 23, 34, 32, 52, 21, -14,
        -6, -5, 0, -1, 15, 15, -6, -4,
        -10, -9, -2, 6, 8, 4, -3, -14,
        -15, -11, -4, -0, 9, -4, -1, -21,
        -4, 1, 2, 5, 15, 4, 14, -11,
        0, 0, 0, 0, 0, 0, 0, 0,
};
int eg_pawn_psqt[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        20, 17, 43, 24, 34, 20, 52, 44,
        22, 31, 12, 1, 6, 5, 31, 28,
        20, 9, -2, -13, -13, -9, 3, 5,
        10, 6, -4, -10, -11, -6, -3, -4,
        6, 2, -3, 1, 1, -5, -7, -5,
        12, 12, 6, 8, 14, 7, 4, 5,
        0, 0, 0, 0, 0, 0, 0, 0,
};

int mg_knight_psqt[64] = {
        -95, -102, -73, -40, -17, -68, -103, -69,
        -38, -28, -25, -13, -18, 4, -21, -13,
        -20, -12, -8, -8, -2, 25, -5, -1,
        -11, -2, 7, 27, 19, 29, 2, 20,
        1, 7, 10, 17, 18, 20, 25, 12,
        -11, -6, -2, -2, 12, 1, 13, 3,
        -15, -11, -8, 6, 3, 5, 7, 9,
        -44, -7, -24, -13, -4, 0, -5, -17,
};
int eg_knight_psqt[64] = {
        -33, -5, 8, -1, 6, -14, -0, -42,
        4, 9, 10, 11, 4, -2, 6, -8,
        1, 3, 14, 16, 13, -5, -4, -7,
        11, 5, 12, 14, 12, 10, 12, 2,
        9, 1, 17, 17, 23, 11, 4, 8,
        -8, -4, 1, 12, 12, -2, -4, -1,
        -9, -4, -7, -5, -6, -7, -8, 6,
        -3, -10, -9, -7, -3, -12, 0, 3,
};

int mg_bishop_psqt[64] = {
        -19, -60, -53, -83, -83, -80, -43, -52,
        -23, -25, -21, -36, -26, -22, -39, -27,
        -9, 5, -4, 1, -5, 25, 6, -1,
        -16, -7, -5, 2, -3, -0, -7, -18,
        -5, -19, -4, 2, 1, -1, -10, 6,
        -4, 8, 4, 4, 8, 7, 10, 16,
        21, 8, 16, 1, 7, 16, 30, 23,
        8, 19, 7, -3, 8, 2, 14, 30,
};
int eg_bishop_psqt[64] = {
        6, 13, 8, 16, 15, 6, 2, -3,
        -4, 5, 2, 8, 1, 0, 7, -3,
        12, 2, 7, -0, 4, 10, 8, 14,
        6, 10, 8, 21, 12, 11, 8, 10,
        0, 8, 11, 14, 13, 5, 6, -7,
        -0, 4, 4, 8, 12, 6, -1, -3,
        2, -11, -9, -3, 1, -6, -8, -12,
        -10, -1, -2, -3, -6, 5, -8, -22,
};

int mg_rook_psqt[64] = {
        7, -5, -13, -19, -1, 3, 14, 28,
        -15, -15, -10, 5, -9, 10, 17, 27,
        -16, 11, -1, 3, 26, 30, 68, 28,
        -14, -1, -2, -3, 5, 17, 22, 13,
        -18, -19, -13, -9, -5, -10, 13, -4,
        -17, -14, -10, -11, -0, 4, 31, 11,
        -16, -11, -3, -1, 6, 11, 30, -3,
        -2, -5, -4, 3, 10, 8, 13, 10,
};
int eg_rook_psqt[64] = {
        21, 26, 33, 30, 24, 26, 26, 20,
        10, 18, 22, 13, 14, 15, 11, 4,
        21, 18, 20, 17, 9, 7, 3, 3,
        19, 15, 18, 15, 4, 1, 6, 2,
        7, 8, 6, 5, 2, 2, -4, -4,
        -3, -4, -6, -4, -9, -13, -24, -22,
        -12, -9, -10, -11, -16, -18, -29, -21,
        -4, -8, -4, -8, -13, -8, -14, -18,
};

int mg_queen_psqt[64] = {
        -27, -19, -25, -0, -16, -9, 51, 4,
        -6, -24, -29, -48, -50, -14, -5, 38,
        -0, -10, -17, -15, -9, 10, 22, 20,
        -10, -7, -17, -29, -21, -5, 5, 12,
        4, -14, -14, -16, -15, -7, 12, 17,
        0, 4, -8, -7, -4, 4, 23, 18,
        6, 5, 10, 11, 11, 16, 28, 28,
        -0, -8, -1, 10, 6, -2, 2, 19,
};
int eg_queen_psqt[64] = {
        12, 5, 26, 22, 27, 23, -24, 8,
        -7, 2, 28, 46, 59, 34, 8, 16,
        -3, 2, 29, 29, 35, 26, 7, 18,
        9, 10, 20, 37, 36, 22, 29, 15,
        -9, 15, 17, 31, 31, 17, 5, 5,
        -23, -7, 10, 8, 13, 2, -15, -15,
        -26, -23, -24, -16, -15, -34, -54, -57,
        -23, -19, -22, -20, -26, -33, -34, -41,
};

int mg_king_psqt[64] = {
        30, 33, 20, -50, 2, -14, 15, 107,
        -62, -2, -2, 86, 43, 36, 49, 71,
        -61, 64, -1, -24, 27, 93, 61, 43,
        16, -4, -33, -76, -75, -26, -35, -77,
        -7, -17, -18, -67, -57, -15, -38, -80,
        6, 25, -2, -18, -8, -11, -3, -21,
        21, -21, -14, -16, -22, -27, -21, 4,
        34, 11, 14, -21, 31, -31, 5, 43,
};
int eg_king_psqt[64] = {
        -90, -33, -17, 10, -3, 11, 16, -86,
        -5, 24, 29, 20, 34, 46, 44, 15,
        -0, 20, 35, 44, 47, 40, 38, 5,
        -18, 15, 32, 43, 45, 40, 32, 6,
        -29, 4, 18, 33, 32, 20, 15, -5,
        -33, -7, 5, 13, 12, 9, 2, -16,
        -28, 0, -1, 1, 5, 5, 7, -15,
        -69, -23, -25, -18, -33, -17, -19, -66,
};

int mg_knight_mobility[9] = { -127, -43, -20, -12, -2, -0, 10, 19, 29, };
int eg_knight_mobility[9] = { -101, -58, -27, -7, 3, 13, 16, 19, 16, };

int mg_bishop_mobility[14] = { -27, -47, -20, -14, -2, 5, 10, 15, 18, 21, 23, 34, 37, 50, };
int eg_bishop_mobility[14] = { -119, -68, -31, -10, -3, 3, 12, 16, 20, 22, 23, 17, 17, 8, };

int mg_rook_mobility[15] = { -69, -46, -14, -9, -3, -1, 0, -4, -0, 4, 9, 9, 12, 18, 22, };
int eg_rook_mobility[15] = { -89, -84, -53, -37, -31, -14, -9, -0, 0, 5, 8, 13, 17, 18, 16, };

int mg_queen_mobility[28] = { 0, 0, -99, -47, -42, -14, -8, -7, -6, -6, -4, -1, 1, 4, 5, 5, 5, 4, 5, 7, 12, 24, 39, 53, 48, 86, 15, -17, };
int eg_queen_mobility[28] = { 0, 0, -68, -147, -92, -69, -56, -37, -22, -3, 2, 5, 11, 12, 14, 19, 19, 22, 22, 18, 14, -3, -12, -27, -28, -52, -44, -53, };

int mg_king_mobility[9] = { 8, -87, -61, -34, -17, 13, -6, 17, 42, };
int eg_king_mobility[9] = { 53, 118, 36, 24, 12, -5, 1, -2, -15, };

int mg_isolated_pawn = -4;
int eg_isolated_pawn = -7;

int mg_doubled_pawn = 1;
int eg_doubled_pawn = -16;

int mg_backward_pawn = 0;
int eg_backward_pawn = 1;

int mg_backward_pawn_half_open = -16;
int eg_backward_pawn_half_open = -15;

int mg_chained_pawn[8] = { 0, 113, 18, 17, 15, 11, 1, 0, };
int eg_chained_pawn[8] = { 0, 66, 56, 22, 10, 8, -3, 0, };

int mg_passed_pawn[8] = { 0, 69, 19, -8, -23, -8, 15, 0, };
int eg_passed_pawn[8] = { 0, 85, 66, 45, 29, 14, 0, 0, };

int mg_passed_pawn_blocked[8] = { 0, 28, 14, -20, -36, -14, 4, 0, };
int eg_passed_pawn_blocked[8] = { 0, 20, 3, 22, 17, 15, 1, 0, };

int mg_passed_pawn_safe_advance = 1;
int eg_passed_pawn_safe_advance = 10;

int mg_passed_pawn_safe_path = -70;
int eg_passed_pawn_safe_path = 39;

int mg_passed_friendly_distance[8] = { 0, -8, 4, 12, 11, 4, 1, 0, };
int eg_passed_friendly_distance[8] = { 0, -11, -17, -16, -11, -5, -3, 0, };

int mg_passed_enemy_distance[8] = { 0, 10, -6, -5, -4, -1, -3, 0, };
int eg_passed_enemy_distance[8] = { 0, 35, 34, 18, 7, -1, -0, 0, };

int mg_knight_outpost = 17;
int eg_knight_outpost = -5;

int mg_knight_outpost_supported = 37;
int eg_knight_outpost_supported = 22;

int mg_double_bishop = 17;
int eg_double_bishop = 56;

int mg_rook_open_file = 26;
int eg_rook_open_file = 7;

int mg_rook_half_open_file = 9;
int eg_rook_half_open_file = 8;

int mg_rook_on_seventh = 2;
int eg_rook_on_seventh = 21;

int mg_pawn_shelter[2][4][8] = {
{
        { 0, 32, 46, 31, 31, 47, 45, 0, },
        { 0, 6, 19, 22, 16, 29, 53, 0, },
        { 0, 12, 12, 14, 16, 13, 41, 0, },
        { 0, 17, -11, 14, 12, 9, 14, 0, },
},
{
        { 0, 25, 55, 29, 43, 60, 70, 0, },
        { 0, 28, 37, 28, 28, 56, 66, 0, },
        { 0, 22, 15, 30, 35, 35, 65, 0, },
        { 0, -24, -26, 15, 27, 25, 11, 0, },
},
};
int eg_pawn_shelter[2][4][8] = { {
        { 0, 27, 12, -4, -5, -11, -26, 0, },
        { 0, 12, 9, 1, -1, -5, -6, 0, },
        { 0, 35, 10, -0, -4, 5, 1, 0, },
        { 0, 6, 5, -0, 1, -2, 3, 0, },
},
{
        { 0, 61, 19, -2, -6, -15, -43, 0, },
        { 0, 31, -1, -2, -4, -5, -13, 0, },
        { 0, 24, 6, 2, -3, 4, 2, 0, },
        { 0, 27, 5, 9, 5, -2, 8, 0, },
},
};

int mg_pawn_storm[2][4][8] = {
{
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
},
{
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
},
};
int eg_pawn_storm[2][4][8] = { {
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
},
{
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
        { 0, 0, 0, 0, 0, 0, 0, 0, },
},
};

int mg_king_attacker_weight[6] = { 0, 9, 26, 18, 6, 0, };
int eg_king_attacker_weight[6] = { 0, 92, 123, 25, 25, 0, };

int mg_king_zone_attack_count_weight = 108;
int eg_king_zone_attack_count_weight = 0;

int mg_king_danger_no_queen_weight = -460;
int eg_king_danger_no_queen_weight = -405;

int mg_safe_knight_check = 379;
int eg_safe_knight_check = 15;

int mg_safe_bishop_check = 228;
int eg_safe_bishop_check = 473;

int mg_safe_rook_check = 363;
int eg_safe_rook_check = 165;

int mg_safe_queen_check = 182;
int eg_safe_queen_check = 139;

int mg_king_danger_offset = 33;
int eg_king_danger_offset = -25;

int mg_center_control = 4;
int eg_center_control = -0;

int mg_minor_threatened_by_pawn = -38;
int eg_minor_threatened_by_pawn = -17;

int mg_minor_threatened_by_minor = -18;
int eg_minor_threatened_by_minor = -19;

int mg_rook_threatened_by_lesser = -41;
int eg_rook_threatened_by_lesser = 2;

int mg_queen_threatened_by_lesser = -36;
int eg_queen_threatened_by_lesser = 8;

int mg_minor_threatened_by_major = -10;
int eg_minor_threatened_by_major = -13;

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
