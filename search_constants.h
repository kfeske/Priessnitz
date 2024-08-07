#include <cmath>

struct Search_constants
{
	unsigned LATE_MOVE_REDUCTION[64][64];

	Search_constants()
	{
		for (unsigned depth = 0; depth < 64; depth++) {
			for (unsigned move_count = 0; move_count < 64; move_count++)
				// Formula from Pirarucu Engine, similar to Ethereal's implementation
				LATE_MOVE_REDUCTION[depth][move_count] = 0.5 + std::log(depth) * std::log(move_count * 1.2) / 2.5;
		}
	}
};
