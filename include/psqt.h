// Piece Square Table
//
// stores information about how good the position of a piece is
//
// this will be used in the evaluation function to motivate the
// engine to develop its pieces


struct PSQT// : Noncopyable
{
	// Phase (Endgame/Midgame), Piece, Square
	Value psqt[16][64] {};

	/*int bonus[2][6][64] =
	{
	 { //Midgame
	  { // Pawn
	     0,  0,  0,  0,  0,  0,  0,  0,
	    50, 50, 50, 50, 50, 50, 50, 50,
	    10, 10, 20, 30, 30, 20, 10, 10,
	     5,  5, 10, 25, 25, 10,  5,  5,
	     0,  0,  0, 20, 20,  0,  0,  0,
	     5, -5,-10,  0,  0,-10, -5,  5,
	     5, 10, 10,-20,-20, 10, 10,  5,
	     0,  0,  0,  0,  0,  0,  0,  0
	  },
	  {
	   -50,-40,-30,-30,-30,-30,-40,-50,
	   -40,-20,  0,  0,  0,  0,-20,-40,
	   -30,  0, 10, 15, 15, 10,  0,-30,
	   -30,  5, 15, 20, 20, 15,  5,-30,
	   -30,  0, 15, 20, 20, 15,  0,-30,
	   -30,  5, 10, 15, 15, 10,  5,-30,
	   -40,-20,  0,  5,  5,  0,-20,-40,
	   -50,-40,-30,-30,-30,-30,-40,-50,
	  },
	  {
	   -20,-10,-10,-10,-10,-10,-10,-20,
	   -10,  0,  0,  0,  0,  0,  0,-10,
	   -10,  0,  5, 10, 10,  5,  0,-10,
	   -10,  5,  5, 10, 10,  5,  5,-10,
	   -10,  0, 10, 10, 10, 10,  0,-10,
	   -10, 10, 10, 10, 10, 10, 10,-10,
	   -10,  5,  0,  0,  0,  0,  5,-10,
	   -20,-10,-10,-10,-10,-10,-10,-20,
	  },
	  {
	    0,  0,  0,  0,  0,  0,  0,  0,
	    5, 10, 10, 10, 10, 10, 10,  5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	   -5,  0,  0,  0,  0,  0,  0, -5,
	    0,  0,  0,  5,  5,  0,  0,  0
	  },
	  {
	   -20,-10,-10, -5, -5,-10,-10,-20,
	   -10,  0,  0,  0,  0,  0,  0,-10,
	   -10,  0,  5,  5,  5,  5,  0,-10,
	    -5,  0,  5,  5,  5,  5,  0, -5,
	     0,  0,  5,  5,  5,  5,  0, -5,
	   -10,  5,  5,  5,  5,  5,  0,-10,
	   -10,  0,  5,  0,  0,  0,  0,-10,
	   -20,-10,-10, -5, -5,-10,-10,-20
	  },
	  {
	   -30,-40,-40,-50,-50,-40,-40,-30,
  	   -30,-40,-40,-50,-50,-40,-40,-30,
  	   -30,-40,-40,-50,-50,-40,-40,-30,
  	   -30,-40,-40,-50,-50,-40,-40,-30,
  	   -20,-30,-30,-40,-40,-30,-30,-20,
  	   -10,-20,-20,-20,-20,-20,-20,-10,
  	    20, 20,  0,  0,  0,  0, 20, 20,
  	    20, 30, 10,  0,  0, 10, 30, 20
	  }
	 },
	 {
	  {
	  }
	 }
	};*/

