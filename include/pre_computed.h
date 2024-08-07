#include <cstring>

struct PreComputed : Noncopyable
{
	uint64_t pawn_attacks[2][64];
	uint64_t knight_attacks[64];
	uint64_t king_attacks[64];
	Magic rook_magics[64];
	Magic bishop_magics[64];
	uint64_t bishop_relevant_mask[64];
	uint64_t rook_relevant_mask[64];
	uint64_t bishop_attacks[64][512];
	uint64_t rook_attacks[64][4096];

	// used for handling checks
	uint64_t between_bb[64][64] {};

	// to restrict movement of pinned pieces
	uint64_t line_bb[64][64] {};

	// castling
	uint8_t prohibiters[64] = {
		0x7, 0xf, 0xf, 0xf, 0x3, 0xf, 0xf, 0xb,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xd, 0xf, 0xf, 0xf, 0xc, 0xf, 0xf, 0xe,
	};
	uint64_t oo_blockers[2] = { 0x6000000000000000, 0x60 };
	uint64_t ooo_blockers[2] = { 0xE00000000000000, 0xE };
	uint64_t oooo = (1ULL << B1) | (1ULL << B8);

	uint64_t passed_pawn_mask[2][64];
	uint64_t forward_file_mask[2][64];
	uint64_t isolated_pawn_mask[8];

	uint64_t rank_bb(unsigned square)
	{
		unsigned index = square >> 3;
		return RANK_1 << (index * 8);
	}

	uint64_t file_bb(unsigned square)
	{
		unsigned index = square & 7;
		return FILE_A << index;
	}

	// precomputed attack data for the leaper pieces (Pawns, Knight, King)
	template <Color color>
	uint64_t pawn_attack_bb(unsigned square)
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

	uint64_t knight_attack_bb(unsigned square)
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

	uint64_t king_attack_bb(unsigned square)
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

