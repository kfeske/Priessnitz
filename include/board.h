#include <string>

#include <utility.h>
#include <pre_computed.h>

// Zobrist Hashing is an efficient way of storing a chess position
// it is used as an index to the transposition table.
// This was, we can easily determine, if the position has been reached before.

// also used to check for threefold-repetitions.

struct Zobrist
{
	uint64_t piece_side_key; // here, we keep track of the piece positions and side to move
	uint64_t key; // final zobrist key  (we add the castling rights and en passant file, only if needed)

	uint64_t piece_rand[16][64];
	uint64_t ep_rand[8];
	uint64_t castling_rand[16];
	uint64_t side_rand;

	void populate()
	{
		srandom(9);

		// a chess position consists of...

		// ...piece positions
		for (Piece p : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING }) {
			for (unsigned square = 0; square <= 63; square++)
				piece_rand[p][square] = random_64();
		}

		// ...en passant file
		for (unsigned file = 0; file < 8; file++)
			ep_rand[file] = random_64();

		// ...castling rights
		for (unsigned rights = 0; rights < 16; rights++)
			castling_rand[rights] = random_64();

		// ...side to move
		side_rand = random_64();
	}

};

// used in unmake_move()
// stores essential position data that cannot be restored

struct UndoInfo {
	Piece captured = NO_PIECE;
	uint8_t ep_sq = SQ_NONE; // save en passant square in case the last move was a double pawn push
	uint8_t castling_rights;
	unsigned rule_50;

	UndoInfo()
	:
		captured(NO_PIECE), ep_sq(SQ_NONE), castling_rights(NO_RIGHTS), rule_50(0)
	{}
};

struct Board_state
{

	Piece board[64];
	uint64_t pieces[15];

	uint64_t color[2];
	uint64_t occ;

	Color side_to_move = WHITE;

	unsigned game_ply;

	UndoInfo history[256];
	uint64_t position_history[256];
	bool repetition;

	int non_pawn_material[2];
};

struct Board : Board_state
{
	PreComputed precomputed;
	Zobrist zobrist;

	uint64_t enemy(Color col) { return color[!col]; }
	uint64_t enemy_or_empty(Color col) { return ~color[col]; }

	// resets information neccessary to start a new game
	void reset()
	{
		static_cast<Board_state&>(*this) = {};
	}

	void add_piece(unsigned square, Piece p)
	{
		Color c = color_of(p);

		set_bit(pieces[p], square);
		set_bit(color[c], square);
		occ = color[WHITE] | color[BLACK];
		board[square] = p;
		non_pawn_material[c] += non_pawn_value(p);
		zobrist.piece_side_key ^= zobrist.piece_rand[p][square];
	}

	void remove_piece(unsigned square)
	{
		Piece p = board[square];
		Color c = color_of(p);

		pop_bit(pieces[p], square);
		pop_bit(color[c], square);
		occ = color[WHITE] | color[BLACK];
		board[square] = NO_PIECE;
		non_pawn_material[c] -= non_pawn_value(p);
		zobrist.piece_side_key ^= zobrist.piece_rand[p][square];
	}

	void push_piece_quiet(unsigned from, unsigned to)
	{
		Piece p = board[from];
		Color c = color_of(p);

		uint64_t mask = 1ULL << from | 1ULL << to;
		pieces[p] ^= mask;
		color[c] ^= mask;
		occ = color[WHITE] | color[BLACK];

		board[to]   = p;
		board[from] = NO_PIECE;
		zobrist.piece_side_key ^= zobrist.piece_rand[p][from];
		zobrist.piece_side_key ^= zobrist.piece_rand[p][to];
	}

