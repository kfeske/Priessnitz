#include "tuner.h"
#include "test.h"
#include "trace.h"

bool const use_default_weights = true;

Trace &trace()
{
	static Trace t {};
	return t;
}

int main()
{
	static Tuner tuner {};
	//if (!test(tuner)) return 1;
	if (use_default_weights) {
		std::cerr << "loading default weights...\n";
		tuner.default_weights();
	}
	else {
		std::cerr << "loading engine weights...\n";
		tuner.engine_weights();
	}
	tuner.tune();
	return 1;
}
