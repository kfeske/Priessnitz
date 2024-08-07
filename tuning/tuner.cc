#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include "tuning_board.h"
//#include <evaluation.h>
#include "tuning_evaluation.h"

enum INDICES {
	MG_PAWN_PSQT = 0,
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
	EG_PASSED_PAWN = MG_PASSED_PAWN + 64,
	MG_ISOLATED = EG_PASSED_PAWN + 64,
	EG_ISOLATED = MG_ISOLATED + 1,
	//AVERAGE_MOBILITY = EG_ISOLATED + 1,
	//MG_MOBILITY = AVERAGE_MOBILITY + 6,
	//EG_MOBILITY = MG_MOBILITY + 6,
	ATTACK_POTENCY = EG_ISOLATED + 1,
	KING_DANGER = ATTACK_POTENCY + 6,
	MG_VALUES = KING_DANGER + 8,
	EG_VALUES = MG_VALUES + 6,
	TAPER_START = EG_VALUES + 6,
	TAPER_END = TAPER_START + 1,
	END_INDEX = TAPER_END + 1
};

enum TUNING_PARAMETER {
	NUM_TRAINING_POSITIONS = 700000, // number of training positions
	NUM_TEST_POSITIONS = 25000,      // number of test positions
	NUM_TABLES = 22,		 // number of tables to be tuned (eg. pawn piece square table)
	NUM_WEIGHTS = END_INDEX,         // values to be tuned
	BATCH_SIZE = 1000	         // how much the training set is split for computational efficiency
};

double random_double()
{
	return rand() / (RAND_MAX + 1.0);
}

double random_double(double min, double max)
{
	return min + (max-min)*random_double();
}

struct Sample
{
	std::string fen;
	double outcome;
};

struct Tuner
{
	INDICES table[NUM_TABLES + 1] = { MG_PAWN_PSQT, MG_KNIGHT_PSQT, MG_BISHOP_PSQT, MG_ROOK_PSQT, MG_QUEEN_PSQT, MG_KING_PSQT,
	       				  EG_PAWN_PSQT, EG_KNIGHT_PSQT, EG_BISHOP_PSQT, EG_ROOK_PSQT, EG_QUEEN_PSQT, EG_KING_PSQT,
			     	      	  MG_PASSED_PAWN, EG_PASSED_PAWN, MG_ISOLATED, EG_ISOLATED,
			     	      	  //AVERAGE_MOBILITY, MG_MOBILITY, EG_MOBILITY,
			     	      	  ATTACK_POTENCY, KING_DANGER,
					  MG_VALUES, EG_VALUES,
					  TAPER_START, TAPER_END,
			     	      	  END_INDEX };

	double const K = 1.5; // scaling constant for our evaluation function
	double const H = 0.001;   // difference quotient step size
	double const LEARN_RATE = 100;

	Board board {};
	Evaluation eval {}; // to get the values from the original function

	std::vector<Sample> training_set {};
	std::vector<Sample> test_set {};
	double gradients[NUM_WEIGHTS];

	// returns the value in a table at a position
	double &table_value(INDICES table_index, unsigned pos)
	{
		switch (table_index) {
		case MG_PAWN_PSQT: return eval.psqt.midgame[W_PAWN][pos];
		case MG_KNIGHT_PSQT: return eval.psqt.midgame[W_KNIGHT][pos];
		case MG_BISHOP_PSQT: return eval.psqt.midgame[W_BISHOP][pos];
		case MG_ROOK_PSQT: return eval.psqt.midgame[W_ROOK][pos];
		case MG_QUEEN_PSQT: return eval.psqt.midgame[W_QUEEN][pos];
		case MG_KING_PSQT: return eval.psqt.midgame[W_KING][pos];
		case EG_PAWN_PSQT: return eval.psqt.endgame[W_PAWN][pos];
		case EG_KNIGHT_PSQT: return eval.psqt.endgame[W_KNIGHT][pos];
		case EG_BISHOP_PSQT: return eval.psqt.endgame[W_BISHOP][pos];
		case EG_ROOK_PSQT: return eval.psqt.endgame[W_ROOK][pos];
		case EG_QUEEN_PSQT: return eval.psqt.endgame[W_QUEEN][pos];
		case EG_KING_PSQT: return eval.psqt.endgame[W_KING][pos];
		case MG_PASSED_PAWN: return eval.mg_passed_bonus[pos];
		case EG_PASSED_PAWN: return eval.eg_passed_bonus[pos];
		case MG_ISOLATED: return eval.mg_isolated_penalty;
		case EG_ISOLATED: return eval.eg_isolated_penalty;
		//case AVERAGE_MOBILITY: return eval.average_mobility[pos];
		//case MG_MOBILITY: return eval.mg_mobility_weight[pos];
		//case EG_MOBILITY: return eval.eg_mobility_weight[pos];
		case ATTACK_POTENCY: return eval.attack_potency[pos];
		case KING_DANGER: return eval.king_danger_weight[pos];
		case MG_VALUES: return eval.mg_piece_value[pos];
		case EG_VALUES: return eval.eg_piece_value[pos];
		case TAPER_START: return eval.taper_start;
		case TAPER_END: return eval.taper_end;
		default: return eval.eg_passed_bonus[pos];
		}
	}

