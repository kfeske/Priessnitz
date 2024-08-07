#include <string>

#include <utility.h>
#include <pre_computed.h>

// data that needs to be cleared when loading new position
struct Board_state
{
	uint64_t pieces[15];
	int non_pawn_material[2];
};

struct Board : Board_state
{
	uint64_t color[2];
	uint64_t occ;

	Color side_to_move = WHITE;
	Piece board[64];

	PreComputed precomputed;

	// resets information neccessary to start a new game
	void reset()
	{
		static_cast<Board_state&>(*this) = {};
	}

	void set_fenpos(std::string fen)
	{
		reset(); // reset board to avoid overwrites
		
		bool rights = false;
		unsigned square = 0;
		uint64_t square_bb = 0ULL;

		for (char &character : fen) {
			if (square <= 63) square_bb = 1ULL << square;

			if (character == ' ')
				rights = true;

			if (!rights) {

				// position of pieces
				if (character == '/') continue;

				else if (character == 'P') pieces[W_PAWN]   |= square_bb;
				else if (character == 'N') pieces[W_KNIGHT] |= square_bb;
				else if (character == 'B') pieces[W_BISHOP] |= square_bb;
				else if (character == 'R') pieces[W_ROOK]   |= square_bb;
				else if (character == 'Q') pieces[W_QUEEN]  |= square_bb;
				else if (character == 'K') pieces[W_KING]   |= square_bb;

				else if (character == 'p') pieces[B_PAWN]   |= square_bb;
				else if (character == 'n') pieces[B_KNIGHT] |= square_bb;
				else if (character == 'b') pieces[B_BISHOP] |= square_bb;
				else if (character == 'r') pieces[B_ROOK]   |= square_bb;
				else if (character == 'q') pieces[B_QUEEN]  |= square_bb;
				else if (character == 'k') pieces[B_KING]   |= square_bb;

				else {
					square += std::atoi(&character);
					continue;
				}
				square++;
			}
			else {
				// side to move
				if      (character == 'w') side_to_move = WHITE;
				else if (character == 'b') side_to_move = BLACK;
			}
		}

		color[WHITE] = pieces[W_PAWN] | pieces[W_KNIGHT] | pieces[W_BISHOP] |
				     pieces[W_ROOK] | pieces[W_QUEEN]  | pieces[W_KING];
		color[BLACK] = pieces[B_PAWN] | pieces[B_KNIGHT] | pieces[B_BISHOP] |
				     pieces[B_ROOK] | pieces[B_QUEEN]  | pieces[B_KING];
		occ = color[WHITE] | color[BLACK];

		// array representation

		for (unsigned square = 0; square < 64; square++)
			board[square] = NO_PIECE;

		for (unsigned p = W_PAWN; p <= B_KING; p++) {
			Piece pc = Piece(p);
			uint64_t bb = pieces[p];
			while (bb) {
				unsigned square = pop_lsb(bb);
				board[square] = pc;
				non_pawn_material[color_of(pc)] += non_pawn_value(pc);
			}
		}
	}
};

std::string name_of_square(Board &board, unsigned square)
{
	uint64_t square_bb = 1ULL << square;
	if (board.pieces[W_PAWN]   & square_bb) return "P";
	if (board.pieces[B_PAWN]   & square_bb) return "p";
	if (board.pieces[W_KNIGHT] & square_bb) return "N";
	if (board.pieces[B_KNIGHT] & square_bb) return "n";
	if (board.pieces[W_BISHOP] & square_bb) return "B";
	if (board.pieces[B_BISHOP] & square_bb) return "b";
	if (board.pieces[W_ROOK]   & square_bb) return "R";
	if (board.pieces[B_ROOK]   & square_bb) return "r";
	if (board.pieces[W_QUEEN]  & square_bb) return "Q";
	if (board.pieces[B_QUEEN]  & square_bb) return "q";
	if (board.pieces[W_KING]   & square_bb) return "K";
	if (board.pieces[B_KING]   & square_bb) return "k";
	return " ";
}

void print_board(Board &board)
{
	std::string str = "    +---+---+---+---+---+---+---+---+\n";

	for (unsigned rank = 0; rank < 8; rank++) { 
		str += "  " + std::to_string(8 - rank) + " ";
		for (unsigned file = 0; file < 8; file++) {
			unsigned square = rank * 8 + file;
			str += "| " + name_of_square(board, square) + " ";

		}
		str += "|\n    +---+---+---+---+---+---+---+---+\n";
	}
	str += "      A   B   C   D   E   F   G   H\n\n";
	std::cerr << str;
}
