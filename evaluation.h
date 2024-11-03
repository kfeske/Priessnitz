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
	uint64_t passed_pawns {};
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
	Pawn_hash_table pawn_hash_table {};

	#define S(mg, eg) merge_score(mg, eg)

	int piece_value[6] = { S(51, 106), S(292, 389), S(316, 403), S(372, 692), S(759, 1303), S(0, 0), };

	int pawn_psqt[64] = {
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	        S(65, 48), S(49, 50), S(14, 65), S(59, 42), S(20, 55), S(21, 54), S(4, 87), S(57, 59),
	        S(11, 27), S(-2, 33), S(19, 7), S(39, -15), S(35, -9), S(12, 15), S(1, 34), S(6, 25),
	        S(-13, 25), S(-11, 11), S(-3, -1), S(0, -19), S(14, -19), S(0, -0), S(-7, 7), S(3, 9),
	        S(-12, 13), S(-9, 8), S(1, -3), S(12, -15), S(13, -16), S(5, -2), S(1, 0), S(-1, -1),
	        S(-19, 10), S(-14, 5), S(-5, -1), S(1, -1), S(9, -2), S(-3, -2), S(4, -5), S(-14, -2),
	        S(-7, 15), S(-6, 12), S(-0, 5), S(5, 6), S(12, 12), S(3, 5), S(14, 3), S(-2, 6),
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	};
	
	int knight_psqt[64] = {
	        S(-111, -45), S(-106, -15), S(-85, 2), S(-48, -9), S(-33, 1), S(-87, -21), S(-126, -6), S(-85, -55),
	        S(-33, 11), S(-22, 16), S(-16, 17), S(-4, 20), S(-12, 13), S(7, 6), S(-11, 13), S(-13, -1),
	        S(-16, 9), S(-5, 10), S(1, 21), S(2, 23), S(11, 19), S(31, 0), S(-8, 5), S(-2, 2),
	        S(-5, 19), S(3, 13), S(14, 18), S(24, 23), S(23, 21), S(30, 20), S(17, 16), S(15, 13),
	        S(1, 4), S(12, -2), S(12, 13), S(17, 13), S(13, 21), S(21, 9), S(11, 6), S(9, 5),
	        S(-15, -16), S(-7, -10), S(-4, -6), S(-3, 5), S(10, 5), S(0, -10), S(12, -10), S(-0, -7),
	        S(-20, -15), S(-16, -10), S(-13, -14), S(1, -13), S(-0, -13), S(0, -14), S(8, -16), S(7, -2),
	        S(-50, -7), S(-11, -19), S(-27, -17), S(-16, -14), S(-8, -11), S(-4, -20), S(-9, -8), S(-20, -2),
	};
	
	int bishop_psqt[64] = {
	        S(-23, -1), S(-66, 11), S(-64, 5), S(-90, 15), S(-87, 10), S(-80, 2), S(-55, -0), S(-63, -4),
	        S(-28, -7), S(-31, 3), S(-21, 1), S(-37, 5), S(-18, -3), S(-18, -3), S(-41, 9), S(-38, -7),
	        S(-11, 8), S(3, 6), S(-7, 7), S(7, 0), S(-5, 5), S(24, 12), S(-7, 9), S(4, 11),
	        S(-13, 4), S(2, 9), S(4, 11), S(6, 27), S(5, 20), S(4, 14), S(6, 11), S(-16, 7),
	        S(-2, -4), S(-8, 11), S(-0, 16), S(6, 21), S(2, 18), S(6, 13), S(-4, 8), S(14, -11),
	        S(-2, -3), S(8, 3), S(4, 8), S(1, 8), S(4, 14), S(6, 4), S(12, -2), S(12, -9),
	        S(16, -2), S(8, -12), S(10, -11), S(-2, -2), S(4, -2), S(13, -7), S(25, -13), S(21, -17),
	        S(9, -10), S(14, -11), S(5, -2), S(-5, -9), S(9, -8), S(-2, 2), S(13, -12), S(26, -26),
	};
	
	int rook_psqt[64] = {
	        S(4, 21), S(-3, 24), S(-15, 34), S(-20, 29), S(-1, 23), S(6, 25), S(13, 27), S(32, 19),
	        S(-10, 6), S(-12, 15), S(-3, 20), S(13, 9), S(-5, 12), S(13, 11), S(14, 8), S(21, 1),
	        S(-19, 19), S(10, 17), S(-5, 21), S(-1, 17), S(17, 9), S(21, 7), S(50, 4), S(6, 4),
	        S(-13, 18), S(6, 14), S(6, 18), S(3, 13), S(14, 4), S(26, 1), S(21, 9), S(3, 3),
	        S(-16, 6), S(-11, 10), S(-6, 7), S(-4, 6), S(5, 3), S(1, 5), S(18, 1), S(-6, -2),
	        S(-17, -4), S(-12, -5), S(-9, -7), S(-6, -6), S(3, -10), S(5, -13), S(28, -23), S(3, -20),
	        S(-16, -13), S(-11, -10), S(-1, -11), S(-0, -13), S(6, -19), S(9, -20), S(23, -31), S(-8, -22),
	        S(-2, -7), S(-5, -10), S(-4, -6), S(4, -11), S(11, -17), S(7, -11), S(6, -15), S(6, -21),
	};
	
	int queen_psqt[64] = {
	        S(-32, 13), S(-19, 2), S(-29, 25), S(6, 17), S(-17, 23), S(-19, 20), S(54, -32), S(1, 4),
	        S(-9, -8), S(-25, 0), S(-29, 28), S(-49, 45), S(-47, 53), S(-28, 31), S(-10, 2), S(22, 17),
	        S(-1, -10), S(-13, -3), S(-18, 23), S(-15, 24), S(-12, 28), S(-8, 15), S(4, -5), S(4, 3),
	        S(-10, 15), S(0, 10), S(-8, 20), S(-23, 34), S(-16, 35), S(4, 13), S(11, 18), S(10, 8),
	        S(5, -6), S(-7, 19), S(-9, 21), S(-14, 33), S(-11, 32), S(0, 14), S(15, 6), S(14, 1),
	        S(-0, -21), S(4, -5), S(-7, 13), S(-5, 7), S(-4, 11), S(4, 5), S(22, -20), S(10, -21),
	        S(5, -26), S(3, -22), S(10, -25), S(10, -17), S(10, -13), S(14, -32), S(22, -56), S(25, -69),
	        S(-2, -20), S(-9, -19), S(-3, -21), S(9, -19), S(5, -24), S(-3, -35), S(2, -37), S(18, -45),
	};
	
	int king_psqt[64] = {
	        S(51, -83), S(63, -42), S(22, -15), S(-54, 16), S(32, -4), S(1, 12), S(24, 12), S(130, -84),
	        S(-96, -2), S(0, 27), S(-1, 34), S(79, 31), S(43, 42), S(33, 56), S(17, 54), S(36, 19),
	        S(-100, 8), S(48, 29), S(-15, 48), S(-36, 57), S(12, 62), S(64, 56), S(32, 52), S(15, 14),
	        S(5, -17), S(-8, 20), S(-39, 43), S(-95, 58), S(-78, 59), S(-39, 53), S(-50, 38), S(-99, 12),
	        S(-16, -33), S(-14, 2), S(-18, 24), S(-60, 43), S(-45, 41), S(-11, 25), S(-42, 12), S(-96, -7),
	        S(-4, -42), S(24, -14), S(3, 5), S(-7, 19), S(7, 17), S(-3, 7), S(1, -7), S(-30, -25),
	        S(11, -39), S(-18, -9), S(-6, -2), S(-4, 6), S(-9, 9), S(-21, 4), S(-23, -1), S(-7, -27),
	        S(15, -67), S(19, -38), S(26, -30), S(2, -20), S(48, -33), S(-18, -22), S(7, -31), S(14, -62),
	};
	
	int knight_mobility[9] = { S(-156, -109), S(-48, -69), S(-24, -35), S(-15, -13), S(-4, -3), S(-2, 9), S(8, 12), S(17, 16), S(26, 14), };
	
	int bishop_mobility[14] = { S(-36, -148), S(-51, -78), S(-19, -35), S(-14, -13), S(-2, -3), S(5, 4), S(10, 13), S(15, 16), S(17, 20), S(21, 20), S(22, 20), S(33, 11), S(35, 10), S(43, -3), };
	
	int rook_mobility[15] = { S(-81, -107), S(-51, -94), S(-18, -62), S(-11, -45), S(-4, -37), S(-3, -19), S(-1, -13), S(-5, -5), S(-0, -5), S(4, 0), S(8, 4), S(9, 9), S(12, 13), S(18, 14), S(22, 12), };
	
	int queen_mobility[28] = { S(0, 0), S(0, 0), S(-130, -87), S(-45, -154), S(-41, -99), S(-12, -74), S(-7, -56), S(-6, -38), S(-5, -22), S(-5, -3), S(-3, 0), S(0, 5), S(2, 10), S(4, 12), S(4, 14), S(4, 19), S(4, 19), S(2, 21), S(3, 20), S(5, 15), S(10, 11), S(24, -10), S(37, -22), S(52, -40), S(50, -44), S(80, -71), S(7, -61), S(-11, -80), };
	
	int king_mobility[9] = { S(16, 71), S(-54, 70), S(-31, 4), S(-14, 9), S(-18, 12), S(8, -0), S(-6, 6), S(8, 3), S(29, -11), };
	
	int isolated_pawn = S(-5, -8);
	
	int doubled_pawn = S(-2, -15);
	
	int backward_pawn[8] = { S(0, 0), S(0, 0), S(0, 0), S(3, 5), S(-2, 1), S(3, -2), S(-4, 0), S(0, 0), };
	
	int backward_pawn_half_open[8] = { S(0, 0), S(0, 0), S(0, 0), S(8, -10), S(-18, -11), S(-14, -17), S(-17, -17), S(0, 0), };
	
	int chained_pawn[8] = { S(0, 0), S(124, 74), S(19, 56), S(20, 22), S(13, 11), S(17, 10), S(0, 0), S(0, 0), };
	
	int phalanx_pawn[8] = { S(0, 0), S(-38, 180), S(67, 97), S(32, 35), S(11, 11), S(8, 2), S(4, -2), S(0, 0), };
	
	int passed_pawn[8] = { S(0, 0), S(92, 138), S(2, 132), S(-18, 62), S(-35, 36), S(-14, 21), S(15, 9), S(0, 0), };
	
	int passed_pawn_blocked[8] = { S(0, 0), S(56, 72), S(-4, 77), S(-28, 41), S(-46, 26), S(-24, 23), S(5, 11), S(0, 0), };
	
	int passed_pawn_safe_advance = S(1, 7);
	
	int passed_pawn_safe_path = S(-59, 37);
	
	int passed_friendly_distance[8] = { S(0, 0), S(-1, -15), S(1, -19), S(9, -16), S(9, -12), S(2, -6), S(-0, -4), S(0, 0), };
	
	int passed_enemy_distance[8] = { S(0, 0), S(-6, 27), S(3, 25), S(-0, 17), S(0, 7), S(2, -1), S(-2, -0), S(0, 0), };
	
	int knight_outpost = S(6, -21);
	
	int knight_outpost_supported = S(27, 6);
	
	int bishop_pawn = S(-4, -7);
	
	int double_bishop = S(17, 60);
	
	int rook_open_file = S(27, 9);
	
	int rook_half_open_file = S(8, 11);
	
	int rook_on_seventh = S(-0, 21);
	
	int pawn_shelter[2][4][8] = {
	{
	        { S(0, 0), S(46, 30), S(21, 23), S(18, 8), S(14, 1), S(33, -12), S(29, -24), S(0, 0), },
	        { S(0, 0), S(28, 11), S(-4, 18), S(16, 13), S(6, 7), S(30, -5), S(39, -9), S(0, 0), },
	        { S(0, 0), S(-8, 46), S(-6, 21), S(10, 10), S(12, 4), S(8, 5), S(40, -3), S(0, 0), },
	        { S(0, 0), S(-17, 14), S(-20, 9), S(11, 4), S(5, 9), S(10, 4), S(18, 3), S(0, 0), },
	},
	{
	        { S(0, 0), S(47, 61), S(55, 27), S(32, 10), S(40, -0), S(62, -16), S(70, -43), S(0, 0), },
	        { S(0, 0), S(50, 29), S(24, 9), S(25, 12), S(23, 6), S(59, -6), S(63, -17), S(0, 0), },
	        { S(0, 0), S(16, 30), S(-4, 17), S(32, 12), S(30, 6), S(35, 5), S(66, -0), S(0, 0), },
	        { S(0, 0), S(-32, 36), S(-33, 7), S(9, 15), S(20, 15), S(27, 8), S(13, 10), S(0, 0), },
	},
	};
	
	int pawn_storm[2][4][8] = {
	{
	        { S(0, 0), S(13, 8), S(15, 7), S(14, 18), S(-3, 42), S(18, 84), S(212, 67), S(0, 0), },
	        { S(0, 0), S(20, 0), S(22, 3), S(11, 15), S(4, 30), S(-34, 92), S(101, 118), S(0, 0), },
	        { S(0, 0), S(2, -0), S(-3, 4), S(-9, 15), S(-19, 29), S(-95, 93), S(-32, 110), S(0, 0), },
	        { S(0, 0), S(-1, 2), S(8, -2), S(-0, -2), S(-14, 8), S(9, 50), S(-37, 95), S(0, 0), },
	},
	{
	        { S(0, 0), S(46, -16), S(29, -10), S(38, -2), S(43, 6), S(58, -28), S(0, 0), S(0, 0), },
	        { S(0, 0), S(61, -10), S(33, -22), S(24, -11), S(27, -4), S(26, -3), S(0, 0), S(0, 0), },
	        { S(0, 0), S(5, -9), S(3, -15), S(2, -14), S(-1, -0), S(-25, 14), S(0, 0), S(0, 0), },
	        { S(0, 0), S(16, -2), S(12, -8), S(13, -18), S(3, -17), S(20, -14), S(0, 0), S(0, 0), },
	},
	};
	
	int king_attacker_weight[6] = { S(0, 0), S(42, 139), S(34, 55), S(33, 52), S(5, 93), S(0, 0), };
	
	int king_zone_attack_count_weight = S(21, -119);
	
	int king_zone_weak_square = S(90, -20);
	
	int safe_knight_check = S(295, 133);
	
	int safe_bishop_check = S(201, 486);
	
	int safe_rook_check = S(295, 278);
	
	int safe_queen_check = S(177, 59);
	
	int unsafe_check = S(41, 88);
	
	int pawn_shelter_king_danger[2][4][8] = {
	{
	        { S(0, 0), S(-2, 7), S(12, -33), S(-5, 68), S(-15, 65), S(-81, 32), S(-142, 150), S(0, 0), },
	        { S(0, 0), S(56, 185), S(-11, -67), S(29, 91), S(-25, 210), S(-42, 28), S(-101, -239), S(0, 0), },
	        { S(0, 0), S(-11, -87), S(-9, -2), S(-2, 223), S(14, 234), S(-37, 81), S(-7, -241), S(0, 0), },
	        { S(0, 0), S(-148, -117), S(-44, -31), S(45, 208), S(-3, 301), S(-43, 206), S(16, -40), S(0, 0), },
	},
	{
	        { S(0, 0), S(100, 10), S(123, 98), S(86, 133), S(72, 82), S(-4, 83), S(17, 131), S(0, 0), },
	        { S(0, 0), S(23, 108), S(41, 63), S(31, 123), S(24, 100), S(-32, -16), S(-65, -188), S(0, 0), },
	        { S(0, 0), S(-11, -43), S(-109, -97), S(38, 95), S(72, 30), S(58, -31), S(29, -211), S(0, 0), },
	        { S(0, 0), S(-19, 13), S(-101, 32), S(-11, 206), S(16, 181), S(4, 336), S(-27, 118), S(0, 0), },
	},
	};
	
	int pawn_storm_king_danger[2][4][8] = {
	{
	        { S(0, 0), S(-15, -148), S(3, 34), S(74, -13), S(102, 188), S(-62, -45), S(32, 29), S(0, 0), },
	        { S(0, 0), S(13, -2), S(-40, 141), S(49, -111), S(156, -159), S(178, 181), S(102, -3), S(0, 0), },
	        { S(0, 0), S(-65, 198), S(-67, 76), S(10, -37), S(104, 5), S(131, 198), S(90, -19), S(0, 0), },
	        { S(0, 0), S(-160, 155), S(-75, 45), S(-49, 71), S(48, 45), S(184, 290), S(162, 76), S(0, 0), },
	},
	{
	        { S(0, 0), S(-41, -82), S(-34, -13), S(-6, 100), S(39, 96), S(54, 424), S(0, 0), S(0, 0), },
	        { S(0, 0), S(57, 41), S(-39, 147), S(-19, 29), S(17, 94), S(130, 8), S(0, 0), S(0, 0), },
	        { S(0, 0), S(39, 172), S(-29, 214), S(-55, -28), S(45, -212), S(147, -11), S(0, 0), S(0, 0), },
	        { S(0, 0), S(-1, 85), S(-67, 231), S(-43, 153), S(21, 89), S(80, 54), S(0, 0), S(0, 0), },
	},
	};
	
	int king_danger_no_queen_weight = S(-454, -528);
	
	int king_danger_offset = S(174, -9);
	
	int center_control = S(3, -1);
	
	int minor_threatened_by_pawn = S(-46, -24);
	
	int minor_threatened_by_minor = S(-18, -19);
	
	int rook_threatened_by_lesser = S(-44, 2);
	
	int queen_threatened_by_lesser = S(-38, 11);
	
	int minor_threatened_by_major = S(-10, -17);
	
	int pawn_push_threat = S(-18, -21);

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
