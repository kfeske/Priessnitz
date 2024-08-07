#include <string>
#include <sstream>

#include "utility.h"
#include "board.h"
#include "search.h"


// the Universal Chess Interface (UCI) allows communication between the Graphical User Interface (GUI) and the engine.
// It allows different chess engines to play against each other
// Note: only the most important commands are handled here

struct UCI
{
	Board board {};
	Search search {};
	bool quit = false;

	Move move_from_string(std::string move);

	void fabricate_position(std::istringstream &iss);

	void go_command(std::istringstream &iss);

	void await_input();
};
