#include "pre_computed.h"
#include "utility.h"

bool edge(unsigned square, Direction d)
{
	switch(d) {
	case UP: 	 return (rank_num(square) == 0);
	case DOWN: 	 return (rank_num(square) == 7);
	case LEFT:  	 return (file_num(square) == 0);
	case RIGHT:  	 return (file_num(square) == 7);
	case UP_LEFT: 	 return (rank_num(square) == 0 || file_num(square) == 0);
	case UP_RIGHT: 	 return (rank_num(square) == 0 || file_num(square) == 7);
	case DOWN_LEFT:  return (rank_num(square) == 7 || file_num(square) == 0);
	case DOWN_RIGHT: return (rank_num(square) == 7 || file_num(square) == 7);
	default: return false;
	}
}

// Precomputed attack data for the leaper pieces (Pawns, Knight, King)
template <Color color>
uint64_t generate_pawn_attacks(unsigned square)
{
	uint64_t pos = 1ULL << square;
	uint64_t attacks = 0ULL;
	if (color == WHITE) {
		if ((pos >> 7) & ~FILE_A) attacks |= pos >> 7;
		if ((pos >> 9) & ~FILE_H) attacks |= pos >> 9;
	}
	else {
		if ((pos << 7) & ~FILE_H) attacks |= pos << 7;
		if ((pos << 9) & ~FILE_A) attacks |= pos << 9;
	}
	return attacks;
}

uint64_t generate_knight_attacks(unsigned square)
{
	uint64_t pos = 1ULL << square;
	uint64_t attacks = 0ULL;
	if ((pos >> 10) & ~(FILE_G | FILE_H)) attacks |= pos >> 10;
	if ((pos << 6)  & ~(FILE_G | FILE_H)) attacks |= pos <<  6;
	if ((pos >> 17) & ~FILE_H           ) attacks |= pos >> 17;
	if ((pos << 15) & ~FILE_H           ) attacks |= pos << 15;
	if ((pos >> 15) & ~FILE_A           ) attacks |= pos >> 15;
	if ((pos << 17) & ~FILE_A           ) attacks |= pos << 17;
	if ((pos >> 6)  & ~(FILE_A | FILE_B)) attacks |= pos >>  6;
	if ((pos << 10) & ~(FILE_A | FILE_B)) attacks |= pos << 10;
	return attacks;
}

uint64_t generate_king_attacks(unsigned square)
{
	uint64_t pos = 1ULL << square;
	uint64_t attacks = 0ULL;
	if (pos << 8)		  attacks |= pos << 8;
	if ((pos << 1) & ~FILE_A) attacks |= pos << 1;
	if ((pos << 7) & ~FILE_H) attacks |= pos << 7;
	if ((pos << 9) & ~FILE_A) attacks |= pos << 9;
	if (pos >> 8)		  attacks |= pos >> 8;
	if ((pos >> 1) & ~FILE_H) attacks |= pos >> 1;
	if ((pos >> 7) & ~FILE_A) attacks |= pos >> 7;
	if ((pos >> 9) & ~FILE_H) attacks |= pos >> 9;
	return attacks;
}

// Sliding attacks on the fly!
// Using this function in the move generator would be too slow, hence all the magic bitboard stuff is needed
uint64_t generate_sliding_attacks(Piece_type type, unsigned square, uint64_t occ)
{
	uint64_t attacks = 0ULL;
	Direction direction_rook[4]   = {UP, DOWN, RIGHT, LEFT};
	Direction direction_bishop[4] = {UP_RIGHT, DOWN_RIGHT, DOWN_LEFT, UP_LEFT};

	for (Direction d : (type == ROOK) ? direction_rook : direction_bishop) {
		unsigned s = square;
		uint64_t target_square = 0ULL;

		while (true) {
			attacks |= target_square;
			if (edge(s, d)) break;
			s += d;
			if (occ & target_square) break;
			target_square = 1ULL << s;
		}
	}
	return attacks;
}

// Returns one combination of set bits in a mask for an index
// used for looping over possible blockers to hash sliding attacks
uint64_t occupancy(unsigned index, unsigned bits_in_mask, uint64_t mask)
{
	uint64_t occ = 0ULL;
	for (unsigned i = 0; i < bits_in_mask; i++) {
		unsigned square = lsb(mask);
		pop_bit(mask, square);
		
		if (index & (1 << i))
			occ |= 1ULL << square;
	}
	return occ;
}

// Fills the lookup tables of the sliding moves for the corresponding magic number
void Pre_computed::cast_magics(Piece_type type)
{
	for (unsigned square = 0; square < 64; square++) {

		uint64_t edges = ((RANK_1 | RANK_8) & ~rank(square)) | ((FILE_A | FILE_H) & ~file(square));

		// The edges of the board are not considered, a piece on the edge cannot block a sliding piece
		uint64_t ray_mask = generate_sliding_attacks(type, square, 0ULL) & ~edges;

		// This is important later, when we want to quickly calculate the magic index
		(type == BISHOP) ? bishop_mask[square] = ray_mask : rook_mask[square] = ray_mask;

		unsigned bits_in_mask = pop_count(ray_mask);
		unsigned possible_occupancies = 1ULL << bits_in_mask;

		// Loop through every possible constellation of occupancies for the squares, the piece can move to
		for (unsigned i = 0; i < possible_occupancies; i++) {

			uint64_t occ = occupancy(i, bits_in_mask, ray_mask);

			// Compute the magic index (the hash key)
			uint64_t magic_number = (type == BISHOP) ? bishop_magics[square] : rook_magics[square];
			uint8_t shift = (type == BISHOP) ? bishop_shifts[square] : rook_shifts[square];

			unsigned magic_index = (occ * magic_number) >> shift;

			// Now, the hash tables are filled for this constellation
			(type == BISHOP) ? bishop_hash[square][magic_index] = generate_sliding_attacks(BISHOP, square, occ) :
					   rook_hash[square][magic_index]   = generate_sliding_attacks(ROOK, square, occ);
		}
	}
}

