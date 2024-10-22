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

	int piece_value[6] = { S(47, 101), S(266, 361), S(293, 382), S(341, 650), S(702, 1234), S(0, 0), };

	int pawn_psqt[64] = {
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	        S(18, 23), S(19, 25), S(12, 46), S(75, 27), S(58, 36), S(47, 21), S(-27, 55), S(-39, 47),
	        S(9, 24), S(4, 33), S(22, 13), S(33, 2), S(32, 7), S(48, 7), S(19, 33), S(-17, 30),
	        S(-6, 21), S(-6, 9), S(3, -2), S(3, -13), S(18, -13), S(17, -8), S(-4, 2), S(0, 4),
	        S(-10, 9), S(-7, 6), S(2, -4), S(9, -10), S(12, -11), S(7, -6), S(1, -4), S(-9, -6),
	        S(-15, 5), S(-12, 1), S(-3, -4), S(2, -1), S(10, -1), S(-2, -5), S(-2, -8), S(-18, -7),
	        S(-6, 10), S(-4, 9), S(-1, 4), S(3, 6), S(11, 12), S(1, 5), S(8, 0), S(-9, 2),
	        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
	};
	
	int knight_psqt[64] = {
	        S(-97, -35), S(-91, -8), S(-71, 7), S(-38, -2), S(-23, 7), S(-80, -13), S(-99, -1), S(-71, -44),
	        S(-35, 4), S(-26, 8), S(-21, 8), S(-11, 11), S(-16, 4), S(-5, -1), S(-19, 6), S(-15, -8),
	        S(-20, -0), S(-12, 1), S(-6, 10), S(-10, 13), S(-3, 10), S(23, -9), S(-16, -3), S(-5, -6),
	        S(-11, 10), S(-3, 3), S(7, 8), S(22, 12), S(14, 11), S(19, 9), S(-1, 10), S(9, 5),
	        S(2, 7), S(7, 0), S(9, 12), S(17, 14), S(11, 20), S(12, 8), S(8, 4), S(5, 7),
	        S(-10, -11), S(-5, -7), S(-1, -4), S(-1, 8), S(13, 8), S(0, -7), S(14, -7), S(2, -2),
	        S(-14, -9), S(-10, -5), S(-8, -9), S(6, -8), S(4, -8), S(5, -9), S(12, -10), S(9, 5),
	        S(-44, -1), S(-5, -12), S(-22, -11), S(-12, -8), S(-4, -5), S(-1, -14), S(-4, -1), S(-15, 1),
	};
	
	int bishop_psqt[64] = {
	        S(-19, -1), S(-57, 10), S(-54, 4), S(-79, 12), S(-78, 9), S(-74, 1), S(-46, -2), S(-53, -6),
	        S(-24, -6), S(-28, 3), S(-17, 1), S(-33, 5), S(-18, -3), S(-18, -3), S(-45, 9), S(-32, -9),
	        S(-10, 7), S(5, 5), S(-5, 6), S(6, -0), S(-3, 3), S(18, 12), S(-1, 7), S(0, 11),
	        S(-13, 4), S(-7, 8), S(-1, 9), S(4, 24), S(-3, 17), S(-4, 12), S(-7, 6), S(-18, 5),
	        S(-5, -5), S(-14, 8), S(-5, 12), S(0, 19), S(-6, 15), S(-3, 7), S(-11, 4), S(5, -13),
	        S(-1, -4), S(7, 2), S(4, 6), S(0, 7), S(5, 13), S(5, 3), S(10, -3), S(12, -9),
	        S(19, -4), S(9, -12), S(11, -12), S(0, -3), S(5, -2), S(14, -9), S(25, -13), S(21, -16),
	        S(10, -11), S(15, -12), S(6, -2), S(-5, -8), S(9, -8), S(-1, 2), S(12, -12), S(27, -27),
	};
	
	int rook_psqt[64] = {
	        S(4, 19), S(-6, 24), S(-19, 34), S(-25, 30), S(-6, 24), S(-1, 26), S(7, 26), S(27, 19),
	        S(-13, 6), S(-15, 15), S(-7, 21), S(6, 11), S(-12, 14), S(6, 13), S(10, 9), S(20, 1),
	        S(-15, 19), S(12, 17), S(-2, 21), S(1, 17), S(21, 9), S(23, 8), S(56, 4), S(15, 4),
	        S(-12, 17), S(1, 13), S(-1, 18), S(0, 13), S(8, 2), S(14, -0), S(16, 5), S(4, 1),
	        S(-16, 5), S(-18, 7), S(-11, 5), S(-8, 4), S(-2, 0), S(-10, 1), S(6, -4), S(-12, -4),
	        S(-16, -4), S(-13, -5), S(-9, -7), S(-8, -6), S(2, -10), S(3, -14), S(24, -25), S(1, -21),
	        S(-15, -14), S(-10, -11), S(-2, -11), S(-1, -13), S(6, -19), S(8, -21), S(23, -31), S(-8, -23),
	        S(-1, -7), S(-4, -11), S(-3, -6), S(4, -11), S(11, -16), S(7, -11), S(8, -15), S(7, -20),
	};
	
	int queen_psqt[64] = {
	        S(-25, 11), S(-15, 2), S(-21, 23), S(12, 17), S(-8, 21), S(-7, 21), S(60, -28), S(8, 6),
	        S(-8, -7), S(-22, 1), S(-24, 29), S(-40, 46), S(-37, 52), S(-9, 29), S(1, 2), S(29, 17),
	        S(1, -8), S(-8, -2), S(-13, 24), S(-7, 26), S(-2, 32), S(4, 19), S(16, -1), S(15, 8),
	        S(-12, 14), S(-6, 13), S(-13, 22), S(-23, 36), S(-20, 36), S(-6, 17), S(2, 26), S(2, 12),
	        S(3, -8), S(-14, 19), S(-13, 19), S(-18, 32), S(-19, 31), S(-8, 12), S(7, 4), S(10, -1),
	        S(-0, -22), S(3, -6), S(-8, 10), S(-6, 5), S(-4, 10), S(2, 1), S(19, -20), S(9, -22),
	        S(5, -27), S(4, -26), S(10, -27), S(10, -19), S(11, -18), S(14, -39), S(22, -61), S(23, -67),
	        S(-2, -24), S(-8, -22), S(-3, -22), S(9, -20), S(6, -27), S(-4, -37), S(-3, -37), S(14, -44),
	};
	
	int king_psqt[64] = {
	        S(25, -95), S(48, -40), S(7, -16), S(-50, 11), S(13, -5), S(-12, 11), S(12, 15), S(78, -89),
	        S(-82, -6), S(2, 21), S(-0, 26), S(70, 24), S(38, 35), S(32, 48), S(21, 49), S(30, 18),
	        S(-93, 3), S(47, 22), S(-14, 37), S(-30, 46), S(14, 51), S(69, 46), S(40, 44), S(13, 9),
	        S(-2, -19), S(-5, 15), S(-37, 34), S(-90, 48), S(-74, 48), S(-31, 43), S(-42, 34), S(-98, 8),
	        S(-21, -31), S(-10, 2), S(-22, 20), S(-69, 36), S(-61, 35), S(-17, 22), S(-39, 15), S(-96, -5),
	        S(-3, -36), S(30, -9), S(-2, 5), S(-20, 14), S(-10, 13), S(-10, 9), S(7, 0), S(-32, -17),
	        S(15, -30), S(-5, -2), S(-10, -1), S(-14, 2), S(-19, 6), S(-22, 5), S(-7, 5), S(-0, -17),
	        S(23, -74), S(22, -27), S(13, -25), S(-22, -19), S(28, -34), S(-31, -17), S(13, -21), S(29, -69),
	};
	
	int knight_mobility[9] = { S(-136, -113), S(-47, -67), S(-24, -35), S(-15, -14), S(-5, -3), S(-3, 7), S(7, 12), S(16, 16), S(26, 15), };
	
	int bishop_mobility[14] = { S(-34, -140), S(-50, -75), S(-21, -34), S(-16, -14), S(-3, -5), S(3, 2), S(8, 10), S(13, 13), S(16, 17), S(19, 17), S(21, 17), S(30, 8), S(30, 8), S(35, -3), };
	
	int rook_mobility[15] = { S(-64, -90), S(-48, -94), S(-15, -58), S(-10, -41), S(-3, -35), S(-2, -17), S(-1, -12), S(-5, -4), S(-1, -3), S(3, 2), S(7, 5), S(8, 11), S(10, 15), S(16, 16), S(18, 14), };
	
	int queen_mobility[28] = { S(0, 0), S(0, 0), S(-90, -62), S(-53, -137), S(-39, -92), S(-12, -69), S(-7, -55), S(-7, -37), S(-6, -23), S(-6, -4), S(-3, -0), S(-1, 4), S(1, 9), S(3, 10), S(3, 12), S(3, 17), S(3, 17), S(1, 19), S(3, 18), S(6, 12), S(11, 8), S(26, -12), S(39, -22), S(56, -40), S(45, -38), S(69, -61), S(10, -54), S(-24, -66), };
	
	int king_mobility[9] = { S(19, 71), S(-40, 125), S(-42, 37), S(-20, 23), S(-13, 11), S(12, -6), S(-10, 1), S(5, -2), S(26, -15), };
	
	int isolated_pawn = S(-5, -7);
	
	int doubled_pawn = S(2, -16);
	
	int backward_pawn = S(0, -0);
	
	int backward_pawn_half_open = S(-15, -15);
	
	int chained_pawn[8] = { S(0, 0), S(137, 52), S(18, 50), S(14, 21), S(13, 10), S(15, 11), S(0, 0), S(0, 0), };
	
	int phalanx_pawn[8] = { S(0, 0), S(8, 170), S(72, 92), S(31, 33), S(13, 10), S(6, 4), S(4, -1), S(0, 0), };
	
	int passed_pawn[8] = { S(0, 0), S(60, 94), S(17, 71), S(-12, 47), S(-27, 31), S(-8, 16), S(18, 1), S(0, 0), };
	
	int passed_pawn_blocked[8] = { S(0, 0), S(18, 28), S(13, 8), S(-23, 25), S(-41, 19), S(-15, 18), S(6, 2), S(0, 0), };
	
	int passed_pawn_safe_advance = S(1, 10);
	
	int passed_pawn_safe_path = S(-62, 38);
	
	int passed_friendly_distance[8] = { S(0, 0), S(-6, -13), S(4, -18), S(12, -17), S(12, -12), S(4, -5), S(1, -2), S(0, 0), };
	
	int passed_enemy_distance[8] = { S(0, 0), S(11, 36), S(-5, 35), S(-5, 19), S(-3, 8), S(-1, -1), S(-3, -0), S(0, 0), };
	
	int knight_outpost = S(18, -6);
	
	int knight_outpost_supported = S(37, 20);
	
	int bishop_pawn = S(-4, -7);
	
	int double_bishop = S(16, 57);
	
	int rook_open_file = S(26, 8);
	
	int rook_half_open_file = S(8, 10);
	
	int rook_on_seventh = S(2, 20);
	
	int pawn_shelter[2][4][8] = {
	{
	        { S(0, 0), S(21, 31), S(45, 12), S(25, -3), S(24, -4), S(41, -11), S(39, -25), S(0, 0), },
	        { S(0, 0), S(0, 12), S(16, 9), S(21, 1), S(15, -1), S(32, -6), S(50, -7), S(0, 0), },
	        { S(0, 0), S(9, 37), S(6, 12), S(14, -1), S(17, -5), S(12, 4), S(40, -1), S(0, 0), },
	        { S(0, 0), S(15, 7), S(-12, 5), S(15, -1), S(12, 1), S(12, -2), S(14, 3), S(0, 0), },
	},
	{
	        { S(0, 0), S(16, 61), S(59, 18), S(26, -1), S(40, -6), S(59, -16), S(65, -44), S(0, 0), },
	        { S(0, 0), S(20, 30), S(32, -1), S(26, -2), S(24, -3), S(56, -6), S(62, -14), S(0, 0), },
	        { S(0, 0), S(14, 25), S(12, 7), S(31, 1), S(35, -3), S(35, 3), S(66, -0), S(0, 0), },
	        { S(0, 0), S(-29, 30), S(-20, 4), S(17, 9), S(26, 6), S(24, 0), S(12, 8), S(0, 0), },
	},
	};
	
	int king_attacker_weight[6] = { S(0, 0), S(38, 72), S(18, 65), S(31, 27), S(10, 26), S(0, 0), };
	
	int king_zone_attack_count_weight = S(50, -17);
	
	int king_danger_no_queen_weight = S(-509, -213);
	
	int safe_knight_check = S(310, -0);
	
	int safe_bishop_check = S(189, 400);
	
	int safe_rook_check = S(294, 137);
	
	int safe_queen_check = S(175, 151);
	
	int king_zone_weak_square = S(100, -3);
	
	int king_danger_offset = S(51, -2);
	
	int center_control = S(3, -1);
	
	int minor_threatened_by_pawn = S(-38, -18);
	
	int minor_threatened_by_minor = S(-17, -18);
	
	int rook_threatened_by_lesser = S(-41, 2);
	
	int queen_threatened_by_lesser = S(-35, 12);
	
	int minor_threatened_by_major = S(-9, -15);

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
