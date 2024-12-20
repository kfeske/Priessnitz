#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include <utility.h>
#include <board.h>
#include <evaluation.h>
#include <trace.h>

enum Parameter_type {
	NORMAL, KING_DANGER
};

struct Term
{
	std::string name;
	unsigned size;
	unsigned print_type;
};

struct Parameter
{
	Parameter_type type;

	unsigned index;

	// The term, the parameter is part of (e.g., piece_value[KNIGHT] is part of piece_value)
	unsigned term_index;

	// Pointers to trace().
	// Holds the derivatives of the parameter in respect to the evaluation.
	// (For example if there are 5 white pawns, the derivative of the pawn value is 5 for white,
	//  because in the evaluation function, we basically have "pawn count * pawn value + knight_count * knight_value + ...").
	unsigned *white_influence {};
	unsigned *black_influence {};

	// Pointers to the evaluation terms.
	int *eval {};

	// The current value for the parameter, which is changed by the tuner.
	double mg_value {};
	double eg_value {};

	// The gradients for gradient descent.
	double mg_gradient {};
	double eg_gradient {};

	// Needed for Adagrad
	//double mg_sum_squared_gradients {};
	//double eg_sum_squared_gradients {};
	
	// Needed for Adam
	double mg_momentum {};
	double eg_momentum {};
	double mg_velocity {};
	double eg_velocity {};
};

struct Parameters
{
	std::vector<Parameter> list {};
	std::vector<Term> terms {};

	int white_influence(unsigned index) { return *list[index].white_influence; }
	int black_influence(unsigned index) { return *list[index].black_influence; }

	int current_mg_eval(unsigned index) { return mg_score(*list[index].eval); }
	int current_eg_eval(unsigned index) { return eg_score(*list[index].eval); }

	double &mg_value(unsigned index) { return list[index].mg_value; }
	double &eg_value(unsigned index) { return list[index].eg_value; }

	double &mg_gradient(unsigned index) { return list[index].mg_gradient; }
	double &eg_gradient(unsigned index) { return list[index].eg_gradient; }

	void _add(Parameter_type type, std::string name, unsigned size, unsigned print_type, unsigned *influence, int *eval)
	{
		unsigned term_index = terms.size();
		terms.emplace_back(Term { name, size, print_type });
		unsigned *white_influence = &influence[0];
		unsigned *black_influence = &influence[size];
		for (unsigned i = 0; i < size; i++) {
			unsigned index = list.size();
			list.emplace_back(Parameter { type, index, term_index, &white_influence[i], &black_influence[i], &eval[i] });
		}
	}

	void print_row(Term &term, unsigned index)
	{
		std::cerr << "int " << term.name;
		for (unsigned i = index; i < index + term.size; i++) {
			std::cerr << "S(" << std::round(mg_value(i)) <<
				     ", " << std::round(eg_value(i)) << "), ";
		}
		std::cerr << "};\n\n";
	}

	void print_field(Term &term, unsigned index)
	{
		std::cerr << "int " << term.name;
		for (unsigned i = 0; i < term.size; i++) {
			if (i % 8 == 0) std::cerr << "\n        ";
			std::cerr << "S(" << std::round(mg_value(i + index)) <<
				     ", " << std::round(eg_value(i + index)) << "), ";
		}
		std::cerr << "\n};\n\n";
	}

	void print_3d(Term &term, unsigned index, unsigned x, unsigned y, unsigned z)
	{
		std::cerr << "int " << term.name << "\n";
		for (unsigned i = 0; i < x; i++) {
			std::cerr << "{\n";
			for (unsigned j = 0; j < y; j++) {
				std::cerr << "        { ";
				for (unsigned k = 0; k < z; k++) {
					unsigned position = index + i * (y * z) + j * z + k;
					std::cerr << "S(" << std::round(list[position].mg_value) <<
						     ", " << std::round(list[position].eg_value) << "), ";
				}
				std::cerr << "}, \n";
			}
			std::cerr << "},\n";
		}
		std::cerr << "};\n\n";
	}

	void print()
	{
		for (unsigned i = 0; i < list.size();) {
			Parameter &parameter = list[i];
			Term &term = terms[parameter.term_index];
			switch (term.print_type) {
			case 0:
				std::cerr << "int " << term.name << "S(" << std::round(parameter.mg_value) <<
					", " << std::round(parameter.eg_value) << ");\n\n";
				break;
			case 1:
				print_row(term, i);
				break;
			case 2:
				print_field(term, i);
				break;
			case 3:
				print_3d(term, i, 2, 4, 8);
				break;
			}

			i += term.size;
		}
	}

