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

	int mg_piece_value[6] = { 42, 229, 241, 310, 651, 0, };
int eg_piece_value[6] = { 88, 288, 302, 557, 1069, 0, };

int mg_pawn_psqt[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        24, 31, 14, 75, 56, 48, -15, -36,
        11, 5, 21, 30, 29, 47, 20, -11,
        -6, -5, 0, -1, 14, 14, -4, -4,
        -10, -9, -1, 5, 8, 4, -5, -16,
        -14, -10, -4, 0, 9, -8, -2, -19,
        -3, 1, 2, 5, 13, 7, 14, -9,
        0, 0, 0, 0, 0, 0, 0, 0,
};
int eg_pawn_psqt[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        25, 20, 51, 25, 33, 32, 52, 48,
        20, 28, 12, -0, 3, 8, 29, 26,
        18, 9, -1, -12, -11, -6, 3, 4,
        8, 5, -5, -9, -10, -7, -2, -3,
        5, 1, -3, -1, -2, -4, -6, -4,
        10, 10, 5, 6, 11, 5, 2, 1,
        0, 0, 0, 0, 0, 0, 0, 0,
};

int mg_knight_psqt[64] = {
        -82, -90, -63, -32, -11, -58, -90, -59,
        -29, -21, -18, -8, -12, 8, -13, -7,
        -14, -7, -3, -3, 3, 26, -0, 3,
        -6, 3, 11, 29, 21, 30, 7, 23,
        6, 12, 14, 21, 22, 23, 28, 16,
        -5, -0, 3, 3, 16, 5, 17, 8,
        -9, -5, -2, 10, 8, 9, 11, 12,
        -35, -1, -16, -7, 1, 5, 0, -11,
};
int eg_knight_psqt[64] = {
        -22, 3, 14, 6, 12, -6, 7, -31,
        11, 15, 16, 17, 11, 5, 12, -1,
        8, 10, 20, 21, 19, 3, 3, 2,
        16, 11, 17, 20, 18, 16, 17, 8,
        15, 8, 21, 22, 27, 16, 10, 14,
        -1, 3, 7, 17, 17, 4, 3, 5,
        -2, 3, -0, 1, 1, 0, -1, 13,
        5, -3, -2, 1, 3, -4, 6, 10,
};

int mg_bishop_psqt[64] = {
        -13, -50, -43, -71, -71, -68, -35, -43,
        -17, -19, -15, -28, -19, -16, -31, -19,
        -4, 9, 0, 5, -1, 27, 10, 4,
        -10, -2, -1, 6, 1, 4, -2, -12,
        -1, -13, 0, 6, 5, 4, -5, 9,
        1, 12, 7, 8, 11, 10, 13, 19,
        23, 12, 19, 5, 10, 18, 31, 25,
        12, 22, 11, 1, 11, 6, 17, 31,
};
int eg_bishop_psqt[64] = {
        10, 17, 12, 20, 19, 10, 8, 2,
        1, 10, 7, 12, 6, 6, 11, 2,
        16, 8, 12, 6, 9, 14, 12, 18,
        11, 15, 12, 25, 16, 16, 12, 14,
        5, 13, 16, 17, 17, 10, 11, -1,
        5, 9, 9, 13, 16, 10, 4, 3,
        7, -5, -4, 3, 6, -0, -1, -6,
        -4, 4, 3, 2, -1, 10, -2, -14,
};

int mg_rook_psqt[64] = {
        7, -3, -10, -16, 0, 2, 11, 26,
        -12, -12, -7, 6, -7, 10, 17, 25,
        -13, 12, 1, 4, 25, 29, 63, 26,
        -11, 0, -0, -1, 6, 17, 21, 12,
        -15, -16, -10, -7, -3, -8, 14, -3,
        -14, -11, -8, -8, 1, 5, 30, 11,
        -13, -8, -1, 1, 6, 11, 28, -2,
        -0, -3, -2, 4, 10, 9, 12, 9,
};
int eg_rook_psqt[64] = {
        24, 28, 34, 31, 26, 27, 29, 23,
        13, 21, 24, 15, 17, 17, 15, 8,
        22, 20, 22, 19, 12, 10, 7, 7,
        21, 17, 20, 17, 7, 4, 8, 5,
        9, 11, 9, 8, 5, 5, -0, -0,
        0, 0, -2, -0, -4, -9, -18, -16,
        -7, -4, -5, -6, -11, -12, -21, -15,
        -0, -3, 0, -4, -9, -4, -8, -11,
};

