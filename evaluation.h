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
	int king_attackers_weight[2];
	uint64_t passed_pawns;

	uint64_t attacked[2];
	uint64_t attacked_by_multiple[2];
	uint64_t attacked_by_piece[2][6];

	void init(Board &board)
	{
		mg_bonus[WHITE] = mg_bonus[BLACK] = 0;
		eg_bonus[WHITE] = eg_bonus[BLACK] = 0;
		king_attackers[WHITE] = king_attackers[BLACK] = 0;
		king_zone_attacks[WHITE] = king_zone_attacks[BLACK] = 0;
		king_attackers_weight[WHITE] = king_attackers_weight[BLACK] = 0;
		unsigned white_king_square = board.square(WHITE, KING);
		unsigned black_king_square = board.square(BLACK, KING);
		king_ring[WHITE] = king_ring_mask(white_king_square);
		king_ring[BLACK] = king_ring_mask(black_king_square);
		passed_pawns = 0ULL;
		attacked[WHITE] = attacked_by_piece[WHITE][KING] = piece_attacks(KING, white_king_square, 0ULL);
		attacked[BLACK] = attacked_by_piece[BLACK][KING] = piece_attacks(KING, black_king_square, 0ULL);
		uint64_t white_pawn_attacks = board.all_pawn_attacks(WHITE);
		attacked_by_multiple[WHITE] = attacked[WHITE] & white_pawn_attacks;
		attacked[WHITE]	 	      |= white_pawn_attacks;
		attacked_by_piece[WHITE][PAWN] = white_pawn_attacks;
		uint64_t black_pawn_attacks = board.all_pawn_attacks(BLACK);
		attacked_by_multiple[BLACK] = attacked[BLACK] & black_pawn_attacks;
		attacked[BLACK]	 	      |= black_pawn_attacks;
		attacked_by_piece[BLACK][PAWN] = black_pawn_attacks;
	}
};

struct Evaluation : Noncopyable
{
	bool use_pawn_hash_table = true;
	Eval_info info {};
	Pawn_hash_table pawn_hash_table;

	#define S(mg, eg) merge_score(mg, eg)

	int piece_value[6] = { S(47, 100), S(266, 358), S(291, 378), S(340, 645), S(697, 1224), S(0, 0), };

	int pawn_psqt[64] = {
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	        S(18, 23), S(18, 25), S(13, 46), S(73, 27), S(57, 35), S(48, 21), S(-26, 55), S(-39, 46),
	        S(9, 24), S(4, 32), S(21, 13), S(33, 2), S(31, 7), S(48, 7), S(19, 33), S(-17, 30),
	        S(-7, 21), S(-6, 9), S(2, -2), S(3, -13), S(18, -13), S(16, -8), S(-5, 2), S(-0, 4),
	        S(-10, 9), S(-7, 6), S(2, -4), S(9, -9), S(11, -11), S(7, -6), S(1, -4), S(-9, -6),
	        S(-16, 5), S(-12, 1), S(-3, -3), S(2, -1), S(9, -1), S(-2, -5), S(-3, -7), S(-18, -7),
	        S(-6, 10), S(-4, 9), S(-1, 4), S(3, 7), S(10, 12), S(1, 5), S(8, 1), S(-10, 2),
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	};
	
	int knight_psqt[64] = {
	        S(-97, -35), S(-92, -7), S(-71, 8), S(-38, -2), S(-22, 7), S(-76, -13), S(-102, -1), S(-70, -44),
	        S(-36, 4), S(-26, 8), S(-23, 8), S(-12, 11), S(-16, 3), S(-6, -1), S(-20, 6), S(-16, -8),
	        S(-20, -0), S(-12, 1), S(-7, 10), S(-10, 13), S(-5, 11), S(20, -8), S(-17, -3), S(-7, -5),
	        S(-11, 10), S(-4, 3), S(7, 8), S(23, 12), S(15, 10), S(21, 9), S(-1, 9), S(10, 4),
	        S(1, 7), S(6, 0), S(8, 12), S(16, 14), S(12, 20), S(13, 8), S(10, 3), S(5, 6),
	        S(-11, -10), S(-5, -6), S(-2, -3), S(-1, 8), S(12, 8), S(-0, -7), S(13, -7), S(2, -2),
	        S(-15, -9), S(-11, -5), S(-8, -9), S(6, -8), S(4, -8), S(4, -9), S(10, -10), S(8, 5),
	        S(-44, -1), S(-6, -11), S(-23, -10), S(-12, -7), S(-4, -4), S(-1, -14), S(-5, -1), S(-15, 1),
	};
	