	// returns the table described with the index. (see table[] array)
	INDICES table_of(unsigned index)
	{
		for (unsigned i = 0; i < NUM_TABLES; i++) {
			if (index >= table[i] && index < table[i + 1])
				return table[i];
		}
		std::cerr << "error: index does not have a table";
		return END_INDEX;
	}

	// returns the weight at an index from the evaluation function
	double &load_weight(unsigned index)
	{
		INDICES table_index = table_of(index);
		return table_value(table_index, index - table_index);
	}

	void print_table_name(INDICES table_index)
	{
		switch (table_index) {
		case MG_PAWN_PSQT: std::cerr << "\nmg pawn psqt\n"; break;
		case MG_KNIGHT_PSQT: std::cerr << "\nmg knight psqt\n"; break;
		case MG_BISHOP_PSQT: std::cerr << "\nmg bishop psqt\n"; break;
		case MG_ROOK_PSQT: std::cerr << "\nmg rook psqt\n"; break;
		case MG_QUEEN_PSQT: std::cerr << "\nmg queen psqt\n"; break;
		case MG_KING_PSQT: std::cerr << "\nmg king psqt\n"; break;
		case EG_PAWN_PSQT: std::cerr << "\neg pawn psqt\n"; break;
		case EG_KNIGHT_PSQT: std::cerr << "\neg knight psqt\n"; break;
		case EG_BISHOP_PSQT: std::cerr << "\neg bishop psqt\n"; break;
		case EG_ROOK_PSQT: std::cerr << "\neg rook psqt\n"; break;
		case EG_QUEEN_PSQT: std::cerr << "\neg queen psqt\n"; break;
		case EG_KING_PSQT: std::cerr << "\neg king psqt\n"; break;
		case MG_PASSED_PAWN: std::cerr << "\nmg passed pawn\n"; break;
		case EG_PASSED_PAWN: std::cerr << "\neg passed pawn\n"; break;
		case MG_ISOLATED: std::cerr << "\nmg isolated pawn\n"; break;
		case EG_ISOLATED: std::cerr << "\neg isolated pawn\n"; break;
		//case AVERAGE_MOBILITY: std::cerr << "\naverage mobility\n"; break;
		//case MG_MOBILITY: std::cerr << "\nmg mobility\n"; break;
		//case EG_MOBILITY: std::cerr << "\neg mobility\n"; break;
		case ATTACK_POTENCY: std::cerr << "\nattack potency\n"; break;
		case KING_DANGER: std::cerr << "\nking danger\n"; break;
		case MG_VALUES: std::cerr << "\nmg values\n"; break;
		case EG_VALUES: std::cerr << "\neg values\n"; break;
		case TAPER_START: std::cerr << "\ntaper start\n"; break;
		case TAPER_END: std::cerr << "\ntaper end\n"; break;
		default: return;
		}
	}

	void print_weights()
	{
		INDICES previous_table = END_INDEX;
		std::cerr << "\ncurrent weights:\n";
		for (unsigned i = 0; i < NUM_WEIGHTS; i++) {

			INDICES table = table_of(i);
			if (table != previous_table) {
				previous_table = table;
				print_table_name(table);
			}

			std::cerr << int(load_weight(i)) << ", ";
			if (table <= EG_PASSED_PAWN && (i + 1) % 8 == 0) std::cerr << "\n";
		}

		std::cerr << "\ntraining error " << average_cost(training_set) << "\n";
		std::cerr << "test error " << average_cost(test_set) << "\n";
	}

