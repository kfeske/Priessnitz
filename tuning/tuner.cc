// the task of the tuner is to optimize all the values in the evaluation function, e.g., the
// value of a pawn. This is done by a gradient descent algorithm, which adjusts the values
// in a way that the evaluation of every position in the training set predicts the outcome
// of the game, the position is from as best as possible.

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include <tuner.h>
#include <trace.h>
#include <board.h>
#include <evaluation.h>

void Tuner::extract_features(Sample &sample)
{
	trace() = {};
	int evaluation = eval.evaluate(board);

	sample.evaluation   = board.side_to_move == WHITE ? evaluation : -evaluation;
	sample.phase        = trace().phase / 256.0;
	sample.scale_factor = trace().scale_factor / 128.0;

	// To reduce memory usage, all influences that have no effect or cancel each other out
	// will not be stored.
	for (unsigned i = 0; i < parameters.list.size(); i++) {
		Parameter &parameter = parameters.list[i];
		// If a normal parameter has no effect on the evaluation, it is irrelevant.
		// This can also happen, when white and black have the same influence and cancel each other out.
		// (For instance, we have the pawn value and white and black have the same amount of pawns)
		if (parameter.type == NORMAL && parameters.white_influence(i) - parameters.black_influence(i) != 0)
			sample.normal_influence_length++;

		// If a king danger parameter has no effect on the evaluation for this position for both white and black, it is irrelevant.
		else if (parameter.type == KING_DANGER && !(parameters.white_influence(i) == 0 && parameters.black_influence(i) == 0))
			sample.king_danger_influence_length++;
	}

	sample.normal_influence            = new int8_t[     sample.normal_influence_length];
	sample.king_danger_white_influence = new unsigned[sample.king_danger_influence_length];
	sample.king_danger_black_influence = new unsigned[sample.king_danger_influence_length];

	sample.normal_influence_index      = new uint16_t[sample.normal_influence_length];
	sample.king_danger_influence_index = new unsigned[sample.king_danger_influence_length];

	unsigned normal_count = 0;
	unsigned king_danger_count = 0;
	for (unsigned i = 0; i < parameters.list.size(); i++) {
		Parameter &parameter = parameters.list[i];
		if (parameter.type == NORMAL && parameters.white_influence(i) - parameters.black_influence(i) != 0) {
			sample.normal_influence[normal_count] = parameters.white_influence(i) - parameters.black_influence(i);
			sample.normal_influence_index[normal_count] = i;
			normal_count++;
		}
		else if (parameter.type == KING_DANGER && !(parameters.white_influence(i) == 0 && parameters.black_influence(i) == 0)) {
			sample.king_danger_white_influence[king_danger_count] = parameters.white_influence(i);
			sample.king_danger_black_influence[king_danger_count] = parameters.black_influence(i);
			sample.king_danger_influence_index[king_danger_count] = i;
			king_danger_count++;
		}
	}
}