	int bishop_psqt[64] = {
	        S(-19, -1), S(-58, 10), S(-53, 4), S(-79, 13), S(-77, 9), S(-73, 2), S(-41, -3), S(-52, -5),
	        S(-25, -6), S(-28, 3), S(-17, 1), S(-34, 5), S(-19, -2), S(-18, -2), S(-48, 9), S(-34, -7),
	        S(-10, 7), S(4, 5), S(-6, 6), S(5, 0), S(-4, 5), S(17, 13), S(-1, 8), S(-2, 13),
	        S(-13, 4), S(-8, 9), S(-2, 10), S(3, 25), S(-4, 17), S(-4, 12), S(-7, 6), S(-18, 5),
	        S(-5, -5), S(-15, 8), S(-5, 12), S(-0, 19), S(-6, 15), S(-3, 7), S(-12, 5), S(5, -13),
	        S(-2, -3), S(7, 2), S(3, 6), S(1, 7), S(5, 13), S(4, 3), S(10, -2), S(12, -9),
	        S(18, -3), S(8, -12), S(12, -12), S(0, -2), S(5, -1), S(14, -9), S(24, -12), S(20, -15),
	        S(10, -11), S(16, -11), S(6, -2), S(-4, -8), S(9, -8), S(-1, 2), S(12, -11), S(27, -26),
	};
	
	int rook_psqt[64] = {
	        S(3, 19), S(-5, 24), S(-19, 34), S(-25, 30), S(-8, 25), S(0, 26), S(9, 25), S(24, 19),
	        S(-14, 6), S(-16, 15), S(-8, 21), S(5, 11), S(-11, 14), S(5, 14), S(9, 10), S(18, 2),
	        S(-15, 20), S(12, 18), S(-2, 21), S(0, 18), S(20, 10), S(21, 9), S(56, 5), S(15, 4),
	        S(-13, 18), S(0, 13), S(-1, 18), S(-1, 13), S(7, 3), S(14, 1), S(16, 6), S(5, 2),
	        S(-16, 5), S(-18, 7), S(-12, 6), S(-8, 4), S(-3, 1), S(-10, 2), S(7, -4), S(-11, -4),
	        S(-16, -4), S(-13, -5), S(-9, -7), S(-8, -6), S(2, -10), S(2, -14), S(25, -24), S(3, -21),
	        S(-15, -14), S(-10, -11), S(-2, -11), S(-1, -12), S(6, -18), S(8, -20), S(23, -30), S(-8, -22),
	        S(-1, -7), S(-4, -11), S(-3, -6), S(4, -11), S(10, -16), S(7, -10), S(8, -15), S(7, -21),
	};
	
	int queen_psqt[64] = {
	        S(-27, 9), S(-19, 1), S(-26, 23), S(6, 16), S(-15, 23), S(-10, 22), S(56, -30), S(3, 4),
	        S(-8, -8), S(-22, 0), S(-23, 27), S(-39, 44), S(-39, 53), S(-12, 32), S(-0, 2), S(29, 14),
	        S(0, -7), S(-8, -2), S(-14, 25), S(-9, 28), S(-3, 33), S(-2, 21), S(14, -0), S(8, 11),
	        S(-11, 13), S(-6, 12), S(-13, 22), S(-23, 38), S(-20, 36), S(-6, 17), S(2, 26), S(4, 11),
	        S(3, -8), S(-14, 18), S(-13, 19), S(-18, 32), S(-17, 31), S(-8, 13), S(8, 5), S(11, -1),
	        S(0, -22), S(3, -6), S(-8, 10), S(-6, 5), S(-4, 11), S(2, 2), S(19, -19), S(10, -21),
	        S(5, -27), S(4, -26), S(10, -27), S(10, -19), S(11, -17), S(14, -37), S(23, -60), S(25, -66),
	        S(-1, -25), S(-8, -23), S(-2, -22), S(9, -21), S(6, -27), S(-3, -37), S(-2, -38), S(15, -45),
	};
	
