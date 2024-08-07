#include <string>
#include <sstream>

#include "uci.h"

#include "utility.h"
#include "board.h"
#include "move_generator.h"
#include "search.h"
#include "test.h"

Move UCI::create_move(std::string move)
{
	Move_list move_list;
	generate_legal(board, move_list);
	for (Scored_move m : move_list.moves) {
		if (move_string(m.move) == move)
			return m.move;
	}
	return INVALID_MOVE;
}

void UCI::fabricate_position(std::istringstream &iss)
{
	std::string parsed {};
	iss >> parsed;

	if (parsed == "startpos") {
		board.set_startpos();
		iss >> parsed; // eats the useless "moves" string
	}

	else if (parsed == "fen") {
		std::string fen_string {};
		while (iss >> parsed && parsed != "moves") // filter out fen string
			fen_string += parsed + " ";
		board.set_fenpos(fen_string);
	}
	else return;

	std::string movelist {};
	while (iss >> parsed)
		board.make_move(create_move(parsed));
}

void UCI::go_command(std::istringstream &iss)
{
	search.max_depth = 63;
	unsigned move_time = INFINITY;
	unsigned w_time = INFINITY;
	unsigned b_time = INFINITY;
	unsigned w_inc = 0;
	unsigned b_inc = 0;

	std::string parsed {};
	while(iss >> parsed) {

		if (parsed == "depth")
			iss >> search.max_depth;

		else if (parsed == "movetime")
			iss >> move_time;

		else if (parsed == "wtime")
			iss >> w_time;

		else if (parsed == "btime")
			iss >> b_time;

		else if (parsed == "winc")
			iss >> w_inc;
		
		else if (parsed == "binc")
			iss >> b_inc;

		else if (parsed == "perft") {
			unsigned depth;
			iss >> depth;
			run_perft(board, depth);
			return;
		}
		else if (parsed == "test") {
			run_test_suite(board, search);
			return;
		}
		else if (parsed == "eval") {
			mirror_test(board, search);
			return;
		}
		else if (parsed == "see") {
			std::string move;
			iss >> move;
			see_test(board, create_move(move));
			return;
		}
	}
	search.think(board, move_time, w_time, b_time, w_inc, b_inc);
}

void UCI::await_input()
{
	std::string line {};
	std::string parsed {};

	std::getline(std::cin, line);
	std::istringstream iss { line };

	while(iss >> parsed) {
		if (parsed == "uci") {
			std::cout << "id name Priessnitz\n";
			std::cout << "id author Kevin Feske\n";
			std::cout << "uciok\n";
		}
		if (parsed == "ucinewgame") break;
		if (parsed == "isready") std::cout << "readyok\n";
		if (parsed == "position") fabricate_position(iss);
		if (parsed == "d") print_board(board);
		if (parsed == "go") go_command(iss);
		if (parsed == "quit") quit = true;
	}
}
