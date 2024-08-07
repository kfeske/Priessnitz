#include <chrono>

#include "board.h"
#include "see.h"

void test_position(Board &board, Search &search, std::string position)
{
	board.set_fenpos(position);
	std::cerr << "3 seconds:\n";
	search.max_time = 3000;
	search.start_search(board);
	std::cerr << "10 seconds:\n";
	search.max_time = 10000;
	search.start_search(board);
}

void run_test_suite(Board &board, Search &search)
{
	// Opening
	std::string pos1 = "r1b1r1k1/1pqn1pbp/p2pp1p1/P7/1n1NPP1Q/2NBBR2/1PP3PP/R6K w - -";
	std::string pos2 = "r1bqkb1r/ppp3pp/2np4/3N1p2/3pnB2/5N2/PPP1QPPP/2KR1B1R b kq -";
	// Midgame
	std::string pos3 = "1nrrb1k1/1qn1bppp/pp2p3/3pP3/N2P3P/1P1B1NP1/PBR1QPK1/2R5 w";
	std::string pos4 = "r3k2r/5ppp/3pbb2/qp1Np3/2BnP3/N7/PP1Q1PPP/R3K2R w KQkq -";
	// Endgame
	std::string pos5 = "3kB3/5K2/7p/3p4/3pn3/4NN2/8/1b4B1 w - -";
	std::string pos6 = "8/8/R7/1b4k1/5p2/1B3r2/7P/7K w - -";

	std::cerr << "\nLate Opening:\n";
	std::cerr << "\npos1 ";
	test_position(board, search, pos1);
	std::cerr << "\npos2 ";
	test_position(board, search, pos2);
	std::cerr << "\nMidgame:\n";
	std::cerr << "\npos3 ";
	test_position(board, search, pos3);
	std::cerr << "\npos4 ";
	test_position(board, search, pos4);
	std::cerr << "\nEndgame:\n";
	std::cerr << "\npos5 ";
	test_position(board, search, pos5);
	std::cerr << "\npos6 ";
	test_position(board, search, pos6);
}

// exchange the colors of the pieces on the board. The evaluation should still be the same
void mirror_test(Board &board, Search &search)
{
	print_board(board);
	std::cerr << "side to move " << search.eval.evaluate(board) << "\n";

	Board mirrored_board {};
	mirrored_board.side_to_move = swap(board.side_to_move);

	for (unsigned p = W_PAWN; p <= B_KING; p++) {
		Piece_type pt = type_of(Piece(p));
		Color color = color_of(Piece(p));
		uint64_t bb = board.pieces[p];
		while (bb) {
			unsigned square = pop_lsb(bb);
			mirrored_board.add_piece(normalize[BLACK][square], piece_of(swap(color), pt));
		}
	}
	print_board(mirrored_board);
	std::cerr << "other side " << search.eval.evaluate(mirrored_board) << "\n";
}

void see_test(Board &board, Move move)
{
	int value = see(board, move);
	std::cerr << "see value " << value << "\n";
}

struct Perft {
	
	long nodes = 0;
	long sub_nodes = 0;
	long captures = 0;
	long ep_captures = 0;
	long promotions = 0;
	unsigned max_depth = 0;

	void perft(Board &board, unsigned depth)
	{
		Move_list move_list;
		generate_legal(board, move_list);

		for (unsigned move_count = 0; move_count < move_list.size; move_count++) {
			Move move = move_list.moves[move_count].move;

			if (depth == max_depth)
				std::cerr << move_string(move) << " ";

			if (depth == 1) {
				nodes++;
				sub_nodes++;
				switch(flags_of(move)) {
				case CAPTURE: case PC_KNIGHT: case PC_BISHOP: case PC_ROOK: case PC_QUEEN:
					captures++;
					break;
				case EP_CAPTURE:
					ep_captures++;
					captures++;
					break;
				default: break;
				}
				if (promotion(move)) promotions++;
			}
			else {
				board.make_move(move);

				perft(board, depth - 1);

				board.unmake_move(move);
			}

			if (depth == max_depth) {
				std::cerr << sub_nodes << "\n";
				sub_nodes = 0;
			}
		}
	}

	Perft(Board &board, unsigned depth, unsigned max_depth)
	:
		max_depth(max_depth)
	{
		perft(board, depth);
	}
};

void run_perft(Board &board, unsigned depth)
{
	auto time_start = std::chrono::high_resolution_clock::now();
	for (unsigned i = 1; i <= depth; i++)
	{
		Perft perft { board, i, depth };
		auto time_end = std::chrono::high_resolution_clock::now();
		std::cerr << "DEPTH: " << i << "  LEGAL_MOVES: " << perft.nodes << "  CAPTURES: " << perft.captures
			  << "  EN PASSANT: " << perft.ep_captures << " PROMOTIONS: " << perft.promotions;
		std::cerr << "  TIME: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count() << "\n";
	}
}
