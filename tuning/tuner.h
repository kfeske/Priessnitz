// the task of the tuner is to optimize all the values in the evaluation function, eg. the
// value of a pawn. This is done by a gradient descent algorithm, which adjusts the values
// in a way that the evaluation of every position in the training set predicts the outcome
// of the game, the position is from as best as possible.

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

#include <board.h>
//#include "tuning_board.h"
#include <evaluation.h>
//#include "tuning_evaluation.h"

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
	EG_PASSED_PAWN = MG_PASSED_PAWN + 64,
	MG_ISOLATED = EG_PASSED_PAWN + 64,
	EG_ISOLATED = MG_ISOLATED + 1,
	MG_DOUBLED = EG_ISOLATED + 1,
	EG_DOUBLED = MG_DOUBLED + 1,
	MG_BACKWARD = EG_DOUBLED + 1,
	EG_BACKWARD = MG_BACKWARD + 1,
	MG_CHAINED = EG_BACKWARD  + 1,
	EG_CHAINED = MG_CHAINED + 1,
	MG_AVERAGE_MOBILITY = EG_CHAINED + 1,
	EG_AVERAGE_MOBILITY = MG_AVERAGE_MOBILITY + 6,
	MG_MOBILITY = EG_AVERAGE_MOBILITY + 6,
	EG_MOBILITY = MG_MOBILITY + 6,
	RING_POTENCY = EG_MOBILITY + 6,
	ZONE_POTENCY = RING_POTENCY + 6,
	RING_PRESSURE = ZONE_POTENCY + 6,
	ZONE_PRESSURE = RING_PRESSURE + 8,
	END_INDEX = ZONE_PRESSURE + 8
};

enum Tuning_params {
	NUM_TRAINING_POSITIONS = 700000, // number of training positions
	NUM_TEST_POSITIONS = 25000,      // number of test positions
	NUM_TABLES = 32,		 // number of tables to be tuned (eg. pawn piece square table)
	NUM_WEIGHTS = END_INDEX,         // values to be tuned
	BATCH_SIZE = 1000	         // how much the training set is split for computational efficiency
};

double random_double()
{
	return rand() / (RAND_MAX + 1.0);
}

double random_double(double min, double max)
{
	return min + (max - min) * random_double();
}


// a single position and the game result
struct Sample
{
	//std::string fen;

	double outcome;

	// stores, how much a weight influences the evaluation of a position
	double influence[NUM_WEIGHTS] {};

	// useful for king evaluation
	uint8_t ring_attackers[2] {};
	uint8_t ring_attacks[15] {};
	uint8_t zone_attackers[2] {};
	uint8_t zone_attacks[15] {};

	// useful for mobility evaluation
	int mobility_squares[6] {};
	int material_difference[6] {};

	double phase = 0;
};

struct Tuner
{
	Indicies table[NUM_TABLES + 1] = { MG_VALUES, EG_VALUES,
					   MG_PAWN_PSQT, MG_KNIGHT_PSQT, MG_BISHOP_PSQT, MG_ROOK_PSQT, MG_QUEEN_PSQT, MG_KING_PSQT,
	       				   EG_PAWN_PSQT, EG_KNIGHT_PSQT, EG_BISHOP_PSQT, EG_ROOK_PSQT, EG_QUEEN_PSQT, EG_KING_PSQT,
			     	      	   MG_PASSED_PAWN, EG_PASSED_PAWN, MG_ISOLATED, EG_ISOLATED, MG_DOUBLED, EG_DOUBLED,
					   MG_BACKWARD, EG_BACKWARD, MG_CHAINED, EG_CHAINED,
			     	      	   MG_AVERAGE_MOBILITY, EG_AVERAGE_MOBILITY, MG_MOBILITY, EG_MOBILITY,
			     	      	   RING_POTENCY, ZONE_POTENCY, RING_PRESSURE, ZONE_PRESSURE,
			     	      	   END_INDEX };

	double const SCALING = 3.45387764 / 400; // scaling constant for our evaluation function
	double const H = 0.000001;   		 // difference quotient step size
	double const LEARN_RATE = 10000;	 // step size

	Board board {};
	Evaluation eval {};

	std::vector<Sample> training_set {};
	std::vector<Sample> test_set {};
	double weights[NUM_WEIGHTS];
	double gradients[NUM_WEIGHTS];

