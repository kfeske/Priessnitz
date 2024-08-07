bool is_promotion(Move move)
{
	MoveFlags flags = flags_of(move);
	switch(flags) {
	case PR_KNIGHT: case PR_BISHOP: case PR_ROOK: case PR_QUEEN:
	case PC_KNIGHT: case PC_BISHOP: case PC_ROOK: case PC_QUEEN:
		return true;

	default: return false;
	}
}

bool capture_promotion(Move move)
{
	MoveFlags flags = flags_of(move);
	switch(flags) {
	case PC_KNIGHT: case PC_BISHOP: case PC_ROOK: case PC_QUEEN:
		return true;

	default: return false;
	}
}

Move promotion_preference(Renderer &renderer, EmptyTexture &board_texture, Cursor &cursor, SDL_Event &e, Board &board, Move move)
{
	int display_x = cursor.x * 100;
	int display_y = (board.side_to_move == WHITE) ? 0 : 400;
	int knight_pos = (board.side_to_move == WHITE) ?  20: 320;
	int bishop_pos = (board.side_to_move == WHITE) ? 120: 220;
	int rook_pos   = (board.side_to_move == WHITE) ? 220: 120;
	int queen_pos  = (board.side_to_move == WHITE) ? 320:  20;
	
	// when we can select a promotion type, the board is greyed out

	EmptyTexture grey_texture { renderer, 800, 800 };
	SDL_SetTextureBlendMode(&grey_texture.texture, SDL_BLENDMODE_BLEND);

	SDL_SetRenderDrawColor(&renderer.renderer, 50, 50, 50, 160);
	SDL_SetRenderTarget(&renderer.renderer, &grey_texture.texture);
	draw_rectangle(renderer, 0, 0, 800, 800);

	// box, which shows the four promotion options

	EmptyTexture white_box { renderer, 100, 400 };
	SDL_SetTextureBlendMode(&white_box.texture, SDL_BLENDMODE_NONE);

	SDL_SetRenderDrawColor(&renderer.renderer, 200, 200, 200, 200);
	SDL_SetRenderTarget(&renderer.renderer, &white_box.texture);
	draw_rectangle(renderer, 0, 0, 100, 400);

	Texture knight { renderer, "images/white_knight.png" };
	knight.render(renderer, 20, knight_pos);
	Texture bishop { renderer, "images/white_bishop.png" };
	bishop.render(renderer, 20, bishop_pos);
	Texture rook   { renderer, "images/white_rook.png" };
	rook.render(renderer, 20, rook_pos);
	Texture queen  { renderer, "images/white_queen.png" };
	queen.render(renderer, 20, queen_pos);
	
	SDL_SetRenderTarget(&renderer.renderer, NULL);

	bool done = false;
	while (!done) {
		SDL_SetRenderDrawColor(&renderer.renderer, 0,0,0,0);
		SDL_RenderClear(&renderer.renderer);
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT or (e.type == SDL_KEYDOWN and e.key.keysym.sym == SDLK_ESCAPE)) done = true;
			if (e.type == SDL_KEYDOWN) {
				auto key  = e.key.keysym.sym;
				if (board.side_to_move == WHITE) {
					if (key == SDLK_UP    and cursor.y > 0)  cursor.y--;
					if (key == SDLK_DOWN  and cursor.y < 3)  cursor.y++;
				}
				else {
					if (key == SDLK_UP    and cursor.y > 4)  cursor.y--;
					if (key == SDLK_DOWN  and cursor.y < 7)  cursor.y++;
				}

				if (key == SDLK_SPACE) {
					unsigned i = (board.side_to_move == WHITE) ? cursor.y : 7 - cursor.y;
					MoveFlags flag = QUIET;
					if (capture_promotion(move))
						switch(i) {
						case 0: flag = PC_KNIGHT;
							break;
						case 1: flag = PC_BISHOP;
							break;
						case 2: flag = PC_ROOK;
							break;
						case 3: flag = PC_QUEEN;
							break;
						}
					else {
						switch(i) {
						case 0: flag = PR_KNIGHT;
							break;
						case 1: flag = PR_BISHOP;
							break;
						case 2: flag = PR_ROOK;
							break;
						case 3: flag = PR_QUEEN;
							break;
						}
					}
					return create_move(move_from(move), move_to(move), flag);
				}
			}
		}
		board_texture.render(renderer, 0, 0);
		grey_texture.render(renderer, 0, 0);
		white_box.render(renderer, display_x, display_y);
		cursor.render(renderer);
		SDL_RenderPresent(&renderer.renderer);
	}
	return move;
}

bool matches(Renderer &renderer, EmptyTexture &board_texture, Cursor &cursor, SDL_Event &e, Board &board, Move &move, MoveGenerator &movegenerator)
{
	std::vector<Move> legal_moves = movegenerator.movelist;

	for (unsigned i = 0; i < legal_moves.size(); i++) {
		Move legal_move = legal_moves.at(i);
		if (move_from(move) == move_from(legal_move) && move_to(move) == move_to(legal_move)) {
			move = legal_move;
			if (is_promotion(move)) move = promotion_preference(renderer, board_texture, cursor, e, board, move);
			return true;
		}
	}
	return false;
}

State update_game_state(Board &board)
{
	MoveGenerator movegenerator {};
	movegenerator.generate_all_moves(board, false);
	if (movegenerator.movelist.size() == 0) {
		if (board.in_check())
			if (board.side_to_move == WHITE) return W_CHECKMATE;
			else				 return B_CHECKMATE;
		else return STALEMATE;
	}
	if (board.history[board.game_ply].rule_50 >= 100)
		return MOVE_50;
	if (board.repetition)
		return REPETITION;
	return ONGOING;
}

bool game_ends(State state)
{
	switch(state) {
	case W_CHECKMATE:
		std::cerr << "\n\nBlack wins!\n";
		return true;
	case B_CHECKMATE:
		std::cerr << "\n\nWhite wins!\n";
		return true;
	case STALEMATE:
		std::cerr << "\n\nDraw by stalemate!\n";
		return true;
	case MOVE_50:
		std::cerr << "\n\nDraw by 50 move rule!\n";
		return true;
	case REPETITION:
		std::cerr << "\n\nDraw by repetition!\n";
		return true;
	default: return false;
	}
}

bool human_to_move(unsigned num_players, Color side_to_move)
{
	return (num_players == 2 || (num_players == 1 && side_to_move == WHITE));
}
