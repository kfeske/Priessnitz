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
	int pawn_king_danger[2];

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
		uint64_t white_pawn_attacks = board.all_pawn_attacks(WHITE, board.pieces(WHITE, PAWN));
		attacked_by_multiple[WHITE] = attacked[WHITE] & white_pawn_attacks;
		attacked[WHITE]	 	      |= white_pawn_attacks;
		attacked_by_piece[WHITE][PAWN] = white_pawn_attacks;
		uint64_t black_pawn_attacks = board.all_pawn_attacks(BLACK, board.pieces(BLACK, PAWN));
		attacked_by_multiple[BLACK] = attacked[BLACK] & black_pawn_attacks;
		attacked[BLACK]	 	      |= black_pawn_attacks;
		attacked_by_piece[BLACK][PAWN] = black_pawn_attacks;
		pawn_king_danger[WHITE] = pawn_king_danger[BLACK] = 0;
	}
};

struct Evaluation : Noncopyable
{
	bool use_pawn_hash_table = true;
	Eval_info info {};
	Pawn_hash_table pawn_hash_table;

	#define S(mg, eg) merge_score(mg, eg)

	int piece_value[6] = { S(46, 102), S(283, 377), S(307, 391), S(359, 672), S(735, 1266), S(0, 0), };

	int pawn_psqt[64] = {
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	        S(21, 27), S(23, 28), S(16, 49), S(83, 29), S(64, 38), S(55, 24), S(-23, 59), S(-38, 51),
	        S(9, 28), S(2, 35), S(23, 15), S(35, 3), S(30, 9), S(48, 9), S(15, 36), S(-21, 33),
	        S(-10, 22), S(-7, 9), S(1, -2), S(2, -14), S(17, -14), S(16, -8), S(-7, 2), S(-5, 4),
	        S(-9, 11), S(-6, 7), S(4, -3), S(12, -10), S(14, -11), S(10, -5), S(-0, -3), S(-9, -5),
	        S(-16, 9), S(-12, 4), S(-3, -1), S(2, 2), S(10, 1), S(-1, -2), S(-3, -4), S(-21, -4),
	        S(-5, 13), S(-4, 12), S(1, 6), S(5, 9), S(13, 14), S(2, 7), S(7, 3), S(-10, 4),
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	};
	
	int knight_psqt[64] = {
	        S(-109, -44), S(-104, -14), S(-83, 2), S(-47, -8), S(-33, 2), S(-87, -19), S(-118, -6), S(-85, -52),
	        S(-30, 10), S(-20, 16), S(-15, 17), S(-3, 19), S(-10, 12), S(6, 7), S(-12, 14), S(-10, -1),
	        S(-14, 8), S(-5, 10), S(1, 20), S(2, 22), S(11, 19), S(31, 0), S(-8, 5), S(-1, 2),
	        S(-4, 18), S(3, 12), S(15, 17), S(23, 22), S(22, 20), S(30, 19), S(17, 15), S(16, 12),
	        S(1, 3), S(12, -2), S(12, 12), S(17, 12), S(12, 20), S(20, 8), S(13, 5), S(9, 4),
	        S(-14, -17), S(-7, -11), S(-3, -7), S(-3, 4), S(10, 4), S(0, -10), S(12, -11), S(-1, -8),
	        S(-19, -17), S(-15, -11), S(-13, -14), S(1, -13), S(-1, -13), S(0, -14), S(7, -16), S(5, -3),
	        S(-49, -9), S(-10, -19), S(-27, -17), S(-16, -14), S(-8, -11), S(-5, -20), S(-10, -8), S(-19, -6),
	};
	
