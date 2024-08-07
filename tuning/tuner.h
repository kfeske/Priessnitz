// the task of the tuner is to optimize all the values in the evaluation function, eg. the
// value of a pawn. This is done by a gradient descent algorithm, which adjusts the values
// in a way that the evaluation of every position in the training set predicts the outcome
// of the game, the position is from as best as possible.

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "../board.h"
#include "../evaluation.h"

enum Indicies {
	MG_VALUES = 0,
	EG_VALUES = MG_VALUES + 6,

	MG_PAWN_PSQT = EG_VALUES + 6,
	MG_KNIGHT_PSQT = MG_PAWN_PSQT + 64,
	MG_BISHOP_PSQT = MG_KNIGHT_PSQT + 64,
	MG_ROOK_PSQT = MG_BISHOP_PSQT + 64,
	MG_QUEEN_PSQT = MG_ROOK_PSQT + 64,
	MG_KING_PSQT = MG_QUEEN_PSQT + 64,
	EG_PAWN_PSQT = MG_KING_PSQT + 64,
	EG_KNIGHT_PSQT = EG_PAWN_PSQT + 64,
	EG_BISHOP_PSQT = EG_KNIGHT_PSQT + 64,
	EG_ROOK_PSQT = EG_BISHOP_PSQT + 64,
	EG_QUEEN_PSQT = EG_ROOK_PSQT + 64,
	EG_KING_PSQT = EG_QUEEN_PSQT + 64,

	MG_PASSED_PAWN = EG_KING_PSQT + 64,
	EG_PASSED_PAWN = MG_PASSED_PAWN + 8,
	MG_PASSED_PAWN_BLOCKED = EG_PASSED_PAWN + 8,
	EG_PASSED_PAWN_BLOCKED = MG_PASSED_PAWN_BLOCKED + 8,
	MG_PASSED_SAFE_ADVANCE = EG_PASSED_PAWN_BLOCKED + 8,
	EG_PASSED_SAFE_ADVANCE = MG_PASSED_SAFE_ADVANCE + 1,
	MG_PASSED_SAFE_PATH = EG_PASSED_SAFE_ADVANCE + 1,
	EG_PASSED_SAFE_PATH = MG_PASSED_SAFE_PATH + 1,
	MG_PASSED_FRIENDLY_DISTANCE = EG_PASSED_SAFE_PATH + 1,
	EG_PASSED_FRIENDLY_DISTANCE = MG_PASSED_FRIENDLY_DISTANCE + 8,
	MG_PASSED_ENEMY_DISTANCE    = EG_PASSED_FRIENDLY_DISTANCE + 8,
	EG_PASSED_ENEMY_DISTANCE    = MG_PASSED_ENEMY_DISTANCE + 8,

	MG_ISOLATED = EG_PASSED_ENEMY_DISTANCE + 8,
	EG_ISOLATED = MG_ISOLATED + 1,
	MG_DOUBLED = EG_ISOLATED + 1,
	EG_DOUBLED = MG_DOUBLED + 1,
	MG_BACKWARD = EG_DOUBLED + 1,
	EG_BACKWARD = MG_BACKWARD + 1,
	MG_CHAINED = EG_BACKWARD  + 1,
	EG_CHAINED = MG_CHAINED + 1,

	MG_DOUBLE_BISHOP = EG_CHAINED + 1,
	EG_DOUBLE_BISHOP = MG_DOUBLE_BISHOP + 1,

	MG_ROOK_OPEN_FILE      = EG_DOUBLE_BISHOP + 1,
	EG_ROOK_OPEN_FILE      = MG_ROOK_OPEN_FILE + 1,
	MG_ROOK_HALF_OPEN_FILE = EG_ROOK_OPEN_FILE + 1,
	EG_ROOK_HALF_OPEN_FILE = MG_ROOK_HALF_OPEN_FILE + 1,

	MG_ROOK_ON_SEVENTH = EG_ROOK_HALF_OPEN_FILE + 1,
	EG_ROOK_ON_SEVENTH = MG_ROOK_ON_SEVENTH + 1,

	MG_KNIGHT_OUTPOST 	    = EG_ROOK_ON_SEVENTH + 1,
	EG_KNIGHT_OUTPOST 	    = MG_KNIGHT_OUTPOST + 1,
	MG_KNIGHT_OUTPOST_SUPPORTED = EG_KNIGHT_OUTPOST + 1,
	EG_KNIGHT_OUTPOST_SUPPORTED = MG_KNIGHT_OUTPOST_SUPPORTED + 1,

	MG_KNIGHT_MOBILITY = EG_KNIGHT_OUTPOST_SUPPORTED + 1,
	EG_KNIGHT_MOBILITY = MG_KNIGHT_MOBILITY + 9,
	MG_BISHOP_MOBILITY = EG_KNIGHT_MOBILITY + 9,
	EG_BISHOP_MOBILITY = MG_BISHOP_MOBILITY + 14,
	MG_ROOK_MOBILITY   = EG_BISHOP_MOBILITY + 14,
	EG_ROOK_MOBILITY   = MG_ROOK_MOBILITY + 15,
	MG_QUEEN_MOBILITY  = EG_ROOK_MOBILITY + 15,
	EG_QUEEN_MOBILITY  = MG_QUEEN_MOBILITY + 28,
	MG_KING_MOBILITY   = EG_QUEEN_MOBILITY + 28,
	EG_KING_MOBILITY   = MG_KING_MOBILITY + 9,

	MG_MINOR_THREATENED_BY_PAWN   = EG_KING_MOBILITY + 9,
	EG_MINOR_THREATENED_BY_PAWN   = MG_MINOR_THREATENED_BY_PAWN + 1,
	MG_MINOR_THREATENED_BY_MINOR  = EG_MINOR_THREATENED_BY_PAWN + 1,
	EG_MINOR_THREATENED_BY_MINOR  = MG_MINOR_THREATENED_BY_MINOR + 1,
	MG_ROOK_THREATENED_BY_LESSER  = EG_MINOR_THREATENED_BY_MINOR + 1,
	EG_ROOK_THREATENED_BY_LESSER  = MG_ROOK_THREATENED_BY_LESSER + 1,
	MG_QUEEN_THREATENED_BY_LESSER = EG_ROOK_THREATENED_BY_LESSER + 1,
	EG_QUEEN_THREATENED_BY_LESSER = MG_QUEEN_THREATENED_BY_LESSER + 1,

	MG_KING_RING_POTENCY = EG_QUEEN_THREATENED_BY_LESSER + 1,
	EG_KING_RING_POTENCY = MG_KING_RING_POTENCY + 6,
	MG_KING_RING_PRESSURE = EG_KING_RING_POTENCY + 6,
	EG_KING_RING_PRESSURE = MG_KING_RING_PRESSURE + 8,

	MG_SAFE_KNIGHT_CHECK = EG_KING_RING_PRESSURE + 8,
	EG_SAFE_KNIGHT_CHECK = MG_SAFE_KNIGHT_CHECK + 1,
	MG_SAFE_BISHOP_CHECK = EG_SAFE_KNIGHT_CHECK + 1,
	EG_SAFE_BISHOP_CHECK = MG_SAFE_BISHOP_CHECK + 1,
	MG_SAFE_ROOK_CHECK   = EG_SAFE_BISHOP_CHECK + 1,
	EG_SAFE_ROOK_CHECK   = MG_SAFE_ROOK_CHECK + 1,
	MG_SAFE_QUEEN_CHECK  = EG_SAFE_ROOK_CHECK + 1,
	EG_SAFE_QUEEN_CHECK  = MG_SAFE_QUEEN_CHECK + 1,

	MG_PAWN_SHELTER = EG_SAFE_QUEEN_CHECK + 1,
	EG_PAWN_SHELTER = MG_PAWN_SHELTER + 64,

	CENTER_CONTROL = EG_PAWN_SHELTER + 64,

	PAWN_COUNT_SCALE_OFFSET = CENTER_CONTROL + 1,
	PAWN_COUNT_SCALE_WEIGHT = PAWN_COUNT_SCALE_OFFSET + 1,
	LONE_MINOR_SCALE = PAWN_COUNT_SCALE_WEIGHT + 1,

	END_INDEX = LONE_MINOR_SCALE + 1
};