	// returns the value in a table at a position
	int table_value(Indicies table_index, unsigned pos)
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
		case MG_DOUBLED: return eval.mg_doubled_penalty;
		case EG_DOUBLED: return eval.eg_doubled_penalty;
		case MG_BACKWARD: return eval.mg_backward_penalty;
		case EG_BACKWARD: return eval.eg_backward_penalty;
		case MG_CHAINED: return eval.mg_chained_bonus;
		case EG_CHAINED: return eval.eg_chained_bonus;
		case MG_AVERAGE_MOBILITY: return eval.mg_average_mobility[pos];
		case EG_AVERAGE_MOBILITY: return eval.eg_average_mobility[pos];
		case MG_MOBILITY: return eval.mg_mobility_weight[pos];
		case EG_MOBILITY: return eval.eg_mobility_weight[pos];
		case RING_POTENCY: return eval.ring_attack_potency[pos];
		case ZONE_POTENCY: return eval.zone_attack_potency[pos];
		case RING_PRESSURE: return eval.ring_pressure_weight[pos];
		case ZONE_PRESSURE: return eval.zone_pressure_weight[pos];
		case MG_VALUES: return eval.mg_piece_value[pos];
		case EG_VALUES: return eval.eg_piece_value[pos];
		default: return eval.eg_passed_bonus[pos];
		}
	}

	// returns the table described with the index. (see table[] array)
	Indicies table_of(unsigned index)
	{
		for (unsigned i = 0; i < NUM_TABLES; i++) {
			if (index >= table[i] && index < table[i + 1])
				return table[i];
		}
		std::cerr << "error: index does not have a table";
		return END_INDEX;
	}

	// returns the weight at an index from the evaluation function
	double load_weight(unsigned index)
	{
		Indicies table_index = table_of(index);
		return table_value(table_index, index - table_index);
	}

	void print_table_name(Indicies table_index)
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
		case MG_DOUBLED: std::cerr << "\nmg doubled pawn\n"; break;
		case EG_DOUBLED: std::cerr << "\neg doubled pawn\n"; break;
		case MG_BACKWARD: std::cerr << "\nmg backward pawn\n"; break;
		case EG_BACKWARD: std::cerr << "\neg backward pawn\n"; break;
		case MG_CHAINED: std::cerr << "\nmg chained pawn\n"; break;
		case EG_CHAINED: std::cerr << "\neg chained pawn\n"; break;
		case MG_AVERAGE_MOBILITY: std::cerr << "\nmg average mobility\n"; break;
		case EG_AVERAGE_MOBILITY: std::cerr << "\neg average mobility\n"; break;
		case MG_MOBILITY: std::cerr << "\nmg mobility\n"; break;
		case EG_MOBILITY: std::cerr << "\neg mobility\n"; break;
		case RING_POTENCY: std::cerr << "\nring attack potency\n"; break;
		case ZONE_POTENCY: std::cerr << "\nzone attack potency\n"; break;
		case RING_PRESSURE: std::cerr << "\nking ring pressure weight\n"; break;
		case ZONE_PRESSURE: std::cerr << "\nking zone pressure weight\n"; break;
		case MG_VALUES: std::cerr << "\nmg values\n"; break;
		case EG_VALUES: std::cerr << "\neg values\n"; break;
		default: return;
		}
	}

	void print_weights()
	{
		Indicies previous_table = END_INDEX;
		std::cerr << "\ncurrent weights:\n";
		for (unsigned i = 0; i < NUM_WEIGHTS; i++) {

			Indicies table = table_of(i);
			if (table != previous_table) {
				previous_table = table;
				print_table_name(table);
			}

			std::cerr << int(weights[i]) << ", ";
			if (table >= MG_PAWN_PSQT && table <= EG_PASSED_PAWN && (i + 13) % 8 == 0) std::cerr << "\n";
		}
	}

	// this functions sets the result of a game; 0 for black win, 0.5 for a draw and 1 for white win
	// the tuner will change all evaluation parameters to best fit this result
	double outcome(std::string &result)
	{
		if (result == "\"0-1\";") return 0.0;
		else if (result == "\"1-0\";") return 1.0;
		else return 0.5;
	}

	// we can precompute the number of king attackers and attacks
	void extract_king_safety(Sample &sample, Piece piece, uint64_t attacks, uint64_t king_ring, uint64_t king_zone)
	{
		uint64_t ring_attacks = attacks & king_ring;
		if (ring_attacks) {
			sample.ring_attacks[piece] += pop_count(ring_attacks);
			if (sample.ring_attackers[color_of(piece)] < 7) sample.ring_attackers[color_of(piece)]++;
		}
		uint64_t zone_attacks = attacks & king_zone;
		if (zone_attacks) {
			sample.zone_attacks[piece] += pop_count(zone_attacks);
			if (sample.zone_attackers[color_of(piece)] < 7) sample.zone_attackers[color_of(piece)]++;
		}
	}

	// necessary for partial derivative of the mobility weight
	void extract_mobility(Sample &sample, Piece piece, uint64_t attacks, int side) {
		sample.mobility_squares[type_of(piece)] += 10 * pop_count(attacks & ~board.pawn_attacks(Color(!color_of(piece)))) * side;
		sample.material_difference[type_of(piece)] += side;
	}

	// stores all evaluation-essential properties of a position
	// (the derivatives of the evaluation in respect to the weights)
	void extract_features(Sample &sample)
	{
		unsigned mg_index;
		unsigned eg_index;
		uint64_t attacks;
		uint64_t ray_blockers;

		int taper_start = 6377;
		int taper_end = 321;

		int material = board.non_pawn_material[WHITE] + board.non_pawn_material[BLACK];
		material = std::max(taper_end, std::min(material, taper_start)); // endgame and midgame limit clamp
		sample.phase = int(((material - taper_end) * 256) / (taper_start - taper_end)); // 0(Endgame) - 256(Midgame) linear interpolation
		sample.phase /= 256;

		double mg_phase = sample.phase;
		double eg_phase = (1 - sample.phase);

		unsigned friendly_king_square = lsb(board.pieces[piece_of(WHITE, KING)]);
		unsigned enemy_king_square = lsb(board.pieces[piece_of(BLACK, KING)]);
		uint64_t king_ring[2] { board.precomputed.attacks_bb<KING>(friendly_king_square, 0ULL),
					board.precomputed.attacks_bb<KING>(enemy_king_square, 0ULL) };

		uint64_t king_zone[2] { board.precomputed.king_zone[WHITE][friendly_king_square], 
			 		board.precomputed.king_zone[BLACK][enemy_king_square] };

		uint64_t all_pieces = board.occ;
		while (all_pieces) {
			unsigned square = pop_lsb(all_pieces);
			Piece piece = board.board[square];
			PieceType type = type_of(piece);
			Color friendly = color_of(piece);
			int side = (friendly == WHITE) ? 1 : -1;

			// material
			mg_index = MG_VALUES + type;
			eg_index = EG_VALUES + type;
			sample.influence[mg_index] += side * mg_phase;
			sample.influence[eg_index] += side * eg_phase;

			// PSQT
			mg_index = MG_PAWN_PSQT + type * 64 + normalize[friendly][square];
			eg_index = EG_PAWN_PSQT + type * 64 + normalize[friendly][square];
			sample.influence[mg_index] += side * mg_phase;
			sample.influence[eg_index] += side * eg_phase;

			switch (type) {
			case PAWN:
				{
				uint64_t friendly_pawns = board.pieces[piece_of(friendly, PAWN)];
				uint64_t enemy_pawns = board.pieces[piece_of(!friendly, PAWN)];
				unsigned forward = (friendly == WHITE) ? NORTH : SOUTH;
				uint64_t adjacent_files = board.precomputed.isolated_pawn_mask[file(square)];

				bool passed = !(board.precomputed.passed_pawn_mask[friendly][square] & enemy_pawns);
				bool doubled = board.precomputed.forward_file_mask[friendly][square] & friendly_pawns;
				bool neighbored = board.precomputed.neighbor_mask[square] & friendly_pawns;
				bool supported = board.precomputed.passed_pawn_mask[!friendly][square] & adjacent_files & friendly_pawns;
				bool chained = board.precomputed.pawn_attacks[!friendly][square] & friendly_pawns;

				// isolated pawns
				if (!(adjacent_files & friendly_pawns)) {
					sample.influence[MG_ISOLATED] += side * mg_phase;
					sample.influence[EG_ISOLATED] += side * eg_phase;
				}

				// doubled pawns
				else if (doubled) {
					sample.influence[MG_DOUBLED] += side * mg_phase;
					sample.influence[EG_DOUBLED] += side * eg_phase;
				}

				// backward pawns
				else if (!(supported || neighbored) && board.precomputed.pawn_attacks[friendly][square + forward] & enemy_pawns) {
					sample.influence[MG_BACKWARD] += side * mg_phase;
					sample.influence[EG_BACKWARD] += side * eg_phase;
				}

				// reward chained pawns
				if (chained) {
					sample.influence[MG_CHAINED] += side * mg_phase;
					sample.influence[EG_CHAINED] += side * eg_phase;
				}

				// passed pawns
				if (passed && !doubled) {
					mg_index = MG_PASSED_PAWN + normalize[friendly][square];
					eg_index = EG_PASSED_PAWN + normalize[friendly][square];
					sample.influence[mg_index] += side * mg_phase;
					sample.influence[eg_index] += side * eg_phase;
				}
				continue;
				}
			// king safety + mobility
			case KNIGHT:
				attacks = board.precomputed.attacks_bb<KNIGHT>(square, 0ULL);
				extract_king_safety(sample, piece, attacks, king_ring[!friendly], king_zone[!friendly]);
				extract_mobility(sample, piece, attacks, side);
				continue;
			case BISHOP:
				ray_blockers = board.occ & ~board.pieces[piece_of(friendly, QUEEN)];
				attacks = board.precomputed.attacks_bb<BISHOP>(square, ray_blockers);
				extract_king_safety(sample, piece, attacks, king_ring[!friendly], king_zone[!friendly]);
				extract_mobility(sample, piece, attacks, side);
				continue;
			case ROOK:
				ray_blockers = board.occ & ~(board.pieces[piece_of(friendly, QUEEN)] | board.pieces[piece_of(friendly, ROOK)]);
				attacks = board.precomputed.attacks_bb<ROOK>(square, ray_blockers);
				extract_king_safety(sample, piece, attacks, king_ring[!friendly], king_zone[!friendly]);
				extract_mobility(sample, piece, attacks, side);
				continue;
			case QUEEN:
				attacks = board.precomputed.attacks_bb<QUEEN>(square, board.occ);
				extract_king_safety(sample, piece, attacks, king_ring[!friendly], king_zone[!friendly]);
				extract_mobility(sample, piece, attacks, side);
				continue;
			case KING: continue;
			}
		}
	}

	void load_training_set()
	{
		std::cerr << "loading training data..." << "\n";
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
			weights[RING_POTENCY + i] = attack_potency[i];
			weights[ZONE_POTENCY + i] = attack_potency[i];
		}

		/*double average_mobility[6] = { 0, 40, 50, 40, 40, 0 };
		for (unsigned i = 0; i < 6; i++) {
			weights[MG_AVERAGE_MOBILITY + i] = average_mobility[i];
			weights[EG_AVERAGE_MOBILITY + i] = average_mobility[i];
		}*/
	}

	void engine_weights()
	{
		for (unsigned w = 0; w < NUM_WEIGHTS; w++)
			weights[w] = load_weight(w);
	}

	template <Color color>
	double king_ring_pressure(Sample &sample)
	{
		double pressure = 0;
		for (PieceType type : { KNIGHT, BISHOP, ROOK, QUEEN }) {
			pressure += weights[RING_POTENCY + type] * sample.ring_attacks[piece_of(color, type)];
		}

		return pressure / 100;
	}

	template <Color color>
	double king_zone_pressure(Sample &sample)
	{
		double pressure = 0;
		for (PieceType type : { KNIGHT, BISHOP, ROOK, QUEEN }) {
			pressure += weights[ZONE_POTENCY + type] * sample.zone_attacks[piece_of(color, type)];
		}

		return pressure / 100;
	}

	double mg_mobility(Sample &sample, PieceType type)
	{
		//std::cerr << "safe squares " << sample.mobility_squares[type] << "\n";
		return double(sample.mobility_squares[type]) - double(sample.material_difference[type]) * weights[MG_AVERAGE_MOBILITY + type];
	}

	double eg_mobility(Sample &sample, PieceType type)
	{
		return double(sample.mobility_squares[type]) - double(sample.material_difference[type]) * weights[EG_AVERAGE_MOBILITY + type];
	}

	double evaluate(Sample &sample)
	{
		// all 'normal' evaluation parts can easily be computed

		double evaluation = 0;
		for (unsigned w = 0; w < NUM_WEIGHTS; w++)
			evaluation += sample.influence[w] * weights[w];

		// king safety is a bit tricky, because two different weights affect each other

		double white_pressure = king_ring_pressure<WHITE>(sample) * weights[RING_PRESSURE + sample.ring_attackers[WHITE]] * sample.phase;
		white_pressure += king_zone_pressure<WHITE>(sample) * weights[ZONE_PRESSURE + sample.zone_attackers[WHITE]] * sample.phase;
		double black_pressure = king_ring_pressure<BLACK>(sample) * weights[RING_PRESSURE + sample.ring_attackers[BLACK]] * sample.phase;
		black_pressure += king_zone_pressure<BLACK>(sample) * weights[ZONE_PRESSURE + sample.zone_attackers[BLACK]] * sample.phase;
		evaluation += (white_pressure - black_pressure);

		// mobility is another inconvenience

		for (PieceType type : { KNIGHT, BISHOP, ROOK, QUEEN }) {
			evaluation += mg_mobility(sample, type) * weights[MG_MOBILITY + type] / 100 * sample.phase;
			evaluation += eg_mobility(sample, type) * weights[EG_MOBILITY + type] / 100 * (1 - sample.phase);
		}

		return evaluation;
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
		return 2 * (sample.outcome - sigmoid);
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
		for (unsigned i = 0; i < NUM_WEIGHTS; i++)
			weights[i] -= gradients[i] * LEARN_RATE / BATCH_SIZE;
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
				weight += H;
				double cost_before = cost(sample);

				// what about the other direction?
				weight -= 2 * H;
				gradients[w] += (cost_before - cost(sample)) / (2 * H);

				// undo adjustment
				weight += H;
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
			// The feature array holds the derivatives of each parameter in respect to the evaluation function
			// (in this case, it is the game phase multiplied by how often it is used in the position)
			for (unsigned w = 0; w < NUM_WEIGHTS; w++)
				gradients[w] -= sample.influence[w] * partial_derivative;

			// King safety and mobility always needs extra work, because they basically have two parameters
			// that affect each other

			// king ring pressure weights
			unsigned white_ring_index = RING_PRESSURE + sample.ring_attackers[WHITE];
			gradients[white_ring_index] -= king_ring_pressure<WHITE>(sample) * partial_derivative * sample.phase;
			unsigned black_ring_index = RING_PRESSURE + sample.ring_attackers[BLACK];
			gradients[black_ring_index] += king_ring_pressure<BLACK>(sample) * partial_derivative * sample.phase;

			// king zone pressure weights
			unsigned white_zone_index = ZONE_PRESSURE + sample.zone_attackers[WHITE];
			gradients[white_zone_index] -= king_zone_pressure<WHITE>(sample) * partial_derivative * sample.phase;
			unsigned black_zone_index = ZONE_PRESSURE + sample.zone_attackers[BLACK];
			gradients[black_zone_index] += king_zone_pressure<BLACK>(sample) * partial_derivative * sample.phase;
			
			for (PieceType type : { KNIGHT, BISHOP, ROOK, QUEEN }) {
				// Ring attack potency
				double white_ring_attacks = sample.ring_attacks[piece_of(WHITE, type)];
				gradients[RING_POTENCY + type] -= white_ring_attacks * weights[white_ring_index] / 100 * partial_derivative * sample.phase;
				double black_ring_attacks = sample.ring_attacks[piece_of(BLACK, type)];
				gradients[RING_POTENCY + type] += black_ring_attacks * weights[black_ring_index] / 100 * partial_derivative * sample.phase;

				// Zone attack potency
				double white_zone_attacks = sample.zone_attacks[piece_of(WHITE, type)];
				gradients[ZONE_POTENCY + type] -= white_zone_attacks * weights[white_zone_index] / 100 * partial_derivative * sample.phase;
				double black_zone_attacks = sample.zone_attacks[piece_of(BLACK, type)];
				gradients[ZONE_POTENCY + type] += black_zone_attacks * weights[black_zone_index] / 100 * partial_derivative * sample.phase;

				// Mobility weights
				gradients[MG_MOBILITY + type] -= mg_mobility(sample, type) / 100 * partial_derivative * sample.phase;
				gradients[EG_MOBILITY + type] -= eg_mobility(sample, type) / 100 * partial_derivative * (1 - sample.phase);

				// Average mobility
				gradients[MG_AVERAGE_MOBILITY + type] -= -sample.material_difference[type] * weights[MG_MOBILITY + type] / 100 * partial_derivative * sample.phase;
				gradients[EG_AVERAGE_MOBILITY + type] -= -sample.material_difference[type] * weights[EG_MOBILITY + type] / 100 * partial_derivative * (1 - sample.phase);
			}
		}
	}

	// Main function of the tuner
	void tune()
	{
		print_weights();
		double best_error = average_cost(test_set);
		std::cerr << "\ntest error " << best_error << "\n";

		unsigned convergence_counter = 0;
		unsigned iteration = 0;
		while (true) {
			iteration++;
			std::cerr << "iteration " << iteration << "\n";
			for (unsigned batch = 0; batch < NUM_TRAINING_POSITIONS / BATCH_SIZE; batch++) {
				//approximate_gradients(batch);
				compute_gradients(batch);
				apply_gradients();
				/*double test_error = average_cost(test_set);
				if (test_error < best_error) {
					best_error = test_error;
					print_weights();
					std::cerr << "\ntest error " << test_error << "\n";
				}*/
			}
			double test_error = average_cost(test_set);
			if (test_error < best_error) {
				best_error = test_error;
				print_weights();
				std::cerr << "\ntest error " << test_error << "\n";
			}
			else convergence_counter++;
			if (convergence_counter >= 200) break;
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
