test:

CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -O3 -Iinclude
CXXFLAGS += -lSDL2 -lSDL2_image

main:
	g++ main.cc -o main $(CXXFLAGS)
main: Makefile

-include *.d

clean:
	rm -f *.o *.d
	rm main

test: main
	./main