	int bishop_psqt[64] = {
	        S(-23, -0), S(-64, 10), S(-61, 5), S(-87, 14), S(-85, 10), S(-77, 2), S(-54, -1), S(-62, -5),
	        S(-27, -7), S(-29, 2), S(-20, 1), S(-36, 5), S(-17, -4), S(-20, -2), S(-42, 9), S(-35, -8),
	        S(-11, 7), S(2, 6), S(-7, 6), S(6, 0), S(-5, 5), S(25, 12), S(-6, 8), S(5, 10),
	        S(-12, 4), S(2, 9), S(4, 10), S(6, 26), S(5, 19), S(4, 13), S(7, 10), S(-16, 6),
	        S(-2, -5), S(-8, 10), S(-0, 15), S(6, 21), S(1, 18), S(6, 12), S(-5, 7), S(14, -13),
	        S(-3, -3), S(8, 3), S(4, 7), S(1, 8), S(4, 13), S(6, 4), S(11, -3), S(10, -9),
	        S(16, -3), S(7, -12), S(10, -12), S(-2, -3), S(4, -3), S(13, -8), S(24, -13), S(20, -17),
	        S(9, -11), S(14, -12), S(5, -2), S(-5, -9), S(8, -8), S(-2, 2), S(12, -12), S(25, -27),
	};
	
	int rook_psqt[64] = {
	        S(3, 21), S(-6, 25), S(-18, 34), S(-24, 30), S(-5, 24), S(0, 26), S(8, 27), S(25, 20),
	        S(-12, 7), S(-15, 16), S(-6, 21), S(8, 11), S(-10, 14), S(6, 13), S(4, 12), S(17, 3),
	        S(-18, 19), S(9, 17), S(-4, 20), S(-2, 17), S(15, 10), S(18, 8), S(48, 5), S(8, 5),
	        S(-11, 18), S(7, 13), S(6, 18), S(3, 13), S(14, 4), S(24, 1), S(23, 8), S(7, 2),
	        S(-15, 6), S(-10, 9), S(-5, 6), S(-4, 5), S(5, 2), S(2, 4), S(17, 1), S(-7, -2),
	        S(-15, -4), S(-11, -5), S(-7, -7), S(-6, -6), S(3, -10), S(6, -14), S(27, -24), S(4, -21),
	        S(-15, -15), S(-10, -11), S(-0, -12), S(0, -13), S(6, -19), S(9, -21), S(24, -31), S(-8, -23),
	        S(-1, -8), S(-4, -11), S(-3, -7), S(5, -12), S(11, -17), S(8, -12), S(8, -16), S(7, -22),
	};
	
	int queen_psqt[64] = {
	        S(-31, 12), S(-19, 2), S(-31, 26), S(4, 17), S(-19, 24), S(-20, 21), S(55, -33), S(2, 2),
	        S(-8, -8), S(-25, 1), S(-29, 28), S(-49, 46), S(-45, 52), S(-28, 31), S(-10, 2), S(26, 14),
	        S(-1, -10), S(-12, -3), S(-17, 23), S(-15, 25), S(-13, 29), S(-7, 16), S(4, -4), S(5, 3),
	        S(-9, 13), S(1, 9), S(-7, 19), S(-23, 34), S(-16, 35), S(3, 14), S(11, 19), S(10, 7),
	        S(5, -7), S(-7, 18), S(-9, 20), S(-13, 32), S(-11, 31), S(1, 14), S(15, 5), S(14, -2),
	        S(-0, -22), S(4, -5), S(-6, 11), S(-5, 6), S(-4, 11), S(4, 5), S(22, -21), S(10, -25),
	        S(5, -26), S(4, -24), S(10, -25), S(9, -17), S(10, -14), S(14, -33), S(22, -56), S(23, -67),
	        S(-2, -21), S(-8, -20), S(-3, -21), S(9, -19), S(5, -25), S(-3, -36), S(2, -37), S(17, -45),
	};
	
