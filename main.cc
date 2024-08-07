#include <iostream>
#include <sdl2.h>

#include <uci.h>

int main()
{
	static UCI uci {};

	while (!uci.quit)
		uci.await_input();

	return 0;
}
