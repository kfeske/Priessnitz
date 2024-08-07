
struct TTEntry
{
	Uint64 key;
	Uint8 depth;
	int16_t evaluation;
	Uint16 best_move;
	Uint8 flag;
};

template <unsigned int size>
struct TranspositionTable
{
	TTEntry entries[size];
	int current_evaluation;
	Move pv_move;

	bool probe(Uint64 key, unsigned depth, int alpha, int beta)
	{
		pv_move = INVALID_MOVE;
		Uint64 index = key % size;
		TTEntry &entry = entries[index];
		if (entry.key == key) {
			pv_move = Move(entry.best_move);
			if (entry.depth >= depth) {

				int evaluation = entry.evaluation;

				if (evaluation > 31000)
					evaluation += depth;
				if (evaluation < -31000)
					evaluation -= depth;

				if (entry.flag == EXACT) {
					current_evaluation = evaluation;
					return true;
				}
				if (entry.flag == LOWERBOUND && evaluation >= beta) {
					current_evaluation = evaluation;
					return true;
				}
				else if (entry.flag == UPPERBOUND && evaluation <= alpha) {
					current_evaluation = evaluation;
					return true;
				}
			}
		}
		return false;
	}

	void store(Uint64 key, unsigned depth, int evaluation, Move best_move, TTEntryFlag flag)
	{
		Uint64 index = key % size;
		TTEntry &entry = entries[index];
		if (entry.depth > depth) return;
		entry.key = key;
		entry.depth = depth;
		entry.best_move = best_move;
		entry.flag = flag;
		if (evaluation > 31000)
			evaluation -= depth;
		if (evaluation < -31000)
			evaluation += depth;
		entry.evaluation = int16_t(evaluation);
	}
};
