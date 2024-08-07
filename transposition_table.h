#pragma once

unsigned const DEFAULT_TT_SIZE = 128; // MB

struct TT_entry
{
	uint64_t key;
	uint8_t depth;
	int16_t evaluation;
	uint16_t best_move;
	uint8_t flag;
	uint8_t age; // Could be combined with flag.
};

struct Transposition_table
{
	unsigned entry_count = (DEFAULT_TT_SIZE * 1024 * 1024) / sizeof(TT_entry);
	TT_entry *entries;
	int current_evaluation = 0;
	Move pv_move = INVALID_MOVE;

	bool probe(uint64_t key, unsigned depth, int alpha, int beta)
	{
		uint64_t index = key % entry_count;
		TT_entry &entry = entries[index];
		if (entry.key == key) {
			pv_move = Move(entry.best_move);
			if (entry.depth >= depth) {

				int evaluation = entry.evaluation;

				// we have an exact score for that position. Great!
				// (that means, we searched all moves and received a new best move)
				if (entry.flag == EXACT) {
					current_evaluation = evaluation;
					return true;
				}
				// this value is too high for us to be concered about, it will cause a beta-cutoff
				if (entry.flag == LOWERBOUND && evaluation >= beta) {
					current_evaluation = beta;
					return true;
				}
				// this value is too low, we will not exceed alpha in the search
				else if (entry.flag == UPPERBOUND && evaluation <= alpha) {
					current_evaluation = alpha;
					return true;
				}
			}
		}
		return false;
	}

	void store(uint64_t key, unsigned depth, int evaluation, Move best_move, TT_flag flag, unsigned age)
	{
		uint64_t index = key % entry_count;
		TT_entry &entry = entries[index];

		// Do not overwrite a more accurate entry
		if (depth >= entry.depth || age != entry.age) {
			entry.key = key;
			entry.depth = depth;
			entry.best_move = best_move;
			entry.flag = flag;
			entry.evaluation = int16_t(evaluation);
			entry.age = age;
		}
	}

	void resize(unsigned size)
	{
		entries = new TT_entry[size];
	}

	Transposition_table()
	:
		 entries(new TT_entry[entry_count])
	{}
};
