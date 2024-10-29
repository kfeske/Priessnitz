#include <tuner.h>
#include <iostream>
#include <fstream>
#include <vector>

bool test(Tuner &tuner)
{
	tuner.eval.use_pawn_hash_table = false;
	bool error = false;

	tuner.eval.tempo_bonus = 0;
	std::cerr << "checking evaluations and gradients...\n";
	tuner.engine_weights();

	std::vector<Sample> verify_set {};
	std::vector<std::string> verify_positions {};

	std::ifstream file;
	file.open("training_data.epd");
	if (file.is_open()) {
		std::string input;
		for (unsigned pos = 0; pos < 100000; pos++) {

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

			Sample sample { tuner.outcome(result) };
			tuner.board.set_fenpos(fen);
			tuner.extract_features(sample);
			verify_set.emplace_back(sample);

			double correct_eval = tuner.eval.evaluate(tuner.board);
			if (tuner.board.side_to_move == BLACK) correct_eval *= -1;
			double tuner_eval = tuner.evaluate(sample);

			// is the evaluation correct?
			if (fabs(tuner_eval - correct_eval) > 2) {
				std::cerr << "error in position " << pos <<
					", where correct " << correct_eval << " and tuner " << tuner_eval << "\n";
				error = true;
			}
		}
	}

	// Do the approximated and computed gradients match?
	tuner.approximate_gradients(verify_set, 0);
	Parameters approximated_parameters = tuner.parameters;

	tuner.compute_gradients(verify_set, 0);
	for (unsigned grad = 0; grad < tuner.parameters.list.size(); grad++) {
		double approximated = approximated_parameters.mg_gradient(grad);
		double computed = tuner.parameters.mg_gradient(grad);
		if (fabs(approximated - computed) > 0.000000001) {
			std::cerr << "gradient error in mg " << tuner.parameters.terms[tuner.parameters.list[grad].term_index].name
				  << ": approx " << approximated << " comp " << computed << "\n";
			error = true;
		}

		approximated = approximated_parameters.eg_gradient(grad);
		computed = tuner.parameters.eg_gradient(grad);
		if (fabs(approximated - computed) > 0.000000001) {
			std::string name = tuner.parameters.terms[tuner.parameters.list[grad].term_index].name;
			// For the endgame king danger terms, the derivative is not defined for zero, because of
			// the "max(0, danger)" line. We just let it slip :O
			if (name == "king_attacker_weight[6] = { "   || name == "king_zone_attack_count_weight = " ||
			    name == "king_danger_no_queen_weight = " || name == "safe_knight_check = " ||
			    name == "safe_bishop_check = "           || name == "safe_rook_check = " ||
			    name == "safe_queen_check = "            || name == "king_zone_weak_square = " ||
			    name == "unsafe_check = "                ||name == "pawn_shelter_king_danger[2][4][8] = { " ||
			    name == "king_danger_offset = ") continue;
			std::cerr << "gradient error in eg " << name
				  << ": approx " << approximated << " comp " << computed << "\n";
			error = true;
		}
	}
	return !error;
}
