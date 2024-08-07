struct PreComputed : Noncopyable
{
	uint64_t pawn_attacks[2][64];
	uint64_t knight_attacks[64];
	uint64_t king_attacks[64];
	uint64_t bishop_mask[64];
	uint64_t rook_mask[64];
	uint64_t bishop_hash[64][512];
	uint64_t rook_hash[64][4096];

	// the magic numbers have been computed by a brute force method, where
	// we pick random numbers and see, if they work
	uint64_t rook_magics[64] = {
	 0x2080008014244000,0x240200010004000,0xa00084010260080,0x1500201000050900,
	 0x600081002000421,0x100020100040008,0x1080010002000080,0x80052300084880,
	 0x4800040088020,0x1001802000804000,0x408802000801000,0x200a001040200a03,
	 0x809001105000801,0x201800200140080,0x2804000104020810,0x1800045000080,
	 0x88000c0002000c0,0x10004000200042,0x2901010040102000,0x1050010010090020,
	 0xc008004810800,0x808004000200,0x2842510100020004,0x200052c4084,
	 0x80004040002010,0x10a0400040201004,0x401201920024c080,0x221050900100020,
	 0x103001500380010,0x3002300080400,0x800080400100102,0x91008200104409,
	 0x804004800020,0x100401000402002,0x810002000801081,0x804802801000,
	 0x800800800400,0x4001002020008,0x4001008441002200,0x600a402001041,
	 0x1320204000808008,0x1040200040008080,0x400802016020040,0x1015090010010020,
	 0x3481000800050050,0x4010040002008080,0x40200010100,0x2202004400820001,
	 0x2028c40210200,0x1020201000400140,0x1402100020008480,0x1810500222c90100,
	 0x10248010084d100,0xa1800201040080,0x2020410810023400,0x502008400410200,
	 0x2000802010410202,0x1084000201081,0x62910040aca001,0x5710010008041021,
	 0x100200041108a002,0x2086001001482422,0x504080150921004,0x148400402102
	};

	unsigned rook_shifts[64] = {
	 52,53,53,53,53,53,53,52,
	 53,54,54,54,54,54,54,53,
	 53,54,54,54,54,54,54,53,
	 53,54,54,54,54,54,54,53,
	 53,54,54,54,54,54,54,53,
	 53,54,54,54,54,54,54,53,
	 53,54,54,54,54,54,54,53,
	 52,53,53,53,53,53,53,52
	};

	uint64_t bishop_magics[64] = {
	 0x4002042810910200,0x409022802022300,0x8280104204800,0x1022209200200400,
	 0x4404102908800022,0x100082a020880000,0xc202300288040a,0x10804110100220,
	 0x20042004042082,0x4008100400c40040,0x1c00041400820040,0x40404088100a4,
	 0x1105088204a4401,0x4818811008040440,0x8564808141108,0x300202f01101010,
	 0x120404004440080,0x20000401140908,0x43001000c28103,0x28e0848802004080,
	 0xe000422010c01,0x420021004200b,0x2082402018400,0x10080212c011823,
	 0x2050048040084201,0x13801200a0402,0x28084004040020,0x2001040000440080,
	 0x410840000802000,0x90020801008202,0x1000812004013800,0x201102009040101,
	 0x11a8020800408804,0x2044024240081081,0x4092003201900180,0x4220100820440400,
	 0x4004140400111010,0x2602019200430240,0x6180512215408,0x809200c08204,
	 0x1a082018002500,0x805015010004200,0x8442022300c0800,0x404010404200,
	 0x100106210102200,0x1404248800402602,0x1404410444122101,0x1010102081900,
	 0x402008208400020,0x145010110028300,0x880042402080940,0x44020880000,
	 0x404000810240111,0x2000041022020420,0x20208c29818000,0x108c8104082002,
	 0x10840042100410,0x90402010400,0x3000042080400,0x4010400012420220,
	 0x58110311089,0x4104008a20080080,0x800a02001821080,0x20021010510840
	};

	unsigned bishop_shifts[64] = {
	 58,59,59,59,59,59,59,58,
	 59,59,59,59,59,59,59,59,
	 59,59,57,57,57,57,59,59,
	 59,59,57,55,55,57,59,59,
	 59,59,57,55,55,57,59,59,
	 59,59,57,57,57,57,59,59,
	 59,59,59,59,59,59,59,59,
	 58,59,59,59,59,59,59,58
	};

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

	uint64_t passed_pawn_mask[2][64];
	uint64_t forward_file_mask[2][64];
	uint64_t isolated_pawn_mask[8];
	uint64_t neighbor_mask[64];
	uint64_t king_zone[2][64];


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
	// using this function in the move generator would be too slow, hence all the magic bitboard stuff is needed
	uint64_t sliding_attack_bb(Piece_type type, unsigned square, uint64_t occ)
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

	// fills the lookup tables of the sliding moves for the corresponding magic number
	void cast_magics(Piece_type type)
	{
		for (unsigned square = 0; square < 64; square++) {

			uint64_t edges = ((RANK_1 | RANK_8) & ~rank(square)) | ((FILE_A | FILE_H) & ~file(square));

			// the edges of the board are not considered, a piece on the edge cannot block a sliding piece
			uint64_t ray_mask = sliding_attack_bb(type, square, 0ULL) & ~edges;

			// this is important later, when we want to quickly calculate the magic index
			(type == BISHOP) ? bishop_mask[square] = ray_mask : rook_mask[square] = ray_mask;

			unsigned bits_in_mask = pop_count(ray_mask);
			unsigned possible_occupancies = 1ULL << bits_in_mask;

			// loop through every possible constellation of occupancies for the squares, the piece can move to
			for (unsigned i = 0; i < possible_occupancies; i++) {

				uint64_t occ = occupancy(i, bits_in_mask, ray_mask);

				// compute the magic index (the hash key)
				uint64_t magic_number = (type == BISHOP) ? bishop_magics[square] : rook_magics[square];
				uint8_t shift = (type == BISHOP) ? bishop_shifts[square] : rook_shifts[square];

				unsigned magic_index = (occ * magic_number) >> shift;

				// now, the hash tables are filled for this constellation
				(type == BISHOP) ? bishop_hash[square][magic_index] = sliding_attack_bb(BISHOP, square, occ) :
						   rook_hash[square][magic_index] = sliding_attack_bb(ROOK, square, occ);
			}
		}
	}

	// access sliding attacks for a given square and occupancy by lookup via magic numbers
	uint64_t bishop_attacks(unsigned square, uint64_t occ)
	{
		occ &= bishop_mask[square];
		occ *= bishop_magics[square];
		occ >>= bishop_shifts[square];
		return bishop_hash[square][occ];
	}

	uint64_t rook_attacks(unsigned square, uint64_t occ)
	{
		occ &= rook_mask[square];
		occ *= rook_magics[square];
		occ >>= rook_shifts[square];
		return rook_hash[square][occ];
	}

	// populates the lookup tables for the attacks of each piece
	// and the magic numbers for the sliding pieces
	void generate_lookup()
	{
		for (unsigned square = 0; square <= 63; square++) {
			pawn_attacks[WHITE][square] = pawn_attack_bb<WHITE>(square);
			pawn_attacks[BLACK][square] = pawn_attack_bb<BLACK>(square);
			knight_attacks[square] = knight_attack_bb(square);
			king_attacks[square] = king_attack_bb(square);
			cast_magics(BISHOP);
			cast_magics(ROOK);

			for (Piece_type type : { BISHOP, ROOK }) {
				for (unsigned square2 = 0; square2 <= 63; square2++) {
					if (sliding_attack_bb(type, square, 0ULL) & 1ULL << square2) {
						between_bb[square][square2] = sliding_attack_bb(type, square, 1ULL << square2) &
									      sliding_attack_bb(type, square2, 1ULL << square);
						line_bb[square][square2] = (sliding_attack_bb(type, square,  0ULL) &
									    sliding_attack_bb(type, square2, 0ULL)) |
									    (1ULL << square) | (1ULL << square2);
					}
					between_bb[square][square2] |= 1ULL << square2;
				}
			}
		}
	}

	// returns the pre-computed moves for a piece on a square
	template <Piece_type p>
	uint64_t attacks_bb(unsigned square, uint64_t occ)
	{
		switch(p) {
		case KNIGHT: return knight_attacks[square];
		case BISHOP: return bishop_attacks(square, occ);
		case ROOK:   return rook_attacks(square, occ);
		case QUEEN:  return bishop_attacks(square, occ) | rook_attacks(square, occ);
		case KING:   return king_attacks[square];
		}
	}

	void generate_evaluation_tables()
	{
		for (unsigned square = 0; square < 64; square++) {

			// the file of the square, and the two adjacent files
			uint64_t adjacent_mask = file(square);
			adjacent_mask |= (adjacent_mask & ~FILE_A) >> 1;
			adjacent_mask |= (adjacent_mask & ~FILE_H) << 1;

			// all rows in front of the square row
			uint64_t all_mask = ~0ULL;
			uint64_t white_forward_mask = (rank_num(square) > 0) ? all_mask >> 8 * (8 - rank_num(square)) : 0;
			uint64_t black_forward_mask = (rank_num(square) < 7) ? all_mask << 8 * (rank_num(square) + 1) : 0;

			// a pawn is passed if there are no enemy pawns on the same or the adjacent files in front of it
			passed_pawn_mask[WHITE][square] = white_forward_mask & adjacent_mask;
			passed_pawn_mask[BLACK][square] = black_forward_mask & adjacent_mask;

			// forward_file_mask is used for detecting doubled or blocked pawns
			forward_file_mask[WHITE][square] = file(square) & white_forward_mask;
			forward_file_mask[BLACK][square] = file(square) & black_forward_mask;

			// a pawn is isolated, if there are no pawns on the neighbor files
			isolated_pawn_mask[file_num(square)] = adjacent_mask & ~file(square);

			neighbor_mask[square] = adjacent_mask & rank(square) & ~(1ULL << square);

			// the king zone squares are the squares, the king can reach plus three squares in the enemy direction
			if (square >= 24) king_zone[WHITE][square] = king_attacks[square - 24] | 1ULL << (square - 24);
			if (square <= 39) king_zone[BLACK][square] = king_attacks[square + 24] | 1ULL << (square + 24);
		}
	}

	PreComputed()
	{
		generate_lookup();
		generate_evaluation_tables();
	}
};