	double outcome(std::string &result)
	{
		if (result == "\"0-1\";") return 0.0;
		else if (result == "\"1-0\";") return 1.0;
		else return 0.5;
	}

	void load_training_set()
	{
		std::ifstream file;
		file.open("training_set.epd");
		if (file.is_open()) {
			std::string input;
			for (unsigned pos = 0; pos < NUM_TRAINING_POSITIONS + NUM_TEST_POSITIONS; pos++) {

				std::string fen;
				std::string result;
				while (true) {
					file >> input;
					if (input == "c9") break;
					fen += input + " ";
				}
				file >> result;

				if (pos <= NUM_TRAINING_POSITIONS) training_set.emplace_back(Sample { fen, outcome(result) });
				else test_set.emplace_back(Sample { fen, outcome(result) });
			}
		}
	}
	
	void initialize_weights()
	{
		for (unsigned i = 0; i < NUM_WEIGHTS; i++) {
			double &weight = load_weight(i);
			//weight = random_double(0, 100);
			if (table_of(i) != ATTACK_POTENCY && table_of(i) != TAPER_START && table_of(i) != TAPER_END) weight = 0;
		}
	}

	double white_evaluation()
	{
		return (board.side_to_move == WHITE) ? eval.evaluate(board) : -eval.evaluate(board);
	}

	double sigmoid(double score)
	{
		return 1 / (1 + std::pow(10, -K * score / 400));
	}

	double cost(Sample &sample)
	{
		double error = sample.outcome - sigmoid(white_evaluation());
		return error * error;
	}

	double average_cost(std::vector<Sample> &set)
	{
		double average_error = 0;
		for (Sample sample : set) {
			board.set_fenpos(sample.fen);
			average_error += cost(sample);
		}
		return average_error / set.size();
	}

	double average_batch_cost(unsigned batch)
	{
		double average_error = 0;
		for (unsigned i = 0; i < BATCH_SIZE; i++) {
			Sample &sample = training_set.at(batch * BATCH_SIZE + i);
			board.set_fenpos(sample.fen);
			average_error += cost(sample);
		}
		return average_error / BATCH_SIZE;
	}

	void apply_gradients()
	{
		for (unsigned i = 0; i < NUM_WEIGHTS; i++) {
			double &weight = load_weight(i);
			// increase or decrease weight by one, based on how good it fared in the training set
			weight -= gradients[i] * LEARN_RATE;
		}
	}

	void compute_gradients(unsigned batch)
	{
		//double cost_before[NUM_WEIGHTS];
		//double cost_after[NUM_WEIGHTS];

		for (unsigned w = 0; w < NUM_WEIGHTS; w++)
			gradients[w] = 0;

		for (unsigned i = 0; i < BATCH_SIZE; i++) {
			Sample &sample = training_set.at(batch * BATCH_SIZE + i);
			board.set_fenpos(sample.fen);
			for (unsigned w = 0; w < NUM_WEIGHTS; w++) {
				double &weight = load_weight(w);

				// how does the cost function change, when we make a slight adjustment to a weight
				weight += H;
				//cost_before[w] += cost(sample);
				double cost_before = cost(sample);
				weight -= 2 * H;
				gradients[w] += (cost_before - cost(sample)) / (2 * H);
				//cost_after[w] += cost(sample);
				weight += H;
			}
		}


		/*
		for (unsigned w = 0; w < NUM_WEIGHTS; w++) {
			cost_before[w] /= BATCH_SIZE;
			cost_after[w] /= BATCH_SIZE;
			// calculate gradient with difference quotient
			gradients[w] = (cost_before[w] - cost_after[w]) / (2 * H);
		}*/
	}

	// main function of the tuner
	void tune()
	{
		unsigned iteration = 0;
		double best_error = 1;
		while (true) {
			iteration++;
			std::cerr << "iteration " << iteration << "\n";
			for (unsigned batch = 0; batch < NUM_TRAINING_POSITIONS / BATCH_SIZE; batch++) {
				//std::cerr << "batch " << batch << " / " << BATCH_SIZE << "\n";
				compute_gradients(batch);
				apply_gradients();
				double test_error = average_cost(test_set);
				if (test_error < best_error) {
					best_error = test_error;
					print_weights();
				}
			}
		}
	}

	Tuner()
	{
		load_training_set();
		initialize_weights();
		print_weights();
	}
};

int main()
{
	static Tuner tuner {};
	tuner.tune();
	return 1;
}
