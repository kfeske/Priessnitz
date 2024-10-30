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

	int piece_value[6] = { S(48, 102), S(287, 381), S(310, 394), S(364, 680), S(742, 1276), S(0, 0), };

	int pawn_psqt[64] = {
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	        S(30, 27), S(29, 28), S(17, 50), S(80, 30), S(61, 40), S(46, 26), S(-14, 59), S(-20, 50),
	        S(9, 28), S(-1, 36), S(21, 15), S(34, 4), S(33, 10), S(33, 12), S(3, 39), S(-13, 32),
	        S(-10, 23), S(-8, 10), S(-0, -1), S(2, -13), S(16, -14), S(9, -6), S(-10, 3), S(-6, 5),
	        S(-10, 12), S(-6, 8), S(4, -3), S(13, -9), S(14, -10), S(10, -5), S(-1, -2), S(-10, -4),
	        S(-17, 10), S(-12, 5), S(-3, -0), S(3, 2), S(11, 2), S(1, -2), S(-1, -4), S(-19, -3),
	        S(-5, 14), S(-4, 13), S(2, 7), S(7, 9), S(14, 15), S(4, 7), S(8, 4), S(-7, 5),
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	};
	
	int knight_psqt[64] = {
	        S(-111, -44), S(-105, -13), S(-84, 2), S(-48, -8), S(-33, 1), S(-87, -19), S(-125, -5), S(-86, -53),
	        S(-31, 10), S(-22, 16), S(-16, 17), S(-5, 20), S(-12, 13), S(6, 7), S(-13, 14), S(-13, -1),
	        S(-16, 8), S(-5, 10), S(1, 20), S(1, 22), S(10, 19), S(30, 0), S(-9, 5), S(-3, 2),
	        S(-5, 18), S(3, 12), S(14, 18), S(23, 23), S(22, 20), S(29, 19), S(17, 15), S(14, 13),
	        S(1, 3), S(12, -2), S(11, 12), S(17, 13), S(12, 21), S(20, 8), S(10, 6), S(8, 4),
	        S(-14, -17), S(-7, -11), S(-4, -7), S(-3, 4), S(10, 4), S(0, -10), S(12, -11), S(-0, -8),
	        S(-20, -17), S(-15, -11), S(-13, -14), S(1, -13), S(-0, -14), S(1, -14), S(7, -16), S(6, -3),
	        S(-48, -9), S(-10, -19), S(-26, -17), S(-16, -14), S(-8, -11), S(-4, -20), S(-9, -8), S(-19, -6),
	};
	
	int bishop_psqt[64] = {
	        S(-23, -0), S(-65, 11), S(-63, 6), S(-88, 14), S(-86, 10), S(-79, 2), S(-53, -0), S(-63, -4),
	        S(-28, -6), S(-30, 3), S(-20, 2), S(-37, 5), S(-18, -3), S(-19, -3), S(-41, 9), S(-36, -7),
	        S(-11, 7), S(3, 6), S(-6, 7), S(6, 0), S(-6, 6), S(24, 12), S(-7, 9), S(4, 11),
	        S(-12, 5), S(2, 9), S(4, 11), S(6, 27), S(5, 20), S(3, 14), S(6, 10), S(-17, 7),
	        S(-2, -4), S(-8, 10), S(-0, 16), S(5, 22), S(2, 18), S(6, 12), S(-4, 8), S(14, -12),
	        S(-2, -3), S(8, 3), S(4, 8), S(1, 8), S(5, 14), S(7, 4), S(12, -2), S(12, -9),
	        S(16, -3), S(8, -12), S(11, -12), S(-1, -2), S(4, -2), S(14, -8), S(25, -13), S(21, -17),
	        S(9, -11), S(15, -12), S(6, -2), S(-4, -9), S(9, -8), S(-1, 2), S(13, -12), S(26, -27),
	};
	
	int rook_psqt[64] = {
	        S(3, 20), S(-4, 25), S(-16, 34), S(-22, 30), S(-3, 24), S(4, 26), S(9, 27), S(29, 19),
	        S(-11, 7), S(-14, 16), S(-5, 21), S(10, 10), S(-8, 13), S(10, 12), S(8, 11), S(18, 3),
	        S(-18, 19), S(9, 17), S(-5, 20), S(-2, 17), S(15, 10), S(19, 8), S(49, 5), S(8, 5),
	        S(-12, 18), S(7, 14), S(6, 18), S(3, 13), S(14, 4), S(24, 1), S(22, 8), S(5, 2),
	        S(-15, 6), S(-10, 9), S(-5, 6), S(-4, 5), S(5, 2), S(2, 4), S(17, 1), S(-6, -2),
	        S(-15, -4), S(-11, -5), S(-8, -7), S(-6, -7), S(4, -10), S(6, -14), S(27, -24), S(3, -21),
	        S(-15, -15), S(-10, -11), S(-1, -12), S(1, -14), S(7, -20), S(10, -21), S(23, -31), S(-8, -23),
	        S(-1, -8), S(-4, -12), S(-3, -7), S(5, -12), S(11, -18), S(8, -12), S(7, -16), S(6, -22),
	};
	
	int queen_psqt[64] = {
	        S(-31, 12), S(-19, 1), S(-29, 24), S(5, 17), S(-18, 24), S(-20, 21), S(54, -34), S(2, 1),
	        S(-9, -8), S(-25, 0), S(-29, 28), S(-48, 45), S(-46, 52), S(-27, 30), S(-10, 1), S(24, 16),
	        S(-2, -10), S(-12, -3), S(-18, 23), S(-15, 24), S(-12, 27), S(-7, 15), S(5, -6), S(6, 1),
	        S(-10, 15), S(-0, 10), S(-8, 19), S(-23, 33), S(-16, 35), S(3, 13), S(11, 17), S(9, 7),
	        S(5, -7), S(-8, 18), S(-9, 20), S(-14, 33), S(-11, 31), S(0, 13), S(14, 5), S(14, -1),
	        S(-0, -22), S(4, -6), S(-7, 12), S(-5, 6), S(-4, 11), S(5, 4), S(22, -21), S(10, -23),
	        S(5, -27), S(3, -23), S(10, -25), S(10, -18), S(10, -14), S(15, -32), S(22, -57), S(25, -70),
	        S(-2, -21), S(-8, -20), S(-3, -21), S(9, -19), S(5, -24), S(-2, -36), S(2, -37), S(17, -45),
	};
	
	int king_psqt[64] = {
	        S(55, -111), S(69, -48), S(24, -21), S(-57, 11), S(31, -9), S(-4, 8), S(27, 11), S(138, -109),
	        S(-96, -8), S(-8, 26), S(-6, 31), S(79, 27), S(37, 39), S(29, 54), S(12, 55), S(41, 15),
	        S(-99, 0), S(45, 26), S(-18, 42), S(-41, 52), S(7, 57), S(64, 50), S(33, 49), S(20, 6),
	        S(6, -24), S(-8, 19), S(-42, 38), S(-98, 52), S(-81, 53), S(-39, 47), S(-44, 38), S(-95, 4),
	        S(-6, -37), S(-12, 5), S(-21, 23), S(-68, 38), S(-56, 37), S(-12, 24), S(-37, 18), S(-86, -10),
	        S(6, -41), S(29, -6), S(-1, 7), S(-20, 17), S(-8, 16), S(-7, 11), S(6, 3), S(-20, -22),
	        S(20, -34), S(-13, 2), S(-14, 3), S(-22, 6), S(-27, 10), S(-27, 9), S(-17, 10), S(3, -20),
	        S(26, -85), S(28, -31), S(22, -28), S(-13, -22), S(32, -35), S(-20, -21), S(17, -25), S(26, -78),
	};
	
	int knight_mobility[9] = { S(-150, -111), S(-47, -68), S(-24, -35), S(-15, -14), S(-5, -4), S(-2, 7), S(7, 11), S(16, 14), S(25, 12), };
	
	int bishop_mobility[14] = { S(-36, -141), S(-48, -76), S(-18, -35), S(-13, -13), S(-2, -3), S(5, 4), S(10, 12), S(15, 15), S(17, 19), S(21, 19), S(22, 19), S(33, 9), S(35, 8), S(43, -4), };
	
	int rook_mobility[15] = { S(-85, -117), S(-53, -96), S(-18, -64), S(-11, -46), S(-4, -39), S(-3, -21), S(-1, -15), S(-5, -7), S(-1, -6), S(4, -1), S(8, 2), S(8, 8), S(11, 11), S(17, 13), S(21, 11), };
	
	int queen_mobility[28] = { S(0, 0), S(0, 0), S(-151, -102), S(-42, -159), S(-40, -99), S(-12, -71), S(-7, -56), S(-6, -37), S(-5, -22), S(-5, -4), S(-3, -0), S(0, 4), S(2, 9), S(4, 10), S(4, 13), S(4, 18), S(4, 17), S(2, 20), S(3, 18), S(5, 13), S(10, 9), S(24, -12), S(37, -23), S(54, -42), S(53, -47), S(86, -75), S(12, -62), S(-3, -83), };
	
	int king_mobility[9] = { S(32, 122), S(-47, 149), S(-32, 46), S(-17, 30), S(-17, 15), S(6, -3), S(-5, -1), S(11, -5), S(32, -20), };
	
	int isolated_pawn = S(-5, -8);
	
	int doubled_pawn = S(1, -16);
	
	int backward_pawn[8] = { S(0, 0), S(0, 0), S(0, 0), S(2, 6), S(-3, 1), S(4, -3), S(-3, -1), S(0, 0), };
	
	int backward_pawn_half_open[8] = { S(0, 0), S(0, 0), S(0, 0), S(9, -11), S(-18, -9), S(-15, -16), S(-16, -17), S(0, 0), };
	
	int chained_pawn[8] = { S(0, 0), S(146, 50), S(20, 50), S(19, 22), S(13, 11), S(17, 10), S(0, 0), S(0, 0), };
	
	int phalanx_pawn[8] = { S(0, 0), S(-35, 201), S(70, 93), S(32, 33), S(11, 11), S(7, 2), S(4, -2), S(0, 0), };
	
	int passed_pawn[8] = { S(0, 0), S(76, 101), S(9, 77), S(-18, 51), S(-37, 33), S(-15, 17), S(17, -1), S(0, 0), };
	
	int passed_pawn_blocked[8] = { S(0, 0), S(33, 32), S(7, 13), S(-28, 29), S(-47, 22), S(-24, 19), S(7, 1), S(0, 0), };
	
	int passed_pawn_safe_advance = S(-0, 11);
	
	int passed_pawn_safe_path = S(-62, 40);
	
	int passed_friendly_distance[8] = { S(0, 0), S(-7, -13), S(4, -19), S(11, -17), S(11, -12), S(3, -5), S(1, -2), S(0, 0), };
	
	int passed_enemy_distance[8] = { S(0, 0), S(9, 37), S(-3, 36), S(-2, 19), S(-0, 8), S(2, -2), S(-3, 0), S(0, 0), };
	
	int knight_outpost = S(6, -20);
	
	int knight_outpost_supported = S(27, 6);
	
	int bishop_pawn = S(-4, -7);
	
	int double_bishop = S(16, 60);
	
	int rook_open_file = S(27, 9);
	
	int rook_half_open_file = S(8, 10);
	
	int rook_on_seventh = S(2, 20);
	
	int pawn_shelter[2][4][8] = {
	{
	        { S(0, 0), S(25, 30), S(37, 14), S(21, -2), S(20, -3), S(34, -10), S(30, -23), S(0, 0), },
	        { S(0, 0), S(17, 11), S(12, 11), S(19, 2), S(7, 1), S(26, -5), S(37, -6), S(0, 0), },
	        { S(0, 0), S(1, 39), S(3, 13), S(16, -0), S(18, -4), S(11, 4), S(40, -2), S(0, 0), },
	        { S(0, 0), S(-15, 9), S(-19, 5), S(17, 1), S(9, 2), S(10, -1), S(17, 2), S(0, 0), },
	},
	{
	        { S(0, 0), S(34, 61), S(69, 19), S(33, 0), S(46, -6), S(63, -16), S(70, -44), S(0, 0), },
	        { S(0, 0), S(35, 28), S(38, -1), S(27, -1), S(23, -3), S(54, -7), S(60, -16), S(0, 0), },
	        { S(0, 0), S(13, 26), S(-2, 8), S(37, 1), S(38, -3), S(39, 2), S(68, -2), S(0, 0), },
	        { S(0, 0), S(-29, 30), S(-33, 4), S(14, 11), S(24, 7), S(24, 2), S(10, 7), S(0, 0), },
	},
	};
	
	int king_attacker_weight[6] = { S(0, 0), S(41, 147), S(34, 49), S(32, 64), S(5, 88), S(0, 0), };
	
	int king_zone_attack_count_weight = S(21, -123);
	
	int king_zone_weak_square = S(87, -14);
	
	int safe_knight_check = S(288, 123);
	
	int safe_bishop_check = S(193, 504);
	
	int safe_rook_check = S(287, 280);
	
	int safe_queen_check = S(172, 48);
	
	int unsafe_check = S(40, 90);
	
	int pawn_shelter_king_danger[2][4][8] = {
	{
	        { S(0, 0), S(1, 7), S(48, -4), S(-10, 93), S(8, 118), S(-71, 114), S(-129, 191), S(0, 0), },
	        { S(0, 0), S(63, 223), S(16, -70), S(29, 105), S(-24, 230), S(-43, 66), S(-93, -268), S(0, 0), },
	        { S(0, 0), S(-6, -72), S(-15, -6), S(-2, 223), S(18, 238), S(-30, 82), S(-1, -301), S(0, 0), },
	        { S(0, 0), S(-144, -117), S(-66, -60), S(41, 229), S(-7, 306), S(-53, 174), S(2, -59), S(0, 0), },
	},
	{
	        { S(0, 0), S(95, -8), S(150, 114), S(84, 133), S(92, 108), S(6, 151), S(20, 208), S(0, 0), },
	        { S(0, 0), S(22, 121), S(64, 70), S(31, 140), S(30, 104), S(-26, 23), S(-53, -225), S(0, 0), },
	        { S(0, 0), S(-4, -50), S(-129, -118), S(29, 116), S(80, 12), S(68, -50), S(37, -273), S(0, 0), },
	        { S(0, 0), S(-27, 19), S(-131, -8), S(-18, 206), S(3, 153), S(-24, 307), S(-54, 106), S(0, 0), },
	},
	};
	
	int pawn_storm_king_danger[2][4][8] = {
	{
	        { S(0, 0), S(-37, -196), S(-23, -2), S(43, -90), S(129, 16), S(-141, -306), S(-217, -142), S(0, 0), },
	        { S(0, 0), S(-32, -3), S(-84, 144), S(16, -142), S(132, -221), S(180, 54), S(-50, -69), S(0, 0), },
	        { S(0, 0), S(-64, 205), S(-62, 79), S(15, -74), S(122, -123), S(219, 81), S(117, -66), S(0, 0), },
	        { S(0, 0), S(-162, 208), S(-105, -25), S(-58, 88), S(67, 9), S(84, 156), S(136, 36), S(0, 0), },
	},
	{
	        { S(0, 0), S(-107, -118), S(-41, -21), S(-91, 73), S(-79, 78), S(-51, 629), S(0, 0), S(0, 0), },
	        { S(0, 0), S(-46, -33), S(-77, 171), S(-57, 60), S(-39, 108), S(110, 34), S(0, 0), S(0, 0), },
	        { S(0, 0), S(55, 207), S(-16, 284), S(-50, 25), S(37, -223), S(169, -49), S(0, 0), S(0, 0), },
	        { S(0, 0), S(-29, 72), S(-99, 243), S(-73, 226), S(14, 249), S(59, 42), S(0, 0), S(0, 0), },
	},
	};
	
	int king_danger_no_queen_weight = S(-476, -553);
	
	int king_danger_offset = S(225, -18);
	
	int center_control = S(3, -1);
	
	int minor_threatened_by_pawn = S(-45, -23);
	
	int minor_threatened_by_minor = S(-18, -19);
	
	int rook_threatened_by_lesser = S(-43, 2);
	
	int queen_threatened_by_lesser = S(-38, 11);
	
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