	int king_psqt[64] = {
	        S(55, -109), S(63, -46), S(19, -20), S(-56, 11), S(28, -8), S(-5, 9), S(26, 12), S(131, -105),
	        S(-92, -8), S(-8, 26), S(-7, 31), S(74, 27), S(35, 39), S(29, 53), S(13, 54), S(41, 15),
	        S(-96, 0), S(44, 26), S(-19, 41), S(-41, 51), S(8, 56), S(67, 49), S(35, 48), S(23, 5),
	        S(8, -23), S(-6, 18), S(-42, 37), S(-96, 51), S(-77, 51), S(-33, 46), S(-40, 37), S(-86, 4),
	        S(-4, -36), S(-11, 5), S(-21, 22), S(-67, 38), S(-55, 37), S(-11, 24), S(-36, 18), S(-81, -10),
	        S(5, -40), S(26, -6), S(-1, 7), S(-19, 16), S(-7, 15), S(-6, 10), S(5, 3), S(-21, -21),
	        S(18, -33), S(-18, 2), S(-16, 3), S(-23, 5), S(-27, 10), S(-28, 8), S(-20, 10), S(1, -19),
	        S(37, -84), S(21, -29), S(19, -28), S(-17, -22), S(30, -35), S(-23, -20), S(13, -24), S(41, -78),
	};
	
	int knight_mobility[9] = { S(-141, -110), S(-46, -67), S(-23, -34), S(-14, -13), S(-4, -3), S(-1, 8), S(8, 11), S(17, 15), S(25, 12), };
	
	int bishop_mobility[14] = { S(-35, -141), S(-48, -76), S(-18, -34), S(-13, -13), S(-2, -4), S(5, 3), S(9, 11), S(14, 14), S(17, 19), S(20, 18), S(21, 18), S(32, 9), S(34, 8), S(41, -4), };
	
	int rook_mobility[15] = { S(-73, -107), S(-49, -95), S(-17, -62), S(-10, -44), S(-4, -37), S(-2, -19), S(-1, -14), S(-5, -6), S(-1, -5), S(4, -0), S(8, 3), S(9, 9), S(12, 12), S(17, 14), S(21, 12), };
	
	int queen_mobility[28] = { S(0, 0), S(0, 0), S(-136, -91), S(-44, -156), S(-39, -96), S(-12, -71), S(-7, -55), S(-6, -38), S(-5, -22), S(-5, -4), S(-3, -0), S(0, 4), S(1, 9), S(4, 10), S(4, 13), S(4, 17), S(3, 17), S(1, 19), S(3, 18), S(5, 13), S(9, 9), S(23, -11), S(35, -22), S(52, -41), S(47, -42), S(85, -73), S(11, -61), S(-10, -77), };
	
	int king_mobility[9] = { S(29, 110), S(-60, 147), S(-51, 46), S(-29, 31), S(-16, 15), S(11, -4), S(-4, -0), S(14, -5), S(37, -20), };
	
	int isolated_pawn = S(-5, -8);
	
	int doubled_pawn = S(2, -16);
	
	int backward_pawn[8] = { S(0, 0), S(0, 0), S(0, 0), S(2, 6), S(-4, 1), S(4, -3), S(-2, -1), S(0, 0), };
	
	int backward_pawn_half_open[8] = { S(0, 0), S(0, 0), S(0, 0), S(11, -11), S(-18, -9), S(-14, -16), S(-16, -17), S(0, 0), };
	
	int chained_pawn[8] = { S(0, 0), S(143, 50), S(18, 49), S(19, 21), S(12, 11), S(17, 10), S(0, 0), S(0, 0), };
	
	int phalanx_pawn[8] = { S(0, 0), S(-25, 194), S(67, 92), S(33, 33), S(11, 11), S(7, 2), S(4, -1), S(0, 0), };
	
	int passed_pawn[8] = { S(0, 0), S(70, 100), S(12, 76), S(-17, 51), S(-35, 32), S(-17, 17), S(14, -1), S(0, 0), };
	
	int passed_pawn_blocked[8] = { S(0, 0), S(25, 32), S(10, 13), S(-26, 29), S(-45, 21), S(-27, 18), S(4, 1), S(0, 0), };
	
	int passed_pawn_safe_advance = S(-0, 11);
	
	int passed_pawn_safe_path = S(-61, 40);
	
	int passed_friendly_distance[8] = { S(0, 0), S(-8, -13), S(5, -19), S(13, -17), S(12, -12), S(5, -5), S(2, -3), S(0, 0), };
	
	int passed_enemy_distance[8] = { S(0, 0), S(13, 36), S(-4, 36), S(-4, 19), S(-2, 8), S(1, -1), S(-3, 0), S(0, 0), };
	
	int knight_outpost = S(5, -20);
	
