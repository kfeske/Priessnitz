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
};

struct Search : Noncopyable
{
	Evaluation eval;
	Move best_root_move = INVALID_MOVE;
	int16_t root_evaluation;

	unsigned max_depth = 63;
	double max_time = 999999;
	unsigned current_depth;

	Statistics statistics;
	Heuristics heuristics;

	Transposition_table tt;

	std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000>>> time_start;
	bool abort_search;

	void reset();

	double time_elapsed();

	int quiescence_search(Board &board, int alpha, int beta, unsigned ply);

	int search(Board &board, int depth, int ply, int alpha, int beta, Move skip, bool allow_null_move);

	void extract_pv_line(Board &board);

	void plot_info(Board &board, unsigned nodes_previous_iteration);

	void plot_final_info(unsigned total_nodes);

	void start_search(Board &board);

	void think(Board &board, unsigned move_time, unsigned w_time, unsigned b_time, unsigned w_inc, unsigned b_inc);
};
