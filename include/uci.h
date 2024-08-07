#include <string>
#include <sstream>

#include <utility.h>
#include <board.h>
#include <move_generator.h>
#include <perft.h>
#include <search.h>
#include <test.h>


// the Universal Chess Interface (UCI) allows communication between the Graphical User Interface (GUI) and the engine
// this is useful since it allows different chess engines to play against each other
// Note: only the most important commands are included


struct UCI
{
	Board board {};
	Search search;
	bool quit = false;

	Move create_move(std::string move)
	{
		Move_generator move_generator {};
		move_generator.generate_all_moves(board);
		for (Scored_move m : move_generator.move_list) {
			if (move_string(m.move) == move)
				return m.move;
		}
		return INVALID_MOVE;
	}

	void fabricate_position(std::istringstream &iss)
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

	void go_command(std::istringstream &iss)
	{
		unsigned move_time = 999999;
		unsigned w_time = 999999;
		unsigned b_time = 999999;
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

	void await_input()
	{
		std::string line {};
		std::string parsed {};

		std::getline(std::cin, line);
		std::istringstream iss { line };

		while(iss >> parsed) {
			if (parsed == "uci") std::cout << "uciok\n";
			if (parsed == "ucinewgame") break;
			if (parsed == "isready") std::cout << "readyok\n";
			if (parsed == "position") fabricate_position(iss);
			if (parsed == "d") print_board(board);
			if (parsed == "go") go_command(iss);
			if (parsed == "quit") quit = true;
		}
	}
};