enum Tuning_params {
	//NUM_TRAINING_POSITIONS = 6500000, // number of training positions
	//NUM_TEST_POSITIONS = 653653,      // number of test positions
	//NUM_TRAINING_POSITIONS = 5000,
	//NUM_TEST_POSITIONS = 0,
	NUM_TRAINING_POSITIONS = 7153653,
	NUM_TEST_POSITIONS = 0,
	NUM_TABLES = 82,		  // number of tables to be tuned (eg. pawn piece square table)
	NUM_WEIGHTS = END_INDEX,          // values to be tuned
	BATCH_SIZE = 1000	          // how much the training set is split for computational efficiency
};

double random_double()
{
	return rand() / (RAND_MAX + 1.0);
}

double random_double(double min, double max)
{
	return min + (max - min) * random_double();
}

// A single position and the game result
struct Sample
{
	double outcome;

	// Stores, how much a weight influences the evaluation of a position
	std::vector<double> mg_influence {};
	std::vector<double> eg_influence {};
	std::vector<Indicies> mg_influence_index {};
	std::vector<Indicies> eg_influence_index {};

	// Useful for king evaluation
	uint8_t ring_attackers[2] {};
	uint8_t ring_attacks[15] {};
	uint8_t zone_attackers[2] {};
	uint8_t zone_attacks[15] {};

	// Needed for endgame scaling
	double scale = 0; // updated during training in evaluate()
	uint8_t pawn_count[2] {};
	Color strong_side = WHITE;
	double eg_evaluation = 0;

	// Midgame / Endgame gamephase of the position
	double phase = 0;
};

struct Tuner
{
	Indicies table[NUM_TABLES + 1] = { MG_VALUES, EG_VALUES,
					   MG_PAWN_PSQT, MG_KNIGHT_PSQT, MG_BISHOP_PSQT, MG_ROOK_PSQT, MG_QUEEN_PSQT, MG_KING_PSQT,
	       				   EG_PAWN_PSQT, EG_KNIGHT_PSQT, EG_BISHOP_PSQT, EG_ROOK_PSQT, EG_QUEEN_PSQT, EG_KING_PSQT,
			     	      	   MG_PASSED_PAWN, EG_PASSED_PAWN, MG_PASSED_PAWN_BLOCKED, EG_PASSED_PAWN_BLOCKED,
					   MG_PASSED_SAFE_ADVANCE, EG_PASSED_SAFE_ADVANCE, MG_PASSED_SAFE_PATH, EG_PASSED_SAFE_PATH,
					   MG_PASSED_FRIENDLY_DISTANCE, EG_PASSED_FRIENDLY_DISTANCE, MG_PASSED_ENEMY_DISTANCE, EG_PASSED_ENEMY_DISTANCE,
					   MG_ISOLATED, EG_ISOLATED, MG_DOUBLED, EG_DOUBLED,
					   MG_BACKWARD, EG_BACKWARD, MG_CHAINED, EG_CHAINED,
					   MG_DOUBLE_BISHOP, EG_DOUBLE_BISHOP,
					   MG_ROOK_OPEN_FILE, EG_ROOK_OPEN_FILE, MG_ROOK_HALF_OPEN_FILE, EG_ROOK_HALF_OPEN_FILE,
					   MG_ROOK_ON_SEVENTH, EG_ROOK_ON_SEVENTH,
					   MG_KNIGHT_OUTPOST, EG_KNIGHT_OUTPOST, MG_KNIGHT_OUTPOST_SUPPORTED, EG_KNIGHT_OUTPOST_SUPPORTED,
					   MG_KNIGHT_MOBILITY, EG_KNIGHT_MOBILITY, MG_BISHOP_MOBILITY, EG_BISHOP_MOBILITY, MG_ROOK_MOBILITY, EG_ROOK_MOBILITY,
					   MG_QUEEN_MOBILITY,  EG_QUEEN_MOBILITY,  MG_KING_MOBILITY,   EG_KING_MOBILITY,
					   MG_MINOR_THREATENED_BY_PAWN, EG_MINOR_THREATENED_BY_PAWN, MG_MINOR_THREATENED_BY_MINOR, EG_MINOR_THREATENED_BY_MINOR,
					   MG_ROOK_THREATENED_BY_LESSER, EG_ROOK_THREATENED_BY_LESSER, MG_QUEEN_THREATENED_BY_LESSER, EG_QUEEN_THREATENED_BY_LESSER,
			     	      	   MG_KING_RING_POTENCY, EG_KING_RING_POTENCY, MG_KING_RING_PRESSURE, EG_KING_RING_PRESSURE,
					   MG_SAFE_KNIGHT_CHECK, EG_SAFE_KNIGHT_CHECK, MG_SAFE_BISHOP_CHECK, EG_SAFE_BISHOP_CHECK,
					   MG_SAFE_ROOK_CHECK, EG_SAFE_ROOK_CHECK, MG_SAFE_QUEEN_CHECK, EG_SAFE_QUEEN_CHECK,
					   MG_PAWN_SHELTER, EG_PAWN_SHELTER,
					   CENTER_CONTROL,
					   PAWN_COUNT_SCALE_OFFSET,
					   PAWN_COUNT_SCALE_WEIGHT,
			     	      	   END_INDEX };

	double const SCALING = 3.45387764 / 400; // scaling constant for our evaluation function
	double const TINY_NUMBER = 0.00000001;   // difference quotient step size
	double const LEARN_RATE = 0.5;	 	 // step size

	Board board {};
	Evaluation eval {};

	std::vector<Sample> training_set {};
	std::vector<Sample> test_set {};
	double weights[NUM_WEIGHTS];
	double gradients[NUM_WEIGHTS];
	double sum_squared_gradients[NUM_WEIGHTS];

