#include <cmath>

struct Search_constants
{
	int FUTILITY_MARGIN[7] = {
		0, 100, 200, 300, 400, 550, 650
	};

	int REVERSE_FUTILITY_MARGIN = 60;

	int RAZOR_MARGIN = 400;

	unsigned LATE_MOVE_REDUCTION[64][64];

	Search_constants()
	{
		for (unsigned depth = 0; depth < 64; depth++) {
			for (unsigned move_count = 0; move_count < 64; move_count++)
				LATE_MOVE_REDUCTION[depth][move_count] = 0.75 + std::log(depth) * std::log(move_count) / 2.5;
		}
	}
};
