
struct PreComputed
{
	Uint64 rank_1 = 0xFF;
	Uint64 rank_2 = rank_1 << (8 * 1);
	Uint64 rank_3 = rank_1 << (8 * 2);
	Uint64 rank_4 = rank_1 << (8 * 3);
	Uint64 rank_5 = rank_1 << (8 * 4);
	Uint64 rank_6 = rank_1 << (8 * 5);
	Uint64 rank_7 = rank_1 << (8 * 6);
	Uint64 rank_8 = rank_1 << (8 * 7);

	Uint64 file_A = 0x0101010101010101ULL;
	Uint64 file_B = file_A << 1;
	Uint64 file_C = file_A << 2;
	Uint64 file_D = file_A << 3;
	Uint64 file_E = file_A << 4;
	Uint64 file_F = file_A << 5;
	Uint64 file_G = file_A << 6;
	Uint64 file_H = file_A << 7;

	Uint64 pawn_attacks[2][64];
	Uint64 knight_attacks[64];
	Uint64 king_attacks[64];
	Magic rook_magics[64];
	Magic bishop_magics[64];
	Uint64 bishop_relevant_mask[64];
	Uint64 rook_relevant_mask[64];
	Uint64 bishop_attacks[64][512];
	Uint64 rook_attacks[64][4096];

	// used for handling checks
	Uint64 between_bb[64][64] {};

	// to restrict movement of pinned pieces
	Uint64 line_bb[64][64] {};

	// castling
	Uint8 prohibiters[64] = {
		0x7, 0xf, 0xf, 0xf, 0x3, 0xf, 0xf, 0xb,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
		0xd, 0xf, 0xf, 0xf, 0xc, 0xf, 0xf, 0xe,
	};
	Uint64 oo_blockers[2] = { 0x6000000000000000, 0x60 };
	Uint64 ooo_blockers[2] = { 0xE00000000000000, 0xE };
	Uint64 oooo = (1ULL << B1) | (1ULL << B8);


	Uint64 rank_bb(unsigned square)
	{
		unsigned index = square >> 3;
		return rank_1 << (index * 8);
	}

	Uint64 file_bb(unsigned square)
	{
		unsigned index = square & 7;
		return file_A << index;
	}

	// precomputed attack data for the leaper pieces (Pawns, Knight, King)

	template <Color color>
	Uint64 pawn_attack_bb(unsigned square)
	{
		Uint64 pos = 1ULL << square;
		Uint64 attacks = 0ULL;
		if (color == WHITE) {
			if ((pos >> 7) & ~file_A) attacks |= pos >> 7;
			if ((pos >> 9) & ~file_H) attacks |= pos >> 9;
		}
		else {
			if ((pos << 7) & ~file_H) attacks |= pos << 7;
			if ((pos << 9) & ~file_A) attacks |= pos << 9;
		}
		return attacks;
	}

	Uint64 knight_attack_bb(unsigned square)
	{
		Uint64 pos = 1ULL << square;
		Uint64 attacks = 0ULL;
		if ((pos >> 10) & ~(file_G | file_H)) attacks |= pos >> 10;
		if ((pos << 6)  & ~(file_G | file_H)) attacks |= pos <<  6;
		if ((pos >> 17) & ~file_H           ) attacks |= pos >> 17;
		if ((pos << 15) & ~file_H           ) attacks |= pos << 15;
		if ((pos >> 15) & ~file_A           ) attacks |= pos >> 15;
		if ((pos << 17) & ~file_A           ) attacks |= pos << 17;
		if ((pos >> 6)  & ~(file_A | file_B)) attacks |= pos >>  6;
		if ((pos << 10) & ~(file_A | file_B)) attacks |= pos << 10;
		return attacks;
	}

	Uint64 king_attack_bb(unsigned square)
	{
		Uint64 pos = 1ULL << square;
		Uint64 attacks = 0ULL;
		if (pos << 8)		  attacks |= pos << 8;
		if ((pos << 1) & ~file_A) attacks |= pos << 1;
		if ((pos << 7) & ~file_H) attacks |= pos << 7;
		if ((pos << 9) & ~file_A) attacks |= pos << 9;
		if (pos >> 8)		  attacks |= pos >> 8;
		if ((pos >> 1) & ~file_H) attacks |= pos >> 1;
		if ((pos >> 7) & ~file_A) attacks |= pos >> 7;
		if ((pos >> 9) & ~file_H) attacks |= pos >> 9;
		return attacks;
	}

	// sliding attacks on the fly!
	// using this function in the move generator would be too slow, hence all the magic stuff is needed

	Uint64 sliding_attack_bb(PieceType p, unsigned square, Uint64 occ)
	{
		Uint64 attacks = 0ULL;
		Direction direction_rook[4]   = {NORTH, SOUTH, EAST, WEST};
		Direction direction_bishop[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

		for (Direction d : (p == ROOK) ? direction_rook : direction_bishop) {
			unsigned s = square;
			Uint64 target_square = 0ULL;

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

	Uint64 occupancy_for_index(unsigned index, unsigned bits_in_mask, Uint64 mask)
	{
		Uint64 occ = 0ULL;
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
		Uint64 occupancies[4096];
		Uint64 attacks[4096];
		Uint64 used_attacks[4096];
		Uint64 edges = ((rank_1 | rank_8) & ~rank_bb(square)) | ((file_A | file_H) & ~file_bb(square));
		Uint64 mask = sliding_attack_bb(p, square, 0ULL) & ~edges;
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

	void sliding_attacks(PieceType p, unsigned square, Uint64 magic_number)
	{
		Uint64 edges = ((rank_1 | rank_8) & ~rank_bb(square)) | ((file_A | file_H) & ~file_bb(square));
		Uint64 mask = sliding_attack_bb(p, square, 0ULL) & ~edges;

		if (p == BISHOP) bishop_relevant_mask[square] = mask;
		if (p == ROOK)     rook_relevant_mask[square] = mask;

		unsigned bits_in_mask = pop_count(mask);
		unsigned possible_occupancies = 1 << bits_in_mask;

		for (unsigned i = 0; i < possible_occupancies; i++) {
			Uint64 occupancy = occupancy_for_index(i, bits_in_mask, mask);

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

	Uint64 get_bishop_attacks(unsigned square, Uint64 occ)
	{
		occ &= bishop_relevant_mask[square];
		occ *= bishop_magics[square].magic;
		occ >>= bishop_magics[square].shift;
		return bishop_attacks[square][occ];
	}

	Uint64 get_rook_attacks(unsigned square, Uint64 occ)
	{
		occ &= rook_relevant_mask[square];
		occ *= rook_magics[square].magic;
		occ >>= rook_magics[square].shift;
		return rook_attacks[square][occ];
	}

	// returns the pre-computed moves for a piece on a square

	template <PieceType p>
	Uint64 attacks_bb(unsigned square, Uint64 occ)
	{
		switch(p) {
		case KNIGHT: return knight_attacks[square];
		case BISHOP: return get_bishop_attacks(square, occ);
		case ROOK:   return get_rook_attacks(square, occ);
		case QUEEN:  return get_bishop_attacks(square, occ) | get_rook_attacks(square, occ);
		case KING:   return king_attacks[square];
		}
	}

	PreComputed()
	{
		generate_attacks();
	}
};