	// Returns the value in a table at a position
	int table_value(Indicies table_index, unsigned pos)
	{
		switch (table_index) {
		case MG_VALUES: return eval.mg_piece_value[pos];
		case EG_VALUES: return eval.eg_piece_value[pos];

		case MG_PAWN_PSQT:   return eval.mg_pawn_psqt[pos];
		case MG_KNIGHT_PSQT: return eval.mg_knight_psqt[pos];
		case MG_BISHOP_PSQT: return eval.mg_bishop_psqt[pos];
		case MG_ROOK_PSQT:   return eval.mg_rook_psqt[pos];
		case MG_QUEEN_PSQT:  return eval.mg_queen_psqt[pos];
		case MG_KING_PSQT:   return eval.mg_king_psqt[pos];
		case EG_PAWN_PSQT:   return eval.eg_pawn_psqt[pos];
		case EG_KNIGHT_PSQT: return eval.eg_knight_psqt[pos];
		case EG_BISHOP_PSQT: return eval.eg_bishop_psqt[pos];
		case EG_ROOK_PSQT:   return eval.eg_rook_psqt[pos];
		case EG_QUEEN_PSQT:  return eval.eg_queen_psqt[pos];
		case EG_KING_PSQT:   return eval.eg_king_psqt[pos];

		case MG_PASSED_PAWN: 	     return eval.mg_passed_pawn[pos];
		case EG_PASSED_PAWN: 	     return eval.eg_passed_pawn[pos];
		case MG_PASSED_PAWN_BLOCKED: return eval.mg_passed_pawn_blocked[pos];
		case EG_PASSED_PAWN_BLOCKED: return eval.eg_passed_pawn_blocked[pos];
		case MG_PASSED_SAFE_ADVANCE: return eval.mg_passed_pawn_safe_advance;
		case EG_PASSED_SAFE_ADVANCE: return eval.eg_passed_pawn_safe_advance;
		case MG_PASSED_SAFE_PATH: return eval.mg_passed_pawn_safe_path;
		case EG_PASSED_SAFE_PATH: return eval.eg_passed_pawn_safe_path;
		case MG_PASSED_FRIENDLY_DISTANCE: return eval.mg_passed_friendly_distance[pos];
		case EG_PASSED_FRIENDLY_DISTANCE: return eval.eg_passed_friendly_distance[pos];
		case MG_PASSED_ENEMY_DISTANCE: return eval.mg_passed_enemy_distance[pos];
		case EG_PASSED_ENEMY_DISTANCE: return eval.eg_passed_enemy_distance[pos];
		case MG_ISOLATED:    return eval.mg_isolated_penalty;
		case EG_ISOLATED:    return eval.eg_isolated_penalty;
		case MG_DOUBLED:     return eval.mg_doubled_penalty;
		case EG_DOUBLED:     return eval.eg_doubled_penalty;
		case MG_BACKWARD:    return eval.mg_backward_penalty;
		case EG_BACKWARD:    return eval.eg_backward_penalty;
		case MG_CHAINED:     return eval.mg_chained_bonus;
		case EG_CHAINED:     return eval.eg_chained_bonus;

		case MG_DOUBLE_BISHOP: return eval.mg_double_bishop;
		case EG_DOUBLE_BISHOP: return eval.eg_double_bishop;

		case MG_ROOK_OPEN_FILE:      return eval.mg_rook_open_file;
		case EG_ROOK_OPEN_FILE:      return eval.eg_rook_open_file;
		case MG_ROOK_HALF_OPEN_FILE: return eval.mg_rook_half_open_file;
		case EG_ROOK_HALF_OPEN_FILE: return eval.eg_rook_half_open_file;

		case MG_ROOK_ON_SEVENTH: return eval.mg_rook_on_seventh;
		case EG_ROOK_ON_SEVENTH: return eval.eg_rook_on_seventh;

		case MG_KNIGHT_OUTPOST:	return eval.mg_knight_outpost;
		case EG_KNIGHT_OUTPOST: return eval.eg_knight_outpost;
		case MG_KNIGHT_OUTPOST_SUPPORTED: return eval.mg_knight_outpost_supported;
		case EG_KNIGHT_OUTPOST_SUPPORTED: return eval.eg_knight_outpost_supported;

		case MG_KNIGHT_MOBILITY: return eval.mg_knight_mobility[pos];
		case EG_KNIGHT_MOBILITY: return eval.eg_knight_mobility[pos];
		case MG_BISHOP_MOBILITY: return eval.mg_bishop_mobility[pos];
		case EG_BISHOP_MOBILITY: return eval.eg_bishop_mobility[pos];
		case MG_ROOK_MOBILITY:   return eval.mg_rook_mobility[pos];
		case EG_ROOK_MOBILITY:   return eval.eg_rook_mobility[pos];
		case MG_QUEEN_MOBILITY:  return eval.mg_queen_mobility[pos];
		case EG_QUEEN_MOBILITY:  return eval.eg_queen_mobility[pos];
		case MG_KING_MOBILITY:   return eval.mg_king_mobility[pos];
		case EG_KING_MOBILITY:   return eval.eg_king_mobility[pos];

		case MG_MINOR_THREATENED_BY_PAWN: return eval.mg_minor_threatened_by_pawn;
		case EG_MINOR_THREATENED_BY_PAWN: return eval.eg_minor_threatened_by_pawn;
		case MG_MINOR_THREATENED_BY_MINOR: return eval.mg_minor_threatened_by_minor;
		case EG_MINOR_THREATENED_BY_MINOR: return eval.eg_minor_threatened_by_minor;
		case MG_ROOK_THREATENED_BY_LESSER: return eval.mg_rook_threatened_by_lesser;
		case EG_ROOK_THREATENED_BY_LESSER: return eval.eg_rook_threatened_by_lesser;
		case MG_QUEEN_THREATENED_BY_LESSER: return eval.mg_queen_threatened_by_lesser;
		case EG_QUEEN_THREATENED_BY_LESSER: return eval.eg_queen_threatened_by_lesser;

		case MG_KING_RING_POTENCY:  return eval.mg_king_ring_attack_potency[pos];
		case EG_KING_RING_POTENCY:  return eval.eg_king_ring_attack_potency[pos];
		case MG_KING_RING_PRESSURE: return eval.mg_king_ring_pressure_weight[pos];
		case EG_KING_RING_PRESSURE: return eval.eg_king_ring_pressure_weight[pos];

		case MG_SAFE_KNIGHT_CHECK: return eval.mg_safe_knight_check;
		case EG_SAFE_KNIGHT_CHECK: return eval.eg_safe_knight_check;
		case MG_SAFE_BISHOP_CHECK: return eval.mg_safe_bishop_check;
		case EG_SAFE_BISHOP_CHECK: return eval.eg_safe_bishop_check;
		case MG_SAFE_ROOK_CHECK:   return eval.mg_safe_rook_check;
		case EG_SAFE_ROOK_CHECK:   return eval.eg_safe_rook_check;
		case MG_SAFE_QUEEN_CHECK:  return eval.mg_safe_queen_check;
		case EG_SAFE_QUEEN_CHECK:  return eval.eg_safe_queen_check;

		case MG_PAWN_SHELTER: {
				int *pawn_shelter = &eval.mg_pawn_shelter[0][0][0];
				return pawn_shelter[pos];
			}
		case EG_PAWN_SHELTER: {
				int *pawn_shelter = &eval.eg_pawn_shelter[0][0][0];
				return pawn_shelter[pos];
			}

		case CENTER_CONTROL: return eval.mg_center_control;

		case PAWN_COUNT_SCALE_OFFSET: return eval.pawn_count_scale_offset;
		case PAWN_COUNT_SCALE_WEIGHT: return eval.pawn_count_scale_weight;

		default: return 0;
		}
	}

	// Returns the table described with the index. (see table[] array)
	Indicies table_of(unsigned index)
	{
		for (unsigned i = 0; i < NUM_TABLES; i++) {
			if (index >= table[i] && index < table[i + 1])
				return table[i];
		}
		std::cerr << "error: index does not have a table";
		return END_INDEX;
	}

	// Returns the weight at an index from the evaluation function
	double load_weight(unsigned index)
	{
		Indicies table_index = table_of(index);
		return table_value(table_index, index - table_index);
	}

	int int_weight(unsigned index)
	{
		return std::round(weights[index]);
	}

	void print_int_weight(unsigned index)
	{
		std::cerr << int_weight(index) << ", ";
	}

	void print_64_field(Indicies index)
	{
		for (unsigned i = 0; i < 64; i++) {
			if (i % 8 == 0) std::cerr << "\n	";
			print_int_weight(index + i);
		}
		std::cerr << "\n};\n";
	}

