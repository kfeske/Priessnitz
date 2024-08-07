struct Perft {
	
	long nodes = 0;
	unsigned captures = 0;
	unsigned ep_captures = 0;
	unsigned max_depth;
	unsigned cap = 0;

	long perft(Board &board, unsigned depth)
	{
		if (depth == 0) {
			nodes++;
			return 1;
		}

		Move_generator move_generator {};
		move_generator.generate_all_moves(board);

		for (unsigned n = 0; n < move_generator.size; n++) {
			Move move = move_generator.move_list[n].move;

			if (depth == 1) {
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
			}
			board.make_move(move);

			perft(board, depth - 1);

			board.unmake_move(move);
		}
		return nodes;
	}

	long get_legal_moves_count(Board &board, unsigned depth)
	{
		long nds = 0;
		if (depth == 0) {
			return 1;
		}

		Move_generator movegenerator {};
		movegenerator.generate_all_moves(board);

		for (unsigned n = 0; n < movegenerator.size; n++) {
			Move move = movegenerator.move_list[n].move;

			board.make_move(move);

			if (depth == max_depth) {
				print_move(move);
				nds += get_legal_moves_count(board, depth - 1);
				std::cerr << nds << "\n\n";
				nds = 0;
			}
			else
				nds += get_legal_moves_count(board, depth - 1);

			board.unmake_move(move);
		}
		return nds;
	}

	Perft(unsigned depth)
	:
		max_depth(depth)
	{}
};

void run_perft(Board &board, unsigned depth)
{
	auto lasttime = SDL_GetTicks();
	for (unsigned i = 1; i <= depth; i++)
	{
		Perft perft { i };
		perft.perft(board, i);
		std::cerr << "captures: " << perft.cap << "\n";
		std::cerr << "DEPTH: " << i << "  LEGAL_MOVES: " << perft.nodes << "  CAPTURES: " << perft.captures
			  << "  EN PASSANT: " << perft.ep_captures << "  TIME: " << SDL_GetTicks() - lasttime << "\n";
	}
}