	void make_move(Move move)
	{
		if (move == INVALID_MOVE) std::cerr << "attempted to play invalid move\n";
		game_ply++;
		history[game_ply] = UndoInfo();
		unsigned from = move_from(move);
		unsigned to   = move_to(move);

		// update castling rights
		// the corresponing castling is illegal if the move was made on a rook or king square
		history[game_ply].castling_rights = history[game_ply - 1].castling_rights &
						    precomputed.prohibiters[from] & precomputed.prohibiters[to];
		history[game_ply].rule_50 = history[game_ply - 1].rule_50 + 1;

		zobrist.key = 0ULL;

		if (type_of(board[move_from(move)]) == PAWN)
			history[game_ply].rule_50 = 0;

		MoveFlags flags = flags_of(move);

		switch(flags) {
		case QUIET:
			push_piece_quiet(from, to);
			break;
		case DOUBLE_PUSH:
			{
			Direction up = (side_to_move == WHITE) ? NORTH : SOUTH;
			history[game_ply].ep_sq = from + up;	// set en passant square
			zobrist.key ^= zobrist.ep_rand[file(from)];
			}
			push_piece_quiet(from, to);
			break;
		case CAPTURE:
			history[game_ply].rule_50 = 0;
			history[game_ply].captured = board[to]; // remember captured piece
			remove_piece(to);
			push_piece_quiet(from, to);
			break;
		case EP_CAPTURE:
			{
			Direction up = (side_to_move == WHITE) ? SOUTH : NORTH;
			push_piece_quiet(from, to);
			remove_piece(to + up);
			}
			break;
		case OO:
			history[game_ply].rule_50 = 0;
			push_piece_quiet(from, to); // king
			push_piece_quiet(to + 1, to - 1); // rook
			break;
		case OOO:
			history[game_ply].rule_50 = 0;
			push_piece_quiet(from, to); // king
			push_piece_quiet(to - 2, to + 1); // rook
			break;
		case PR_KNIGHT:
			remove_piece(from);
			add_piece(to, piece_of(side_to_move, KNIGHT));
			break;
		case PR_BISHOP:
			remove_piece(from);
			add_piece(to, piece_of(side_to_move, BISHOP));
			break;
		case PR_ROOK:
			remove_piece(from);
			add_piece(to, piece_of(side_to_move, ROOK));
			break;
		case PR_QUEEN:
			remove_piece(from);
			add_piece(to, piece_of(side_to_move, QUEEN));
			break;
		case PC_KNIGHT:
			history[game_ply].captured = board[to];
			remove_piece(from);
			remove_piece(to);
			add_piece(to, piece_of(side_to_move, KNIGHT));
			break;
		case PC_BISHOP:
			history[game_ply].captured = board[to];
			remove_piece(from);
			remove_piece(to);
			add_piece(to, piece_of(side_to_move, BISHOP));
			break;
		case PC_ROOK:
			history[game_ply].captured = board[to];
			remove_piece(from);
			remove_piece(to);
			add_piece(to, piece_of(side_to_move, ROOK));
			break;
		case PC_QUEEN:
			history[game_ply].captured = board[to];
			remove_piece(from);
			remove_piece(to);
			add_piece(to, piece_of(side_to_move, QUEEN));
			break;
		default: return;
		}

		zobrist.piece_side_key ^= zobrist.side_rand;
		zobrist.key ^= zobrist.piece_side_key;
		zobrist.key ^= zobrist.castling_rand[history[game_ply].castling_rights];


		// check for any three-fold-repetitions

		repetition = false;
		if (history[game_ply].rule_50 >= 4) { // a move that resets the 50 move rule also prevents repetitions.
						      // We do not need to search them in the whole history
			unsigned cycles = 0;
			int end = int(game_ply) - int(history[game_ply].rule_50);
			for (int i = game_ply; i >= end; i -= 2) {
				if (position_history[i] == zobrist.key)
					cycles++;
			}
			if (cycles >= 2) repetition = true;
		}

		position_history[game_ply] = zobrist.key;

		side_to_move = Color(!side_to_move);
	}

	void unmake_move(Move move)
	{
		UndoInfo info = history[game_ply];
		position_history[game_ply] = 0ULL;
		game_ply--;
		unsigned from = move_to(move);
		unsigned to   = move_from(move);
		MoveFlags flags = flags_of(move);

		zobrist.key = 0ULL;
		
		switch(flags) {
		case QUIET:
			push_piece_quiet(from, to);
			break;
		case DOUBLE_PUSH:
			push_piece_quiet(from, to);
			break;
		case CAPTURE:
			push_piece_quiet(from, to);
			add_piece(from, info.captured);
			break;
		case EP_CAPTURE:
			push_piece_quiet(from, to);
			{
			Direction up = (side_to_move == WHITE) ? NORTH : SOUTH;
			add_piece(from + up, piece_of(side_to_move, PAWN));
			}
			break;
		case OO:
			push_piece_quiet(from, to); // king
			push_piece_quiet(from - 1, from + 1); // rook
			break;
		case OOO:
			push_piece_quiet(from, to); // king
			push_piece_quiet(from + 1, from - 2); // rook
			break;
		case PR_KNIGHT: case PR_BISHOP: case PR_ROOK: case PR_QUEEN:
			remove_piece(from);
			add_piece(to, piece_of(!side_to_move, PAWN));
			break;
		case PC_KNIGHT: case PC_BISHOP: case PC_ROOK: case PC_QUEEN:
			remove_piece(from);
			add_piece(to, piece_of(!side_to_move, PAWN));
			add_piece(from, info.captured);
			break;
		default: return;
		}

		zobrist.piece_side_key ^= zobrist.side_rand;
		zobrist.key ^= zobrist.piece_side_key;
		zobrist.key ^= zobrist.castling_rand[history[game_ply].castling_rights];
		if (history[game_ply].ep_sq != SQ_NONE)
			zobrist.key ^= zobrist.ep_rand[file(history[game_ply].ep_sq)];

		repetition = false;
		side_to_move = Color(!side_to_move);
	}
	