	Parameters(Evaluation &eval)
	{
		Trace &t = trace();
		
		_add(NORMAL, "piece_value[6] = { ", 6, 1, &t.piece_value[0][0], &eval.piece_value[0]);

		_add(NORMAL, "pawn_psqt[64] = {",   64, 2, &t.pawn_psqt[0][0],   &eval.pawn_psqt[0]);
		_add(NORMAL, "knight_psqt[64] = {", 64, 2, &t.knight_psqt[0][0], &eval.knight_psqt[0]);
		_add(NORMAL, "bishop_psqt[64] = {", 64, 2, &t.bishop_psqt[0][0], &eval.bishop_psqt[0]);
		_add(NORMAL, "rook_psqt[64] = {",   64, 2, &t.rook_psqt[0][0],   &eval.rook_psqt[0]);
		_add(NORMAL, "queen_psqt[64] = {",  64, 2, &t.queen_psqt[0][0],  &eval.queen_psqt[0]);
		_add(NORMAL, "king_psqt[64] = {",   64, 2, &t.king_psqt[0][0],   &eval.king_psqt[0]);

		_add(NORMAL, "knight_mobility[9] = { ", 9,  1,  &t.knight_mobility[0][0], &eval.knight_mobility[0]);
		_add(NORMAL, "bishop_mobility[14] = { ", 14, 1, &t.bishop_mobility[0][0], &eval.bishop_mobility[0]);
		_add(NORMAL, "rook_mobility[15] = { ",   15, 1, &t.rook_mobility[0][0],   &eval.rook_mobility[0]);
		_add(NORMAL, "queen_mobility[28] = { ",  28, 1, &t.queen_mobility[0][0],  &eval.queen_mobility[0]);
		_add(NORMAL, "king_mobility[9] = { ",  9, 1,    &t.king_mobility[0][0],   &eval.king_mobility[0]);

		_add(NORMAL, "isolated_pawn = ",                1, 0, &t.isolated_pawn[0],              &eval.isolated_pawn);
		_add(NORMAL, "doubled_pawn = ",                 1, 0, &t.doubled_pawn[0],               &eval.doubled_pawn);
		_add(NORMAL, "backward_pawn[8] = { ",           8, 1, &t.backward_pawn[0][0],           &eval.backward_pawn[0]);
		_add(NORMAL, "backward_pawn_half_open[8] = { ", 8, 1, &t.backward_pawn_half_open[0][0], &eval.backward_pawn_half_open[0]);
		_add(NORMAL, "chained_pawn[8] = { ",            8, 1, &t.chained_pawn[0][0],            &eval.chained_pawn[0]);
		_add(NORMAL, "phalanx_pawn[8] = { ",            8, 1, &t.phalanx_pawn[0][0],            &eval.phalanx_pawn[0]);

		_add(NORMAL, "passed_pawn[8] = { ",              8, 1, &t.passed_pawn[0][0],              &eval.passed_pawn[0]);
		_add(NORMAL, "passed_pawn_blocked[8] = { ",      8, 1, &t.passed_pawn_blocked[0][0],      &eval.passed_pawn_blocked[0]);
		_add(NORMAL, "passed_pawn_safe_advance = ",      1, 0, &t.passed_pawn_safe_advance[0],    &eval.passed_pawn_safe_advance);
		_add(NORMAL, "passed_pawn_safe_path = ",         1, 0, &t.passed_pawn_safe_path[0],       &eval.passed_pawn_safe_path);
		_add(NORMAL, "passed_friendly_distance[8] = { ", 8, 1, &t.passed_friendly_distance[0][0], &eval.passed_friendly_distance[0]);
		_add(NORMAL, "passed_enemy_distance[8] = { ",    8, 1, &t.passed_enemy_distance[0][0],    &eval.passed_enemy_distance[0]);

		_add(NORMAL, "knight_outpost = ",           1, 0, &t.knight_outpost[0],           &eval.knight_outpost);
		_add(NORMAL, "knight_outpost_supported = ", 1, 0, &t.knight_outpost_supported[0], &eval.knight_outpost_supported);

		_add(NORMAL, "bishop_pawn = ",   1, 0, &t.bishop_pawn[0],   &eval.bishop_pawn);
		_add(NORMAL, "double_bishop = ", 1, 0, &t.double_bishop[0], &eval.double_bishop);

		_add(NORMAL, "rook_open_file = ",      1, 0, &t.rook_open_file[0],      &eval.rook_open_file);
		_add(NORMAL, "rook_half_open_file = ", 1, 0, &t.rook_half_open_file[0], &eval.rook_half_open_file);
		_add(NORMAL, "rook_on_seventh = ",     1, 0, &t.rook_on_seventh[0],     &eval.rook_on_seventh);

		_add(NORMAL, "pawn_shelter[2][4][8] = { ", 64, 3, &t.pawn_shelter[0][0][0][0], &eval.pawn_shelter[0][0][0]);
		_add(NORMAL, "pawn_storm[2][4][8] = { ",   64, 3, &t.pawn_storm[0][0][0][0],   &eval.pawn_storm[0][0][0]);

		_add(KING_DANGER, "king_attacker_weight[6] = { ", 6, 1, &t.king_attacker_weight[0][0], &eval.king_attacker_weight[0]);
		_add(KING_DANGER, "king_zone_attack_count_weight = ", 1, 0, &t.king_zone_attack_count_weight[0], &eval.king_zone_attack_count_weight);
		_add(KING_DANGER, "king_zone_weak_square = ",  1, 0, &t.king_zone_weak_square[0],  &eval.king_zone_weak_square);
		_add(KING_DANGER, "safe_knight_check = ", 1, 0, &t.safe_knight_check[0], &eval.safe_knight_check);
		_add(KING_DANGER, "safe_bishop_check = ", 1, 0, &t.safe_bishop_check[0], &eval.safe_bishop_check);
		_add(KING_DANGER, "safe_rook_check = ",   1, 0, &t.safe_rook_check[0],   &eval.safe_rook_check);
		_add(KING_DANGER, "safe_queen_check = ",  1, 0, &t.safe_queen_check[0],  &eval.safe_queen_check);
		_add(KING_DANGER, "unsafe_check = ", 1, 0, &t.unsafe_check[0], &eval.unsafe_check);
		_add(KING_DANGER, "pawn_shelter_king_danger[2][4][8] = { ", 64, 3, &t.pawn_shelter_king_danger[0][0][0][0], &eval.pawn_shelter_king_danger[0][0][0]);
		_add(KING_DANGER, "pawn_storm_king_danger[2][4][8] = { ",   64, 3, &t.pawn_storm_king_danger[0][0][0][0],   &eval.pawn_storm_king_danger[0][0][0]);
		_add(KING_DANGER, "king_danger_no_queen_weight = ", 1, 0, &t.king_danger_no_queen_weight[0], &eval.king_danger_no_queen_weight);
		_add(KING_DANGER, "king_danger_offset = ", 1, 0, &t.king_danger_offset[0], &eval.king_danger_offset);

		_add(NORMAL, "center_control = ", 1, 0, &t.center_control[0], &eval.center_control);

		_add(NORMAL, "minor_threatened_by_pawn = ",   1, 0, &t.minor_threatened_by_pawn[0],   &eval.minor_threatened_by_pawn);
		_add(NORMAL, "minor_threatened_by_minor = ",  1, 0, &t.minor_threatened_by_minor[0],  &eval.minor_threatened_by_minor);
		_add(NORMAL, "rook_threatened_by_lesser = ",  1, 0, &t.rook_threatened_by_lesser[0],  &eval.rook_threatened_by_lesser);
		_add(NORMAL, "queen_threatened_by_lesser = ", 1, 0, &t.queen_threatened_by_lesser[0], &eval.queen_threatened_by_lesser);
		_add(NORMAL, "minor_threatened_by_major = ",  1, 0, &t.minor_threatened_by_major[0],  &eval.minor_threatened_by_major);
		_add(NORMAL, "pawn_push_threat = ",           1, 0, &t.pawn_push_threat[0],           &eval.pawn_push_threat);
	}
};

