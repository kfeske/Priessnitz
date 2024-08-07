#include <iostream>
#include <sdl2.h>

#include <uci.h>

//TranspositionTable<(500 * 1024 * 1024) / sizeof(TTEntry)> Search::tt = {};

int main()
{
	static UCI uci {};

	while (!uci.quit)
		uci.await_input();

	return 0;
}
