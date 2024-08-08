#pragma once

#include "utility.h"
#include "board.h"

/*struct Move_generator : Noncopyable
{
	uint8_t size = 0;
	Scored_move move_list[MAX_MOVES];

	void generate_pawn_moves(Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq, uint64_t sliders);

	void generate_moves(Piece_type type, Board &board, Color col, uint64_t targets, uint64_t quiets, uint64_t pinned, unsigned ksq);
	
	void generate_all_moves(Board &board);
	
	void generate_quiescence(Board &board);

	void append_attacks(Move_flags flags, unsigned from, uint64_t attacks)
	{
		while (attacks) move_list[size++].move = create_move(flags, from, pop_lsb(attacks));
	}
};
*/

// ******************************
// *** - New Move Generator - ***
// *************************+****

enum Gen_stage {
	CAPTURE_GEN, QUIET_GEN, IN_CHECK_GEN
};


struct Move_list
{
	uint8_t size = 0;
	struct {
	Scored_move moves[MAX_MOVES]; // uninitialized
	};

	void add(Move move)
	{
		moves[size++].move = move;
	}

	void add(Scored_move scored_move)
	{
		moves[size++] = scored_move;
	}
	
	Move_list()
	{
	}
};

void generate(Board &board, Move_list &move_list, Gen_stage stage);

void generate_legal(Board &board, Move_list &move_list);