void Tuner::load_training_set()
{
	std::cerr << "loading training data..." << "\n";
	std::ifstream file;
	file.open(TRAINING_DATA_PATH);
	if (file.is_open()) {
		std::string input;
		for (unsigned pos = 0; pos < NUM_TRAINING_POSITIONS + NUM_TEST_POSITIONS; pos++) {

			if (pos % 100000 == 0) std::cerr << "position " << pos << "\n";
			std::string fen {};
			std::string result {};
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
void Tuner::default_weights()
{
	for (Parameter &parameter : parameters.list) {
		if (parameters.terms[parameter.term_index].name == "piece_value[6] = { ") continue;
		if (parameters.terms[parameter.term_index].name == "king_danger_offset = ") {
			parameter.mg_value = 20; parameter.eg_value = 20; continue;
		}
		parameter.mg_value = 0;
		parameter.eg_value = 0;
	}
}

// load the current weights from the engine
void Tuner::engine_weights()
{
	for (unsigned w = 0; w < parameters.list.size(); w++)
	{
		parameters.mg_value(w) = parameters.current_mg_eval(w);
		parameters.eg_value(w) = parameters.current_eg_eval(w);
	}
}

double Tuner::evaluate(Sample &sample)
{
	// For all normal parameters, the evaluation is a cakewalk, because they are linear.
	// (For example, pawns * the pawn value + knights * knight value + ...).
	double mg_evaluation = 0;
	double eg_evaluation = 0;
	for (unsigned i = 0; i < sample.normal_influence_length; i++) {
		mg_evaluation += sample.normal_influence[i] * parameters.mg_value(sample.normal_influence_index[i]);
		eg_evaluation += sample.normal_influence[i] * parameters.eg_value(sample.normal_influence_index[i]);
	}

	// We have to put more work in the king danger parameters.
	// At its core, king danger is a linear funtion. However, we run it through a final adjusting function, which is quadratic.
	double mg_white_king_danger = 0;
	double mg_black_king_danger = 0;
	double eg_white_king_danger = 0;
	double eg_black_king_danger = 0;
	for (unsigned i = 0; i < sample.king_danger_influence_length; i++) {
		mg_white_king_danger += sample.king_danger_white_influence[i] * parameters.mg_value(sample.king_danger_influence_index[i]);
		mg_black_king_danger += sample.king_danger_black_influence[i] * parameters.mg_value(sample.king_danger_influence_index[i]);
		eg_white_king_danger += sample.king_danger_white_influence[i] * parameters.eg_value(sample.king_danger_influence_index[i]);
		eg_black_king_danger += sample.king_danger_black_influence[i] * parameters.eg_value(sample.king_danger_influence_index[i]);
	}
	mg_white_king_danger = std::fmax(0, mg_white_king_danger);
	mg_black_king_danger = std::fmax(0, mg_black_king_danger);
	eg_white_king_danger = std::fmax(0, eg_white_king_danger);
	eg_black_king_danger = std::fmax(0, eg_black_king_danger);

	mg_evaluation -= mg_white_king_danger * mg_white_king_danger / 4096.0 - mg_black_king_danger * mg_black_king_danger / 4096.0;
	eg_evaluation -= eg_white_king_danger / 16.0 - eg_black_king_danger / 16.0;

	return mg_evaluation * sample.phase + eg_evaluation * (1 - sample.phase) * sample.scale_factor;
}

double Tuner::cost(Sample &sample)
{
	double error = sample.outcome - sigmoid(evaluate(sample));
	return error * error;
}

double Tuner::cost_derivative(Sample &sample, double sigmoid)
{
	return -2 * (sample.outcome - sigmoid);
}

double Tuner::average_cost(std::vector<Sample> &set)
{
	double average_error = 0;
	for (Sample sample : set)
		average_error += cost(sample);
	return average_error / set.size();
}

double Tuner::engine_evaluation_error()
{
	double error = 0;
	for (Sample &sample : training_set) {
		double difference = sample.outcome - sigmoid(sample.evaluation);
		error += difference * difference;
	}
	return error / double(training_set.size());
}

void Tuner::find_optimal_scaling()
{
	double start = 0;
	double end = 10;
	double step_size = 1;
	double current = start;
	SCALING = start;
	double best_error = engine_evaluation_error();
	double optimum = start;

	for (unsigned precision = 0; precision < 10; precision++) {
		
		while (current <= end) {
			SCALING = current;
			double error = engine_evaluation_error();
			if (error < best_error) {
				best_error = error;
				optimum = current;
				std::cerr << "found new scaling K = " << current << " error " << error << "\n";
			}
			current += step_size;
		}
		start = optimum - step_size;
		end   = optimum + step_size;
		step_size /= 10.0;
		current = start;
	}
	SCALING = optimum;
	std::cerr << "K set to " << optimum << "\n";
}

// go one step towards the steepest descent
void Tuner::apply_gradients()
{
	/*for (Parameter &p : parameters.list) {
		p.mg_gradient /= BATCH_SIZE;
		p.eg_gradient /= BATCH_SIZE;
		p.mg_sum_squared_gradients += p.mg_gradient * p.mg_gradient;
		p.eg_sum_squared_gradients += p.eg_gradient * p.eg_gradient;
		p.mg_value -= p.mg_gradient * (LEARN_RATE / std::sqrt(p.mg_sum_squared_gradients + TINY_NUMBER));
		p.eg_value -= p.eg_gradient * (LEARN_RATE / std::sqrt(p.eg_sum_squared_gradients + TINY_NUMBER));
	}*/
	for (Parameter &p : parameters.list) {
		p.mg_gradient /= BATCH_SIZE;
		p.eg_gradient /= BATCH_SIZE;
		p.mg_momentum = 0.9 * p.mg_momentum + (1 - 0.9) * p.mg_gradient;
		p.eg_momentum = 0.9 * p.eg_momentum + (1 - 0.9) * p.eg_gradient;
		p.mg_velocity = 0.999 * p.mg_velocity + (1 - 0.999) * p.mg_gradient * p.mg_gradient; 
		p.eg_velocity = 0.999 * p.eg_velocity + (1 - 0.999) * p.eg_gradient * p.eg_gradient; 

		p.mg_value -= p.mg_momentum * LEARN_RATE / (std::sqrt(p.mg_velocity) + 1e-8);
		p.eg_value -= p.eg_momentum * LEARN_RATE / (std::sqrt(p.eg_velocity) + 1e-8);
	}
}

// returns the numerical gradients by tweaking the weights a tiny bit and
// measuring, how the cost function changes
void Tuner::approximate_gradients(std::vector<Sample> &set, unsigned batch)
{
	for (Parameter &parameter : parameters.list) {
		parameter.mg_gradient = 0;
		parameter.eg_gradient = 0;
	}

	for (unsigned i = 0; i < BATCH_SIZE; i++) {
		Sample &sample = set.at(batch * BATCH_SIZE + i);
		for (Parameter &parameter : parameters.list) {

			// how does the cost function change, when we make a slight adjustment to a parameter
			parameter.mg_value += TINY_NUMBER;
			double cost_before = cost(sample);

			// what about the other direction?
			parameter.mg_value -= 2 * TINY_NUMBER;

			// Approximate the gradient with the difference quotient
			parameter.mg_gradient += (cost_before - cost(sample)) / (2 * TINY_NUMBER);

			// undo adjustment
			parameter.mg_value += TINY_NUMBER;

			// Now do the same for the endgame parameter
			parameter.eg_value += TINY_NUMBER;
			cost_before = cost(sample);

			parameter.eg_value -= 2 * TINY_NUMBER;

			parameter.eg_gradient += (cost_before - cost(sample)) / (2 * TINY_NUMBER);

			parameter.eg_value += TINY_NUMBER;
		}
	}
}

// calculates the gradients with the chain rule
void Tuner::compute_gradients(std::vector<Sample> &set, unsigned batch)
{
	for (Parameter &parameter : parameters.list) {
		parameter.mg_gradient = 0;
		parameter.eg_gradient = 0;
	}

	for (unsigned i = 0; i < BATCH_SIZE; i++) {
		Sample &sample = set.at(batch * BATCH_SIZE + i);
		double evaluation = evaluate(sample);
		double sigm = sigmoid(evaluation);
		double partial_derivative = cost_derivative(sample, sigm) * sigmoid_derivative(evaluation);

		// Most gradients can easily be computed
		// The influence array holds the derivatives of each parameter in respect to the evaluation function
		// (in this case, it is how often it is used in the position)
		double mg_phase = sample.phase;
		double eg_scaled_phase = (1 - sample.phase) * sample.scale_factor;
		for (unsigned i = 0; i < sample.normal_influence_length; i++)
			parameters.mg_gradient(sample.normal_influence_index[i]) += sample.normal_influence[i] * partial_derivative * mg_phase;
		for (unsigned i = 0; i < sample.normal_influence_length; i++)
			parameters.eg_gradient(sample.normal_influence_index[i]) += sample.normal_influence[i] * partial_derivative * eg_scaled_phase;

		// King safety is a bit more tricky, because it is quadratic.
		double mg_white_king_danger = 0;
		double mg_black_king_danger = 0;
		double eg_white_king_danger = 0;
		double eg_black_king_danger = 0;
		for (unsigned i = 0; i < sample.king_danger_influence_length; i++) {
			mg_white_king_danger += sample.king_danger_white_influence[i] * parameters.mg_value(sample.king_danger_influence_index[i]);
			mg_black_king_danger += sample.king_danger_black_influence[i] * parameters.mg_value(sample.king_danger_influence_index[i]);
			eg_white_king_danger += sample.king_danger_white_influence[i] * parameters.eg_value(sample.king_danger_influence_index[i]);
			eg_black_king_danger += sample.king_danger_black_influence[i] * parameters.eg_value(sample.king_danger_influence_index[i]);
		}
		mg_white_king_danger = std::fmax(0, mg_white_king_danger);
		mg_black_king_danger = std::fmax(0, mg_black_king_danger);
		eg_white_king_danger = std::fmax(0, eg_white_king_danger);
		eg_black_king_danger = std::fmax(0, eg_black_king_danger);

		for (unsigned i = 0; i < sample.king_danger_influence_length; i++) {
			parameters.mg_gradient(sample.king_danger_influence_index[i]) -=
				(mg_white_king_danger * sample.king_danger_white_influence[i] -
				 mg_black_king_danger * sample.king_danger_black_influence[i]) * 2.0 / 4096.0 * partial_derivative * mg_phase;

			// Be careful not to compute gradients when the king danger is negative, because we clamp it to positive values.
			parameters.eg_gradient(sample.king_danger_influence_index[i]) -=
				(double((eg_white_king_danger > 0.0) * sample.king_danger_white_influence[i]) -
				 double((eg_black_king_danger > 0.0) * sample.king_danger_black_influence[i])) / 16.0 * partial_derivative * eg_scaled_phase;
		}
	}
}

// Main function of the tuner
void Tuner::tune()
{
	// Very important! We rely on the evaluation function of the main program.
	// Remember to turn off the hash table.
	eval.use_pawn_hash_table = false;

	std::cerr << parameters.list.size() << " weights will be tuned\n";

	parameters.print();

	load_training_set();

	std::cerr << "searching for optimal scaling constant...\n";
	find_optimal_scaling();

	parameters.print();

	double best_error = average_cost(test_set);

	for (unsigned iteration = 0; iteration < 4000; iteration++) {
		std::cerr << "iteration " << iteration << "\n";
		for (unsigned batch = 0; batch < NUM_TRAINING_POSITIONS / BATCH_SIZE; batch++) {
			compute_gradients(training_set, batch);
			apply_gradients();
		}
		double error = average_cost(test_set);
		if (best_error - error > 1e-8) {
			best_error = error;
			parameters.print();
			std::cerr << "\ntest error " << best_error << "\n";
		}
		else break;
	}
	std::cerr << "\ntraining error " << average_cost(training_set) << "\n";
	std::cerr << "\ntest error " << average_cost(test_set) << "\n";
}