	int bonus[2][6][64] = 
	 { 
	  { // Midgame
	   { // Pawn
	     0,   0,   0,   0,  0,   0,   0,   0,
	     2,   4,  11,  18, 16,  21,   9,  -3,
	    -9, -15,  11,  15, 31,  23,   6, -20,
	    -3, -20,   8,  19, 39,  17,   2,  -5,
	    11,  -4, -11,   2, 11,   0, -12,   5,
	     3, -11,  -6,  22, -8,  -5, -14, -11,
	    -7,   6,  -2, -11,  4, -14,  10,  -9,
	     0,   0,   0,   0,  0,   0,   0,   0
	   },
	   { // Knight
	    -175, -92, -74, -73, -73, -74, -92, -175,
	     -77, -41, -27, -15, -15, -27, -41, -77,
	     -61, -17,   6,  12,  12,   6, -17, -61,
	     -35,   8,  40,  49,  49,  40,   8, -35,
	     -34,  13,  44,  51,  51,  44,  13, -34,
	      -9,  22,  58,  53,  53,  58,  22, -9,
	     -67, -27,   4,  37,  37,   4, -27, -67,
	    -201, -83, -56, -26  -26, -56, -83, -201
	   },
	   { // Bishop
	    -37, -4 ,  -6, -16, -16,  -6,  -4, -37,
   	    -11,   6,  13,   3,   3,  13,   6, -11,
   	    -5 ,  15,  -4,  12,  12,  -4,  15,  -5,
   	    -4 ,   8,  18,  27,  27,  18,   8,  -4,
   	    -8 ,  20,  15,  22,  22,  15,  20,  -8,
   	    -11,   4,   1,   8,   8,   1,   4, -11,
   	    -12, -10,   4,   0,   0,   4, -10, -12,
   	    -34,   1, -10, -16, -16, -10,   1, -34
	   },
	   { // Rook
	    -31, -20, -14, -5, -5, -14, -20, -31,
   	    -21, -13,  -8,  6,  6,  -8, -13, -21,
   	    -25, -11,  -1,  3,  3,  -1, -11, -25,
   	    -13,  -5,  -4, -6, -6,  -4,  -5, -13,
   	    -27, -15,  -4,  3,  3,  -4, -15, -27,
   	    -22,  -2,   6, 12, 12,   6,  -2, -22,
   	     -2,  12,  16, 18, 18,  16,  12,  -2,
   	    -17, -19,  -1,  9,  9,  -1, -19, -17
	   },
	   { // Queen
	     3, -5, -5,  4,  4, -5, -5,  3,
   	    -3,  5,  8, 12, 12,  8,  5, -3,
   	    -3,  6, 13,  7,  7, 13,  6, -3,
   	     4,  5,  9,  8,  8,  9,  5,  4,
   	     0, 14, 12,  5,  5, 12, 14,  0,
   	    -4, 10,  6,  8,  8,  6, 10, -4,
   	    -5,  6, 10,  8,  8, 10,  6, -5,
   	    -2, -2,  1, -2, -2,  1, -2, -2
	   },
	   { // King
	    271, 327, 271, 198, 198, 271, 327, 271,
   	    278, 303, 234, 179, 179, 234, 303, 278,
   	    195, 258, 169, 120, 120, 169, 258, 195,
   	    164, 190, 138,  98,  98, 138, 190, 164,
   	    154, 179, 105,  70,  70, 105, 179, 154,
   	    123, 145,  81,  31,  31,  81, 145, 123,
   	     88, 120,  65,  33,  33,  65, 120,  88,
   	     59,  89,  45,  -1,  -1,  45,  89,  59
	   }
	  },

	  { // Endgame
 	   { // Pawn
	     -8,  -6,   9,   5,  16,   6,  -6, -18,
   	     -9,  -7, -10,   5,   2,   3,  -8,  -5,
              7,   1,  -8,  -2, -14, -13, -11,  -6,
             12,   6,   2,  -6,  -5,  -4,  14,   9,
             27,  18,  19,  29,  30,   9,   8,  14,
             -1, -14,  13,  22,  24,  17,   7,   7
	   }
	  }
	 };

	void prepare()
	{
		//for (Phase ph : { MIDGAME, ENDGAME }) {
			for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING }) {
				Value value = get_piece_value(pc, MIDGAME);
				
				for (unsigned square = 0; square < 64; square++) {
					psqt[pc][square] = Value(value + bonus[MIDGAME][type_of(pc)][square]);
					psqt[pc + 8][mirrored(square)] = Value(-psqt[pc][square]);
				}
			}
		//}
		/*
		for (unsigned i = 0; i < 8; i++) {
			for (unsigned j = 0; j < 8; j++) {
				std::cerr << psqt[W_KNIGHT][i * 8 + j] << " ";
			}
			std::cerr << "\n";
		}
		std::cerr << "\n";*/
	}

	PSQT()
	{
		prepare();
	}
};
