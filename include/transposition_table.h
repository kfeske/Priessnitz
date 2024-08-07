
struct TT_entry
{
	uint64_t key;
	uint8_t depth;
	int16_t evaluation;
	uint16_t best_move;
	uint8_t flag;
};

template <unsigned int size>
struct Transposition_table
{
	TT_entry entries[size];
	int current_evaluation;
	Move pv_move;

	bool probe(uint64_t key, unsigned depth, int alpha, int beta)
	{
		uint64_t index = key % size;
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

	void store(uint64_t key, unsigned depth, int evaluation, Move best_move, TT_flag flag)
	{
		uint64_t index = key % size;
		TT_entry &entry = entries[index];
		if (entry.depth > depth) return; // do not overwrite a more accurate entry
		entry.key = key;
		entry.depth = depth;
		entry.best_move = best_move;
		entry.flag = flag;
		entry.evaluation = int16_t(evaluation);
	}
};
