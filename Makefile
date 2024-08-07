MAKEFLAGS += -j8
test:
#CXXFLAGS += -fsanitize=undefined -Wall -Wextra -Weffc++ -Werror -MMD -O3
profile:
	g++ main.cc -o main $(CXXFLAGS) -g -pg 
	./main
CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -O3

OBJECTS := main.o pre_computed.o search.o board.o evaluation.o move_generator.o uci.o

main:
	g++ $(OBJECTS) -o main $(CXXFLAGS)
main: Makefile $(OBJECTS)

%.o: %.cc
	g++ -c $< -o $@ $(CXXFLAGS)

-include *.d

clean:
	rm -f *.o *.d
	rm main

test: main
	./main
