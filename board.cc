#include <string>
#include <sstream>

#include "board.h"

#include "utility.h"
#include "pre_computed.h"

// Zobrist Hashing is an efficient way of storing a chess position
// it is used as an index to the transposition table.
// This way, we can easily determine, if the position has been reached before.

// also used to check for threefold-repetitions.


void Zobrist::populate()
{
	srandom(9);

	// a chess position consists of...

	// ...piece positions
	for (Color color : { WHITE, BLACK }) {
		for (Piece piece : ALL_PIECES[color]) {
			for (unsigned square = 0; square <= 63; square++)
				piece_rand[piece][square] = random_64();
		}
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

void Board::make_move(Move move)
{
	if (move == INVALID_MOVE) std::cerr << "attempted to play invalid move\n";
	game_ply++;
	history[game_ply] = {};
	unsigned from = move_from(move);
	unsigned to   = move_to(move);

	// update castling rights
	// the corresponing castling is illegal if the move was made on a rook or king square
	history[game_ply].castling_rights = history[game_ply - 1].castling_rights &
					    prohibiters(from) & prohibiters(to);
	history[game_ply].rule_50 = history[game_ply - 1].rule_50 + 1;

	history[game_ply].move = move;

	zobrist.key = 0ULL;

	if (type_of(board[move_from(move)]) == PAWN)
		history[game_ply].rule_50 = 0;

	Move_flags flags = flags_of(move);

	switch (flags) {
	case QUIET:
		push_piece_quiet(from, to);
		break;
	case DOUBLE_PUSH:
		{
		Direction up = (side_to_move == WHITE) ? UP : DOWN;
		history[game_ply].ep_sq = from + up;	// set en passant square
		zobrist.key ^= zobrist.ep_rand[file_num(from)];
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
		Direction up = (side_to_move == WHITE) ? DOWN : UP;
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

	// Check for any three-fold-repetitions

	repetition = false;
	if (history[game_ply].rule_50 >= 4) { // a move that resets the 50 move rule also prevents repetitions.
					      // We do not need to search them in the whole history
		int end = int(game_ply) - int(history[game_ply].rule_50);
		for (int i = game_ply; i >= end; i -= 2) {
			if (position_history[i] == zobrist.key) {
				// Two- and three-fold repetitions are both treated as a draw in the search
				repetition = true;
				break;
			}
		}
	}

	position_history[game_ply] = zobrist.key;

	side_to_move = swap(side_to_move);

	update_checkers_and_pinners();
}

void Board::unmake_move(Move move)
{
	Undo_info info = history[game_ply];
	position_history[game_ply] = 0ULL;
	game_ply--;
	unsigned from = move_to(move);
	unsigned to   = move_from(move);
	Move_flags flags = flags_of(move);

	zobrist.key = 0ULL;
	
	switch (flags) {
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
		Direction up = (side_to_move == WHITE) ? UP : DOWN;
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
		add_piece(to, piece_of(swap(side_to_move), PAWN));
		break;
	case PC_KNIGHT: case PC_BISHOP: case PC_ROOK: case PC_QUEEN:
		remove_piece(from);
		add_piece(to, piece_of(swap(side_to_move), PAWN));
		add_piece(from, info.captured);
		break;
	default: return;
	}

	zobrist.piece_side_key ^= zobrist.side_rand;
	zobrist.key ^= zobrist.piece_side_key;
	zobrist.key ^= zobrist.castling_rand[history[game_ply].castling_rights];
	if (history[game_ply].ep_sq != NO_SQUARE)
		zobrist.key ^= zobrist.ep_rand[file_num(history[game_ply].ep_sq)];

	repetition = false;
	side_to_move = swap(side_to_move);
}

// useful for Null Move Pruning
unsigned Board::make_null_move()
{
	side_to_move = swap(side_to_move);
	zobrist.piece_side_key ^= zobrist.side_rand;
	zobrist.key ^= zobrist.side_rand;

	unsigned ep = history[game_ply].ep_sq;
	if (ep != NO_SQUARE) {
		history[game_ply].ep_sq = NO_SQUARE;
		zobrist.key ^= zobrist.ep_rand[file_num(ep)];
	}

	update_checkers_and_pinners();

	// returns a square to restore the ep square when we undo the null move
	return ep;
}

void Board::unmake_null_move(unsigned ep)
{
	side_to_move = swap(side_to_move);
	zobrist.piece_side_key ^= zobrist.side_rand;
	zobrist.key ^= zobrist.side_rand;

	if (ep != NO_SQUARE) {
		history[game_ply].ep_sq = ep;
		zobrist.key ^= zobrist.ep_rand[file_num(ep)];
	}
}

// is the side to move in check?
void Board::update_checkers_and_pinners()
{
	if (pieces(side_to_move, KING) == 0ULL) {
		std::cerr << "no king -> retracing steps...\n";
		for (unsigned ply = 1; ply <= game_ply; ply++) {
			std::cerr << move_string(history[ply].move) << " ";
		}
		std::cerr << "\n";
		abort();
		return;
	}
	Color friendly = side_to_move;
	Color enemy = swap(friendly);
	unsigned king_square = square(side_to_move, KING);
	uint64_t enemy_diag_sliders = pieces(enemy, BISHOP) | pieces(enemy, QUEEN);
	uint64_t enemy_orth_sliders = pieces(enemy, ROOK  ) | pieces(enemy, QUEEN);

	history[game_ply].checkers |= pawn_attacks(side_to_move, king_square)  & pieces(enemy, PAWN);
	history[game_ply].checkers |= piece_attacks(KNIGHT, king_square, 0ULL) & pieces(enemy, KNIGHT);

	uint64_t king_sliders = (piece_attacks(BISHOP, king_square, pieces(enemy)) & enemy_diag_sliders) |
			        (piece_attacks(ROOK,   king_square, pieces(enemy)) & enemy_orth_sliders);
	while (king_sliders) {
		unsigned square = pop_lsb(king_sliders);
		uint64_t pin_ray = ray_between(king_square, square) & pieces(friendly);

		// No pieces block the ray - our king is in check.
		if (pin_ray == 0) history[game_ply].checkers |= 1ULL << square;

		// One of our pieces blocks the ray - the piece is pinned.
		else if ((pin_ray & (pin_ray - 1)) == 0) history[game_ply].pinned |= pin_ray;
	}
}

// Pseudo legal moves are checked for legality by making sure to restrict pinned pieces and not allowing the king
// to move into a check.
bool Board::legal(Move move)
{
	unsigned from = move_from(move);
	unsigned to   = move_to(move);
	if (type_of(board[from]) == KING) {
		// The king cannot move to attacked squares.
		if (square_attackers(to, occ & ~(1ULL << from)) & pieces(swap(side_to_move))) return false;

		// Castling through attacks is not allowed.
		Move_flags flag = flags_of(move);
		if (flag == OO) {
			if ((square_attackers(from + 1, occ) & pieces(swap(side_to_move))) != 0ULL) return false;
		}
		else if (flag == OOO) {
			if ((square_attackers(from - 1, occ) & pieces(swap(side_to_move))) != 0ULL) return false;
		}
	}
	// Restrict pinned pieces to only move along the ray away or towards the king.
	else if ((history[game_ply].pinned & (1ULL << from)) && !(ray(square(side_to_move, KING), from) & (1ULL << to)))
		return false;

	// En passant captures have a tricky case, where a horizontal check can be revealed.
	else if (flags_of(move) == EP_CAPTURE) {
		Color enemy = swap(side_to_move);
		unsigned king_square = square(side_to_move, KING);
		unsigned ep_square = history[game_ply].ep_sq;
		Direction forward = side_to_move == WHITE ? UP : DOWN;
		if (piece_attacks(ROOK, king_square, occ & ~(1ULL << from) & ~(1ULL << (ep_square - forward))) &
		    rank(king_square) & (pieces(enemy, ROOK) | pieces(enemy, QUEEN))) return false;
	}
	return true;
}

// Can this move be played in this position, if we do not care about pins and checks?
bool Board::pseudo_legal(Move move)
{
	if (move == INVALID_MOVE) return false;

	unsigned from = move_from(move);
	unsigned to   = move_to(move);
	Piece moving_piece   = board[from];
	Piece captured_piece = board[to];
	Move_flags flag = flags_of(move);

	if (moving_piece   == NO_PIECE || color_of(moving_piece)   != side_to_move) return false;
	if (flag == QUIET && captured_piece != NO_PIECE) return false;
	if (capture(move) && flag != EP_CAPTURE && (captured_piece == NO_PIECE || color_of(captured_piece) == side_to_move)) return false;

	// The piece must be able to go to the destination.
	if (type_of(moving_piece) == PAWN) {
		Direction forward         = (side_to_move == WHITE) ? UP : DOWN;
		unsigned double_push_rank = (side_to_move == WHITE) ? 6  : 1;
		unsigned promotion_rank   = (side_to_move == WHITE) ? 1  : 6;
		// En passant captures
		if (flag == EP_CAPTURE) {
			unsigned ep_square = history[game_ply].ep_sq;
			if (ep_square == NO_SQUARE || !(pawn_attacks(side_to_move, from) & (1ULL << to) & (1ULL << ep_square))) return false;
		}
		// Normal pawn captures and promotions
		else if (capture(move) && !(pawn_attacks(side_to_move, from) & (1ULL << to) & pieces(swap(side_to_move))))
			return false;
		// Quiet pawn moves
		if ((flag == QUIET || (promotion(move) && !capture(move))) && !((from + forward == to) && board[to] == NO_PIECE))
			return false;
		// Double pushes
		if (flag == DOUBLE_PUSH && !((from + 2 * forward == to) && board[to] == NO_PIECE &&
			 board[from + forward] == NO_PIECE && rank_num(from) == double_push_rank))
			return false;
		// For a promotion, the pawn must be on the 7th rank.
		if (promotion(move)) {
				if (rank_num(from) != promotion_rank) return false;
		}
		else if (rank_num(from) == promotion_rank) return false;
	}
	else {
		if (promotion(move) || flag == DOUBLE_PUSH || flag == EP_CAPTURE) return false;
		if (flag != OO && flag != OOO && !(piece_attacks(type_of(moving_piece), from, occ) & (1ULL << to))) return false;
	}

	if (flag == OO || flag == OOO) {
		if (type_of(moving_piece) == KING && !history[game_ply].checkers) {
			// The castling path must not be obstructed.
			uint8_t castling_rights = history[game_ply].castling_rights;
			unsigned king_square = square(side_to_move, KING);
			if (flag == OO && to == king_square + 2) {
				if (!(castling_rights & king_side[side_to_move])  || (oo_blockers(side_to_move)  & occ)) return false;
			}
			else if (flag == OOO && to == king_square - 2) {
			       	if (!(castling_rights & queen_side[side_to_move]) || (ooo_blockers(side_to_move) & occ)) return false;
			}
			else return false;
		}
		else return false;
	}

	// In check, there are some corner cases, which are handled in the move generator.
	if (history[game_ply].checkers) {
		if (type_of(moving_piece) != KING) {
			// In double check, only the king can move.
			if (pop_count(history[game_ply].checkers) >= 2) return false;

			// Moves that do not block a check, are illegal.
			if (!(ray_between(square(side_to_move, KING), lsb(history[game_ply].checkers)) & (1ULL << to)))
				return false;
		}
	}
	return true;
}

// used in the SEE function
uint64_t Board::all_pawn_attacks(Color friendly)
{
	Direction up_right = (friendly == WHITE) ? UP_RIGHT : DOWN_RIGHT;
	Direction up_left  = (friendly == WHITE) ? UP_LEFT : DOWN_LEFT;
	uint64_t pawns = pieces(friendly, PAWN);
	return shift(pawns & ~FILE_A, up_left) | shift(pawns & ~FILE_H, up_right);
}

// Detect completely drawn positions KvK, KvKN, KvKB, KvKNN
bool Board::insufficient_material()
{
	return !(pieces(PAWN) || pieces(ROOK) || pieces(QUEEN)) &&
		(pop_count(occ) <= 3 || (pop_count(occ) == 4 && pop_count(pieces(KNIGHT)) == 2));
}

void Board::set_fenpos(std::string fen)
{
	reset(); // reset board to avoid overwrites
	history[0] = {};
	zobrist.piece_side_key = 0ULL;

	std::istringstream iss { fen };
	std::string parsed {};

	// Piece positions
	iss >> parsed;
	unsigned square = 0;
	for (char &character : parsed) {
		if (character == '/') continue;

		else if (character == 'P') add_piece(square, W_PAWN);
		else if (character == 'N') add_piece(square, W_KNIGHT);
		else if (character == 'B') add_piece(square, W_BISHOP);
		else if (character == 'R') add_piece(square, W_ROOK);
		else if (character == 'Q') add_piece(square, W_QUEEN);
		else if (character == 'K') add_piece(square, W_KING);

		else if (character == 'p') add_piece(square, B_PAWN);
		else if (character == 'n') add_piece(square, B_KNIGHT);
		else if (character == 'b') add_piece(square, B_BISHOP);
		else if (character == 'r') add_piece(square, B_ROOK);
		else if (character == 'q') add_piece(square, B_QUEEN);
		else if (character == 'k') add_piece(square, B_KING);

		else {
			square += std::atoi(&character);
			continue;
		}
		square++;
	}

	// Side to move
	iss >> parsed;
	side_to_move = (parsed == "w") ? WHITE : BLACK;
	if (side_to_move == BLACK) zobrist.piece_side_key ^= zobrist.side_rand;

	zobrist.key = zobrist.piece_side_key;

	// Castling rights
	if (iss >> parsed) {
		for (char &character : parsed) {
			if (character == 'K') history[0].castling_rights |= WHITE_OO;
			if (character == 'Q') history[0].castling_rights |= WHITE_OOO;
			if (character == 'k') history[0].castling_rights |= BLACK_OO;
			if (character == 'q') history[0].castling_rights |= BLACK_OOO;
		}
		zobrist.key ^= zobrist.castling_rand[history[game_ply].castling_rights];
	}

	// En passant square
	if (iss >> parsed) {
		char &character = parsed[0];
		if (character >= 'a' && character <= 'h') {
			unsigned file = unsigned(character - 'a');
			unsigned rank = 8 - std::atoi(&parsed[1]);
			
			history[game_ply].ep_sq = rank * 8 + file;
			zobrist.key ^= zobrist.ep_rand[file_num(history[game_ply].ep_sq)];
		}
	}

	// Halfmove clock
	if (iss >> parsed) {
		history[game_ply].rule_50 = std::atoi(&parsed[0]);
	}

	update_checkers_and_pinners();
}

void Board::set_startpos()
{
	set_fenpos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

std::string piece_string(Board &board, unsigned square)
{
	uint64_t square_bb = 1ULL << square;
	if (board.pieces(WHITE, PAWN)   & square_bb) return "P";
	if (board.pieces(BLACK, PAWN)   & square_bb) return "p";
	if (board.pieces(WHITE, KNIGHT) & square_bb) return "N";
	if (board.pieces(BLACK, KNIGHT) & square_bb) return "n";
	if (board.pieces(WHITE, BISHOP) & square_bb) return "B";
	if (board.pieces(BLACK, BISHOP) & square_bb) return "b";
	if (board.pieces(WHITE, ROOK)   & square_bb) return "R";
	if (board.pieces(BLACK, ROOK)   & square_bb) return "r";
	if (board.pieces(WHITE, QUEEN)  & square_bb) return "Q";
	if (board.pieces(BLACK, QUEEN)  & square_bb) return "q";
	if (board.pieces(WHITE, KING)   & square_bb) return "K";
	if (board.pieces(BLACK, KING)   & square_bb) return "k";
	return " ";
}

void print_board(Board &board)
{
	std::string str = "    +---+---+---+---+---+---+---+---+\n";

	for (unsigned rank = 0; rank < 8; rank++) { 
		str += "  " + std::to_string(8 - rank) + " ";
		for (unsigned file = 0; file < 8; file++) {
			unsigned square = rank * 8 + file;
			str += "| " + piece_string(board, square) + " ";

		}
		str += "|\n    +---+---+---+---+---+---+---+---+\n";
	}
	str += "      A   B   C   D   E   F   G   H\n\n";
	std::cerr << str;
	std::cerr << "key " << board.zobrist.key << "\n";
}

Board::Board()
{
	set_startpos();
}
