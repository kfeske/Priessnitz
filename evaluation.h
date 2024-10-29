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
		uint64_t white_pawn_attacks = board.all_pawn_attacks(WHITE, board.pieces(WHITE, PAWN));
		attacked_by_multiple[WHITE] = attacked[WHITE] & white_pawn_attacks;
		attacked[WHITE]	 	      |= white_pawn_attacks;
		attacked_by_piece[WHITE][PAWN] = white_pawn_attacks;
		uint64_t black_pawn_attacks = board.all_pawn_attacks(BLACK, board.pieces(BLACK, PAWN));
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

	int piece_value[6] = { S(46, 102), S(277, 373), S(303, 389), S(351, 663), S(722, 1255), S(0, 0), };

	int pawn_psqt[64] = {
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	        S(18, 25), S(20, 26), S(12, 48), S(76, 28), S(61, 37), S(49, 23), S(-24, 57), S(-41, 49),
	        S(9, 26), S(3, 33), S(23, 13), S(34, 2), S(32, 7), S(49, 7), S(19, 34), S(-18, 31),
	        S(-10, 20), S(-7, 8), S(-0, -3), S(1, -15), S(15, -15), S(15, -10), S(-6, 0), S(-4, 2),
	        S(-9, 9), S(-6, 6), S(3, -5), S(11, -11), S(13, -12), S(9, -7), S(0, -4), S(-8, -6),
	        S(-16, 7), S(-13, 3), S(-4, -2), S(1, 0), S(9, -0), S(-2, -4), S(-3, -6), S(-20, -6),
	        S(-5, 11), S(-3, 10), S(1, 4), S(5, 7), S(12, 12), S(2, 5), S(9, 1), S(-9, 2),
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	};
	
	int knight_psqt[64] = {
	        S(-104, -44), S(-96, -15), S(-80, 2), S(-43, -8), S(-28, 1), S(-80, -19), S(-103, -9), S(-77, -53),
	        S(-30, 10), S(-20, 15), S(-15, 16), S(-6, 19), S(-9, 11), S(5, 6), S(-11, 13), S(-8, -2),
	        S(-14, 7), S(-6, 9), S(-1, 20), S(-1, 22), S(11, 18), S(31, -0), S(-9, 5), S(1, 1),
	        S(-5, 18), S(4, 12), S(15, 17), S(19, 22), S(20, 20), S(24, 20), S(16, 16), S(15, 12),
	        S(2, 3), S(12, -2), S(13, 11), S(18, 12), S(12, 20), S(21, 8), S(11, 6), S(9, 4),
	        S(-13, -17), S(-7, -11), S(-3, -7), S(-3, 4), S(10, 5), S(0, -10), S(12, -11), S(-1, -8),
	        S(-19, -16), S(-15, -11), S(-12, -14), S(1, -13), S(-0, -14), S(-0, -14), S(7, -16), S(5, -2),
	        S(-49, -9), S(-10, -19), S(-26, -17), S(-15, -14), S(-8, -11), S(-5, -20), S(-9, -8), S(-19, -6),
	};
	
	int bishop_psqt[64] = {
	        S(-22, -1), S(-61, 9), S(-58, 4), S(-85, 12), S(-83, 8), S(-75, 0), S(-48, -3), S(-59, -7),
	        S(-26, -8), S(-29, 2), S(-20, 0), S(-35, 4), S(-17, -5), S(-18, -4), S(-40, 8), S(-33, -10),
	        S(-11, 6), S(2, 5), S(-6, 6), S(6, -1), S(-3, 3), S(24, 10), S(-3, 6), S(4, 9),
	        S(-12, 3), S(2, 8), S(4, 9), S(5, 25), S(4, 18), S(3, 12), S(7, 8), S(-16, 5),
	        S(-2, -5), S(-8, 9), S(-1, 14), S(4, 20), S(-1, 17), S(5, 10), S(-5, 6), S(13, -14),
	        S(-3, -4), S(6, 2), S(3, 6), S(-1, 7), S(3, 12), S(5, 3), S(10, -3), S(10, -9),
	        S(14, -4), S(6, -13), S(8, -13), S(-3, -4), S(3, -3), S(11, -9), S(22, -14), S(18, -18),
	        S(8, -12), S(12, -13), S(3, -3), S(-7, -9), S(6, -9), S(-4, 1), S(11, -13), S(24, -28),
	};
	
	int rook_psqt[64] = {
	        S(3, 20), S(-6, 24), S(-18, 34), S(-25, 30), S(-6, 24), S(1, 25), S(7, 27), S(25, 19),
	        S(-12, 6), S(-15, 15), S(-6, 20), S(7, 10), S(-11, 13), S(6, 12), S(9, 10), S(19, 2),
	        S(-18, 19), S(8, 17), S(-5, 20), S(-3, 17), S(16, 9), S(18, 8), S(52, 4), S(11, 4),
	        S(-12, 17), S(7, 13), S(6, 18), S(2, 13), S(13, 4), S(22, 1), S(23, 8), S(7, 2),
	        S(-15, 6), S(-10, 9), S(-5, 6), S(-4, 5), S(5, 2), S(0, 4), S(16, 1), S(-8, -2),
	        S(-16, -4), S(-12, -5), S(-9, -7), S(-8, -6), S(2, -10), S(4, -14), S(25, -24), S(2, -21),
	        S(-16, -14), S(-11, -11), S(-2, -12), S(-1, -13), S(5, -19), S(7, -21), S(21, -31), S(-9, -23),
	        S(-2, -8), S(-5, -11), S(-4, -7), S(3, -12), S(10, -17), S(6, -12), S(7, -16), S(6, -22),
	};
	
	int queen_psqt[64] = {
	        S(-29, 12), S(-18, 1), S(-28, 22), S(4, 16), S(-20, 23), S(-16, 20), S(53, -28), S(3, 6),
	        S(-9, -7), S(-25, 0), S(-29, 27), S(-51, 45), S(-52, 52), S(-26, 31), S(-9, 4), S(24, 18),
	        S(-1, -9), S(-12, -2), S(-17, 22), S(-14, 23), S(-11, 27), S(-13, 15), S(3, -3), S(-1, 9),
	        S(-9, 13), S(1, 9), S(-7, 18), S(-22, 33), S(-16, 32), S(1, 11), S(10, 22), S(7, 10),
	        S(5, -7), S(-7, 18), S(-9, 21), S(-13, 31), S(-12, 30), S(1, 13), S(14, 5), S(13, 1),
	        S(-1, -21), S(4, -4), S(-7, 11), S(-6, 6), S(-4, 10), S(4, 2), S(20, -20), S(9, -22),
	        S(4, -25), S(3, -24), S(10, -27), S(9, -18), S(10, -16), S(14, -38), S(23, -62), S(23, -65),
	        S(-3, -21), S(-9, -20), S(-3, -21), S(9, -20), S(5, -27), S(-4, -38), S(1, -39), S(15, -43),
	};
	
	int king_psqt[64] = {
	        S(25, -101), S(42, -42), S(9, -18), S(-48, 9), S(18, -7), S(-8, 9), S(16, 13), S(77, -94),
	        S(-86, -8), S(1, 23), S(1, 28), S(75, 26), S(41, 37), S(34, 51), S(18, 52), S(30, 16),
	        S(-92, 0), S(48, 24), S(-9, 39), S(-30, 48), S(17, 53), S(71, 47), S(39, 46), S(14, 7),
	        S(0, -22), S(0, 16), S(-35, 35), S(-86, 48), S(-70, 49), S(-29, 44), S(-37, 35), S(-94, 5),
	        S(-14, -34), S(-6, 4), S(-18, 21), S(-65, 37), S(-53, 36), S(-11, 23), S(-36, 17), S(-93, -8),
	        S(-5, -38), S(31, -7), S(1, 6), S(-16, 16), S(-6, 14), S(-5, 10), S(7, 2), S(-34, -19),
	        S(14, -32), S(-7, -0), S(-10, 1), S(-16, 4), S(-21, 8), S(-22, 7), S(-9, 7), S(-2, -19),
	        S(25, -79), S(21, -29), S(13, -27), S(-22, -20), S(27, -35), S(-30, -18), S(12, -23), S(30, -74),
	};
	
	int knight_mobility[9] = { S(-137, -110), S(-46, -65), S(-23, -33), S(-14, -12), S(-4, -3), S(-1, 8), S(8, 11), S(17, 15), S(25, 13), };
	
	int bishop_mobility[14] = { S(-36, -139), S(-49, -76), S(-20, -35), S(-15, -14), S(-3, -5), S(4, 2), S(8, 10), S(13, 13), S(15, 17), S(19, 17), S(20, 17), S(31, 7), S(34, 6), S(41, -6), };
	
	int rook_mobility[15] = { S(-59, -84), S(-47, -93), S(-16, -59), S(-10, -42), S(-3, -35), S(-2, -18), S(-1, -12), S(-5, -4), S(-1, -3), S(4, 1), S(8, 4), S(8, 10), S(11, 14), S(17, 15), S(20, 13), };
	
	int queen_mobility[28] = { S(0, 0), S(0, 0), S(-90, -62), S(-53, -133), S(-41, -90), S(-11, -70), S(-6, -56), S(-6, -38), S(-5, -22), S(-5, -4), S(-3, -1), S(-0, 3), S(1, 9), S(4, 10), S(3, 12), S(4, 17), S(3, 17), S(1, 18), S(2, 17), S(4, 12), S(8, 8), S(22, -13), S(34, -22), S(50, -41), S(40, -40), S(62, -63), S(-3, -58), S(-26, -72), };
	
	int king_mobility[9] = { S(22, 77), S(-39, 132), S(-45, 42), S(-22, 27), S(-13, 13), S(13, -5), S(-11, 0), S(6, -4), S(27, -18), };
	
	int isolated_pawn = S(-5, -8);
	
	int doubled_pawn = S(3, -16);
	
	int backward_pawn[8] = { S(0, 0), S(0, 0), S(0, 0), S(2, 6), S(-3, 1), S(4, -3), S(-2, -1), S(0, 0), };
	
	int backward_pawn_half_open[8] = { S(0, 0), S(0, 0), S(0, 0), S(11, -11), S(-17, -9), S(-14, -16), S(-16, -17), S(0, 0), };
	
	int chained_pawn[8] = { S(0, 0), S(137, 51), S(17, 49), S(19, 21), S(12, 11), S(17, 10), S(0, 0), S(0, 0), };
	
	int phalanx_pawn[8] = { S(0, 0), S(5, 174), S(66, 90), S(32, 32), S(11, 11), S(7, 2), S(4, -2), S(0, 0), };
	
	int passed_pawn[8] = { S(0, 0), S(63, 97), S(15, 73), S(-14, 49), S(-31, 31), S(-11, 15), S(13, -1), S(0, 0), };
	
	int passed_pawn_blocked[8] = { S(0, 0), S(20, 30), S(14, 11), S(-23, 28), S(-41, 20), S(-20, 16), S(3, 1), S(0, 0), };
	
	int passed_pawn_safe_advance = S(0, 11);
	
	int passed_pawn_safe_path = S(-61, 40);
	
	int passed_friendly_distance[8] = { S(0, 0), S(-6, -13), S(5, -19), S(13, -17), S(12, -12), S(4, -5), S(1, -2), S(0, 0), };
	
	int passed_enemy_distance[8] = { S(0, 0), S(11, 37), S(-5, 36), S(-5, 19), S(-3, 8), S(0, -1), S(-3, 0), S(0, 0), };
	
	int knight_outpost = S(7, -20);
	
	int knight_outpost_supported = S(28, 6);
	
	int bishop_pawn = S(-4, -7);
	
	int double_bishop = S(17, 58);
	
	int rook_open_file = S(27, 8);
	
	int rook_half_open_file = S(8, 10);
	
	int rook_on_seventh = S(-0, 20);
	
	int pawn_shelter[2][4][8] = {
	{
	        { S(0, 0), S(22, 31), S(46, 12), S(26, -3), S(24, -4), S(42, -11), S(40, -26), S(0, 0), },
	        { S(0, 0), S(1, 11), S(15, 10), S(20, 1), S(14, -1), S(32, -6), S(49, -7), S(0, 0), },
	        { S(0, 0), S(9, 37), S(7, 12), S(15, -1), S(16, -5), S(12, 4), S(40, -1), S(0, 0), },
	        { S(0, 0), S(15, 7), S(-11, 5), S(16, -1), S(11, 0), S(12, -2), S(14, 3), S(0, 0), },
	},
	{
	        { S(0, 0), S(16, 62), S(61, 17), S(26, -1), S(39, -6), S(59, -16), S(64, -45), S(0, 0), },
	        { S(0, 0), S(21, 30), S(30, -1), S(24, -2), S(22, -4), S(53, -7), S(60, -15), S(0, 0), },
	        { S(0, 0), S(18, 25), S(11, 8), S(31, 1), S(33, -3), S(33, 3), S(65, -1), S(0, 0), },
	        { S(0, 0), S(-24, 30), S(-19, 3), S(17, 9), S(26, 5), S(23, 0), S(11, 8), S(0, 0), },
	},
	};
	
	int king_attacker_weight[6] = { S(0, 0), S(38, 94), S(23, 87), S(38, 51), S(4, 20), S(0, 0), };
	
	int king_zone_attack_count_weight = S(26, -91);
	
	int king_zone_weak_square = S(92, -14);
	
	int safe_knight_check = S(301, 94);
	
	int safe_bishop_check = S(201, 376);
	
	int safe_rook_check = S(314, 170);
	
	int safe_queen_check = S(172, 139);
	
	int unsafe_check = S(44, 81);
	
	int king_danger_no_queen_weight = S(-432, -203);
	
	int king_danger_offset = S(38, -1);
	
	int center_control = S(3, -1);
	
	int minor_threatened_by_pawn = S(-44, -22);
	
	int minor_threatened_by_minor = S(-17, -18);
	
	int rook_threatened_by_lesser = S(-42, 2);
	
	int queen_threatened_by_lesser = S(-37, 11);
	
	int minor_threatened_by_major = S(-10, -16);
	
	int pawn_push_threat = S(-18, -19);

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
