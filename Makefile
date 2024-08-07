MAKEFLAGS += -j8
test:

CXXFLAGS += -Wall -Wextra -Weffc++ -Werror -MMD -O3

OBJECTS := main.o pre_computed.o search.o board.o evaluation.o move_generator.o uci.o

main: Makefile $(OBJECTS)
	g++ $(OBJECTS) -o main $(CXXFLAGS)

profile: Makefile $(OBJECTS)
	g++ $(OBJECTS) -o main $(CXXFLAGS) -g -pg 
	./main
	gprof main | gprof2dot | dot -Tpng -o profile.png

%.o: %.cc
	g++ -c $< -o $@ $(CXXFLAGS)

-include *.d

clean:
	rm -f *.o *.d
	rm main

test: main
	./main