int mg_queen_psqt[64] = {
        -25, -17, -22, -1, -14, -8, 45, 4,
        -5, -22, -26, -43, -46, -13, -4, 34,
        0, -9, -15, -14, -8, 10, 21, 19,
        -8, -6, -15, -26, -18, -4, 5, 11,
        4, -13, -13, -14, -13, -6, 11, 15,
        0, 3, -7, -6, -3, 4, 20, 16,
        6, 5, 9, 10, 10, 14, 25, 25,
        0, -7, -1, 9, 5, -2, 1, 17,
};
int eg_queen_psqt[64] = {
        15, 9, 27, 26, 29, 25, -17, 12,
        -2, 6, 30, 46, 58, 35, 12, 20,
        1, 6, 29, 30, 36, 28, 10, 20,
        11, 13, 22, 37, 36, 23, 31, 18,
        -4, 17, 19, 31, 31, 19, 9, 9,
        -17, -2, 13, 11, 15, 6, -9, -10,
        -19, -17, -18, -11, -9, -28, -45, -47,
        -17, -13, -16, -14, -19, -26, -27, -34,
};

int mg_king_psqt[64] = {
        25, 31, 19, -44, 3, -10, 16, 104,
        -58, 4, 4, 84, 46, 39, 51, 68,
        -57, 60, 1, -17, 30, 87, 59, 37,
        11, -3, -30, -67, -66, -23, -29, -74,
        -21, -20, -21, -63, -54, -21, -42, -89,
        -18, 10, -14, -21, -12, -26, -17, -46,
        11, -21, -20, -14, -18, -30, -20, -7,
        40, 17, 21, -18, 28, -21, 9, 49,
};
int eg_king_psqt[64] = {
        -81, -33, -18, 5, -5, 7, 10, -80,
        -5, 19, 22, 14, 26, 37, 37, 11,
        1, 18, 29, 36, 39, 34, 34, 6,
        -15, 13, 27, 35, 37, 34, 28, 6,
        -24, 4, 15, 26, 24, 16, 14, -3,
        -25, -4, 4, 9, 8, 6, 5, -10,
        -23, 0, 0, 2, 4, 5, 5, -12,
        -65, -23, -16, -11, -23, -9, -17, -64,
};

int mg_knight_mobility[9] = { -106, -29, -8, -1, 8, 10, 19, 27, 36, };
int eg_knight_mobility[9] = { -77, -36, -9, 10, 20, 28, 31, 34, 32, };

int mg_bishop_mobility[14] = { -16, -35, -10, -4, 7, 13, 17, 22, 25, 28, 30, 40, 42, 55, };
int eg_bishop_mobility[14] = { -100, -50, -17, 2, 9, 15, 22, 26, 30, 31, 32, 26, 27, 19, };

int mg_rook_mobility[15] = { -60, -39, -12, -7, -1, 1, 2, -2, 2, 6, 10, 10, 13, 18, 23, };
int eg_rook_mobility[15] = { -79, -69, -39, -25, -19, -5, 0, 8, 8, 12, 15, 19, 23, 24, 21, };

int mg_queen_mobility[28] = { 0, 0, -91, -41, -38, -12, -7, -6, -5, -5, -3, -0, 2, 4, 5, 5, 5, 3, 4, 6, 11, 22, 37, 49, 44, 80, 16, -13, };
int eg_queen_mobility[28] = { 0, 0, -63, -131, -77, -56, -44, -26, -13, 5, 9, 12, 17, 18, 20, 25, 24, 27, 28, 24, 20, 5, -4, -18, -18, -42, -33, -43, };

int mg_king_mobility[9] = { 7, -79, -55, -29, -14, 13, -7, 11, 35, };
int eg_king_mobility[9] = { 46, 109, 29, 19, 10, -4, 0, -1, -13, };

int mg_isolated_pawn = -4;
int eg_isolated_pawn = -7;

int mg_doubled_pawn = 2;
int eg_doubled_pawn = -16;

int mg_backward_pawn = 0;
int eg_backward_pawn = 1;

int mg_backward_pawn_half_open = -14;
int eg_backward_pawn_half_open = -14;