	int king_psqt[64] = {
	        S(29, -95), S(55, -41), S(14, -17), S(-50, 11), S(17, -5), S(-9, 10), S(14, 14), S(86, -90),
	        S(-74, -6), S(2, 21), S(3, 26), S(72, 23), S(36, 34), S(32, 48), S(25, 48), S(37, 17),
	        S(-90, 2), S(50, 21), S(-12, 37), S(-29, 46), S(13, 51), S(73, 45), S(39, 44), S(15, 9),
	        S(-1, -19), S(-3, 15), S(-33, 34), S(-86, 48), S(-79, 49), S(-34, 43), S(-42, 34), S(-100, 9),
	        S(-19, -31), S(-13, 2), S(-22, 20), S(-66, 36), S(-61, 35), S(-16, 22), S(-37, 15), S(-92, -6),
	        S(-4, -36), S(30, -9), S(-2, 5), S(-18, 14), S(-8, 13), S(-10, 8), S(6, 0), S(-30, -17),
	        S(16, -30), S(-8, -2), S(-12, -1), S(-16, 2), S(-20, 6), S(-24, 5), S(-10, 5), S(0, -17),
	        S(25, -73), S(21, -27), S(13, -25), S(-21, -18), S(28, -33), S(-29, -17), S(13, -21), S(31, -68),
	};
	
	int knight_mobility[9] = { S(-133, -113), S(-47, -66), S(-24, -35), S(-16, -13), S(-6, -3), S(-3, 8), S(6, 12), S(15, 16), S(25, 15), };
	
	int bishop_mobility[14] = { S(-34, -139), S(-50, -74), S(-21, -34), S(-16, -13), S(-4, -4), S(3, 3), S(7, 11), S(12, 14), S(15, 18), S(19, 18), S(20, 18), S(30, 10), S(29, 9), S(38, -2), };
	
	int rook_mobility[15] = { S(-65, -93), S(-49, -93), S(-16, -57), S(-10, -40), S(-4, -35), S(-3, -18), S(-1, -12), S(-5, -4), S(-1, -3), S(3, 2), S(7, 5), S(7, 11), S(10, 15), S(15, 16), S(18, 14), };
	
	int queen_mobility[28] = { S(0, 0), S(0, 0), S(-93, -64), S(-52, -137), S(-39, -91), S(-13, -68), S(-8, -55), S(-7, -36), S(-6, -22), S(-6, -4), S(-4, 0), S(-1, 4), S(1, 9), S(3, 10), S(3, 13), S(3, 17), S(3, 17), S(1, 19), S(3, 18), S(6, 12), S(10, 8), S(25, -12), S(39, -22), S(53, -39), S(44, -38), S(73, -63), S(7, -53), S(-27, -65), };
	
	int king_mobility[9] = { S(20, 72), S(-45, 126), S(-46, 37), S(-22, 23), S(-14, 11), S(12, -6), S(-9, 1), S(7, -2), S(30, -15), };
	
	int isolated_pawn = S(-5, -7);
	
	int doubled_pawn = S(2, -16);
	
	int backward_pawn = S(0, -0);
	
	int backward_pawn_half_open = S(-15, -15);
	
	int chained_pawn[8] = { S(0, 0), S(134, 51), S(18, 49), S(14, 20), S(13, 10), S(15, 11), S(0, 0), S(0, 0), };
	
	int phalanx_pawn[8] = { S(0, 0), S(4, 175), S(70, 92), S(31, 32), S(13, 10), S(6, 4), S(4, -1), S(0, 0), };
	
	int passed_pawn[8] = { S(0, 0), S(61, 93), S(17, 70), S(-13, 47), S(-26, 31), S(-8, 17), S(17, 1), S(0, 0), };
	
