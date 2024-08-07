#include <string>
#include <sstream>

#include "uci.h"

#include "utility.h"
#include "board.h"
#include "move_generator.h"
#include "search.h"
#include "test.h"

Move UCI::move_from_string(std::string move)
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
		board.make_move(move_from_string(parsed));
}

void UCI::go_command(std::istringstream &iss)
{
	search.time_management = false;
	search.fixed_time = false;
	search.max_depth = 63;
	unsigned move_time = 0;
	unsigned w_time = 0;
	unsigned b_time = 0;
	unsigned w_inc = 0;
	unsigned b_inc = 0;
	unsigned moves_to_go = 0;

	std::string parsed {};
	while(iss >> parsed) {

		if (parsed == "depth")
			iss >> search.max_depth;

		else if (parsed == "movetime") {
			iss >> move_time;
			search.fixed_time = true;
		}

		else if (parsed == "wtime") {
			iss >> w_time;
			search.time_management = true;
		}

		else if (parsed == "btime") {
			iss >> b_time;
			search.time_management = true;
		}

		else if (parsed == "winc")
			iss >> w_inc;
		
		else if (parsed == "binc")
			iss >> b_inc;

		else if (parsed == "movestogo") {
			iss >> moves_to_go;
		}

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
			see_test(board, move_from_string(move));
			return;
		}
	}
	search.think(board, move_time, w_time, b_time, w_inc, b_inc, moves_to_go);
}

void UCI::setoption_command(std::istringstream &iss)
{
	std::string parsed;
	iss >> parsed; // name token
	iss >> parsed;
	if (parsed == "FpMargin1") {
		iss >> parsed; // value token
		int &value = search.search_constants.FUTILITY_MARGIN[1];
		iss >> value;
	}
	if (parsed == "FpMargin2") {
		iss >> parsed; // value token
		int &value = search.search_constants.FUTILITY_MARGIN[2];
		iss >> value;
	}
	if (parsed == "FpMargin3") {
		iss >> parsed; // value token
		int &value = search.search_constants.FUTILITY_MARGIN[3];
		iss >> value;
	}
	if (parsed == "FpMargin4") {
		iss >> parsed; // value token
		int &value = search.search_constants.FUTILITY_MARGIN[4];
		iss >> value;
	}
	if (parsed == "FpMargin5") {
		iss >> parsed; // value token
		int &value = search.search_constants.FUTILITY_MARGIN[5];
		iss >> value;
	}
	if (parsed == "FpMargin6") {
		iss >> parsed; // value token
		int &value = search.search_constants.FUTILITY_MARGIN[6];
		iss >> value;
	}
	if (parsed == "RfpMargin") {
		iss >> parsed; // value token
		int &value = search.search_constants.REVERSE_FUTILITY_MARGIN;
		iss >> value;
	}
	if (parsed == "LmpMargin1") {
		iss >> parsed; // value token
		int &value = search.search_constants.LMP_MARGIN[1];
		iss >> value;
	}
	if (parsed == "LmpMargin2") {
		iss >> parsed; // value token
		int &value = search.search_constants.LMP_MARGIN[2];
		iss >> value;
	}
	if (parsed == "LmpMargin3") {
		iss >> parsed; // value token
		int &value = search.search_constants.LMP_MARGIN[3];
		iss >> value;
	}
	if (parsed == "LmpMargin4") {
		iss >> parsed; // value token
		int &value = search.search_constants.LMP_MARGIN[4];
		iss >> value;
	}
	if (parsed == "Tempo") {
		iss >> parsed; // value token
		int &value = search.eval.tempo_bonus;
		iss >> value;
	}
}

void UCI::await_input()
{
	std::string line {};
	std::string parsed {};

	std::getline(std::cin, line);
	std::istringstream iss { line };

	while (iss >> parsed) {
		if (parsed == "uci") {
			std::cout << "id name Priessnitz\n";
			std::cout << "id author Kevin Feske\n\n";
			//std::cout << "option name FpMargin3 type spin default 100 min 0 max 200\n";
			std::cout << "uciok\n";
		}
		if (parsed == "ucinewgame") search.reset();
		if (parsed == "isready") std::cout << "readyok\n";
		if (parsed == "position") fabricate_position(iss);
		if (parsed == "d") print_board(board);
		if (parsed == "go") go_command(iss);
		if (parsed == "setoption") setoption_command(iss);
		if (parsed == "quit") quit = true;
	}
}
