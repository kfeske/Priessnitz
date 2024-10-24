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

	int piece_value[6] = { S(48, 102), S(270, 366), S(298, 387), S(347, 659), S(715, 1249), S(0, 0), };

	int pawn_psqt[64] = {
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	        S(18, 24), S(20, 25), S(12, 47), S(76, 28), S(60, 37), S(49, 22), S(-23, 56), S(-39, 47),
	        S(9, 25), S(4, 33), S(22, 14), S(34, 2), S(33, 8), S(49, 7), S(21, 33), S(-18, 30),
	        S(-7, 21), S(-5, 9), S(3, -2), S(4, -14), S(19, -13), S(17, -8), S(-4, 2), S(0, 4),
	        S(-10, 10), S(-7, 6), S(2, -4), S(10, -10), S(12, -11), S(7, -6), S(1, -4), S(-9, -6),
	        S(-16, 6), S(-12, 1), S(-3, -3), S(2, -1), S(9, -2), S(-2, -5), S(-2, -8), S(-19, -7),
	        S(-6, 10), S(-4, 9), S(-1, 4), S(3, 7), S(11, 12), S(0, 5), S(8, 1), S(-10, 2),
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	};
	
	int knight_psqt[64] = {
	        S(-98, -34), S(-90, -7), S(-73, 8), S(-36, -2), S(-22, 7), S(-74, -13), S(-97, -1), S(-70, -45),
	        S(-35, 4), S(-26, 9), S(-21, 8), S(-11, 12), S(-14, 4), S(0, -2), S(-16, 6), S(-14, -8),
	        S(-19, 0), S(-11, 1), S(-6, 10), S(-6, 12), S(4, 9), S(28, -10), S(-11, -4), S(-3, -6),
	        S(-10, 11), S(-4, 4), S(8, 8), S(15, 13), S(19, 10), S(15, 11), S(7, 8), S(9, 5),
	        S(2, 8), S(7, 0), S(8, 13), S(16, 14), S(7, 21), S(12, 7), S(1, 5), S(5, 7),
	        S(-10, -10), S(-5, -6), S(-1, -3), S(-0, 8), S(12, 8), S(1, -7), S(14, -7), S(3, -2),
	        S(-14, -9), S(-10, -5), S(-8, -9), S(6, -8), S(4, -8), S(4, -9), S(11, -10), S(10, 6),
	        S(-44, -0), S(-5, -11), S(-22, -10), S(-11, -7), S(-3, -5), S(-0, -14), S(-4, -0), S(-15, 2),
	};
	
	int bishop_psqt[64] = {
	        S(-18, -1), S(-58, 10), S(-55, 4), S(-81, 13), S(-79, 9), S(-70, 1), S(-45, -2), S(-54, -5),
	        S(-23, -6), S(-27, 3), S(-17, 1), S(-32, 5), S(-14, -4), S(-16, -3), S(-38, 8), S(-29, -9),
	        S(-9, 7), S(5, 5), S(-5, 6), S(9, -1), S(-2, 4), S(26, 11), S(-0, 7), S(7, 10),
	        S(-12, 4), S(-8, 9), S(0, 9), S(4, 25), S(-1, 17), S(-5, 13), S(-7, 6), S(-19, 5),
	        S(-5, -5), S(-13, 8), S(-6, 12), S(1, 20), S(-7, 15), S(-3, 7), S(-12, 5), S(5, -13),
	        S(-0, -3), S(7, 2), S(4, 6), S(0, 8), S(4, 13), S(5, 3), S(10, -3), S(12, -9),
	        S(18, -3), S(9, -12), S(11, -12), S(-0, -2), S(5, -1), S(14, -9), S(25, -13), S(21, -16),
	        S(11, -11), S(15, -12), S(6, -1), S(-4, -8), S(9, -8), S(-1, 2), S(13, -12), S(27, -26),
	};
	
	int rook_psqt[64] = {
	        S(5, 20), S(-4, 24), S(-18, 35), S(-23, 30), S(-4, 24), S(1, 26), S(9, 26), S(28, 19),
	        S(-11, 6), S(-13, 16), S(-5, 21), S(8, 11), S(-10, 14), S(7, 13), S(10, 10), S(20, 2),
	        S(-16, 20), S(12, 18), S(-3, 21), S(-1, 18), S(18, 10), S(21, 9), S(55, 4), S(12, 5),
	        S(-12, 18), S(1, 13), S(-0, 18), S(0, 13), S(7, 3), S(14, 0), S(14, 6), S(4, 2),
	        S(-15, 5), S(-18, 7), S(-11, 6), S(-8, 4), S(-2, 1), S(-10, 1), S(5, -4), S(-12, -4),
	        S(-15, -4), S(-12, -5), S(-9, -7), S(-8, -6), S(2, -10), S(4, -15), S(23, -25), S(3, -21),
	        S(-15, -14), S(-9, -11), S(-2, -11), S(-0, -12), S(6, -19), S(8, -21), S(21, -31), S(-7, -23),
	        S(-0, -6), S(-3, -10), S(-3, -6), S(4, -10), S(11, -16), S(8, -11), S(8, -15), S(7, -20),
	};
	
	int queen_psqt[64] = {
	        S(-26, 12), S(-15, 1), S(-26, 23), S(6, 16), S(-18, 23), S(-15, 21), S(56, -28), S(6, 6),
	        S(-8, -6), S(-23, 1), S(-28, 28), S(-48, 45), S(-49, 52), S(-24, 32), S(-6, 4), S(26, 19),
	        S(0, -7), S(-9, -2), S(-15, 22), S(-11, 23), S(-10, 28), S(-11, 16), S(5, -2), S(1, 10),
	        S(-11, 14), S(-5, 13), S(-13, 21), S(-24, 35), S(-21, 31), S(-8, 11), S(1, 22), S(2, 9),
	        S(5, -7), S(-13, 19), S(-12, 20), S(-17, 32), S(-17, 29), S(-6, 10), S(8, 3), S(12, -2),
	        S(1, -20), S(5, -4), S(-6, 12), S(-5, 6), S(-2, 10), S(4, 2), S(21, -20), S(12, -22),
	        S(6, -24), S(5, -23), S(11, -26), S(11, -17), S(12, -16), S(15, -38), S(24, -61), S(26, -66),
	        S(-1, -20), S(-7, -20), S(-1, -20), S(11, -19), S(7, -27), S(-2, -37), S(-1, -37), S(15, -41),
	};
	
	int king_psqt[64] = {
	        S(27, -97), S(47, -41), S(11, -17), S(-47, 11), S(19, -5), S(-10, 11), S(14, 15), S(78, -90),
	        S(-84, -6), S(2, 21), S(3, 26), S(74, 24), S(41, 35), S(33, 49), S(17, 50), S(28, 19),
	        S(-90, 2), S(48, 22), S(-10, 37), S(-31, 47), S(15, 52), S(70, 46), S(37, 45), S(15, 9),
	        S(0, -20), S(-3, 15), S(-37, 34), S(-88, 48), S(-74, 48), S(-32, 44), S(-41, 34), S(-97, 8),
	        S(-15, -32), S(-9, 2), S(-22, 20), S(-67, 36), S(-58, 35), S(-15, 22), S(-40, 15), S(-94, -6),
	        S(-3, -36), S(28, -9), S(-1, 4), S(-17, 14), S(-7, 13), S(-7, 8), S(5, 0), S(-32, -17),
	        S(15, -30), S(-8, -2), S(-11, -1), S(-17, 2), S(-21, 6), S(-23, 5), S(-10, 6), S(-1, -17),
	        S(25, -75), S(21, -27), S(15, -25), S(-21, -18), S(29, -33), S(-29, -17), S(13, -21), S(30, -70),
	};
	
	int knight_mobility[9] = { S(-136, -115), S(-46, -68), S(-24, -36), S(-15, -14), S(-5, -3), S(-2, 8), S(8, 12), S(17, 17), S(27, 16), };
	
	int bishop_mobility[14] = { S(-34, -142), S(-50, -76), S(-21, -35), S(-15, -14), S(-3, -4), S(4, 2), S(8, 11), S(14, 14), S(17, 18), S(20, 18), S(22, 18), S(33, 9), S(35, 8), S(43, -3), };
	
	int rook_mobility[15] = { S(-64, -91), S(-49, -95), S(-16, -58), S(-10, -40), S(-3, -35), S(-2, -17), S(-1, -12), S(-5, -3), S(-1, -2), S(4, 2), S(8, 5), S(8, 11), S(11, 15), S(17, 16), S(20, 15), };
	
	int queen_mobility[28] = { S(0, 0), S(0, 0), S(-93, -63), S(-52, -137), S(-39, -90), S(-12, -69), S(-7, -56), S(-6, -37), S(-5, -22), S(-5, -4), S(-3, 0), S(-0, 4), S(1, 10), S(4, 10), S(3, 13), S(4, 18), S(3, 17), S(1, 19), S(3, 18), S(5, 13), S(10, 8), S(24, -12), S(36, -22), S(52, -40), S(42, -39), S(63, -61), S(-1, -55), S(-23, -70), };
	
	int king_mobility[9] = { S(20, 74), S(-43, 127), S(-44, 37), S(-22, 24), S(-14, 11), S(12, -6), S(-10, 1), S(7, -2), S(29, -16), };
	
	int isolated_pawn = S(-5, -7);
	
	int doubled_pawn = S(2, -16);
	
	int backward_pawn = S(0, -0);
	
	int backward_pawn_half_open = S(-15, -15);
	
	int chained_pawn[8] = { S(0, 0), S(139, 52), S(19, 50), S(14, 21), S(13, 10), S(16, 12), S(0, 0), S(0, 0), };
	
	int phalanx_pawn[8] = { S(0, 0), S(3, 174), S(74, 93), S(31, 33), S(13, 11), S(6, 4), S(4, -1), S(0, 0), };
	
	int passed_pawn[8] = { S(0, 0), S(63, 95), S(17, 72), S(-13, 48), S(-28, 31), S(-12, 17), S(15, 2), S(0, 0), };
	
	int passed_pawn_blocked[8] = { S(0, 0), S(20, 28), S(13, 8), S(-24, 26), S(-43, 20), S(-19, 19), S(3, 3), S(0, 0), };
	
	int passed_pawn_safe_advance = S(1, 10);
	
	int passed_pawn_safe_path = S(-63, 39);
	
	int passed_friendly_distance[8] = { S(0, 0), S(-6, -13), S(4, -18), S(12, -17), S(12, -12), S(4, -5), S(1, -2), S(0, 0), };
	
	int passed_enemy_distance[8] = { S(0, 0), S(11, 37), S(-5, 36), S(-5, 19), S(-3, 8), S(-0, -1), S(-3, -0), S(0, 0), };
	
	int knight_outpost = S(18, -6);
	
	int knight_outpost_supported = S(37, 20);
	
	int bishop_pawn = S(-4, -7);
	
	int double_bishop = S(17, 58);
	
	int rook_open_file = S(26, 8);
	
	int rook_half_open_file = S(8, 10);
	
	int rook_on_seventh = S(1, 20);
	
	int pawn_shelter[2][4][8] = {
	{
	        { S(0, 0), S(22, 31), S(47, 13), S(26, -3), S(25, -5), S(42, -11), S(40, -26), S(0, 0), },
	        { S(0, 0), S(-1, 12), S(16, 10), S(21, 1), S(14, -1), S(32, -6), S(49, -7), S(0, 0), },
	        { S(0, 0), S(9, 38), S(5, 12), S(14, -1), S(17, -5), S(12, 4), S(40, -1), S(0, 0), },
	        { S(0, 0), S(17, 7), S(-12, 5), S(15, -1), S(11, 1), S(12, -2), S(13, 3), S(0, 0), },
	},
	{
	        { S(0, 0), S(16, 62), S(59, 18), S(25, -1), S(39, -6), S(58, -16), S(64, -45), S(0, 0), },
	        { S(0, 0), S(21, 30), S(31, -1), S(24, -2), S(22, -3), S(54, -6), S(60, -14), S(0, 0), },
	        { S(0, 0), S(18, 25), S(11, 7), S(29, 1), S(33, -3), S(33, 3), S(64, -0), S(0, 0), },
	        { S(0, 0), S(-22, 29), S(-20, 4), S(16, 9), S(25, 6), S(23, 1), S(11, 8), S(0, 0), },
	},
	};
	
	int king_attacker_weight[6] = { S(0, 0), S(38, 83), S(23, 78), S(39, 48), S(4, 12), S(0, 0), };
	
	int king_zone_attack_count_weight = S(25, -83);
	
	int king_zone_weak_square = S(93, -12);
	
	int safe_knight_check = S(301, 92);
	
	int safe_bishop_check = S(203, 376);
	
	int safe_rook_check = S(316, 164);
	
	int safe_queen_check = S(172, 144);
	
	int unsafe_check = S(45, 78);
	
	int king_danger_no_queen_weight = S(-427, -197);
	
	int king_danger_offset = S(31, 0);
	
	int center_control = S(3, -1);
	
	int minor_threatened_by_pawn = S(-39, -18);
	
	int minor_threatened_by_minor = S(-17, -18);
	
	int rook_threatened_by_lesser = S(-41, 2);
	
	int queen_threatened_by_lesser = S(-36, 12);
	
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
