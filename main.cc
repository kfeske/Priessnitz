#include <sdl2.h>
#include <iostream>
#include <string>

#include <utility.h>
#include <board.h>
#include <move_generator.h>
#include <graphics.h> 
#include <misc.h>
#include <search.h>

//TranspositionTable<(500 * 1024 * 1024) / sizeof(TTEntry)> Search::tt = {};

int main()
{
	// SDL

	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init (IMG_INIT_PNG);
	SDL_Event e;

	Window window { 800, 800 };
	Renderer renderer (window);
	SDL_SetRenderDrawBlendMode(&renderer.renderer, SDL_BLENDMODE_BLEND);
	EmptyTexture board_texture { renderer, 800, 800 };
	SDL_SetTextureBlendMode(&board_texture.texture, SDL_BLENDMODE_BLEND);

	// board 

	static Board board = Board::create("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	//static Board board = Board::create("rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq - 0 1");
	//static Board board = Board::create("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
	//static Board board = Board::create("8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 0");
	//static Board board = Board::create("6K1/8/3Q4/8/8/8/5kp1/8 w - - 0 0");
	
	
	
	//run_perft(board, 6);

	MoveGenerator movegenerator {};
	movegenerator.generate_all_moves(board, false);
	Search search {};

	// board representation

	images_to_board_texture(board, renderer, board_texture);
	board_texture.render(renderer, 0, 0);
	Cursor cursor { 0, 0 };
	unsigned selected_square = SQ_NONE;
	unsigned cursor_square   = SQ_NONE;
	Move chosen_move = INVALID_MOVE;
	Move last_move = INVALID_MOVE;

	// gamestate

	unsigned const num_players = 1;
	bool editmode = false;

	bool win = false;
	bool done = false;
	while (!done) {

		// clear screen

		SDL_SetRenderDrawColor(&renderer.renderer, 0,0,0,0);
		SDL_RenderClear(&renderer.renderer);

		// computer to move

		if (!human_to_move(num_players, board.side_to_move) && !win) {
			int evaluation = 0;
			int lasttime = SDL_GetTicks();
			evaluation = search.start_search(board);
			Move move = search.best_move;
			board.make_move(move);
			print_move(move);
			std::cerr << ",  depth: " << search.current_depth;
			std::cerr << ",  nodes: " << search.nodes_searched;
			std::cerr << ",  eval: "  << evaluation;
			std::cerr << ",  time: "  << SDL_GetTicks() - lasttime;
			//std::cerr << ",  tt_cutoffs: " << search.tt_cutoffs;
			/*std::cerr << ",  pv: ";
			for (unsigned i = 0; i < search.heuristics.pv_lenght[0] - 1; i++) {
				print_move(search.heuristics.previous_pv_line[i]);
				std::cerr << "-> ";
			}
			std::cerr << " ...\n\n";*/
			images_to_board_texture(board, renderer, board_texture);
			if (game_ends(update_game_state(board))) win = true;
			last_move = move;
			//SDL_Delay(200);
		}

		// events

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT or (e.type == SDL_KEYDOWN and e.key.keysym.sym == SDLK_ESCAPE)) done = true;
			if (e.type == SDL_KEYDOWN) {
			

				auto key = e.key.keysym.sym;
				if (key == SDLK_UP    and cursor.y > 0)  cursor.y--;
				if (key == SDLK_DOWN  and cursor.y < 7)  cursor.y++;
				if (key == SDLK_LEFT  and cursor.x > 0)  cursor.x--;
				if (key == SDLK_RIGHT and cursor.x < 7)  cursor.x++;
				else if (key == SDLK_e) {
					editmode = !editmode;
					std::cerr << "edit mode: " << editmode << "\n";
				}
				

				else if (key == SDLK_SPACE) {

					// player to move
 

					if (human_to_move(num_players, board.side_to_move) && !win) {

						cursor_square = cursor.y * 8 + cursor.x;
						chosen_move = create_move(selected_square, cursor_square);

						if (matches(renderer, board_texture, cursor, e, board, chosen_move, movegenerator)
						    and selected_square != SQ_NONE) {
							board.make_move(chosen_move);
							if (editmode) board.unmake_move(chosen_move);
							images_to_board_texture(board, renderer, board_texture);
							selected_square = SQ_NONE;
							last_move = chosen_move;
						}
						else {
							selected_square = cursor_square;
							chosen_move = create_move(selected_square, cursor_square);
							movegenerator.movelist = {};
							movegenerator.generate_all_moves(board, false);
						}
						if (game_ends(update_game_state(board))) win = true;
					}
				}
			}
		}
		/*if (win) {
			Uint64 z_key = 0ULL;
			for (unsigned square = 0; square <= 63; square++) {
				z_key ^= board.zobrist.piece_rand[board.board[square]][square];
			}
			if (board.side_to_move == BLACK)
				z_key ^= board.zobrist.side_rand;
			if (board.history[board.game_ply].ep_sq != SQ_NONE)
				z_key ^= board.zobrist.ep_rand[file(board.history[board.game_ply].ep_sq)];
			z_key ^= board.zobrist.castling_rand[board.history[board.game_ply].castling_rights]; // queenside
			std::cerr << "correct: " << z_key << "\n";

			std::cerr << "current: " << board.zobrist.key << "\n";
				
		}*/


		// rendering
	
		board_texture.render(renderer, 0, 0);

		show_last_move(renderer, last_move);
		if (human_to_move(num_players, board.side_to_move)) {
			if (selected_square != SQ_NONE)
				show_legal_moves(renderer, movegenerator, chosen_move);
			cursor.render(renderer);
		}

		// update screen

		SDL_RenderPresent(&renderer.renderer);
	} 
	SDL_Quit();
	return 0;
}
