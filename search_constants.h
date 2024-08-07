#include <cmath>

struct Search_constants
{
	int FUTILITY_MARGIN[7] = {
		0, 100, 200, 300, 400, 550, 650
	};

	int REVERSE_FUTILITY_MARGIN[10] = {
		0, 60, 120, 180, 240, 300, 360, 420, 480, 540
	};

	int LMP_MARGIN[5] = {
		0, 6, 11, 24, 24
	};

	int RAZOR_MARGIN = 400;

	unsigned LATE_MOVE_REDUCTION[64][64];

	Search_constants()
	{
		for (unsigned depth = 0; depth < 64; depth++) {
			for (unsigned move_count = 0; move_count < 64; move_count++)
				// Formula from Pirarucu Engine, similar to Ethereal's implementation
				LATE_MOVE_REDUCTION[depth][move_count] = std::log(depth) * std::log(move_count * 1.2) / 2.5;
		}
	}
};