	unsigned make_null_move()
	{
		side_to_move = Color(!side_to_move);
		zobrist.piece_side_key ^= zobrist.side_rand;
		zobrist.key ^= zobrist.side_rand;

		unsigned ep = history[game_ply].ep_sq;
		if (ep != SQ_NONE) {
			history[game_ply].ep_sq = SQ_NONE;
			zobrist.key ^= zobrist.ep_rand[file(ep)];
		}

		// returns a square to restore the ep square when we undo the null move
		return ep;
	}

	void unmake_null_move(unsigned ep)
	{
		side_to_move = Color(!side_to_move);
		zobrist.piece_side_key ^= zobrist.side_rand;
		zobrist.key ^= zobrist.side_rand;

		if (ep != SQ_NONE) {
			history[game_ply].ep_sq = ep;
			zobrist.key ^= zobrist.ep_rand[file(ep)];
		}
	}


	bool in_check()
	{
		unsigned ksq = lsb(pieces[piece_of(side_to_move, KING)]);
		uint64_t enemy_diag_sliders = pieces[piece_of(!side_to_move, BISHOP)] | pieces[piece_of(!side_to_move, QUEEN)];
		uint64_t enemy_orth_sliders = pieces[piece_of(!side_to_move, ROOK  )] | pieces[piece_of(!side_to_move, QUEEN)];

		uint64_t checkers = 0ULL;
		checkers |= precomputed.pawn_attacks[side_to_move][ksq] & pieces[piece_of(!side_to_move, PAWN)];
		checkers |= precomputed.attacks_bb<KNIGHT>(ksq, 0ULL) & pieces[piece_of(!side_to_move, KNIGHT)];
		checkers |= precomputed.attacks_bb<BISHOP>(ksq, occ) & enemy_diag_sliders;
		checkers |= precomputed.attacks_bb<ROOK  >(ksq, occ) & enemy_orth_sliders;

		return (checkers != 0);
	}

	void set_fenpos(std::string fen)
	{
		history[0] = UndoInfo();
		
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

				// castling rights
				else if (character == 'K') history[0].castling_rights |= WHITE_OO;
				else if (character == 'Q') history[0].castling_rights |= WHITE_OOO;
				else if (character == 'k') history[0].castling_rights |= BLACK_OO;
				else if (character == 'q') history[0].castling_rights |= BLACK_OOO;
			}
		}

		color[WHITE] = pieces[W_PAWN] | pieces[W_KNIGHT] | pieces[W_BISHOP] |
				     pieces[W_ROOK] | pieces[W_QUEEN]  | pieces[W_KING];
		color[BLACK] = pieces[B_PAWN] | pieces[B_KNIGHT] | pieces[B_BISHOP] |
				     pieces[B_ROOK] | pieces[B_QUEEN]  | pieces[B_KING];
		occ = color[WHITE] | color[BLACK];

		zobrist.populate();

		// array

		for (unsigned square = 0; square < 64; square++)
			board[square] = NO_PIECE;

		for (unsigned p = W_PAWN; p <= B_KING; p++) {
			uint64_t bb = pieces[p];
			while (bb) {
				Piece pc = Piece(p);
				unsigned square = pop_lsb(bb);
				board[square] = pc;
				non_pawn_material[color_of(pc)] += non_pawn_value(pc);
				zobrist.piece_side_key ^= zobrist.piece_rand[pc][square];
			}
		}
	}

	void set_startpos()
	{
		set_fenpos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