// A single position and the game result
struct Sample
{
	double outcome;

	// Stores, how much a weight influences the evaluation of a position
	uint8_t normal_influence_length = 0;
	int8_t *normal_influence {};
	uint16_t *normal_influence_index {};

	unsigned king_danger_influence_length = 0;
	unsigned *king_danger_white_influence {};
	unsigned *king_danger_black_influence {};
	unsigned *king_danger_influence_index {};

	// Midgame / Endgame gamephase of the position
	double phase = 0;

	// Drawish endgame scaling
	double scale_factor = 0;

	// Current evaluation of the position from the evaluation function of the engine
	int evaluation = 0;
};

enum Tuning_params {
	//NUM_TRAINING_POSITIONS = 100000,
	//NUM_TEST_POSITIONS = 100000,
	//NUM_TRAINING_POSITIONS = 9000000,
	//NUM_TEST_POSITIONS = 996883,
	NUM_TRAINING_POSITIONS = 6600000,
	NUM_TEST_POSITIONS = 553653,
	BATCH_SIZE = 1000
};

struct Tuner
{
	double SCALING = 3.45387764 / 400; // Scaling constant for our evaluation function 
					   // (basically to transform centipawns to win/loss/draw probability). Used in sigmoid function
	double const TINY_NUMBER = 0.00001;
	double const LEARN_RATE = 0.001; // Step size for the optimizer.

	std::string TRAINING_DATA_PATH = "lichess-big3-resolved.book";

	Board board {};
	Evaluation eval {};

	Parameters parameters { eval };
	std::vector<Sample> training_set {};
	std::vector<Sample> test_set {};

	// this functions sets the result of a game; 0 for black win, 0.5 for a draw and 1 for white win
	// the tuner will change all evaluation parameters to best fit this result
	double outcome(std::string &result)
	{
		if (result == "[0.0]") return 0.0;
		else if (result == "[1.0]") return 1.0;
		else return 0.5;
	}

	void extract_features(Sample &);

	void load_training_set();

	void default_weights();

	void engine_weights();

	double evaluate(Sample &sample);

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

	double cost(Sample &sample);

	double cost_derivative(Sample &sample, double sigmoid);

	double average_cost(std::vector<Sample> &set);

	double engine_evaluation_error();

	void find_optimal_scaling();

	void apply_gradients();

	void approximate_gradients(std::vector<Sample> &set, unsigned batch);

	void compute_gradients(std::vector<Sample> &set, unsigned batch);

	void tune();
};