	void print_weights()
	{
		std::cerr << "\ncurrent weights:\n";

		std ::cerr << "int mg_piece_value[6] = { ";
		for (unsigned i = 0; i < 6; i++) print_int_weight(MG_VALUES + i);
		std::cerr << "};\n";
		std::cerr << "int eg_piece_value[6] = { ";
		for (unsigned i = 0; i < 6; i++) print_int_weight(EG_VALUES + i);
		std::cerr << "};\n";

		std::cerr << "\nint mg_pawn_psqt[64] = {";
		print_64_field(MG_PAWN_PSQT);
		std::cerr << "int mg_knight_psqt[64] = {";
		print_64_field(MG_KNIGHT_PSQT);
		std::cerr << "int mg_bishop_psqt[64] = {";
		print_64_field(MG_BISHOP_PSQT);
		std::cerr << "int mg_rook_psqt[64] = {";
		print_64_field(MG_ROOK_PSQT);
		std::cerr << "int mg_queen_psqt[64] = {";
		print_64_field(MG_QUEEN_PSQT);
		std::cerr << "int mg_king_psqt[64] = {";
		print_64_field(MG_KING_PSQT);
		std::cerr << "int eg_pawn_psqt[64] = {";
		print_64_field(EG_PAWN_PSQT);
		std::cerr << "int eg_knight_psqt[64] = {";
		print_64_field(EG_KNIGHT_PSQT);
		std::cerr << "int eg_bishop_psqt[64] = {";
		print_64_field(EG_BISHOP_PSQT);
		std::cerr << "int eg_rook_psqt[64] = {";
		print_64_field(EG_ROOK_PSQT);
		std::cerr << "int eg_queen_psqt[64] = {";
		print_64_field(EG_QUEEN_PSQT);
		std::cerr << "int eg_king_psqt[64] = {";
		print_64_field(EG_KING_PSQT);

		std::cerr << "\nint mg_king_ring_attack_potency[6] = { ";
		for (unsigned i = 0; i < 6; i++) print_int_weight(MG_KING_RING_POTENCY + i);
		std::cerr << "};\n";
		std::cerr << "int eg_king_ring_attack_potency[6] = { ";
		for (unsigned i = 0; i < 6; i++) print_int_weight(EG_KING_RING_POTENCY + i);
		std::cerr << "};\n";
		std::cerr << "\nint mg_king_ring_pressure_weight[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(MG_KING_RING_PRESSURE + i);
		std::cerr << "};\n";
		std::cerr << "int eg_king_ring_pressure_weight[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(EG_KING_RING_PRESSURE + i);
		std::cerr << "};\n";

		std::cerr << "\nint mg_safe_knight_check = " << int_weight(MG_SAFE_KNIGHT_CHECK) << ";\n";
		std::cerr <<   "int eg_safe_knight_check = " << int_weight(EG_SAFE_KNIGHT_CHECK) << ";\n";
		std::cerr << "\nint mg_safe_bishop_check = " << int_weight(MG_SAFE_BISHOP_CHECK) << ";\n";
		std::cerr <<   "int eg_safe_bishop_check = " << int_weight(EG_SAFE_BISHOP_CHECK) << ";\n";
		std::cerr << "\nint mg_safe_rook_check = "   << int_weight(MG_SAFE_ROOK_CHECK)   << ";\n";
		std::cerr <<   "int eg_safe_rook_check = "   << int_weight(EG_SAFE_ROOK_CHECK)   << ";\n";
		std::cerr << "\nint mg_safe_queen_check = "  << int_weight(MG_SAFE_QUEEN_CHECK)  << ";\n";
		std::cerr <<   "int eg_safe_queen_check = "  << int_weight(EG_SAFE_QUEEN_CHECK)  << ";\n";

		std::cerr << "\nint mg_pawn_shelter[2][4][8] = {\n";
		for (unsigned i = 0; i < 2; i++) {
			std::cerr << "{\n";
			for (unsigned j = 0; j < 4; j++) {
				std::cerr << "        { ";
				for (unsigned k = 0; k < 8; k++) {
					print_int_weight(MG_PAWN_SHELTER + i * 32 + j * 8 + k);
				}
				std::cerr << "},\n";
			}
			std::cerr << "},\n";
		}
		std::cerr << "};\n";

		std::cerr << "\nint eg_pawn_shelter[2][4][8] = {\n";
		for (unsigned i = 0; i < 2; i++) {
			std::cerr << "{\n";
			for (unsigned j = 0; j < 4; j++) {
				std::cerr << "        { ";
				for (unsigned k = 0; k < 8; k++) {
					print_int_weight(EG_PAWN_SHELTER + i * 32 + j * 8 + k);
				}
				std::cerr << "},\n";
			}
			std::cerr << "},\n";
		}
		std::cerr << "};\n";

		std::cerr << "\nint mg_knight_mobility[9] = { ";
		for (unsigned i = 0; i < 9; i++) print_int_weight(MG_KNIGHT_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "int eg_knight_mobility[9] = { ";
		for (unsigned i = 0; i < 9; i++) print_int_weight(EG_KNIGHT_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "\nint mg_bishop_mobility[14] = { ";
		for (unsigned i = 0; i < 14; i++) print_int_weight(MG_BISHOP_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "int eg_bishop_mobility[14] = { ";
		for (unsigned i = 0; i < 14; i++) print_int_weight(EG_BISHOP_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "\nint mg_rook_mobility[15] = { ";
		for (unsigned i = 0; i < 15; i++) print_int_weight(MG_ROOK_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "int eg_rook_mobility[15] = { ";
		for (unsigned i = 0; i < 15; i++) print_int_weight(EG_ROOK_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "\nint mg_queen_mobility[28] = { ";
		for (unsigned i = 0; i < 28; i++) print_int_weight(MG_QUEEN_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "int eg_queen_mobility[28] = { ";
		for (unsigned i = 0; i < 28; i++) print_int_weight(EG_QUEEN_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "\nint mg_king_mobility[9] = { ";
		for (unsigned i = 0; i < 9; i++) print_int_weight(MG_KING_MOBILITY + i);
		std::cerr << "};\n";
		std::cerr << "int eg_king_mobility[9] = { ";
		for (unsigned i = 0; i < 9; i++) print_int_weight(EG_KING_MOBILITY + i);
		std::cerr << "};\n";

		std::cerr << "\nint mg_minor_threatened_by_pawn = "   << int_weight(MG_MINOR_THREATENED_BY_PAWN) << ";\n";
		std::cerr <<   "int eg_minor_threatened_by_pawn = "   << int_weight(EG_MINOR_THREATENED_BY_PAWN) << ";\n";
		std::cerr << "\nint mg_minor_threatened_by_minor = "  << int_weight(MG_MINOR_THREATENED_BY_MINOR) << ";\n";
		std::cerr <<   "int eg_minor_threatened_by_minor = "  << int_weight(EG_MINOR_THREATENED_BY_MINOR) << ";\n";
		std::cerr << "\nint mg_rook_threatened_by_lesser = "  << int_weight(MG_ROOK_THREATENED_BY_LESSER) << ";\n";
		std::cerr <<   "int eg_rook_threatened_by_lesser = "  << int_weight(EG_ROOK_THREATENED_BY_LESSER) << ";\n";
		std::cerr << "\nint mg_queen_threatened_by_lesser = " << int_weight(MG_QUEEN_THREATENED_BY_LESSER) << ";\n";
		std::cerr <<   "int eg_queen_threatened_by_lesser = " << int_weight(EG_QUEEN_THREATENED_BY_LESSER) << ";\n";

		std::cerr << "\nint mg_passed_pawn[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(MG_PASSED_PAWN + i);
		std::cerr << "};\n";
		std::cerr << "int eg_passed_pawn[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(EG_PASSED_PAWN + i);
		std::cerr << "};\n";
		std::cerr << "int mg_passed_pawn_blocked[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(MG_PASSED_PAWN_BLOCKED + i);
		std::cerr << "};\n";
		std::cerr << "int eg_passed_pawn_blocked[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(EG_PASSED_PAWN_BLOCKED + i);
		std::cerr << "};\n";

		std::cerr << "\nint mg_passed_pawn_safe_advance = " << int_weight(MG_PASSED_SAFE_ADVANCE) << ";\n";
		std::cerr <<   "int eg_passed_pawn_safe_advance = " << int_weight(EG_PASSED_SAFE_ADVANCE) << ";\n";

		std::cerr << "\nint mg_passed_pawn_safe_path = " << int_weight(MG_PASSED_SAFE_PATH) << ";\n";
		std::cerr <<   "int eg_passed_pawn_safe_path = " << int_weight(EG_PASSED_SAFE_PATH) << ";\n";

		std::cerr << "\nint mg_passed_friendly_distance[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(MG_PASSED_FRIENDLY_DISTANCE + i);
		std::cerr << "};\n";
		std::cerr << "int eg_passed_friendly_distance[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(EG_PASSED_FRIENDLY_DISTANCE + i);
		std::cerr << "};\n";
		std::cerr << "int mg_passed_enemy_distance[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(MG_PASSED_ENEMY_DISTANCE + i);
		std::cerr << "};\n";
		std::cerr << "int eg_passed_enemy_distance[8] = { ";
		for (unsigned i = 0; i < 8; i++) print_int_weight(EG_PASSED_ENEMY_DISTANCE + i);
		std::cerr << "};\n";

		std::cerr << "\nint mg_isolated_penalty = " << int_weight(MG_ISOLATED) << ";\n";
		std::cerr <<   "int eg_isolated_penalty = " << int_weight(EG_ISOLATED) << ";\n";

		std::cerr << "\nint mg_doubled_penalty = " << int_weight(MG_DOUBLED) << ";\n";
		std::cerr <<   "int eg_doubled_penalty = " << int_weight(EG_DOUBLED) << ";\n";

		std::cerr << "\nint mg_backward_penalty = " << int_weight(MG_BACKWARD) << ";\n";
		std::cerr <<   "int eg_backward_penalty = " << int_weight(EG_BACKWARD) << ";\n";

		std::cerr << "\nint mg_chained_bonus = " << int_weight(MG_CHAINED) << ";\n";
		std::cerr <<   "int eg_chained_bonus = " << int_weight(EG_CHAINED) << ";\n";

		std::cerr << "\nint mg_double_bishop = " << int_weight(MG_DOUBLE_BISHOP) << ";\n";
		std::cerr <<   "int eg_double_bishop = " << int_weight(EG_DOUBLE_BISHOP) << ";\n";

		std::cerr << "\nint mg_rook_open_file = "      << int_weight(MG_ROOK_OPEN_FILE) << ";\n";
		std::cerr <<   "int eg_rook_open_file = "      << int_weight(EG_ROOK_OPEN_FILE) << ";\n";
		std::cerr << "\nint mg_rook_half_open_file = " << int_weight(MG_ROOK_HALF_OPEN_FILE) << ";\n";
		std::cerr <<   "int eg_rook_half_open_file = " << int_weight(EG_ROOK_HALF_OPEN_FILE) << ";\n";
		
		std::cerr << "\nint mg_rook_on_seventh = " << int_weight(MG_ROOK_ON_SEVENTH) << ";\n";
		std::cerr <<   "int eg_rook_on_seventh = " << int_weight(EG_ROOK_ON_SEVENTH) << ";\n";

		std::cerr << "\nint mg_knight_outpost = "           << int_weight(MG_KNIGHT_OUTPOST) << ";\n";
		std::cerr <<   "int eg_knight_outpost = "	    << int_weight(EG_KNIGHT_OUTPOST) << ";\n";
		std::cerr << "\nint mg_knight_outpost_supported = " << int_weight(MG_KNIGHT_OUTPOST_SUPPORTED) << ";\n";
		std::cerr <<   "int eg_knight_outpost_supported = " << int_weight(EG_KNIGHT_OUTPOST_SUPPORTED) << ";\n";

		std::cerr << "\nint mg_center_control = " << int_weight(CENTER_CONTROL) << ";\n";

		std::cerr << "\nint pawn_count_scale_offset = " << int_weight(PAWN_COUNT_SCALE_OFFSET) << ";\n";
		std::cerr <<   "int pawn_count_scale_weight = " << int_weight(PAWN_COUNT_SCALE_WEIGHT) << ";\n";
	}

	// this functions sets the result of a game; 0 for black win, 0.5 for a draw and 1 for white win
	// the tuner will change all evaluation parameters to best fit this result
	double outcome(std::string &result)
	{
		if (result == "[0.0]") return 0.0;
		else if (result == "[1.0]") return 1.0;
		else return 0.5;
	}

	// we can precompute the number of king attackers and attacks
	void extract_king_safety(Sample &sample, Piece piece, uint64_t attacks, Color enemy)
	{
		unsigned enemy_king_square = board.square(enemy, KING);

		uint64_t ring = king_ring_mask(enemy_king_square);
		uint64_t ring_attacks = attacks & ring;
		if (ring_attacks) {
			sample.ring_attacks[piece] += pop_count(ring_attacks);
			if (sample.ring_attackers[color_of(piece)] < 7) sample.ring_attackers[color_of(piece)]++;
		}
	}

	// stores all evaluation-essential properties of a position
	// (the derivatives of the evaluation in respect to the weights)
	void extract_features(Sample &sample)
	{
		double mg_influences[NUM_WEIGHTS] {};
		double eg_influences[NUM_WEIGHTS] {};
		int taper_start = 6377;
		int taper_end = 321;

		int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
		material = std::max(taper_end, std::min(material, taper_start)); // endgame and midgame limit clamp
		sample.phase = int(((material - taper_end) * 256) / (taper_start - taper_end)); // 0(Endgame) - 256(Midgame) linear interpolation
		sample.phase /= 256;

		double mg_phase = sample.phase;
		double eg_phase = (1 - sample.phase);

		uint64_t passed_pawns[2] {};
		uint64_t attacked[2] {};
		uint64_t attacked_by_piece[2][6] {};
		uint64_t white_pawn_attacks = board.all_pawn_attacks(WHITE);
		uint64_t black_pawn_attacks = board.all_pawn_attacks(BLACK);
		attacked[WHITE] = attacked_by_piece[WHITE][KING] = piece_attacks(KING, board.square(WHITE, KING), 0ULL);
		attacked[BLACK] = attacked_by_piece[BLACK][KING] = piece_attacks(KING, board.square(BLACK, KING), 0ULL);
		attacked[WHITE] |= white_pawn_attacks;
		attacked[BLACK] |= black_pawn_attacks;
		attacked_by_piece[WHITE][PAWN] = white_pawn_attacks;
		attacked_by_piece[BLACK][PAWN] = black_pawn_attacks;

		uint64_t all_pieces = board.occ;
		while (all_pieces) {
			unsigned square = pop_lsb(all_pieces);
			Piece piece = board.board[square];
			Piece_type type = type_of(piece);
			Color friendly = color_of(piece);
			Color enemy = swap(friendly);
			unsigned relative_square = normalize_square[friendly][square];
			int side = (friendly == WHITE) ? 1 : -1;

			// material
			mg_influences[MG_VALUES + type] += side * mg_phase;
			eg_influences[EG_VALUES + type] += side * eg_phase;

			// PSQT
			unsigned mg_index = MG_PAWN_PSQT + type * 64 + relative_square;
			unsigned eg_index = EG_PAWN_PSQT + type * 64 + relative_square;
			mg_influences[mg_index] += side * mg_phase;
			eg_influences[eg_index] += side * eg_phase;

			switch (type) {
			case PAWN:
				{
				uint64_t friendly_pawns = board.pieces(friendly, PAWN);
				uint64_t enemy_pawns = board.pieces(enemy, PAWN);
				unsigned forward = (friendly == WHITE) ? UP : DOWN;
				uint64_t adjacent_files = isolated_pawn_mask(file_num(square));

				bool passed = !(passed_pawn_mask(friendly, square) & enemy_pawns);
				bool doubled = forward_file_mask(friendly, square) & friendly_pawns;
				bool neighbored = neighbor_mask(square) & friendly_pawns;
				bool supported = passed_pawn_mask(enemy, square) & adjacent_files & friendly_pawns;
				bool chained = pawn_attacks(enemy, square) & friendly_pawns;

				// isolated pawns
				if (!(adjacent_files & friendly_pawns)) {
					mg_influences[MG_ISOLATED] += side * mg_phase;
					eg_influences[EG_ISOLATED] += side * eg_phase;
				}

				// doubled pawns
				if (doubled) {
					mg_influences[MG_DOUBLED] += side * mg_phase;
					eg_influences[EG_DOUBLED] += side * eg_phase;
				}

				// backward pawns
				if (!(supported || neighbored) && pawn_attacks(friendly, square + forward) & enemy_pawns) {
					mg_influences[MG_BACKWARD] += side * mg_phase;
					eg_influences[EG_BACKWARD] += side * eg_phase;
				}

				// reward chained pawns
				else if (chained) {
					mg_influences[MG_CHAINED] += side * mg_phase;
					eg_influences[EG_CHAINED] += side * eg_phase;
				}

				// passed pawns
				if (passed && !doubled)
					passed_pawns[friendly] |= 1ULL << square;
				continue;
				}

			// king safety + mobility
			case KNIGHT:
				{
				uint64_t attacks = piece_attacks(KNIGHT, square, 0ULL);
				attacked[friendly] |= attacks;
				attacked_by_piece[friendly][KNIGHT] |= attacks;
				extract_king_safety(sample, piece, attacks, enemy);

				unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
				mg_influences[MG_KNIGHT_MOBILITY + safe_squares] += side * mg_phase;
				eg_influences[EG_KNIGHT_MOBILITY + safe_squares] += side * eg_phase;

				if (1ULL << square & outpost_mask(friendly) &&
				    !(pawn_threat_mask(friendly, square) & board.pieces(enemy, PAWN))) {
					if (pawn_attacks(enemy, square) & board.pieces(friendly, PAWN)) {
						mg_influences[MG_KNIGHT_OUTPOST_SUPPORTED] += side * mg_phase;
						eg_influences[EG_KNIGHT_OUTPOST_SUPPORTED] += side * eg_phase;
					}
					else {
						mg_influences[MG_KNIGHT_OUTPOST] += side * mg_phase;
						eg_influences[EG_KNIGHT_OUTPOST] += side * eg_phase;
					}

				}
				continue;
				}
			case BISHOP:
				{
				uint64_t ray_blockers = board.occ & ~board.pieces(friendly, QUEEN);
				uint64_t attacks = piece_attacks(BISHOP, square, ray_blockers);
				attacked[friendly] |= attacks;
				attacked_by_piece[friendly][BISHOP] |= attacks;
				extract_king_safety(sample, piece, attacks, enemy);

				unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
				mg_influences[MG_BISHOP_MOBILITY + safe_squares] += side * mg_phase;
				eg_influences[EG_BISHOP_MOBILITY + safe_squares] += side * eg_phase;
				continue;
				}
			case ROOK:
				{
				uint64_t ray_blockers = board.occ & ~(board.pieces(friendly, QUEEN) | board.pieces(friendly, ROOK));
				uint64_t attacks = piece_attacks(ROOK, square, ray_blockers);
				attacked[friendly] |= attacks;
				attacked_by_piece[friendly][ROOK] |= attacks;
				extract_king_safety(sample, piece, attacks, enemy);

				unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
				mg_influences[MG_ROOK_MOBILITY + safe_squares] += side * mg_phase;
				eg_influences[EG_ROOK_MOBILITY + safe_squares] += side * eg_phase;

				uint64_t rook_file = file(square);
				if (!(rook_file & board.pieces(friendly, PAWN))) {
					if (!(rook_file & board.pieces(enemy, PAWN))) {
						mg_influences[MG_ROOK_OPEN_FILE] += side * mg_phase;
						eg_influences[EG_ROOK_OPEN_FILE] += side * eg_phase;
					}
					else {
						mg_influences[MG_ROOK_HALF_OPEN_FILE] += side * mg_phase;
						eg_influences[EG_ROOK_HALF_OPEN_FILE] += side * eg_phase;
					}
				}
				if (rank_num(relative_square) == 1 && rank_num(normalize_square[friendly][board.square(enemy, KING)]) <= 1) {
					mg_influences[MG_ROOK_ON_SEVENTH] += side * mg_phase;
					eg_influences[EG_ROOK_ON_SEVENTH] += side * eg_phase;
				}
				continue;
				}
			case QUEEN:
				{
				uint64_t attacks = piece_attacks(QUEEN, square, board.occ);
				attacked[friendly] |= attacks;
				attacked_by_piece[friendly][QUEEN] |= attacks;
				extract_king_safety(sample, piece, attacks, enemy);

				unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
				mg_influences[MG_QUEEN_MOBILITY + safe_squares] += side * mg_phase;
				eg_influences[EG_QUEEN_MOBILITY + safe_squares] += side * eg_phase;
				continue;
				}
			case KING: continue;
			}
		}
		uint64_t kings = board.pieces(KING);
		while (kings) {
			unsigned square = pop_lsb(kings);
			Color friendly = color_of(board.board[square]);
			Color enemy = swap(friendly);
			int side = (friendly == WHITE) ? 1 : -1;

			uint64_t attacks = piece_attacks(KING, square, 0ULL);

			unsigned safe_squares = pop_count(attacks & ~board.all_pawn_attacks(swap(friendly)));
			mg_influences[MG_KING_MOBILITY + safe_squares] += side * mg_phase;
			eg_influences[EG_KING_MOBILITY + safe_squares] += side * eg_phase;

			// Safe checks by enemy pieces
			uint64_t safe = ~attacked[friendly] & ~board.pieces(enemy);
			int knight_checks = pop_count(piece_attacks(KNIGHT, square, 0ULL)      & safe & attacked_by_piece[enemy][KNIGHT]);
			mg_influences[MG_SAFE_KNIGHT_CHECK] += side * knight_checks * mg_phase;
			eg_influences[EG_SAFE_KNIGHT_CHECK] += side * knight_checks * eg_phase;

			int bishop_checks = pop_count(piece_attacks(BISHOP, square, board.occ) & safe & attacked_by_piece[enemy][BISHOP]);
			mg_influences[MG_SAFE_BISHOP_CHECK] += side * bishop_checks * mg_phase;
			eg_influences[EG_SAFE_BISHOP_CHECK] += side * bishop_checks * eg_phase;

			int rook_checks   = pop_count(piece_attacks(ROOK,   square, board.occ) & safe & attacked_by_piece[enemy][ROOK]);
			mg_influences[MG_SAFE_ROOK_CHECK] += side * rook_checks * mg_phase;
			eg_influences[EG_SAFE_ROOK_CHECK] += side * rook_checks * eg_phase;

			int queen_checks  = pop_count(piece_attacks(QUEEN,  square, board.occ) & safe & attacked_by_piece[enemy][QUEEN]);
			mg_influences[MG_SAFE_QUEEN_CHECK] += side * queen_checks * mg_phase;
			eg_influences[EG_SAFE_QUEEN_CHECK] += side * queen_checks * eg_phase;

			// Pawn Shelter
			unsigned shelter_center = std::max(1U, std::min(6U, file_num(square)));
			for (unsigned shelter_file = shelter_center - 1; shelter_file <= shelter_center + 1; shelter_file++) {
				uint64_t shelter_pawns = board.pieces(friendly, PAWN) & file(shelter_file) & (forward_mask(friendly, square) | rank(square));
				if (shelter_pawns) {
					unsigned pawn_square = (friendly == WHITE) ? msb(shelter_pawns) : lsb(shelter_pawns);
					unsigned king_distance = rank_distance(pawn_square, square);
					unsigned edge_distance = std::min(shelter_file, 7 - shelter_file);
					bool king_file_pawn = shelter_file == file_num(square);
					mg_influences[MG_PAWN_SHELTER + king_file_pawn * 32 + edge_distance * 8 + king_distance] += side * mg_phase;
					eg_influences[EG_PAWN_SHELTER + king_file_pawn * 32 + edge_distance * 8 + king_distance] += side * eg_phase;
				}
			}
			/*uint64_t shield_pawns = board.pieces(friendly, PAWN) & pawn_shield(friendly, square);
			while (shield_pawns) {
				unsigned pawn_square = pop_lsb(shield_pawns);
				unsigned shield_square = rank_distance(square, pawn_square) * 2 + file_distance(square, pawn_square);
				mg_influences[PAWN_SHIELD + shield_square] += side * mg_phase;
			}*/
		}

		for (Color friendly : { WHITE, BLACK }) {
			Color enemy = swap(friendly);
			int side = (friendly == WHITE) ? 1 : -1;

			while (passed_pawns[friendly]) {
				unsigned square = pop_lsb(passed_pawns[friendly]);
				unsigned relative_square = normalize_square[friendly][square];
				unsigned relative_rank = rank_num(relative_square);
				uint64_t advance_square = pawn_pushes(friendly, 1ULL << square);
				bool blocked = advance_square & board.occ;
				bool safe_advance = advance_square & ~attacked[enemy];
				bool safe_path = !(passed_pawn_mask(friendly, square) & file(square) & (board.pieces(enemy) | attacked[enemy]));

				if (!blocked) {
					mg_influences[MG_PASSED_PAWN + relative_rank] += side * mg_phase;
					eg_influences[EG_PASSED_PAWN + relative_rank] += side * eg_phase;
				}
				else {
					mg_influences[MG_PASSED_PAWN_BLOCKED + relative_rank] += side * mg_phase;
					eg_influences[EG_PASSED_PAWN_BLOCKED + relative_rank] += side * eg_phase;
				}

				if (safe_advance) {
					mg_influences[MG_PASSED_SAFE_ADVANCE] += side * mg_phase;
					eg_influences[EG_PASSED_SAFE_ADVANCE] += side * eg_phase;
				}

				if (safe_path) {
					mg_influences[MG_PASSED_SAFE_PATH] += side * mg_phase;
					eg_influences[EG_PASSED_SAFE_PATH] += side * eg_phase;
				}

				unsigned friendly_distance = square_distance(square, board.square(friendly, KING));
				mg_influences[MG_PASSED_FRIENDLY_DISTANCE + relative_rank] += side * mg_phase * friendly_distance;
				eg_influences[EG_PASSED_FRIENDLY_DISTANCE + relative_rank] += side * eg_phase * friendly_distance;

				unsigned enemy_distance    = square_distance(square, board.square(enemy,    KING));
				mg_influences[MG_PASSED_ENEMY_DISTANCE + relative_rank] += side * mg_phase * enemy_distance;
				eg_influences[EG_PASSED_ENEMY_DISTANCE + relative_rank] += side * eg_phase * enemy_distance;
			}
		}

		// bishop pair
		if (pop_count(board.pieces(WHITE, BISHOP)) >= 2) {
			mg_influences[MG_DOUBLE_BISHOP] += mg_phase;
			eg_influences[EG_DOUBLE_BISHOP] += eg_phase;
		}
		if (pop_count(board.pieces(BLACK, BISHOP)) >= 2) {
			mg_influences[MG_DOUBLE_BISHOP] -= mg_phase;
			eg_influences[EG_DOUBLE_BISHOP] -= eg_phase;
		}

		// Threats
		uint64_t white_knights = board.pieces(WHITE, KNIGHT);
		uint64_t white_bishops = board.pieces(WHITE, BISHOP);
		uint64_t white_rooks   = board.pieces(WHITE, ROOK);
		uint64_t white_queens  = board.pieces(WHITE, QUEEN);
		uint64_t black_knights = board.pieces(BLACK, KNIGHT);
		uint64_t black_bishops = board.pieces(BLACK, BISHOP);
		uint64_t black_rooks   = board.pieces(BLACK, ROOK);
		uint64_t black_queens  = board.pieces(BLACK, QUEEN);

		uint64_t attacked_by_white_pawn  = attacked_by_piece[WHITE][PAWN];
		uint64_t attacked_by_white_minor = attacked_by_piece[WHITE][KNIGHT] | attacked_by_piece[WHITE][BISHOP];
		uint64_t attacked_by_black_pawn  = attacked_by_piece[BLACK][PAWN];
		uint64_t attacked_by_black_minor = attacked_by_piece[BLACK][KNIGHT] | attacked_by_piece[BLACK][BISHOP];

		// Our minors attacked by enemy pawns
		int white_minors_threatened_by_pawn = pop_count((white_knights | white_bishops) & attacked_by_black_pawn);
		int black_minors_threatened_by_pawn = pop_count((black_knights | black_bishops) & attacked_by_white_pawn);
		mg_influences[MG_MINOR_THREATENED_BY_PAWN] = (white_minors_threatened_by_pawn - black_minors_threatened_by_pawn) * mg_phase;
		eg_influences[EG_MINOR_THREATENED_BY_PAWN] = (white_minors_threatened_by_pawn - black_minors_threatened_by_pawn) * eg_phase;

		// Our minors attacked by enemy minors
		int white_minors_threatened_by_minors = pop_count((white_knights | white_bishops) & attacked_by_black_minor);
		int black_minors_threatened_by_minors = pop_count((black_knights | black_bishops) & attacked_by_white_minor);
		mg_influences[MG_MINOR_THREATENED_BY_MINOR] = (white_minors_threatened_by_minors - black_minors_threatened_by_minors) * mg_phase;
		eg_influences[EG_MINOR_THREATENED_BY_MINOR] = (white_minors_threatened_by_minors - black_minors_threatened_by_minors) * eg_phase;

		// Our rooks attacked by enemy minors or pawns
		int white_rooks_threatened_by_lesser = pop_count(white_rooks & (attacked_by_black_pawn | attacked_by_black_minor));
		int black_rooks_threatened_by_lesser = pop_count(black_rooks & (attacked_by_white_pawn | attacked_by_white_minor));
		mg_influences[MG_ROOK_THREATENED_BY_LESSER] = (white_rooks_threatened_by_lesser - black_rooks_threatened_by_lesser) * mg_phase;
		eg_influences[EG_ROOK_THREATENED_BY_LESSER] = (white_rooks_threatened_by_lesser - black_rooks_threatened_by_lesser) * eg_phase;

		// Our queens attacked by lesser pieces
		int white_queens_threatened_by_lesser = pop_count(white_queens & (attacked[BLACK] & !attacked_by_piece[BLACK][QUEEN]));
		int black_queens_threatened_by_lesser = pop_count(black_queens & (attacked[WHITE] & !attacked_by_piece[WHITE][QUEEN]));
		mg_influences[MG_QUEEN_THREATENED_BY_LESSER] = (white_queens_threatened_by_lesser - black_queens_threatened_by_lesser) * mg_phase;
		eg_influences[EG_QUEEN_THREATENED_BY_LESSER] = (white_queens_threatened_by_lesser - black_queens_threatened_by_lesser) * eg_phase;

		// Center control
		uint64_t white_center_attacks = ~attacked[BLACK] & attacked[WHITE] & CENTER;
		uint64_t black_center_attacks = ~attacked[WHITE] & attacked[BLACK] & CENTER;
		mg_influences[CENTER_CONTROL] = (double(pop_count(white_center_attacks)) - double(pop_count(black_center_attacks))) * mg_phase / 128;

		// To reduce memory usage, all influences that are 0 will not be stored, because they are irrelevent.
		for (unsigned i = 0; i < NUM_WEIGHTS; i++) {
			if (mg_influences[i] != 0) {
				sample.mg_influence.emplace_back(mg_influences[i]);
				sample.mg_influence_index.emplace_back(Indicies(i));
			}
			if (eg_influences[i] != 0) {
				sample.eg_influence.emplace_back(eg_influences[i]);
				sample.eg_influence_index.emplace_back(Indicies(i));
			}
		}
		sample.pawn_count[WHITE] = pop_count(board.pieces(WHITE, PAWN));
		sample.pawn_count[BLACK] = pop_count(board.pieces(BLACK, PAWN));
	}

	void load_training_set()
	{
		std::cerr << "loading training data..." << "\n";
		std::ifstream file;
		file.open("lichess-big3-resolved.book");
		if (file.is_open()) {
			std::string input;
			for (unsigned pos = 0; pos < NUM_TRAINING_POSITIONS + NUM_TEST_POSITIONS; pos++) {

				if (pos % 100000 == 0) std::cerr << "position " << pos << "\n";
				std::string fen;
				std::string result;
				while (true) {
					file >> input;
					if (input == "[0.0]" || input == "[0.5]" || input == "[1.0]") {
						result = input;
						break;
					}
					fen += input + " ";
				}

				// precompute the features of a position to simplify evaluation and allow gradient calculation
				Sample position { outcome(result) };
				board.set_fenpos(fen);
				extract_features(position);

				if (pos <= NUM_TRAINING_POSITIONS)
					training_set.emplace_back(position);
				else
					test_set.emplace_back(position);
			}
		}
	}
	
	// set the weights to reasonable start values
	void default_weights()
	{
		for (double &weight : weights)
			weight = 0;

		double values[6] = { 0, 300, 310, 500, 900, 0 };
		for (unsigned i = 0; i < 6; i++) {
			weights[MG_VALUES + i] = values[i];
			weights[EG_VALUES + i] = values[i];
		}

		double attack_potency[6] = { 0, 10, 30, 30, 80, 0 };
		for (unsigned i = 0; i < 6; i++) {
			weights[MG_KING_RING_POTENCY + i] = attack_potency[i];
			weights[EG_KING_RING_POTENCY + i] = attack_potency[i];
		}
		weights[CENTER_CONTROL] = 300;
	}

	void engine_weights()
	{
		for (unsigned w = 0; w < NUM_WEIGHTS; w++)
			weights[w] = load_weight(w);
	}

	template <Color color>
	double mg_king_ring_pressure(Sample &sample)
	{
		double pressure = 0;
		for (Piece_type type : { KNIGHT, BISHOP, ROOK, QUEEN }) {
			pressure += weights[MG_KING_RING_POTENCY + type] * sample.ring_attacks[piece_of(color, type)];
		}

		return pressure / 100;
	}
	template <Color color>
	double eg_king_ring_pressure(Sample &sample)
	{
		double pressure = 0;
		for (Piece_type type : { KNIGHT, BISHOP, ROOK, QUEEN }) {
			pressure += weights[EG_KING_RING_POTENCY + type] * sample.ring_attacks[piece_of(color, type)];
		}

		return pressure / 100;
	}

	double scale_factor(Sample &sample, double eg_evaluation)
	{
		sample.strong_side = (eg_evaluation > 0) ? WHITE : BLACK;

		return weights[PAWN_COUNT_SCALE_OFFSET] + sample.pawn_count[sample.strong_side] * weights[PAWN_COUNT_SCALE_WEIGHT];
	}

	double evaluate(Sample &sample)
	{
		// all 'normal' evaluation parts can easily be computed

		double mg_evaluation = 0;
		for (unsigned i = 0; i < sample.mg_influence.size(); i++)
			mg_evaluation += sample.mg_influence[i] * weights[sample.mg_influence_index[i]];

		sample.eg_evaluation = 0;
		for (unsigned i = 0; i < sample.eg_influence.size(); i++)
			sample.eg_evaluation += sample.eg_influence[i] * weights[sample.eg_influence_index[i]];

		// king safety is a bit tricky, because two different weights affect each other
		mg_evaluation        += mg_king_ring_pressure<WHITE>(sample) * weights[MG_KING_RING_PRESSURE + sample.ring_attackers[WHITE]] * sample.phase;
		sample.eg_evaluation += eg_king_ring_pressure<WHITE>(sample) * weights[EG_KING_RING_PRESSURE + sample.ring_attackers[WHITE]] * (1 - sample.phase);
		mg_evaluation 	     -= mg_king_ring_pressure<BLACK>(sample) * weights[MG_KING_RING_PRESSURE + sample.ring_attackers[BLACK]] * sample.phase;
		sample.eg_evaluation -= eg_king_ring_pressure<BLACK>(sample) * weights[EG_KING_RING_PRESSURE + sample.ring_attackers[BLACK]] * (1 - sample.phase);

		sample.scale = scale_factor(sample, sample.eg_evaluation / (1 - sample.phase));
		return mg_evaluation + sample.eg_evaluation * sample.scale / 128;
	}

	double sigmoid(double eval)
	{
		return 1 / (1 + std::exp(-SCALING * eval));
	}

	double sigmoid_derivative(double eval)
	{
		double a = std::exp(-SCALING * eval);
		double b = 1 + a;
		return SCALING * a / (b * b);
	}

	double cost(Sample &sample)
	{
		double error = sample.outcome - sigmoid(evaluate(sample));
		return error * error;
	}

	double cost_derivative(Sample &sample, double sigmoid)
	{
		return -2 * (sample.outcome - sigmoid);
	}

	double average_cost(std::vector<Sample> &set)
	{
		double average_error = 0;
		for (Sample sample : set)
			average_error += cost(sample);
		return average_error / set.size();
	}

	// go one step towards the steepest descent
	void apply_gradients()
	{
		for (unsigned i = 0; i < NUM_WEIGHTS; i++) {
			gradients[i] /= BATCH_SIZE;
			sum_squared_gradients[i] += gradients[i] * gradients[i];
			weights[i] -= gradients[i] * (LEARN_RATE / std::sqrt(sum_squared_gradients[i] + TINY_NUMBER));
		}
	}

	// returns the numerical gradients by tweaking the weights a tiny bit and
	// measuring, how the cost function changes
	void approximate_gradients(unsigned batch)
	{
		for (unsigned w = 0; w < NUM_WEIGHTS; w++)
			gradients[w] = 0;

		for (unsigned i = 0; i < BATCH_SIZE; i++) {
			Sample &sample = training_set.at(batch * BATCH_SIZE + i);
			for (unsigned w = 0; w < NUM_WEIGHTS; w++) {
				double &weight = weights[w];

				// how does the cost function change, when we make a slight adjustment to a weight
				weight += TINY_NUMBER;
				double cost_before = cost(sample);

				// what about the other direction?
				weight -= 2 * TINY_NUMBER;
				gradients[w] += (cost_before - cost(sample)) / (2 * TINY_NUMBER);

				// undo adjustment
				weight += TINY_NUMBER;
			}
		}
	}

	// calculates the gradients via partial derivatives
	void compute_gradients(unsigned batch)
	{
		for (unsigned w = 0; w < NUM_WEIGHTS; w++)
			gradients[w] = 0;

		for (unsigned i = 0; i < BATCH_SIZE; i++) {
			Sample &sample = training_set.at(batch * BATCH_SIZE + i);
			double evaluation = evaluate(sample);
			double sigm = sigmoid(evaluation);
			double partial_derivative = cost_derivative(sample, sigm) * sigmoid_derivative(evaluation);

			// Most gradients can easily be computed
			// The influence array holds the derivatives of each parameter in respect to the evaluation function
			// (in this case, it is the game phase multiplied by how often it is used in the position)
			for (unsigned i = 0; i < sample.mg_influence.size(); i++)
				gradients[sample.mg_influence_index[i]] += sample.mg_influence[i] * partial_derivative;
			for (unsigned i = 0; i < sample.eg_influence.size(); i++)
				gradients[sample.eg_influence_index[i]] += sample.eg_influence[i] * sample.scale / 128 * partial_derivative;

			// King safety always needs extra work, because two parameters affect each other

			// king ring pressure weights
			unsigned mg_white_ring_index = MG_KING_RING_PRESSURE + sample.ring_attackers[WHITE];
			gradients[mg_white_ring_index] += mg_king_ring_pressure<WHITE>(sample) * partial_derivative * sample.phase;
			unsigned eg_white_ring_index = EG_KING_RING_PRESSURE + sample.ring_attackers[WHITE];
			gradients[eg_white_ring_index] += eg_king_ring_pressure<WHITE>(sample) * partial_derivative * (1 - sample.phase) * sample.scale / 128;

			unsigned mg_black_ring_index = MG_KING_RING_PRESSURE + sample.ring_attackers[BLACK];
			gradients[mg_black_ring_index] -= mg_king_ring_pressure<BLACK>(sample) * partial_derivative * sample.phase;
			unsigned eg_black_ring_index = EG_KING_RING_PRESSURE + sample.ring_attackers[BLACK];
			gradients[eg_black_ring_index] -= eg_king_ring_pressure<BLACK>(sample) * partial_derivative * (1 - sample.phase) * sample.scale / 128;

			for (Piece_type type : { KNIGHT, BISHOP, ROOK, QUEEN }) {
				// Ring attack potency
				double white_ring_attacks = sample.ring_attacks[piece_of(WHITE, type)];
				gradients[MG_KING_RING_POTENCY + type] += white_ring_attacks * weights[mg_white_ring_index] / 100 * partial_derivative * sample.phase;
				gradients[EG_KING_RING_POTENCY + type] += white_ring_attacks * weights[eg_white_ring_index] / 100 *
					partial_derivative * (1 - sample.phase) * sample.scale / 128;
				double black_ring_attacks = sample.ring_attacks[piece_of(BLACK, type)];
				gradients[MG_KING_RING_POTENCY + type] -= black_ring_attacks * weights[mg_black_ring_index] / 100 * partial_derivative * sample.phase;
				gradients[EG_KING_RING_POTENCY + type] -= black_ring_attacks * weights[eg_black_ring_index] / 100 * 
					partial_derivative * (1 - sample.phase) * sample.scale / 128;
			}
			// Endgame scaling
			gradients[PAWN_COUNT_SCALE_OFFSET] += sample.eg_evaluation / 128 * partial_derivative;
			gradients[PAWN_COUNT_SCALE_WEIGHT] += sample.pawn_count[sample.strong_side] * sample.eg_evaluation / 128 * partial_derivative;
		}
	}

	// Main function of the tuner
	void tune()
	{
		print_weights();
		//double best_error = average_cost(test_set);
		//std::cerr << "\ntest error " << best_error << "\n";

		for (unsigned iteration = 0; iteration < 700; iteration++) {
			std::cerr << "iteration " << iteration << "\n";
			for (unsigned batch = 0; batch < NUM_TRAINING_POSITIONS / BATCH_SIZE; batch++) {
				compute_gradients(batch);
				apply_gradients();
			}
			//double test_error = average_cost(test_set);
			//if (test_error < best_error - 0.0000001) {
			//	best_error = test_error;
			//	print_weights();
			//	std::cerr << "\ntest error " << test_error << "\n";
			//}
			//else break;
			print_weights();
		}
		std::cerr << "\ntraining error " << average_cost(training_set) << "\n";
		std::cerr << "\ntest error " << average_cost(test_set) << "\n";
	}

	Tuner()
	{
		std::cerr << NUM_WEIGHTS << " weights will be tuned\n";
		load_training_set();
	}
};