	int knight_outpost_supported = S(27, 6);
	
	int bishop_pawn = S(-4, -7);
	
	int double_bishop = S(16, 59);
	
	int rook_open_file = S(26, 9);
	
	int rook_half_open_file = S(8, 10);
	
	int rook_on_seventh = S(2, 20);
	
	int pawn_shelter[2][4][8] = {
	{
	        { S(0, 0), S(26, 30), S(44, 12), S(24, -2), S(23, -4), S(36, -10), S(32, -24), S(0, 0), },
	        { S(0, 0), S(18, 10), S(14, 10), S(19, 1), S(8, 1), S(25, -5), S(36, -6), S(0, 0), },
	        { S(0, 0), S(6, 37), S(9, 11), S(15, -0), S(17, -4), S(10, 4), S(38, -2), S(0, 0), },
	        { S(0, 0), S(-13, 9), S(-17, 5), S(17, 0), S(9, 2), S(8, -0), S(15, 2), S(0, 0), },
	},
	{
	        { S(0, 0), S(25, 63), S(76, 17), S(32, 0), S(47, -6), S(62, -16), S(69, -45), S(0, 0), },
	        { S(0, 0), S(32, 29), S(40, -1), S(27, -2), S(23, -3), S(52, -6), S(58, -15), S(0, 0), },
	        { S(0, 0), S(13, 26), S(-5, 8), S(36, 1), S(38, -4), S(39, 2), S(67, -2), S(0, 0), },
	        { S(0, 0), S(-30, 31), S(-34, 4), S(14, 10), S(22, 7), S(23, 2), S(6, 9), S(0, 0), },
	},
	};
	
	int king_attacker_weight[6] = { S(0, 0), S(37, 164), S(31, 52), S(41, 33), S(7, 78), S(0, 0), };
	
	int king_zone_attack_count_weight = S(21, -120);
	
	int king_zone_weak_square = S(95, -35);
	
	int safe_knight_check = S(289, 125);
	
	int safe_bishop_check = S(190, 516);
	
	int safe_rook_check = S(294, 247);
	
	int safe_queen_check = S(175, 45);
	
	int unsafe_check = S(38, 95);
	
	int pawn_shelter_king_danger[2][4][8] = {
	{
	        { S(0, 0), S(38, -22), S(-17, -22), S(-27, 75), S(-14, 131), S(-85, 111), S(-123, 126), S(0, 0), },
	        { S(0, 0), S(73, 187), S(14, -77), S(11, 96), S(-44, 243), S(-58, 90), S(-101, -272), S(0, 0), },
	        { S(0, 0), S(8, -106), S(15, -18), S(-16, 280), S(-16, 265), S(-47, 80), S(-14, -340), S(0, 0), },
	        { S(0, 0), S(-118, -135), S(-39, -74), S(13, 295), S(-25, 360), S(-65, 252), S(9, -61), S(0, 0), },
	},
	{
	        { S(0, 0), S(120, -18), S(125, 84), S(68, 99), S(68, 117), S(3, 102), S(35, 119), S(0, 0), },
	        { S(0, 0), S(34, 138), S(68, 66), S(18, 104), S(7, 137), S(-60, 92), S(-61, -294), S(0, 0), },
	        { S(0, 0), S(1, -54), S(-111, -114), S(38, 122), S(53, 26), S(51, -35), S(18, -194), S(0, 0), },
	        { S(0, 0), S(-8, 19), S(-122, 9), S(-57, 290), S(-35, 268), S(-19, 321), S(-61, 157), S(0, 0), },
	},
	};
	
	int king_danger_no_queen_weight = S(-466, -502);
	
	int king_danger_offset = S(151, 70);
	
	int center_control = S(3, -1);
	
	int minor_threatened_by_pawn = S(-44, -23);
	
	int minor_threatened_by_minor = S(-17, -19);
	
	int rook_threatened_by_lesser = S(-43, 2);
	
	int queen_threatened_by_lesser = S(-37, 10);
	
	int minor_threatened_by_major = S(-10, -16);
	
	int pawn_push_threat = S(-17, -20);

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