// Populates the lookup tables for the attacks of each piece
// and the magic numbers for the sliding pieces
Pre_computed generate_lookup()
{
	Pre_computed p;
	for (unsigned square = 0; square < 64; square++) {
		p.pawn_attacks[WHITE][square] = generate_pawn_attacks<WHITE>(square);
		p.pawn_attacks[BLACK][square] = generate_pawn_attacks<BLACK>(square);
		p.knight_attacks[square]      = generate_knight_attacks(square);
		p.king_attacks[square]        = generate_king_attacks(square);
		p.cast_magics(BISHOP);
		p.cast_magics(ROOK);

		for (unsigned square2 = 0; square2 <= 63; square2++) {
			for (Piece_type type : { BISHOP, ROOK }) {
				if (generate_sliding_attacks(type, square, 0ULL) & 1ULL << square2) {
					p.ray_between[square][square2] = generate_sliding_attacks(type, square, 1ULL << square2) &
								         generate_sliding_attacks(type, square2, 1ULL << square);
					p.ray[square][square2] = (generate_sliding_attacks(type, square,  0ULL) &
								  generate_sliding_attacks(type, square2, 0ULL)) |
								 (1ULL << square) | (1ULL << square2);
				}
			}
			p.ray_between[square][square2] |= 1ULL << square2;

			p.rank_distance[square][square2] = abs(rank_num(square) - rank_num(square2));
			p.file_distance[square][square2] = abs(file_num(square) - file_num(square2));
			p.square_distance[square][square2] = std::max(p.rank_distance[square][square2], p.file_distance[square][square2]);
		}
	}
	// Evaluation tables
	for (unsigned square = 0; square < 64; square++) {

		// The file of the square, and the two adjacent files
		uint64_t adjacent_mask = file(square);
		adjacent_mask |= (adjacent_mask & ~FILE_A) >> 1;
		adjacent_mask |= (adjacent_mask & ~FILE_H) << 1;

		// All rows in front of the square row
		uint64_t all_mask = ~0ULL;
		uint64_t white_forward_mask = (rank_num(square) > 0) ? all_mask >> 8 * (8 - rank_num(square)) : 0;
		uint64_t black_forward_mask = (rank_num(square) < 7) ? all_mask << 8 * (rank_num(square) + 1) : 0;

		// A pawn is passed if there are no enemy pawns on the same or the adjacent files in front of it
		p.passed_pawn_mask[WHITE][square] = white_forward_mask & adjacent_mask;
		p.passed_pawn_mask[BLACK][square] = black_forward_mask & adjacent_mask;

		// Forward_file_mask is used for detecting doubled or blocked pawns
		p.forward_file_mask[WHITE][square] = file(square) & white_forward_mask;
		p.forward_file_mask[BLACK][square] = file(square) & black_forward_mask;

		// A pawn is isolated, if there are no pawns on the neighbor files
		p.isolated_pawn_mask[file_num(square)] = adjacent_mask & ~file(square);

		// Mask of the left and right neighbor squares
		p.neighbor_mask[square] = adjacent_mask & rank(square) & ~(1ULL << square);

		// Used to check if an opponent pawn might easily attack the square
		p.pawn_threat_mask[WHITE][square] = white_forward_mask & adjacent_mask & ~file(square);
		p.pawn_threat_mask[BLACK][square] = black_forward_mask & adjacent_mask & ~file(square);

		// The king ring consists of the squares, the king attacks. If the king is on the edge, the ring is extended a bit.
		unsigned ring_center = square;
		//if (file_num(square) == 0) ring_center += RIGHT;
		//if (file_num(square) == 7) ring_center += LEFT;
		//if (rank_num(square) == 0) ring_center += DOWN;
		//if (rank_num(square) == 7) ring_center += UP;
		p.king_ring_mask[square] = p.king_attacks[ring_center] | (1ULL << ring_center);

		// Pawn Shelter consists of pawns in front of the king
		if (square >= 8)  p.pawn_shield[WHITE][square] = p.king_attacks[square - 8] | 1ULL << (square - 8);
		if (square <= 55) p.pawn_shield[BLACK][square] = p.king_attacks[square + 8] | 1ULL << (square + 8);
	}
	p.outpost_mask[WHITE] = RANK_5 | RANK_6 | RANK_7;
	p.outpost_mask[BLACK] = RANK_4 | RANK_3 | RANK_2;

	return p;
}

Pre_computed const pre_computed = generate_lookup();
