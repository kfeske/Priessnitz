#pragma once

#include "utility.h"
#include "board.h"

struct Move_generator : Noncopyable
{
	uint8_t size = 0;
	Scored_move move_list[MAX_MOVES];

	template <Move_flags mf>
	void append_attacks(unsigned from, uint64_t attacks);

	void generate_pawn_moves(Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq, uint64_t sliders);

	template <Piece_type p>
	void generate_moves(Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq);
	
	void generate_all_moves(Board &board);
	
	void generate_quiescence(Board &board);
};