int mg_chained_pawn[8] = { 0, 105, 16, 15, 13, 10, 1, 0, };
int eg_chained_pawn[8] = { 0, 60, 51, 20, 9, 7, -2, 0, };

int mg_passed_pawn[8] = { 0, 66, 17, -7, -22, -11, 8, 0, };
int eg_passed_pawn[8] = { 0, 92, 67, 40, 25, 11, -1, 0, };

int mg_passed_pawn_blocked[8] = { 0, 29, 13, -17, -34, -16, -2, 0, };
int eg_passed_pawn_blocked[8] = { 0, 32, 9, 19, 14, 13, 0, 0, };

int mg_passed_pawn_safe_advance = 0;
int eg_passed_pawn_safe_advance = 10;

int mg_passed_pawn_safe_path = -63;
int eg_passed_pawn_safe_path = 35;

int mg_passed_friendly_distance[8] = { 0, -8, 4, 10, 10, 4, 1, 0, };
int eg_passed_friendly_distance[8] = { 0, -14, -17, -15, -10, -4, -2, 0, };

int mg_passed_enemy_distance[8] = { 0, 9, -5, -5, -3, -0, -2, 0, };
int eg_passed_enemy_distance[8] = { 0, 32, 31, 17, 7, -1, -0, 0, };

int mg_knight_outpost = 16;
int eg_knight_outpost = -5;

int mg_knight_outpost_supported = 34;
int eg_knight_outpost_supported = 19;

int mg_double_bishop = 16;
int eg_double_bishop = 51;

int mg_rook_open_file = 24;
int eg_rook_open_file = 6;

int mg_rook_half_open_file = 8;
int eg_rook_half_open_file = 7;

int mg_rook_on_seventh = 2;
int eg_rook_on_seventh = 19;

int mg_pawn_shelter[2][4][8] = {
{
        { 27, 38, 42, 31, 26, 44, 19, 0, },
        { 50, 43, 22, 22, 12, 18, -24, 0, },
        { 40, 34, 16, 16, 17, 11, -10, 0, },
        { 10, 14, 8, 14, 17, -8, -38, 0, },
},
{
        { 0, 54, 51, 33, 17, 16, -16, 0, },
        { 0, 57, 50, 26, 23, 30, 1, 0, },
        { 0, 54, 29, 34, 23, -3, -23, 0, },
        { 0, 16, 21, 28, 17, -36, -54, 0, },
},
};
int eg_pawn_shelter[2][4][8] = { {
        { -9, -17, -8, -3, -0, 19, 31, 0, },
        { -1, -9, -4, -9, -7, 11, 46, 0, },
        { 5, -3, -2, -11, -15, 1, 32, 0, },
        { 10, 0, -4, -5, -16, 1, 41, 0, },
},
{
        { 0, -19, -4, -3, 19, 48, 59, 0, },
        { 0, -10, -4, -11, -11, 3, 34, 0, },
        { 0, -1, 2, -11, -10, 2, 3, 0, },
        { 0, 6, 5, -9, -8, 20, 40, 0, },
},
};

int mg_king_attacker_weight[6] = { 0, 8, 25, 17, 6, 0, };
int eg_king_attacker_weight[6] = { 0, 92, 113, 28, 26, 0, };

int mg_king_zone_attack_count_weight = 102;
int eg_king_zone_attack_count_weight = 0;

int mg_king_danger_no_queen_weight = -444;
int eg_king_danger_no_queen_weight = -384;

int mg_safe_knight_check = 362;
int eg_safe_knight_check = 8;

int mg_safe_bishop_check = 216;
int eg_safe_bishop_check = 434;

int mg_safe_rook_check = 344;
int eg_safe_rook_check = 154;

int mg_safe_queen_check = 172;
int eg_safe_queen_check = 127;

int mg_king_danger_offset = 33;
int eg_king_danger_offset = -26;

int mg_center_control = 3;
int eg_center_control = 0;

int mg_minor_threatened_by_pawn = -34;
int eg_minor_threatened_by_pawn = -16;

int mg_minor_threatened_by_minor = -17;
int eg_minor_threatened_by_minor = -18;

int mg_rook_threatened_by_lesser = -38;
int eg_rook_threatened_by_lesser = 2;

int mg_queen_threatened_by_lesser = -33;
int eg_queen_threatened_by_lesser = 7;

int mg_minor_threatened_by_major = -9;
int eg_minor_threatened_by_major = -12;

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
