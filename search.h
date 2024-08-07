#pragma once

#include <chrono>

#include "board.h"
#include "evaluation.h"
#include "move_ordering.h"
#include "transposition_table.h"

struct Statistics
{
	long search_nodes;
	long quiescence_nodes;
	double branching_factor;
	long cutoffs;
	long cutoffspv;
	long null_cuts;

	unsigned hash_full(Transposition_table &tt);
};

struct Search : Noncopyable
{
	Evaluation eval;
	Move best_root_move = INVALID_MOVE;
	int16_t root_evaluation;

	int max_depth = 63;
	int current_depth;
	bool fixed_time = false;
	bool time_management = false;;
	bool infinite_search = false;
	unsigned soft_time_cap;
	unsigned hard_time_cap;
	unsigned search_time_increment;

	Statistics statistics;
	Heuristics heuristics;

	unsigned age;
	Transposition_table tt;

	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000>>> time_start;
	bool abort_search;

	void reset();

	double time_elapsed();

	int quiescence_search(Board &board, int alpha, int beta, unsigned ply);

	int search(Board &board, int depth, int ply, int alpha, int beta, Move skip, bool allow_null_move);

	void start_search(Board &board);

	void think(Board &board, unsigned move_time, unsigned w_time, unsigned b_time, unsigned w_inc, unsigned b_inc);

	void update_heuristics(Board &board, Move move, int depth, int ply, Move_list &bad_quiets_searched);

	void increment_history(Board &board, Move move, int bonus);

	void extract_pv_line(Board &board);

	void plot_info(Board &board, unsigned nodes_previous_iteration);

	void plot_final_info(unsigned total_nodes);
};
