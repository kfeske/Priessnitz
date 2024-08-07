#pragma once
#include "utility.h"

struct Pre_computed
{
	uint64_t pawn_attacks[2][64];
	uint64_t knight_attacks[64];
	uint64_t king_attacks[64];
	uint64_t bishop_mask[64];
	uint64_t rook_mask[64];
	uint64_t bishop_hash[64][512];
	uint64_t rook_hash[64][4096];

	// The magic numbers have been computed by a brute force method, where
	// we pick random numbers and see, if they can hash the attacks for the
	// sliding pieces for every possible combination of blockers.
	uint64_t rook_magics[64] = {
	 0x2080008014244000, 0x240200010004000,  0xa00084010260080,  0x1500201000050900,
	 0x600081002000421,  0x100020100040008,  0x1080010002000080, 0x80052300084880,
	 0x4800040088020,    0x1001802000804000, 0x408802000801000,  0x200a001040200a03,
	 0x809001105000801,  0x201800200140080,  0x2804000104020810, 0x1800045000080,
	 0x88000c0002000c0,  0x10004000200042,   0x2901010040102000, 0x1050010010090020,
	 0xc008004810800,    0x808004000200,     0x2842510100020004, 0x200052c4084,
	 0x80004040002010,   0x10a0400040201004, 0x401201920024c080, 0x221050900100020,
	 0x103001500380010,  0x3002300080400,    0x800080400100102,  0x91008200104409,
	 0x804004800020,     0x100401000402002,  0x810002000801081,  0x804802801000,
	 0x800800800400,     0x4001002020008,    0x4001008441002200, 0x600a402001041,
	 0x1320204000808008, 0x1040200040008080, 0x400802016020040,  0x1015090010010020,
	 0x3481000800050050, 0x4010040002008080, 0x40200010100,      0x2202004400820001,
	 0x2028c40210200,    0x1020201000400140, 0x1402100020008480, 0x1810500222c90100,
	 0x10248010084d100,  0xa1800201040080,   0x2020410810023400, 0x502008400410200,
	 0x2000802010410202, 0x1084000201081,    0x62910040aca001,   0x5710010008041021,
	 0x100200041108a002, 0x2086001001482422, 0x504080150921004,  0x148400402102
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
	 0x4002042810910200, 0x409022802022300,  0x8280104204800,    0x1022209200200400,
	 0x4404102908800022, 0x100082a020880000, 0xc202300288040a,   0x10804110100220,
	 0x20042004042082,   0x4008100400c40040, 0x1c00041400820040, 0x40404088100a4,
	 0x1105088204a4401,  0x4818811008040440, 0x8564808141108,    0x300202f01101010,
	 0x120404004440080,  0x20000401140908,   0x43001000c28103,   0x28e0848802004080,
	 0xe000422010c01,    0x420021004200b,    0x2082402018400,    0x10080212c011823,
	 0x2050048040084201, 0x13801200a0402,    0x28084004040020,   0x2001040000440080,
	 0x410840000802000,  0x90020801008202,   0x1000812004013800, 0x201102009040101,
	 0x11a8020800408804, 0x2044024240081081, 0x4092003201900180, 0x4220100820440400,
	 0x4004140400111010, 0x2602019200430240, 0x6180512215408,    0x809200c08204,
	 0x1a082018002500,   0x805015010004200,  0x8442022300c0800,  0x404010404200,
	 0x100106210102200,  0x1404248800402602, 0x1404410444122101, 0x1010102081900,
	 0x402008208400020,  0x145010110028300,  0x880042402080940,  0x44020880000,
	 0x404000810240111,  0x2000041022020420, 0x20208c29818000,   0x108c8104082002,
	 0x10840042100410,   0x90402010400,      0x3000042080400,    0x4010400012420220,
	 0x58110311089,      0x4104008a20080080, 0x800a02001821080,  0x20021010510840
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

	// A mask of all the squares in the line between two squares including the second one.
	// Used for handling checks.
	uint64_t ray_between[64][64] {};

	// A mask of the whole rank, file or diagonal, both squares are on.
	// Empty, when the squares do not align.
	uint64_t ray[64][64] {};

	// Rook or king moves can disable castling
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
	// Mask of the castling squares to tell, if they are blocked
	uint64_t oo_blockers[2] = { 0x6000000000000000, 0x60 };
	uint64_t ooo_blockers[2] = { 0xE00000000000000, 0xE };

	// Quick distance lookup
	unsigned rank_distance[64][64];
	unsigned file_distance[64][64];
	unsigned square_distance[64][64];

	// Evaluation tables
	uint64_t passed_pawn_mask[2][64];
	uint64_t forward_file_mask[2][64];
	uint64_t isolated_pawn_mask[8];
	uint64_t neighbor_mask[64];
	uint64_t outpost_mask[2];
	uint64_t pawn_threat_mask[2][64];
	uint64_t king_ring_mask[64];
	uint64_t pawn_shield[2][64];

	void cast_magics(Piece_type type);
};

extern Pre_computed const pre_computed;

bool edge(unsigned square, Direction d);

template <Color color>
uint64_t generate_pawn_attacks(unsigned square);

uint64_t generate_knight_attacks(unsigned square);

uint64_t generate_king_attacks(unsigned square);

uint64_t generate_sliding_attacks(Piece_type type, unsigned square, uint64_t occ);

// Access sliding attacks for a given square and occupancy by lookup via magic numbers
static inline uint64_t bishop_attacks(unsigned square, uint64_t occ)
{
	occ &= pre_computed.bishop_mask[square];
	occ *= pre_computed.bishop_magics[square];
	occ >>= pre_computed.bishop_shifts[square];
	return pre_computed.bishop_hash[square][occ];
}

static inline uint64_t rook_attacks(unsigned square, uint64_t occ)
{
	occ &= pre_computed.rook_mask[square];
	occ *= pre_computed.rook_magics[square];
	occ >>= pre_computed.rook_shifts[square];
	return pre_computed.rook_hash[square][occ];
}

// Returns the pre-computed moves for a piece on a square
static inline uint64_t piece_attacks(Piece_type type, unsigned square, uint64_t occ)
{
	switch (type) {
	case KNIGHT: return pre_computed.knight_attacks[square];
	case BISHOP: return bishop_attacks(square, occ);
	case ROOK:   return rook_attacks(square, occ);
	case QUEEN:  return bishop_attacks(square, occ) | rook_attacks(square, occ);
	case KING:   return pre_computed.king_attacks[square];
	default: return 0ULL;
	}
}

static inline uint64_t pawn_attacks(Color friendly, unsigned square) { return pre_computed.pawn_attacks[friendly][square]; };
static inline uint8_t  prohibiters(unsigned square) { return pre_computed.prohibiters[square]; };
static inline uint64_t ooo_blockers(Color friendly) { return pre_computed.ooo_blockers[friendly]; };
static inline uint64_t oo_blockers(Color friendly) { return pre_computed.oo_blockers[friendly]; };

static inline uint64_t king_ring_mask(unsigned square) { return pre_computed.king_ring_mask[square]; };
static inline uint64_t pawn_shield(Color friendly, unsigned square) { return pre_computed.pawn_shield[friendly][square]; };
static inline uint64_t passed_pawn_mask(Color friendly, unsigned square) { return pre_computed.passed_pawn_mask[friendly][square]; };
static inline uint64_t neighbor_mask(unsigned square) { return pre_computed.neighbor_mask[square]; };
static inline uint64_t forward_file_mask(Color friendly, unsigned square) { return pre_computed.forward_file_mask[friendly][square]; };
static inline uint64_t isolated_pawn_mask(unsigned file) { return pre_computed.isolated_pawn_mask[file]; };
static inline uint64_t outpost_mask(Color friendly) { return pre_computed.outpost_mask[friendly]; };
static inline uint64_t pawn_threat_mask(Color friendly, unsigned square) { return pre_computed.pawn_threat_mask[friendly][square]; };

static inline uint64_t ray_between(unsigned square_1, unsigned square_2) { return pre_computed.ray_between[square_1][square_2]; };
static inline uint64_t ray(unsigned square_1, unsigned square_2) { return pre_computed.ray[square_1][square_2]; };
static inline unsigned rank_distance(unsigned square_1, unsigned square_2) { return pre_computed.rank_distance[square_1][square_2]; };
static inline unsigned file_distance(unsigned square_1, unsigned square_2) { return pre_computed.file_distance[square_1][square_2]; };
static inline unsigned square_distance(unsigned square_1, unsigned square_2) { return pre_computed.square_distance[square_1][square_2]; };
