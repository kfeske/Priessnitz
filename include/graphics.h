void draw_rectangle(Renderer &renderer, int x, int y, int w, int h)
{
	SDL_Rect rect = { x, y, w, h };
	SDL_RenderFillRect(&renderer.renderer, &rect);
}

struct Cursor
{
	int x,y;

	void render(Renderer &renderer)
	{
		SDL_SetRenderDrawColor(&renderer.renderer, 0x3a,0x73,0x9c,200);
		SDL_Rect square = { x*100, y*100, 100, 100 };
		SDL_RenderFillRect(&renderer.renderer, &square);
	}
};

// board visuals

std::string name_of_square(Board &board, unsigned square)
{
	Uint64 square_bb = 1ULL << square;
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

std::string image_of(Board &board, unsigned square)
{
	std::string name = name_of_square(board, square);

	if (name == "P") return "images/white_pawn.png";
	if (name == "p") return "images/black_pawn.png";
	if (name == "N") return "images/white_knight.png";
	if (name == "n") return "images/black_knight.png";
	if (name == "B") return "images/white_bishop.png";
	if (name == "b") return "images/black_bishop.png";
	if (name == "R") return "images/white_rook.png";
	if (name == "r") return "images/black_rook.png";
	if (name == "Q") return "images/white_queen.png";
	if (name == "q") return "images/black_queen.png";
	if (name == "K") return "images/white_king.png";
	if (name == "k") return "images/black_king.png";
	return "None";
}

void draw_square(Renderer &renderer, int row, int column, int r, int g, int b, int a)
{
	SDL_SetRenderDrawColor(&renderer.renderer, r,g,b,a);
	SDL_Rect square = { row*100, column*100, 100, 100 };
	SDL_RenderFillRect(&renderer.renderer, &square);
}


void show_legal_moves(Renderer &renderer, MoveGenerator &movegenerator, Move chosen_move)
{
	for (unsigned n = 0; n < movegenerator.movelist.size(); n++) {
		Move legal_move = movegenerator.movelist.at(n);
		if (move_from(legal_move) != move_to(chosen_move)) continue;
		unsigned square_to = move_to(legal_move);
		unsigned y = square_to / 8;
		unsigned x = square_to - y * 8;
		draw_square(renderer, x, y, 200, 200, 200, 200);
	}
}

void show_last_move(Renderer &renderer, Move move)
{
	if (move == INVALID_MOVE) return;

	unsigned from = move_from(move);
	unsigned y = from / 8;
	unsigned x = from - y * 8;
	draw_square(renderer, x, y, 139, 152, 106, 255);
}

void images_to_board_texture(Board &board, Renderer &renderer, EmptyTexture &texture)
{
	SDL_RenderClear(&renderer.renderer);
	SDL_SetRenderTarget(&renderer.renderer, &texture.texture);

	for (unsigned rank = 0; rank < 8; rank++) {
		for (unsigned file = 0; file < 8; file++) {
			if ((file + rank) % 2 == 0)
				draw_square(renderer, file, rank, 247,204,154,255);
			else
				draw_square(renderer, file, rank, 156,98,58,255);

			std::string path = image_of(board, rank * 8 + file);
			if (path != "None") {
				Texture img { renderer, path };
				img.render(renderer, file*100 + 20, rank*100 + 20);
			}
		}
	}
	SDL_SetRenderTarget(&renderer.renderer, NULL);
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