	// sliding attacks on the fly!
	// using this function in the move generator would be too slow, hence all the magic stuff is needed
	uint64_t sliding_attack_bb(PieceType p, unsigned square, uint64_t occ)
	{
		uint64_t attacks = 0ULL;
		Direction direction_rook[4]   = {NORTH, SOUTH, EAST, WEST};
		Direction direction_bishop[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

		for (Direction d : (p == ROOK) ? direction_rook : direction_bishop) {
			unsigned s = square;
			uint64_t target_square = 0ULL;

			while (true) {
				attacks |= target_square;
				if (is_edge(s, d)) break;
				s += d;
				if (occ & target_square) break;
				target_square = 1ULL << s;
			}
		}
		return attacks;
	}

	// populates the occupancy bitboard
	uint64_t occupancy_for_index(unsigned index, unsigned bits_in_mask, uint64_t mask)
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

	// generates the magic numbers for sliding pieces for each square
	void setup_magic_tables(PieceType p, Magic magics[])
	{
		for (unsigned square = 0; square <= 63; square++) {
			Magic &m = magics[square];
			cast_magics(p, square, m);
		}
	}

	void cast_magics(PieceType p, unsigned square, Magic &m)
	{
		uint64_t occupancies[4096];
		uint64_t attacks[4096];
		uint64_t used_attacks[4096];
		uint64_t edges = ((RANK_1 | RANK_8) & ~rank_bb(square)) | ((FILE_A | FILE_H) & ~file_bb(square));
		uint64_t mask = sliding_attack_bb(p, square, 0ULL) & ~edges;
		unsigned bits_in_mask = pop_count(mask);
		m.shift = 64 - bits_in_mask;
		
		unsigned possible_occupancies = 1 << bits_in_mask;
		for (unsigned i = 0; i < possible_occupancies; i++) {
			occupancies[i] = occupancy_for_index(i, bits_in_mask, mask);
			attacks[i] = sliding_attack_bb(p, square, occupancies[i]);
		}

		while (true) {

			for (m.magic = 0; pop_count((m.magic * mask) >> 56) < 6;)
				m.magic = random_64() & random_64() & random_64();
			memset(used_attacks, 0ULL, sizeof(used_attacks));
			int fail = 0;
			unsigned i = 0;

			for (i = 0, fail = 0; !fail && i < possible_occupancies; i++) {
				unsigned magic_index = (occupancies[i] * m.magic) >> m.shift;

				if (used_attacks[magic_index] == 0ULL)
					used_attacks[magic_index] = attacks[i];
				else if (used_attacks[magic_index] != attacks[i])
					fail = 1;
			}
			if (!fail)
				return;
		}
	}

	// fills the lookup tables of the sliding moves for the corresponding magic number
	void sliding_attacks(PieceType p, unsigned square, uint64_t magic_number)
	{
		uint64_t edges = ((RANK_1 | RANK_8) & ~rank_bb(square)) | ((FILE_A | FILE_H) & ~file_bb(square));
		uint64_t mask = sliding_attack_bb(p, square, 0ULL) & ~edges;

		if (p == BISHOP) bishop_relevant_mask[square] = mask;
		if (p == ROOK)     rook_relevant_mask[square] = mask;

		unsigned bits_in_mask = pop_count(mask);
		unsigned possible_occupancies = 1 << bits_in_mask;

		for (unsigned i = 0; i < possible_occupancies; i++) {
			uint64_t occupancy = occupancy_for_index(i, bits_in_mask, mask);

			unsigned magic_index = (occupancy * magic_number) >> (64 - bits_in_mask);

			(p == BISHOP) ? bishop_attacks[square][magic_index] = sliding_attack_bb(p, square, occupancy) :
					  rook_attacks[square][magic_index] = sliding_attack_bb(p, square, occupancy);
		}
	}

	// populates the lookup tables for the attacks of each piece
	// and the magic numbers for the sliding pieces
	void generate_attacks()
	{
		srandom(30);
		setup_magic_tables(ROOK, rook_magics);
		setup_magic_tables(BISHOP, bishop_magics);

		for (unsigned square = 0; square <= 63; square++) {
			pawn_attacks[WHITE][square] = pawn_attack_bb<WHITE>(square);
			pawn_attacks[BLACK][square] = pawn_attack_bb<BLACK>(square);
			knight_attacks[square] = knight_attack_bb(square);
			king_attacks[square] = king_attack_bb(square);
			sliding_attacks(ROOK, square, rook_magics[square].magic);
			sliding_attacks(BISHOP, square, bishop_magics[square].magic);

			for (PieceType p : { BISHOP, ROOK }) {
				for (unsigned square2 = 0; square2 <= 63; square2++) {
					if (sliding_attack_bb(p, square, 0ULL) & 1ULL << square2) {
						between_bb[square][square2] = sliding_attack_bb(p, square, 1ULL << square2) &
									      sliding_attack_bb(p, square2, 1ULL << square);
						line_bb[square][square2] = (sliding_attack_bb(p, square,  0ULL) &
									    sliding_attack_bb(p, square2, 0ULL)) |
									    (1ULL << square) | (1ULL << square2);
					}
					between_bb[square][square2] |= 1ULL << square2;
				}
			}
		}
	}

	// access sliding attacks for a given square and occupancy by lookup via magic numbers
	uint64_t get_bishop_attacks(unsigned square, uint64_t occ)
	{
		occ &= bishop_relevant_mask[square];
		occ *= bishop_magics[square].magic;
		occ >>= bishop_magics[square].shift;
		return bishop_attacks[square][occ];
	}

	uint64_t get_rook_attacks(unsigned square, uint64_t occ)
	{
		occ &= rook_relevant_mask[square];
		occ *= rook_magics[square].magic;
		occ >>= rook_magics[square].shift;
		return rook_attacks[square][occ];
	}

	// returns the pre-computed moves for a piece on a square
	template <PieceType p>
	uint64_t attacks_bb(unsigned square, uint64_t occ)
	{
		switch(p) {
		case KNIGHT: return knight_attacks[square];
		case BISHOP: return get_bishop_attacks(square, occ);
		case ROOK:   return get_rook_attacks(square, occ);
		case QUEEN:  return get_bishop_attacks(square, occ) | get_rook_attacks(square, occ);
		case KING:   return king_attacks[square];
		}
	}

	void generate_evaluation_tables()
	{
		for (unsigned square = 0; square < 64; square++) {

			// the file of the square, and the two adjacent files
			uint64_t adjacent_mask = file_bb(square);
			adjacent_mask |= (adjacent_mask & ~FILE_A) >> 1;
			adjacent_mask |= (adjacent_mask & ~FILE_H) << 1;

			// all rows in front of the square row
			uint64_t all_mask = ~0ULL;
			uint64_t white_forward_mask = (rank(square) > 0) ? all_mask >> 8 * (8 - rank(square)) : 0;
			uint64_t black_forward_mask = (rank(square) < 7) ? all_mask << 8 * (rank(square) + 1) : 0;

			// a pawn is passed if there are no enemy pawns on the same or the adjacent files in front of it
			passed_pawn_mask[WHITE][square] = white_forward_mask & adjacent_mask;
			passed_pawn_mask[BLACK][square] = black_forward_mask & adjacent_mask;

			// forward_file_mask is used for detecting doubled or blocked pawns
			forward_file_mask[WHITE][square] = file_bb(square) & white_forward_mask;
			forward_file_mask[BLACK][square] = file_bb(square) & black_forward_mask;

			// a pawn is isolated, if there are no neighbor pawns
			isolated_pawn_mask[file(square)] = adjacent_mask & ~file_bb(square);
		}
	}

	PreComputed()
	{
		generate_attacks();
		generate_evaluation_tables();
	}
};