	int passed_pawn_blocked[8] = { S(0, 0), S(18, 27), S(13, 9), S(-24, 25), S(-40, 19), S(-15, 18), S(6, 2), S(0, 0), };
	
	int passed_pawn_safe_advance = S(1, 10);
	
	int passed_pawn_safe_path = S(-64, 38);
	
	int passed_friendly_distance[8] = { S(0, 0), S(-6, -12), S(4, -18), S(12, -16), S(12, -11), S(4, -5), S(1, -2), S(0, 0), };
	
	int passed_enemy_distance[8] = { S(0, 0), S(12, 36), S(-5, 35), S(-5, 19), S(-3, 8), S(-1, -1), S(-3, -0), S(0, 0), };
	
	int knight_outpost = S(17, -6);
	
	int knight_outpost_supported = S(36, 20);
	
	int bishop_pawn = S(-4, -7);
	
	int double_bishop = S(16, 57);
	
	int rook_open_file = S(26, 8);
	
	int rook_half_open_file = S(8, 10);
	
	int rook_on_seventh = S(3, 21);
	
	int pawn_shelter[2][4][8] = {
	{
	        { S(0, 0), S(20, 31), S(45, 12), S(26, -3), S(25, -4), S(41, -11), S(39, -25), S(0, 0), },
	        { S(0, 0), S(-3, 12), S(18, 9), S(21, 1), S(15, -1), S(32, -6), S(50, -7), S(0, 0), },
	        { S(0, 0), S(8, 37), S(6, 12), S(14, -1), S(17, -5), S(12, 4), S(40, -0), S(0, 0), },
	        { S(0, 0), S(14, 7), S(-13, 5), S(15, -1), S(12, 1), S(12, -2), S(14, 3), S(0, 0), },
	},
	{
	        { S(0, 0), S(15, 60), S(59, 18), S(26, -1), S(40, -6), S(59, -16), S(64, -44), S(0, 0), },
	        { S(0, 0), S(21, 30), S(32, -1), S(25, -2), S(24, -3), S(55, -6), S(61, -14), S(0, 0), },
	        { S(0, 0), S(14, 25), S(12, 7), S(30, 1), S(35, -3), S(34, 3), S(64, -0), S(0, 0), },
	        { S(0, 0), S(-30, 30), S(-21, 4), S(16, 9), S(26, 6), S(24, 0), S(12, 7), S(0, 0), },
	},
	};
	
	int king_attacker_weight[6] = { S(0, 0), S(32, 92), S(18, 86), S(33, 5), S(16, 14), S(0, 0), };
	
	int king_zone_attack_count_weight = S(68, -11);
	
	int king_danger_no_queen_weight = S(-531, -328);
	
	int safe_knight_check = S(321, 2);
	
	int safe_bishop_check = S(228, 454);
	
	int safe_rook_check = S(339, 190);
	
	int safe_queen_check = S(175, 136);
	
	int king_zone_weak_square = S(105, -2);
	
	int king_danger_offset = S(4, 8);
	
	int center_control = S(3, -1);
	
	int minor_threatened_by_pawn = S(-38, -18);
	
	int minor_threatened_by_minor = S(-17, -18);
	
	int rook_threatened_by_lesser = S(-41, 2);
	
	int queen_threatened_by_lesser = S(-36, 11);
	
	int minor_threatened_by_major = S(-10, -15);

	#undef S

	int tempo_bonus = 19;

	int taper_start = 6377;
	int taper_end = 321;

	int evaluate_pawns(  Board &board, Color friendly);
	int evaluate_knights(Board &board, Color friendly);
	int evaluate_bishops(Board &board, Color friendly);
	int evaluate_rooks(  Board &board, Color friendly);
	int evaluate_queens( Board &board, Color friendly);
	int evaluate_kings(  Board &board, Color friendly);

	int evaluate_passed_pawns(Board &board, Color friendly);
	
	int evaluate_threats(Board &board, Color friendly);

	int evaluate_center_control(Color friendly);

	void note_king_attacks(Piece_type type, uint64_t attacks, Color friendly);

	int scale_factor(Board &board, int eg_value);

	int evaluate(Board &board);
};
